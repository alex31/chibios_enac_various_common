/**
 * @file    pubSub.hpp
 * @brief   C++ publish subscribe wrapper over libcanard
 *
 * @{
 */

#pragma once

#include <ch.h>
#include <hal.h>

#include "dronecan_msgs.h"
#include "stdutil.h"
#include "stdutil++.hpp"
#include "etl/map.h"
#include "etl/string.h"
#include "etl/string_view.h"
#include <utility>
#include <array>
#include <type_traits>
#include <concepts>

/**
 * @brief   Maximum number of subscriptions to
            broadcast, request, response messages
 * @notes   Wish it could be estimated at compile time
 */
#ifndef UAVNODE_DICTIONARY_SIZE
#define UAVNODE_DICTIONARY_SIZE 32
#endif



#if (!CANARD_ENABLE_TAO_OPTION) || (!CANARD_ENABLE_CANFD)
#error "CANARD_ENABLE_TAO_OPTION and CANARD_ENABLE_CANFD must be set to true"
#endif

/*
  TODO:

  ° review of mutex usage : can that be simplified ?

  ° unsubscribe API : is it needed ?

  ° alive message should be sent in classic CAN : check
    + when sending a message to classic can node : should be sent accordingly


  ° find a scheme to attribute ID (1..127) and roles to the can slaves

  ° mettre en œuvre un tunnel au dessus du shell :
    + chaque node a un shell dans un thread au dessus du message uavcan.protocol.AccessCommandShell
    + on sacrifie un montage à base de microcan + ftdi pour faire un pont qui permette de communiquer au shell des nodes connectés
      sur ce node de communication, il y aura un shell avec des commandes locale, et une commande type select qui permet de se connecter
      sur un node distant et de quitter ce node distant
*/

namespace  PubSub_private {

  /**
   * @brief   test if a value is between limits
   */
  template <typename T>
  consteval bool in_bounds(T value, T lower, T upper)
  {
    return value >= lower && value <= upper;
  }
  
  /**
   * @brief   structure describing CAN timing
   */
  struct DivSeg {
    /**
     * @brief   This determines the duration of Time Quanta
     * @notes   tq = prescaler / CAN clock source
     */
    uint32_t prescaler;
    
    
    /**
     * @brief   Time Segment 1: The sum of the Propagation Segment and Phase Segment 1 in tq
     *
     */
    uint32_t s1;
    
    /**
     * @brief    Time Segment 2: Phase Segment 2 in tq
     *
     */
    uint32_t s2;  
  };
  
	 
  /**
   * @brief       calculate CAN timings
   * @details     constexpr : evaluated @ compile time
   *
   * @param[in]   clockRatio : CAN clock source frequency / bus bitrate
   * @param[in]   preMax : prescaler will be max clamped to this value 
   * @param[in]   s1Max : s1 will be max clamped to this value 
   * @param[in]   ratios1s2 : ratio between s1 and s2 in the range [0 .. 1]
   * @param[in]   usePremax : force to set prescaler at the premax value
   * @return      DivSeg structure containing calculated timing values
   *
   * @notapi
   */
  consteval DivSeg findDivSeg(uint32_t clockRatio,  uint32_t preMax, uint32_t s1max, 
			      float ratios1s2, bool usePremax = false) 
  {
    uint32_t s2max = s1max / 2U;
    
    for (uint32_t prescaler = usePremax ? preMax : 1u; prescaler <= preMax; ++prescaler) {
      if (clockRatio % prescaler == 0) { 
	uint32_t Stotal = clockRatio / prescaler;
	uint32_t S1 = Stotal * ratios1s2;
	uint32_t S2 = Stotal - S1;
	S1-- ;// retreive sync seg
	if (S1 >= 1 && S2 >= 1 && S1 <= s1max && S2 <= s2max) {
	  return {prescaler, S1, S2};
	}
      } 
    }
    
    return {0, 0, 0};
  }
}

/**
 * @brief       determine type of first arguments of subscribe message callback
 * @details     constexpr : evaluated @ compile time
 *
 * @notapi
 */
template<typename T>
struct cbTraits;  

template<typename Arg1, typename Arg2>
struct cbTraits<void(Arg1, Arg2)> {
  using raw_second_arg_type =  Arg2;
  using msgt = std::remove_cvref_t<raw_second_arg_type>;
};

