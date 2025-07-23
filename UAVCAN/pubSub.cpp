/**
 * @file    pubSub.cpp
 * @brief   C++ wrapper over libcanard
 *
 * @{
 */
#include "pubSub.hpp"
#include <cstring>
#include "canard.h"

namespace {
  inline uint64_t getTimestampUS() {
    return chVTGetTimeStamp() * (1'000'000U / CH_CFG_ST_FREQUENCY);
  }
  inline uint64_t getTimestampMS() {
    return getTimestampUS() / 1000U;
  }
  inline uint32_t getTimestampS() {
    return getTimestampMS() / 1000U;
  }

  /**
   * @brief   Get bus status 
   * @note    This is an STM32-specific API.
   *          under certain circumstances, can peripheral can be set offline
   * @param[in] canp    pointer to the @p CANDriver object
   * @param[out]        true if the peripheral is online
   *
   * @notapi
   */
  bool can_lld_is_online(CANDriver *canp) {
    return (canp->fdcan->CCCR & FDCAN_CCCR_INIT) == 0;
  }
  
  /**
   * @brief   Set bus status 
   * @note    This is an STM32-specific API.
   *          force peripheral to be online or offline
   * @param[in] canp    pointer to the @p CANDriver object
   * @param[in] online  set the peripheral online if true, offline otherwise
   *
   * @notapi
   */
  void can_lld_set_online(CANDriver *canp, bool online) {
    if (online) {
      canp->fdcan->CCCR &= ~FDCAN_CCCR_INIT;
    } else {
      canp->fdcan->CCCR |= FDCAN_CCCR_INIT;
    }
  }
  
  
  
  /**
   * @brief   Resume peripheral online after offline event occurred
   * @note    This is an STM32-specific API.
   *          In certain circumstances, the CAN peripheral may disconnect
   *          itself from the bus.
   * @param[in] canp    pointer to the @p CANDriver object
   *
   */
  void canSTM32ResumeOnline(CANDriver *canp) {
    osalDbgAssert(canp->state == CAN_READY, "invalid state");
    if (!can_lld_is_online(canp)) {
      can_lld_set_online(canp, true);
    }
  }

  typedef struct {
    uint8_t id[16];
  } UniqId_t;

  
  /**
   * @brief   DLC -> Data length converter.
   */

  static uint16_t dlcToDataLength(uint16_t dlc) {
    static const uint16_t lookup[] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
  
    chDbgAssert(dlc <= 15, "invalid DLC value");
    //  DebugTrace("dlcToDataLength(%u) = %u", dlc, lookup[dlc]);
    return lookup[dlc];
  }

  /**
   * @brief   DLC <- Data length converter.
   */

  static uint8_t dataLengthToDlc(uint16_t data_length) {
    static const uint8_t lookup[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
				     9, 9, 9, 9, 10,10,10,10,
				     11,11,11,11,12,12,12,12,
				     13,13,13,13,13,13,13,13,
				     14,14,14,14,14,14,14,14,
				     14,14,14,14,14,14,14,14,
				     15,15,15,15,15,15,15,15,
				     15,15,15,15,15,15,15,15};
    chDbgAssert(data_length <= 64, "invalid data length value");
    //  DebugTrace("dataLengthToDlc(%u) = %u", data_length, lookup[data_length]);
    return lookup[data_length];
  }

  /*===========================================================================*/
  /* Module exported functions.                                                */
  /*===========================================================================*/

  /**
   * @brief   ChibiOS CANRxFrame to Canard CanardCANFrame Type converter.
   */
  CanardCANFrame chibiRx2canard(const CANRxFrame frame){
    CanardCANFrame out_frame;


    out_frame.id = frame.ext.EID | frame.common.XTD << 31U |
      frame.common.RTR << 30U | frame.common.ESI << 29U;
#if CANARD_ENABLE_CANFD
    out_frame.canfd = frame.FDF | frame.BRS;
#endif
    out_frame.iface_id = 0;
    out_frame.data_len = dlcToDataLength(frame.DLC);
    memcpy(out_frame.data, frame.data8, out_frame.data_len);
    return out_frame;
  }

