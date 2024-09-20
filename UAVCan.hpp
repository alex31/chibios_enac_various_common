#pragma once

#include <ch.h>
#include <hal.h>

#include "canard.h"
#include "canard_chutils.h"
#include "dronecan_msgs.h"
#include "stdutil.h"
#include "etl/map.h"
#include "etl/string.h"
#include <array>
#include <type_traits>
#include <concepts>

#ifndef UAVNODE_MAX_MSGID
#define UAVNODE_MAX_MSGID 16
#endif
#ifndef UAVNODE_MAX_REQ_MSGID
#define UAVNODE_MAX_REQ_MSGID 8
#endif
#ifndef UAVNODE_MAX_RESP_MSGID
#define UAVNODE_MAX_RESP_MSGID 8
#endif
/*
  TODO :

  ° porter sur devboardH7 et tester une comm H7 <->  H7
    module CANCOM avec tout ce qui est FDCAN planqué dedans
           + l'esclave attend un uavcan_equipment_actuator_Command (4 servos)
	   + l'autopilote envoie des commandes (bouton rotatif + selection par joystick)
	   + l'esclave a un capteur (baro) et renvoie temperature et pression
	   + l'autopilote affiche temperature et pression
	   + chacun utilise une led tricolore : vert comm ok, rouge : pas de comm
	   + en fonction d'un switch, prendre un role ou l'autre pour avoir le même code sur les 2 cartes

  ° tester une comm H7 <-> G491 à 4Mbit, puis 4.25 Mbits

 */
namespace {

}

// to determine type of first arguments of a function
template<typename T>
struct cbTraits;  

template<typename R, typename Arg1, typename Arg2, typename... Args>
struct cbTraits<R(Arg1, Arg2, Args...)> {
  using raw_second_arg_type =  Arg2;
  using msgt = std::remove_cvref_t<raw_second_arg_type>;
};
 

#define UAVCAN_REQ(cb) {cbTraits<decltype(cb)>::msgt::cxx_iface::ID, \
      {cbTraits<decltype(cb)>::msgt::cxx_iface::REQUEST_SIGNATURE,	\
	  UAVCAN::Node::requestMessageCb<cb>}}

#define UAVCAN_RESP(cb) {cbTraits<decltype(cb)>::msgt::cxx_iface::ID, \
      {cbTraits<decltype(cb)>::msgt::cxx_iface::RESPONSE_SIGNATURE,	 \
	  UAVCAN::Node::responseMessageCb<cb>}}

#define UAVCAN_BCAST(cb) {cbTraits<decltype(cb)>::msgt::cxx_iface::ID, \
      {cbTraits<decltype(cb)>::msgt::cxx_iface::SIGNATURE,		  \
	  UAVCAN::Node::broadcastMessageCb<cb>}}


namespace UAVCAN {
  static constexpr size_t MAX_CAN_NODES = 128;
  static constexpr size_t MEMORYPOOL_SIZE = 4096;
  
  using receivedCbPtr_t = void (*) (CanardInstance *ins,
				    CanardRxTransfer *transfer);
  using flagCbPtr_t = uint8_t (*) ();
  struct canardHandle {
    uint64_t signature = 0;
    receivedCbPtr_t cb = nullptr;
  };

  
  using idToHandleRequest_t = etl::map<uint16_t, canardHandle, UAVNODE_MAX_REQ_MSGID>;
  using idToHandleResponse_t = etl::map<uint16_t, canardHandle, UAVNODE_MAX_RESP_MSGID>;
  using idToHandleBroadcast_t = etl::map<uint16_t, canardHandle, UAVNODE_MAX_MSGID>;
  
  struct Config {
    CANDriver		&cand;
    const CANConfig	&cancfg;
    uint8_t		nodeId = 0;
    idToHandleRequest_t   &idToHandleRequest;
    idToHandleResponse_t  &idToHandleResponse;
    idToHandleBroadcast_t &idToHandleBroadcast;
    uavcan_protocol_GetNodeInfoResponse nodeInfo;
    flagCbPtr_t		flagCb; // dynamic return for optional_field_flags field
  };
  
  struct node_activity {
    uint64_t timestamp_usec;
    uint8_t health;
    bool active;
    etl::string<20> name;
  };

  enum specificStatusCode {SPECIFIC_OK, PS5V_UNDERVOLTAGE, PS5V_OVERVOLTAGE,
			   TEMP_LOW, TEMP_HIGH};
 
 
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
 

  class Node {
  public:
    Node(const Config &_config);
    void start();
    void setStatus(uint8_t health, uint8_t mode,  specificStatusCode specific_code);
    uint8_t getNbActiveAgents();
    const std::array<node_activity, MAX_CAN_NODES> &getNodeList() {return nodes_list;}
    template<typename MSG_T>
    void broadcastMessage(MSG_T &msg, const uint8_t priority);
    template<typename MSG_T>
    void sendRequest(MSG_T &msg, const uint8_t priority, const uint8_t dest_id);
    template<typename MSG_T>
    void sendResponse(MSG_T &msg, CanardRxTransfer *transfer);

    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void requestMessageCb(CanardInstance *, CanardRxTransfer *transfer);
    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void responseMessageCb(CanardInstance *, CanardRxTransfer *transfer);
    template<auto Fn> requires MessageCb<decltype(Fn)>
    static void broadcastMessageCb(CanardInstance *, CanardRxTransfer *transfer);