template<typename Arg1, typename Arg2>
struct cbTraits<void(*)(Arg1, Arg2)> {
  using raw_second_arg_type =  Arg2;
  using msgt = std::remove_cvref_t<raw_second_arg_type>;
};


namespace UAVCAN {
  static constexpr size_t MAX_CAN_NODES = 128;
  static constexpr size_t MEMORYPOOL_SIZE = 4096;


  /**
   * @brief   structure describing STM32 FDcan peripheral register for timing
   */
  struct RegTimings {
    /**
     * @brief   arbitration phase timing register value
     * @notes   in case of classic can, it's also the data timing
     */
    uint32_t nbtp;

    
    /**
     * @brief   data phase timing register value
     * @notes   only used for FDCan
     */
    uint32_t dbtp;

    /**
     * @brief   transmission delay compensation
     * @notes   only used for FDCan, needed to achieve high data rate
     *          in a reliable way
     */
    uint32_t tdcr;
  };

  /**
   * @brief       calculate NBTP register value from field values for arbitration timing
   * @details     consteval : evaluated @ compile time
   *
   * @param[in]   prescaler : commun prescaler for timing
   * @param[in]   seg1 : segment 1 duration in tq
   * @param[in]   seg2 : segment 2 duration in tq
   * @param[in]   synchroJumpWidth : synchro jump width duration in tq
   * @return      NBTP register value or 0 if input parameters are out of bound
   *
   * @notapi
   */
  consteval uint32_t getNBTP(uint32_t prescaler, uint32_t seg1, uint32_t seg2,
			     uint32_t synchroJumpWidth)
  {
    if (not PubSub_private::in_bounds(prescaler, 1UL, 512UL))
      return 0;
    if (not PubSub_private::in_bounds(seg1, 1UL, 256UL))
      return 0;
    if (not PubSub_private::in_bounds(seg2, 1UL, 128UL))
      return 0;
    if (not PubSub_private::in_bounds(synchroJumpWidth, 1UL, 128UL))
      return 0;
    
    return FDCAN_CONFIG_NBTP_NSJW(synchroJumpWidth - 1U) |
      FDCAN_CONFIG_NBTP_NBRP(prescaler - 1U) |
      FDCAN_CONFIG_NBTP_NTSEG1(seg1 - 1U) |
      FDCAN_CONFIG_NBTP_NTSEG2(seg2 - 1U);
  }

  /**
   * @brief       calculate DBTP register value from field values for data timing
   * @details     consteval : evaluated @ compile time
   *
   * @param[in]   prescaler : commun prescaler for timing
   * @param[in]   seg1 : segment 1 duration in tq
   * @param[in]   seg2 : segment 2 duration in tq
   * @param[in]   synchroJumpWidth : synchro jump width duration in tq
   * @return      DBTP register value or 0 if input parameters are out of bound
   *
   * @notapi
   */
  consteval uint32_t getDBTP(uint32_t prescaler, uint32_t seg1, uint32_t seg2,
			     uint32_t synchroJumpWidth, bool tdc)
  {
    if (not PubSub_private::in_bounds(prescaler, 1UL, 32UL))
      return 0;
    if (not PubSub_private::in_bounds(seg1, 1UL, 32UL))
      return 0;
    if (not PubSub_private::in_bounds(seg2, 1UL, 16UL))
      return 0;
    if (not PubSub_private::in_bounds(synchroJumpWidth, 1UL, 16UL))
      return 0;

    return FDCAN_CONFIG_DBTP_DSJW(synchroJumpWidth - 1U) |
      FDCAN_CONFIG_DBTP_DBRP(prescaler - 1U) |
      FDCAN_CONFIG_DBTP_DTSEG1(seg1 - 1U) |
      FDCAN_CONFIG_DBTP_DTSEG2(seg2 - 1U) |
      (tdc ? FDCAN_CONFIG_DBTP_TDC : 0);
  }