  /**
   * @brief   Canard CanardCANFrame to ChibiOS CANTxFrame Type converter.
   */
  CANTxFrame canard2chibiTx(const CanardCANFrame* framep){
    CANTxFrame out_frame;
    out_frame.common.XTD = 1;
    out_frame.common.RTR = 0;
    out_frame.ext.EID = framep->id & 0x1FFFFFFF;
#if CANARD_ENABLE_CANFD
    out_frame.FDF = framep->canfd;         /* FDCAN frame format. */
    out_frame.BPS = framep->canfd;
#else
    out_frame.FDF = false;
    out_frame.BPS = false;
#endif
    out_frame.DLC = dataLengthToDlc(framep->data_len);
    memcpy(out_frame.data8, framep->data, framep->data_len);
    return out_frame;
  }

  /**
   * @brief   Unique ID Fetcher.
   * @note    Retrieves the first two bytes of the MCU's unique id.
   */
  void getUniqueID(UniqId_t *uid)
  {
    memcpy(uid->id, (const uint8_t *)UID_BASE, sizeof(uid->id));
  } 




  
}

namespace UAVCAN
{
  template<typename MSG_T>
  void nullAppCb(CanardRxTransfer *, const MSG_T) {
    // DebugTrace("nullAppCb type = %u id = %u",
    // 	       t->transfer_type, 
    // 	       t->data_type_id);
    
  }
  
  
  Node::Node(const Config &_config) : config(_config),
				      node_status{
			.uptime_sec = 0,
			.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
			.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION,
			.sub_mode = 0,
			.vendor_specific_status_code = SPECIFIC_OK},
				      canFD(_config.cancfg.op_mode == OPMODE_FDCAN)
    
  {
    chMtxObjectInit(&canard_mtx_s);
    chMtxObjectInit(&canard_mtx_r);
    chEvtObjectInit(&canard_tx_not_empty);
}
  
  int8_t Node::configureHardwareFilters()
  {
    int8_t filterIndex = 0;
    static constexpr uint8_t filtersSize = 
#ifdef FDCAN_CONFIG_RXGFC_RRFS // ugly meanway to know if we use HDCANV1 ou V2
      8; // FDCANV1 : each FDCan interface has 8 extended filters
#else // FDCANV2 : 32 extended filters are shared between FDCan interface(s) in use
#    if ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 1)
    32;
#    elif ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 2)
    16;
#    elif ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 3)
    10;
#    endif
#endif


    static constexpr uint32_t masks[] = {
      0x00'ff'80'00, // response
      0x00'ff'80'00, // request
      0x00'ff'ff'00  // broadcast    
    };
    
    CANFilter  filterList[filtersSize];
    
    for (const auto& [msg_id_type, _] :  idToHandleMessage) {
      const auto& [msg_id, cttype] = msg_id_type;
      uint32_t filterId;
      switch (cttype) {
      case CanardTransferTypeRequest:
      chDbgAssert(msg_id < 256, "request service message id should be in the range 0 .. 255");
      filterId = (msg_id << 16) | (0x80 << 8);
      break;
      case CanardTransferTypeResponse:
      chDbgAssert(msg_id < 256, "response service message id should be in the range 0 .. 255");
      filterId = (msg_id << 16) | (0x00 << 8);
      break;
      default:
	filterId =  msg_id << 8;
      }
      filterList[filterIndex] = {
	.filter_type = CAN_FILTER_TYPE_EXT,
	.filter_mode = CAN_FILTER_MODE_CLASSIC,
	.filter_cfg =  CAN_FILTER_CFG_FIFO_0,
	.identifier1 = filterId,
	.identifier2 = masks[cttype]
      };
      if (++filterIndex >= filtersSize)
	goto overfill;
    }
    
   canSTM32SetFilters(&config.cand, filterIndex, filterList);
   return filterIndex;

  overfill: // if we don't have enough filter, we do nothing and rely on sofware filtering
   // TODO : use filters to etablish reject list (probably mean modifying driver to enable reject list)
    return -1;
  }
  