  private:
    const Config &config;
    mutex_t canard_mtx;
    event_source_t canard_tx_not_empty;
    CanardInstance canard;
    uavcan_protocol_NodeStatus node_status;
    uint8_t memory_pool[MEMORYPOOL_SIZE] = {};
    std::array<node_activity, MAX_CAN_NODES> nodes_list;
    thread_t *sender_thd;
    thread_t *receiver_thd;
    thread_t *heartbeat_thd;

    // incrémenté à chaque transfert pour détecter la
    // perte de paquets.
    uint8_t  transferRequestId = 0, transferBroadcastId = 0;
    
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
    

    void handleNodeInfoResponse(CanardInstance *ins,
				CanardRxTransfer *transfer,
				const uavcan_protocol_GetNodeInfoResponse &res);
    void handleNodeStatusBroadcast(CanardInstance *ins,
				CanardRxTransfer *transfer);
    void handleNodeInfoRequest(CanardInstance *ins,
			       CanardRxTransfer *transfer);
   };



  template<typename MSG_T>
  void Node::broadcastMessage(MSG_T &msg, const uint8_t priority)
  {
    uint8_t buffer[MSG_T::cxx_iface::MAX_SIZE];
    const uint16_t len = MSG_T::cxx_iface::encode(&msg, buffer, !(canard.tao_disabled));
    CanardTxTransfer broadcast = {
      .transfer_type = CanardTransferTypeBroadcast,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transferBroadcastId,
      .priority = priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = true,
      .tao = !(canard.tao_disabled)
    };
    canardBroadcastObj(&canard, &broadcast);
    chEvtBroadcast(&canard_tx_not_empty);
  }
  
  template<typename MSG_T>
  void Node::sendRequest(MSG_T &msg, const uint8_t priority, const uint8_t dest_id)
  {
    uint8_t buffer[MSG_T::cxx_iface::REQ_MAX_SIZE];
    const uint16_t len = MSG_T::cxx_iface::req_encode(&msg, buffer, !(canard.tao_disabled));
    CanardTxTransfer request = {
      .transfer_type = CanardTransferTypeRequest,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transferRequestId,
      .priority = priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = true,
      .tao = !(canard.tao_disabled)
    };
    canardRequestOrRespondObj(&canard, dest_id, &request);
    chEvtBroadcast(&canard_tx_not_empty);
  }

  template<typename MSG_T>
  void Node::sendResponse(MSG_T &msg, CanardRxTransfer *transfer)
  {
    uint8_t buffer[MSG_T::cxx_iface::RSP_MAX_SIZE];
    const uint16_t len = MSG_T::cxx_iface::rsp_encode(&msg, buffer, !(canard.tao_disabled));
    CanardTxTransfer response = {
      .transfer_type = CanardTransferTypeRequest,
      .data_type_signature = MSG_T::cxx_iface::SIGNATURE,
      .data_type_id = MSG_T::cxx_iface::ID,
      .inout_transfer_id = &transfer->transfer_id,
      .priority =  transfer->priority,
      .payload = buffer,
      .payload_len = len,
      .canfd = true,
      .tao = !(canard.tao_disabled)
    };
    canardRequestOrRespondObj(&canard, transfer->source_node_id, &response);
    chEvtBroadcast(&canard_tx_not_empty);
  }
  

  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::requestMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::reqtype msg = {};
    const bool decode_error = MsgType::cxx_iface::req_decode(transfer, &msg);
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_GetNodeInfoRequest>) {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->handleNodeInfoRequest(ins, transfer);
	Fn(transfer, msg);
      } else {
	DebugTrace("requestMessageCb decode error on id %u", MsgType::cxx_iface::ID);
      }
    }
  }


  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::responseMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::rsptype msg = {};
    const bool decode_error = MsgType::cxx_iface::rsp_decode(transfer, &msg);
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_GetNodeInfoResponse>) {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->handleNodeInfoResponse(ins, transfer, msg);
	Fn(transfer, msg);
      } else {
	DebugTrace("responseMessageCb decode error on id %u", MsgType::cxx_iface::ID);
      }
    }
  }
  
  template<auto Fn> requires MessageCb<decltype(Fn)>
  void Node::broadcastMessageCb(CanardInstance *ins, CanardRxTransfer *transfer)
  {
    using MsgType = std::remove_cvref_t<SecondArgType<decltype(Fn)>>;
    typename MsgType::cxx_iface::msgtype msg = {};
    const bool decode_error = MsgType::cxx_iface::decode(transfer, &msg);
    if (!decode_error) {
      if constexpr (std::is_same_v<MsgType, uavcan_protocol_NodeStatus>) {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->handleNodeStatusBroadcast(ins, transfer);
      }
      Fn(transfer, msg);
    } else {
      DebugTrace("broadcastMessageCb decode error on id %u", MsgType::cxx_iface::ID);
    }
  }
  
 
}