  /**
   * @brief       calculate timing registers values for desired bitrate
   * @details     consteval : evaluated @ compile time
   *              respect rules found in https://www.can-cia.org/fileadmin/cia/documents/publications/cnlm/march_2018/18-1_p28_recommendation_for_the_canfd_bit-timing_holger_zeltwanger_cia.pdf
   * @param[in]   canClk: can clock in Hz
   * @param[in]   arbitrationBitRateK : arbitration phase bit rate in Kilo bits per second
   * @param[in]   arbitrationS1s2Ratio : s1 s2 ratio for the arbitration phase in range 0 .. 1
   * @param[in]   dataBitRateK : data phase bit rate in Kilo bits per second
   * @param[in]   dataS1s2Ratio : s1 s2 ratio for the data phase in range 0 .. 1
   * @param[in]   transDelayCompens : activate transmission delay compensation mechanism
   * @return      RegTimings structure that contains STM32 FDcan timing register values
   *
   */
  consteval RegTimings getTimings(uint32_t canClk,
				  uint32_t arbitrationBitRateK, float arbitrationS1s2Ratio,
				  uint32_t dataBitRateK, float dataS1s2Ratio, bool transDelayCompens
				  )
  {
    const PubSub_private::DivSeg divSegData =
      PubSub_private::findDivSeg(canClk / (dataBitRateK * 1000U), 32U, 32U, dataS1s2Ratio);
    const PubSub_private::DivSeg divSegArbitration =
      PubSub_private::findDivSeg(canClk / (arbitrationBitRateK * 1000U),
						divSegData.prescaler, 256U,
						arbitrationS1s2Ratio, true);
    
    const uint32_t nbtp = getNBTP(divSegArbitration.prescaler,
				  divSegArbitration.s1, divSegArbitration.s2,
				  std::min(divSegArbitration.s1, divSegArbitration.s2));

    const uint32_t dbtp = getDBTP(divSegData.prescaler,
				  divSegData.s1, divSegData.s2,
				  std::min(divSegData.s1, divSegData.s2),
				  transDelayCompens);
    const uint32_t offset = divSegData.prescaler * divSegData.s1;
    const uint32_t tdc_offset = offset <= 127U ? offset : 127U;
    const uint32_t tdc_filter = tdc_offset * 2 / 3;
    const uint32_t tdcr = (tdc_offset << FDCAN_TDCR_TDCO_Pos) | 
      (tdc_filter << FDCAN_TDCR_TDCF_Pos); 
    return {nbtp, dbtp, transDelayCompens ? tdcr : 0};
  }

  using receivedCbPtr_t = void (*) (CanardInstance *ins,
				    CanardRxTransfer *transfer);
  using flagCbPtr_t = uint8_t (*) ();
  using errorCbPtr_t = void (*) (const etl::string_view sv);

  /**
   * @brief       structure associating message signature with message callback
   *
   */
  struct canardHandle_t {
    uint64_t signature = 0;
    receivedCbPtr_t cb = nullptr;
  };

  using subscribeMapKey_t = std::pair<uint16_t, CanardTransferType>;
  
   /**
   * @brief       pair associating message Id with canardHandle
   * @note	  represent an entry in an ordered list used as a dictionary
   */
 using subscribeMapEntry_t = std::pair<subscribeMapKey_t, canardHandle_t>;
  
  /**
   * @brief       type of map associating message id with canardHandle_t for 
   *		  broadcast, request, response mesages
   */
  using idToHandleMessage_t = etl::map<subscribeMapKey_t,
				       canardHandle_t, UAVNODE_DICTIONARY_SIZE>;

  enum busNodeType_t {BUS_FD_ONLY, BUS_FD_BX_MIXED};

  
  /**
   * @brief       Node configuration structure
   *
   */
  struct Config {
    
    /**
     * @brief       FDCan driver used for communication
     *
     */
    CANDriver		&cand;

    /**
     * @brief       FDCan configuration
     *
     */
    const CANConfig	&cancfg;

    /**
     * @brief       FD only or classic CAN and FDCan mixed bus topoly
     *
     */
    busNodeType_t	busNodeType;

    /**
     * @brief       id [1 .. 127] of the node
     *
     */
    uint8_t		nodeId = 0;

  
    /**
     * @brief    node information that others node can gather on demand   
     *
     */
    uavcan_protocol_GetNodeInfoResponse nodeInfo;

    /**
     * @brief   callback that is called to fill pkt.software_version.optional_field_flags
     *
     */
    flagCbPtr_t		flagCb; 