  void Node::start()
  {
    /*
      if application has not bind callback to any of these 3 message id,
      library should create entry in the map for internal message management
      if  application has bind callback, the entry is already there and management
      will be done
     */
    if (not idToHandleMessage.contains({UAVCAN_PROTOCOL_GETNODEINFO_ID,
	  CanardTransferTypeRequest})) {
      if (not idToHandleMessage.full()) {
	idToHandleMessage[{UAVCAN_PROTOCOL_GETNODEINFO_ID, CanardTransferTypeRequest}] = {
	  UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE,
	  UAVCAN::Node::requestMessageCb<nullAppCb<uavcan_protocol_GetNodeInfoRequest>>
	};
      } else {
	errorCb("idToHandleMessage is full");
      }
    }

    if (not idToHandleMessage.contains({UAVCAN_PROTOCOL_GETNODEINFO_ID,
	  CanardTransferTypeResponse})) {
      if (not idToHandleMessage.full()) {
	idToHandleMessage[{UAVCAN_PROTOCOL_GETNODEINFO_ID,
	  CanardTransferTypeResponse}] = {
	  UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE,
	  UAVCAN::Node::responseMessageCb<nullAppCb<uavcan_protocol_GetNodeInfoResponse>>
	};
      } else {
	errorCb("idToHandleMessage is full");
      }
    }

    if (not idToHandleMessage.contains({UAVCAN_PROTOCOL_NODESTATUS_ID,
	  CanardTransferTypeBroadcast})) {
      if (not idToHandleMessage.full()) {
	idToHandleMessage[{UAVCAN_PROTOCOL_NODESTATUS_ID,
	  CanardTransferTypeBroadcast}] = {
	  UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
	  UAVCAN::Node::broadcastMessageCb<nullAppCb<uavcan_protocol_NodeStatus>>
	};
      } else {
	errorCb("idToHandleMessage is full");
      }
    }

    canardInit(&canard, memory_pool, MEMORYPOOL_SIZE,
	       &onTransferReceivedDispatch,
	       &shouldAcceptTransferDispatch, this);
    canardSetLocalNodeID(&canard, config.nodeId);
    initNodesList();
    canStart(&config.cand, &config.cancfg);
    
    int8_t filtersInUse = configureHardwareFilters();
    const bool rejectNonAcceptedId =
#ifdef FDCAN_CONFIG_RXGFC_ANFE_REJECT
      (config.cancfg.RXGFC & FDCAN_CONFIG_RXGFC_ANFE_REJECT) != 0;
#else
    (config.cancfg.RXGFC & FDCAN_CONFIG_GFC_ANFE_REJECT) != 0;
#endif
    //    int8_t filtersInUse = 0;
    if (filtersInUse < 0) {
      errorCb("WARN: too many messages fo hardware filtering, "
	      "revert to software filtering");
       chDbgAssert(not rejectNonAcceptedId,
		  "cancfg.RXGFC must be corrected to accept all msg id");
    } else {
      chDbgAssert(rejectNonAcceptedId || filtersInUse == 0,
		  "cancfg.RXGFC must be corrected to reject filtered msg id");
      errorCb("INFO: hardware filtering use %d slots", filtersInUse);
    }
    sender_thd = chThdCreateFromHeap(nullptr, 2048U, "sender_thd",
				      NORMALPRIO,
				      &senderThdDispatch, this);
    receiver_thd = chThdCreateFromHeap(nullptr, 4096U, "receiver_thd",
				      NORMALPRIO,
				      &receiverThdDispatch, this);
    heartbeat_thd = chThdCreateFromHeap(nullptr, 2048U, "heartbeat_thd",
				       NORMALPRIO,
				       &heartbeatThdDispatch, this);
    can_error_thd = chThdCreateFromHeap(nullptr, 1024U, "can_error_thd",
				       NORMALPRIO,
				       &canErrorThdDispatch, this);
  }
  