    /**
     * @brief   error cb mainly used to return string describing
     *          internal errors during development
     *
     */
    errorCbPtr_t  errorCb; 
  };
  
  struct node_activity {
    uint64_t timestamp_usec;
    uint8_t health;
    bool active;
    etl::string<20> name;
  };

 
  template<typename MSG_T>
  concept IsUavCanMessage = requires(MSG_T msg) {
    (void) MSG_T::cxx_iface::ID;
    (void) MSG_T::cxx_iface::SIGNATURE;
    (void) MSG_T::cxx_iface::MAX_SIZE;
  };

  // to determine type of first arguments of a function
  template<typename T>
  struct function_traits;  
  
  template<typename R, typename Arg1, typename Arg2, typename... Args>
  struct function_traits<R(*)(Arg1, Arg2, Args...)> {
    using first_arg_type = Arg1;
    using second_arg_type = Arg2;
  };
  template<typename Func>
  using FirstArgType = typename function_traits<Func>::first_arg_type;
  template<typename Func>
  using SecondArgType = typename function_traits<Func>::second_arg_type;
  // Concept to check if the argument is a function pointer with a void return type, 
  // first argument as 'int', and second argument as an aggregate with
  // 'cxx_iface::ID' field
  template<typename Func>
  concept MessageCb = requires(Func f) {
    { f } -> std::convertible_to<void(*)(CanardRxTransfer *, SecondArgType<Func>)>;
    (void) std::remove_cvref_t<SecondArgType<Func>>::cxx_iface::ID;
  };
 

  /**
   * @brief       UAVCan object
   * @details     manage messaging agents on an UAVCan bus
   *
   */
  class Node {
  public:
    enum specificStatusCode_t {SPECIFIC_OK, PS5V_UNDERVOLTAGE, PS5V_OVERVOLTAGE,
			       TEMP_LOW, TEMP_HIGH};
    enum canStatus_t {CAN_OK, TRANSMIT_RESET, TRANSMIT_TIMOUT, UAVCAN_TIMEWRAP,
		      REQUEST_DECODE_ERROR, RESPONSE_DECODE_ERROR,
		      BROADCAST_DECODE_ERROR, REQUEST_UNHANDLED_ID,
		      RESPONSE_UNHANDLED_ID, BROADCAST_UNHANDLED_ID,
		      NODE_OFFLINE
    };
   
    /**
     * @brief       Construct Node object
     * @param[in]   _config Configuration structure
     *
     */
    Node(const Config &_config);

    /**
     * @brief       Construct Node object
     * @param[in]   _config Configuration structure
     *
     */
    void start();

    /**
     * @brief       set internal node health status
     * @notes	    can be gathered by other nodes
     *
     */
    void setStatus(const uavcan_protocol_NodeStatus& status);

    /**
     * @brief       return number of active nodes on the bus
     * @notes       actives nodes are the ones which have sent alive messages
     *              in the 3 preceding seconds
     */
    uint8_t getNbActiveAgents();

    /**
     * @brief       return list of all nodes that have been seen on the bus
     * @notes       actives nodes are the ones which have sent alive messages
     *              in the 3 preceding seconds. The lists also contains inactive
     *              nodes
     */
    const std::array<node_activity, MAX_CAN_NODES> &getNodeList() {return nodes_list;}
    
    /**
     * @brief       send broadcast message
     * @notes	    templated function : can send any UAVNode message type
     * @param[in]   msg : UAVCan message  
     * @param[in]   priority : 5 bit priority [0 .. 31] (lowest = higher priority)
     * @param[in]   forceBxCan : force sending message with classical CAN protocol 
     */
    template<typename MSG_T>
    void sendBroadcast(MSG_T &msg, const uint8_t priority,
		       bool forceBxCan = false);

    /**
     * @brief       send request message
     * @notes	    templated function : can send any UAVNode message type
     * @param[in]   msg : UAVCan message  
     * @param[in]   priority : 5 bit priority [0 .. 31] (lowest = higher priority)
     * @param[in]   forceBxCan : force sending message with classical CAN protocol 
     */
    template<typename MSG_T>
    void sendRequest(MSG_T &msg, const uint8_t priority, const uint8_t dest_id,
		     bool forceBxCan = false);

    /**
     * @brief       send response message
     * @notes	    templated function : can send any UAVNode message type
     * @param[in]   msg : UAVCan message  
     * @param[in]   priority : 5 bit priority [0 .. 31] (lowest = higher priority)
     * @param[in]   forceBxCan : force sending message with classical CAN protocol 
     */
    template<typename MSG_T>
    void sendResponse(MSG_T &msg, CanardRxTransfer *transfer);

    /**
     * @brief       get fdcan layer status
     * @notes	    clear status, so next call will return CAN_OK unless new error
     *		    occurs
     *
     */
    canStatus_t getAndResetCanStatus() {return std::exchange(canStatus, CAN_OK);}

    /**
     * @brief       return true if underlying CAN driver is in OPMODE_FDCAN mode
     *
     */
    bool isCanfdEnabled() {return canFD;}

    template<auto Fn>
    static constexpr subscribeMapEntry_t makeRequestCb();
    
    template<auto Fn>
    static constexpr subscribeMapEntry_t makeResponseCb();
    
    template<auto Fn>
    static constexpr subscribeMapEntry_t makeBroadcastCb();
    

    template<auto ...Fn>
    bool subscribeBroadcastMessages() {
      return ( subscribeBroadcastOneMessage<Fn>() && ... );
    }
    template<auto ...Fn>
    bool subscribeRequestMessages() {
      return ( subscribeRequestOneMessage<Fn>() && ... );
    }
    template<auto ...Fn>
    bool subscribeResponseMessages() {
      return ( subscribeResponseOneMessage<Fn>() && ... );
    }

  private:
   /**
     * @brief	  map of message id to callback
     *
     */
    idToHandleMessage_t   idToHandleMessage;
    
    const Config &config;
    mutex_t canard_mtx_r, canard_mtx_s;
    event_source_t canard_tx_not_empty;
    CanardInstance canard;
    uavcan_protocol_NodeStatus node_status;
    uint8_t memory_pool[MEMORYPOOL_SIZE] = {};
    std::array<node_activity, MAX_CAN_NODES> nodes_list;
    thread_t *sender_thd;
    thread_t *receiver_thd;
    thread_t *heartbeat_thd;
    thread_t *can_error_thd;
    canStatus_t canStatus = {};
    bool	hasReceiveMsg = false;

    // incrémenté à chaque transfert pour détecter la
    // perte de paquets.
    uint8_t  transferRequestId = 0, transferBroadcastId = 0;
    bool canFD;
    
    bool shouldAcceptTransfer(const CanardInstance *ins,
			      uint64_t *out_data_type_signature,
			      uint16_t data_type_id,
			      CanardTransferType transfer_type,
			      uint8_t source_node_id);
    
    void onTransferReceived(CanardInstance *ins,
			    CanardRxTransfer *transfer);
 
    void senderStep();
    void receiverStep();
    void heartbeatStep();
    void initNodesList();
    void updateNodesList(uint8_t node_id, uint64_t timestamp);
    void askNodeInfo(uint8_t dest_node_id);
    void transmitQueue();
    void sendNodeStatus();
    void setCanStatus(canStatus_t cs) {canStatus = cs;}
    int8_t configureHardwareFilters();

    template<auto Fn>
    bool subscribeBroadcastOneMessage();
    template<auto Fn>
    bool subscribeRequestOneMessage();
    template<auto Fn>
    bool subscribeResponseOneMessage();

     
   /**
     * @brief       request message callback proxy
     * @notes	    call user supplied callback from libcanard transfer object
     * @param[in]   libcanard instance (not used)
     * @param[in]   libcanard transfert object
     *
     * @notapi
     */
    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void requestMessageCb(CanardInstance *, CanardRxTransfer *transfer);
 
    /**
     * @brief       response message callback proxy
     * @notes	    call user supplied callback from libcanard transfer object
     * @param[in]   libcanard instance (not used)
     * @param[in]   libcanard transfert object
     *
     * @notapi
     */
    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void responseMessageCb(CanardInstance *, CanardRxTransfer *transfer);