  /*
    #                        _              _      _                 
    #                       | |            | |    (_)                
    #                 ___   | |_     __ _  | |_    _     ___         
    #                / __|  | __|   / _` | | __|  | |   / __|        
    #                \__ \  \ |_   | (_| | \ |_   | |  | (__         
    #                |___/   \__|   \__,_|  \__|  |_|   \___|        
    #                                    _      _                   _          
    #                                   | |    | |                 | |         
    #                 _ __ ___     ___  | |_   | |__     ___     __| |         
    #                | '_ ` _ \   / _ \ | __|  | '_ \   / _ \   / _` |         
    #                | | | | | | |  __/ \ |_   | | | | | (_) | | (_| |         
    #                |_| |_| |_|  \___|  \__|  |_| |_|  \___/   \__,_|         
  */
  
  // dispatch to member function
  bool Node::shouldAcceptTransferDispatch(const CanardInstance *ins,
				    uint64_t *out_data_type_signature,
				    uint16_t data_type_id,
				    CanardTransferType transfer_type,
				    uint8_t source_node_id)
  {
    Node *node = static_cast<Node *>(canardGetUserReference(ins));
    return node->shouldAcceptTransfer(ins, out_data_type_signature, data_type_id,
					  transfer_type, source_node_id);
  }
    
  // dispatch to member function
  void Node::onTransferReceivedDispatch(CanardInstance *ins,
				  CanardRxTransfer *transfer)
  {
    Node *node = static_cast<Node *>(canardGetUserReference(ins));
    node->onTransferReceived(ins, transfer);
  }

  void Node::senderThdDispatch(void *opt)
  {
    Node *node = static_cast<Node *>(opt);
    event_listener_t el;
    chEvtRegister(&node->canard_tx_not_empty, &el, 0);
    while(not chThdShouldTerminateX()) {
      chEvtWaitAny(ALL_EVENTS);
      node->senderStep();
    }
    chEvtUnregister(&node->canard_tx_not_empty, &el);
    chThdExit(0);
  }
  
  void Node::receiverThdDispatch(void *opt)
  {
    Node *node = static_cast<Node *>(opt);
    while(not chThdShouldTerminateX()) {
      node->receiverStep();
    }
    chThdExit(0);
  }
  
  void Node::heartbeatThdDispatch(void *opt)
  {
    Node *node = static_cast<Node *>(opt);

    while(not chThdShouldTerminateX()) {
      node->heartbeatStep();
    }
    chThdExit(0);
  }

  void Node::canErrorThdDispatch(void *opt)
  {
    Node *node = static_cast<Node *>(opt);
    event_listener_t el;
    constexpr eventmask_t fdcanBusOffEvent = 1 << 0;
    chEvtRegisterMaskWithFlags(&node->config.cand.error_event, &el,
			       fdcanBusOffEvent, CAN_BUSOFF_ERROR);

    // reset online if no message where ever received (alone on the bus)
    // after messages are received wait 5mn before each resume online
    while(not chThdShouldTerminateX()) {
      chEvtWaitOne(fdcanBusOffEvent);
      canSTM32ResumeOnline(&node->config.cand);
      node->setCanStatus(NODE_OFFLINE);
      node->errorCb("canErrorThdDispatch bus_off condition");
      if (node->hasReceiveMsg) {
	chThdSleepSeconds(300);
      }
    }

    chThdExit(0);
  }

/*
#                                    _      _                   _          
#                                   | |    | |                 | |         
#                 _ __ ___     ___  | |_   | |__     ___     __| |         
#                | '_ ` _ \   / _ \ | __|  | '_ \   / _ \   / _` |         
#                | | | | | | |  __/ \ |_   | | | | | (_) | | (_| |         
#                |_| |_| |_|  \___|  \__|  |_| |_|  \___/   \__,_|         
*/
uint8_t Node::getNbActiveAgents()
{
  uint8_t sum = 0;
  for (auto& node : nodes_list)
    if (node.active)
      sum++;
  return sum;
}



  bool Node::shouldAcceptTransfer(const CanardInstance *ins,
			    uint64_t *out_data_type_signature,
			    uint16_t data_type_id,
			    CanardTransferType transfer_type,
			    uint8_t source_node_id)
  {
    (void) ins;
    (void) source_node_id;
    
    if (auto sigItr = idToHandleMessage.find({data_type_id,transfer_type});
	sigItr !=  idToHandleMessage.end()) {
      *out_data_type_signature = sigItr->second.signature;
      return true;
    }
    
    errorCb("INFO: id %x of type %d is not hardware filtered",
	    transfer_type, data_type_id);
    return false;
  }
  
  void Node::onTransferReceived(CanardInstance *ins,
			  CanardRxTransfer *transfer)
  {
    if (auto sigItr = idToHandleMessage.find(
		      {transfer->data_type_id,
		       static_cast<CanardTransferType>(transfer->transfer_type)});
      sigItr !=  idToHandleMessage.end()) {
      sigItr->second.cb(ins, transfer);
    } else {
      Node *node = static_cast<Node *>(canardGetUserReference(ins));
      node->setCanStatus(REQUEST_UNHANDLED_ID);
      node->errorCb("INFO onTransferReceived Request id %u type %d not handled",
		    transfer->data_type_id, transfer->transfer_type);
    }
  }
  
  void Node::senderStep()
  {
    MutexGuard gard(canard_mtx_s);
    transmitQueue();
  }
  
  void Node::receiverStep()
  {
    CANRxFrame rxmsg;
    while (canReceiveTimeout(&config.cand, CAN_ANY_MAILBOX, &rxmsg,
                             TIME_INFINITE) == MSG_OK) {
      // Traitement de la trame.
      // errorCb("INFO Receive Msg from 0x%x, length %u",
      //  		    uint32_t(rxmsg.ext.EID), uint32_t(rxmsg.DLC));
      hasReceiveMsg = true;
      CanardCANFrame rx_frame_canard = chibiRx2canard(rxmsg);
      MutexGuard gard(canard_mtx_r);
      /*
       * On reconstruit un message unique pour les transferts de plusieurs
       * trames et on les passe à travers shouldAcceptTransfer() puis
       * onTransferRecieved().
       */
      canardHandleRxFrame(&canard, &rx_frame_canard,  getTimestampUS());
    }
  }
  
  void Node::heartbeatStep()
  {
    // nettoyage de NodesList.
    const auto current_time_us = getTimestampUS();
    const auto current_time_ms = current_time_us / 1000U;

    for (auto& node : nodes_list) {
      if (node.active) {
	const uint32_t timestamp_ms = node.timestamp_usec / (1000UL);
        /*
         * DroneCAN spécifie qu'un noeud ne peut plus être considéré actif après
         * 3 secondes sans émission d'un message NodeStatus, ce qu'on vérifie
         * ici.
         */
	if (current_time_ms > timestamp_ms) {
          node.active = ((current_time_ms - timestamp_ms) <
			 UAVCAN_PROTOCOL_NODESTATUS_OFFLINE_TIMEOUT_MS);
	} else {
	  setCanStatus(UAVCAN_TIMEWRAP);
	  errorCb("Warning : current time wraparound or "
		  "message recieved in the future (unlikely)");
	  
	}
      }
    }

    {
      MutexGuard gard(canard_mtx_s);
      // nettoyage recommandé par la librairie.
      canardCleanupStaleTransfers(&canard, current_time_us);
    }
    // Génération de NodeStatus.
    sendNodeStatus();
    chThdSleepMilliseconds(1000);
  }


  void Node::initNodesList()
  {
    for (auto& node : nodes_list)
      node = {};
  }

  void Node::updateNodesList(uint8_t node_id, uint64_t timestamp)
  {
    /*
     * Actualise la liste des noeuds actifs.
     * Appelée à chaque réception de NodeStatus.
     */
    chDbgAssert(node_id <  nodes_list.size(), "erreur sur nodeID");
    if (nodes_list[node_id].active == false) {
      askNodeInfo(node_id);
    } else {
      nodes_list[node_id].timestamp_usec = timestamp;
    };
  }