    /**
     * @brief       broadcast message callback proxy
     * @notes	    call user supplied callback from libcanard transfer object
     * @param[in]   libcanard instance (not used)
     * @param[in]   libcanard transfert object
     *
     * @notapi
     */
    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void broadcastMessageCb(CanardInstance *, CanardRxTransfer *transfer);
 
    static bool shouldAcceptTransferDispatch(const CanardInstance *ins,
					     uint64_t *out_data_type_signature,
					     uint16_t data_type_id,
					     CanardTransferType transfer_type,
					     uint8_t source_node_id);
    
    static void onTransferReceivedDispatch(CanardInstance *ins,
					   CanardRxTransfer *transfer);

    static void senderThdDispatch(void *opt);
    static void receiverThdDispatch(void *opt);
    static void heartbeatThdDispatch(void *opt);
    static void canErrorThdDispatch(void *opt);
    

    void handleNodeInfoResponse(CanardInstance *ins,
				CanardRxTransfer *transfer,
				const uavcan_protocol_GetNodeInfoResponse &res);
    void handleNodeStatusBroadcast(CanardInstance *ins,
				   CanardRxTransfer *transfer);
    void handleNodeInfoRequest(CanardInstance *ins,
			       CanardRxTransfer *transfer);
    template<typename...Params>
    void errorCb(const char * format, Params&&... params);
  };




  template<typename MSG_T>
  void Node::sendBroadcast(MSG_T &msg, const uint8_t priority,
			   bool forceBxCan)
  {
    uint8_t buffer[MSG_T::cxx_iface::MAX_SIZE];
    
    const bool fdFrame = isCanfdEnabled() and not forceBxCan;
    const uint16_t len = MSG_T::cxx_iface::encode(&msg, buffer,
						  not fdFrame);
    CanardTxTransfer broadcast = {
      .transfer_type = CanardTransferTypeBroadcast,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transferBroadcastId,
      .priority = priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = fdFrame,
      .tao = not fdFrame
    };
    {
      MutexGuard gard(canard_mtx_s);
      canardBroadcastObj(&canard, &broadcast);
    }
    chEvtBroadcast(&canard_tx_not_empty);
  }
  
  template<typename MSG_T>
  void Node::sendRequest(MSG_T &msg, const uint8_t priority, const uint8_t dest_id,
			 bool forceBxCan)
  {
    uint8_t buffer[MSG_T::cxx_iface::REQ_MAX_SIZE];
    const bool fdFrame = isCanfdEnabled() and not forceBxCan;
    const uint16_t len = MSG_T::cxx_iface::req_encode(&msg, buffer,
						      not fdFrame);
    CanardTxTransfer request = {
      .transfer_type = CanardTransferTypeRequest,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transferRequestId,
      .priority = priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = fdFrame,
      .tao = not fdFrame
    };
    {
      MutexGuard gard(canard_mtx_s);
      canardRequestOrRespondObj(&canard, dest_id, &request);
    }
    chEvtBroadcast(&canard_tx_not_empty);
  }
  