  void Node::askNodeInfo(uint8_t dest_node_id)
  {
    /*
     * Génère et place une requête d'infos pour un noeud dans la queue de
     * transmission canard. Appelée par updateNodesList().
     */
    uavcan_protocol_GetNodeInfoRequest req = {};
    sendRequest(req, CANARD_TRANSFER_PRIORITY_LOW, dest_node_id,
		config.busNodeType == BUS_FD_BX_MIXED);
  }

 
  void Node::handleNodeInfoResponse(CanardInstance *,
				    CanardRxTransfer *transfer,
				    const uavcan_protocol_GetNodeInfoResponse &res)
  {
    const uint8_t node_id = transfer->source_node_id;
    const uint64_t timestamp = transfer->timestamp_usec;
    
    chDbgAssert(node_id < nodes_list.size(), "erreur sur nodeID");
    nodes_list[node_id].active = true;
    nodes_list[node_id].timestamp_usec = timestamp;
    nodes_list[node_id].health = res.status.health;
    nodes_list[node_id].name.assign(reinterpret_cast<const char*>(res.name.data), res.name.len);
  }

  void Node::handleNodeStatusBroadcast(CanardInstance *,
				       CanardRxTransfer *transfer)
  {
    const uint8_t node_id = transfer->source_node_id;
    const uint64_t timestamp = transfer->timestamp_usec;
    updateNodesList(node_id, timestamp);
  }

  
  void Node::handleNodeInfoRequest(CanardInstance *ins,
				   CanardRxTransfer *transfer)
  {
#if CH_DBG_ENABLE_ASSERTS
    chDbgAssert(ins == &canard, "internal incoherency");
#else
    (void) ins;
#endif
    uavcan_protocol_GetNodeInfoResponse pkt = config.nodeInfo;
    node_status.uptime_sec = getTimestampS();
    pkt.status = node_status;
    if (config.flagCb != nullptr) {
      pkt.software_version.optional_field_flags = config.flagCb();
    }
    
    getUniqueID((UniqId_t *)pkt.hardware_version.unique_id);
    Node::sendResponse(pkt, transfer);
  }

  void Node::transmitQueue()
  {
    CANTxFrame tx_frame;
    for (const CanardCANFrame *canard_tx_frame = NULL;
	 (canard_tx_frame = canardPeekTxQueue(&canard)) != NULL;) {
      tx_frame = canard2chibiTx(canard_tx_frame);
      msg_t tx_ok =
        canTransmitTimeout(&config.cand, CAN_ANY_MAILBOX, &tx_frame, chTimeMS2I(100));
      switch (tx_ok) {
      case MSG_OK: {
	canardPopTxQueue(&canard);
	// static uint32_t countOk = 0;
	// errorCb("INFO Transmit OK [%ld]", ++countOk);
	break;
      }
      case MSG_TIMEOUT: {
	canSTM32ResumeOnline(&config.cand);
	setCanStatus(TRANSMIT_TIMOUT);
	errorCb("Transmit MSG_TIMEOUT");
	break;
      }
      case MSG_RESET: {
	setCanStatus(TRANSMIT_RESET);
	canardPopTxQueue(&canard);
	errorCb("Transmit MSG_RESET");
	break;
      }
      default: {
	errorCb("ERROR canTransmit status %ld not handled in switch", tx_ok);
	break;
      }
      }
    }
  }


void Node::sendNodeStatus()
{
  /*
   * Génère et place le statut du noeud dans la queue de transmission canard.
   * (doit être appelée à au moins 1 Hz).
   */
  node_status.uptime_sec =  getTimestampS();
  sendBroadcast(node_status, CANARD_TRANSFER_PRIORITY_LOW,
		config.busNodeType == BUS_FD_BX_MIXED);
}

void Node::setStatus(const uavcan_protocol_NodeStatus& status)
{
  node_status.health = status.health;
  node_status.mode = status.mode;
  node_status.vendor_specific_status_code = status.vendor_specific_status_code;
}


}

/** @} */