  template<typename MSG_T>
  void Node::sendResponse(MSG_T &msg, CanardRxTransfer *transfer)
  {
    uint8_t buffer[MSG_T::cxx_iface::RSP_MAX_SIZE];
    const bool fdFrame = transfer->canfd;
    const uint16_t len = MSG_T::cxx_iface::rsp_encode(&msg, buffer,
						      not fdFrame);
    CanardTxTransfer response = {
      .transfer_type = CanardTransferTypeResponse,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transfer->transfer_id,
      .priority =  transfer->priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = fdFrame,
      .tao = not fdFrame
    };
    {
      MutexGuard gard(canard_mtx_s);
      canardRequestOrRespondObj(&canard, transfer->source_node_id, &response);
    }
    chEvtBroadcast(&canard_tx_not_empty);
  }
  

  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::requestMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::reqtype msg = {};
    const bool decode_error = MsgType::cxx_iface::req_decode(transfer, &msg);
    Node *node = static_cast<Node *>(canardGetUserReference(ins));
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_GetNodeInfoRequest>) {
	node->handleNodeInfoRequest(ins, transfer);
      }
      Fn(transfer, msg);
    } else {
      node->setCanStatus(REQUEST_DECODE_ERROR);
      node->errorCb("requestMessageCb decode error on id %u", MsgType::cxx_iface::ID);
    }
  }

  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::responseMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::rsptype msg = {};
    const bool decode_error = MsgType::cxx_iface::rsp_decode(transfer, &msg);
    Node *node = static_cast<Node *>(canardGetUserReference(ins));
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_GetNodeInfoResponse>) {
	node->handleNodeInfoResponse(ins, transfer, msg);
      }
      Fn(transfer, msg);
    } else {
      node->setCanStatus(RESPONSE_DECODE_ERROR);
      node->errorCb("responseMessageCb decode error on id %u", MsgType::cxx_iface::ID);
    }
  }
  
  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::broadcastMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::msgtype msg = {};
    const bool decode_error = MsgType::cxx_iface::decode(transfer, &msg);
    Node *node = static_cast<Node *>(canardGetUserReference(ins));
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_NodeStatus>) {
	node->handleNodeStatusBroadcast(ins, transfer);
      }
      Fn(transfer, msg);
    } else {
      node->setCanStatus(BROADCAST_DECODE_ERROR);
      node->errorCb("broadcastMessageCb decode error on id %u", MsgType::cxx_iface::ID);
    }
  }

  template<auto Fn>
  constexpr subscribeMapEntry_t Node::makeRequestCb() {
    using msgt = cbTraits<decltype(Fn)>::msgt;
    return  std::pair{
      subscribeMapKey_t {msgt::cxx_iface::ID, CanardTransferTypeRequest},
      (canardHandle_t) {
	.signature = msgt::cxx_iface::SIGNATURE,		
	.cb = UAVCAN::Node::requestMessageCb<Fn>}
    };
  }

  template<auto Fn>
  constexpr subscribeMapEntry_t Node::makeResponseCb() {
    using msgt = cbTraits<decltype(Fn)>::msgt;
    return  std::pair{
      subscribeMapKey_t {msgt::cxx_iface::ID, CanardTransferTypeResponse},
      (canardHandle_t) {
	.signature = msgt::cxx_iface::SIGNATURE,		
	.cb = UAVCAN::Node::responseMessageCb<Fn>}
    };
  }

  template<auto Fn>
  constexpr subscribeMapEntry_t Node::makeBroadcastCb() {
    using msgt = cbTraits<decltype(Fn)>::msgt;
    return  std::pair{
      subscribeMapKey_t {msgt::cxx_iface::ID, CanardTransferTypeBroadcast},
      (canardHandle_t) {
	.signature = msgt::cxx_iface::SIGNATURE,		
	.cb = UAVCAN::Node::broadcastMessageCb<Fn>}
    };
  }

  template<auto Fn>
    bool Node::subscribeBroadcastOneMessage() {
      if (idToHandleMessage.full()) {
	errorCb("idToHandleMessage is full");
	return false;
      }
      constexpr subscribeMapEntry_t keyValue = makeBroadcastCb<Fn>();
      MutexGuard gard(canard_mtx_r);
      idToHandleMessage.insert(keyValue);
      return true;
    }

  template<auto Fn>
    bool Node::subscribeRequestOneMessage() {
      if (idToHandleMessage.full()) {
	errorCb("idToHandleMessage is full");
	return false;
      }
      constexpr subscribeMapEntry_t keyValue = makeRequestCb<Fn>();
      MutexGuard gard(canard_mtx_r);
      idToHandleMessage.insert(keyValue);
      return true;
    }

  template<auto Fn>
    bool Node::subscribeResponseOneMessage() {
      if (idToHandleMessage.full()) {
	errorCb("idToHandleMessage is full");
	return false;
      }
      constexpr subscribeMapEntry_t keyValue = makeResponseCb<Fn>();
      MutexGuard gard(canard_mtx_r);
      idToHandleMessage.insert(keyValue);
      return true;
    }


template<typename...Params>
void Node::errorCb(const char * format, [[maybe_unused]] Params&&... params)
{
#if CH_DBG_ENABLE_CHECKS
  if (config.errorCb) {
    etl::string<80> s;
    const auto len = chsnprintf(s.data(), s.capacity(), format, std::forward<Params> (params)...);
    s.uninitialized_resize(len);
    config.errorCb(etl::string_view(s));
  }
#else
  (void) format;
#endif
}

  
}

/** @} */
