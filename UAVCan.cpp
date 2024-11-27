#include "UAVCan.hpp"
#include <cstring>

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

    /*
      if application has not bind callback to any of these 3 message id,
      library should create entry in the map for internel message management
      if  application has bind callback, the entry is already there and management
      will be done
     */
    if (not config.idToHandleRequest.contains(UAVCAN_PROTOCOL_GETNODEINFO_ID)) {
      config.idToHandleRequest[UAVCAN_PROTOCOL_GETNODEINFO_ID] = {
	UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE,
	UAVCAN::Node::requestMessageCb<nullAppCb<uavcan_protocol_GetNodeInfoRequest>>
      };
    }

    if (not config.idToHandleResponse.contains(UAVCAN_PROTOCOL_GETNODEINFO_ID)) {
      config.idToHandleResponse[UAVCAN_PROTOCOL_GETNODEINFO_ID] = {
	UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE,
	UAVCAN::Node::responseMessageCb<nullAppCb<uavcan_protocol_GetNodeInfoResponse>>
      };
    }

    if (not config.idToHandleBroadcast.contains(UAVCAN_PROTOCOL_NODESTATUS_ID)) {
      config.idToHandleBroadcast[UAVCAN_PROTOCOL_NODESTATUS_ID] = {
	UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
	UAVCAN::Node::sendBroadcastCb<nullAppCb<uavcan_protocol_NodeStatus>>
      };
    }
  }

  int8_t Node::configureHardwareFilters()
  {
    int8_t filterIndex = 0;
    constexpr uint8_t filtersSize = 
#ifdef FDCAN_CONFIG_RXGFC_RRFS // ugly meanway to know if we use HDCANV1 ou V2
      8; // FDCANV1 : each FDCan interface has 8 extended filters
#else // FDCANV2 : 32 extended filters are shared between FDCan interface(s) in use
#    if ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 1)
    32;
#    elif ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 2)
    16;
#    elif ((STM32_CAN_USE_FDCAN1 + STM32_CAN_USE_FDCAN2 + STM32_CAN_USE_FDCAN3) == 2)
    10;
#    endif
#endif

    CANFilter  filterList[filtersSize];
    
    for (const auto& [msg_id, _] :  config.idToHandleRequest) {
      chDbgAssert(msg_id < 256, "service message id should be in the range 0 .. 255");
      constexpr uint32_t mask = 0x00'ff'80'00;
      const uint32_t id = (msg_id << 16) | (0x80 << 8);
      filterList[filterIndex] = {
	.filter_type = CAN_FILTER_TYPE_EXT,
	.filter_mode = CAN_FILTER_MODE_CLASSIC,
	.filter_cfg =  CAN_FILTER_CFG_FIFO_0,
	.identifier1 = id,
	.identifier2 = mask
      };
      if (++filterIndex > filtersSize)
	goto overfill;
    }
    
   for (const auto& [msg_id, _] :  config.idToHandleResponse) {
      chDbgAssert(msg_id < 256, "service message id should be in the range 0 .. 255");
      constexpr uint32_t mask = 0x00'ff'80'00;
      const uint32_t id = (msg_id << 16) | (0x00 << 8);
      filterList[filterIndex] = {
	.filter_type = CAN_FILTER_TYPE_EXT,
	.filter_mode = CAN_FILTER_MODE_CLASSIC,
	.filter_cfg =  CAN_FILTER_CFG_FIFO_0,
	.identifier1 = id,
	.identifier2 = mask
      };
      if (++filterIndex > filtersSize)
	goto overfill;
    }

   for (const auto& [msg_id, _] :  config.idToHandleBroadcast) {
      constexpr uint32_t mask = 0x00'ff'ff'00;
      const uint32_t id = msg_id << 8;
      filterList[filterIndex] = {
	.filter_type = CAN_FILTER_TYPE_EXT,
	.filter_mode = CAN_FILTER_MODE_CLASSIC,
	.filter_cfg =  CAN_FILTER_CFG_FIFO_1,
	.identifier1 = id,
	.identifier2 = mask
      };
      if (++filterIndex > filtersSize)
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
    canardInit(&canard, memory_pool, MEMORYPOOL_SIZE,
	       &onTransferReceivedDispatch,
	       &shouldAcceptTransferDispatch, this);
    canardSetLocalNodeID(&canard, config.nodeId);
    initNodesList();
    canStart(&config.cand, &config.cancfg);
    int8_t filtersInUse = configureHardwareFilters();
    //    int8_t filtersInUse = 0;
    if (filtersInUse < 0) {
      StrCbHelper m("WARN: too many messages fo hardware filtering, "
		    "revert to software filtering");
      if (config.errorCb) config.errorCb(m.view());
    } else {
      const bool rejectNonAcceptedId =
#ifdef FDCAN_CONFIG_RXGFC_ANFE_REJECT
	(config.cancfg.RXGFC & FDCAN_CONFIG_RXGFC_ANFE_REJECT) != 0;
#else
      (config.cancfg.RXGFC & FDCAN_CONFIG_GFC_ANFE_REJECT) != 0;
#endif
      chDbgAssert(rejectNonAcceptedId,
		  "cancfg.RXGFC must be corrected to reject filtered id");
      StrCbHelper m("INFO: hardware filtering use %d slots", filtersInUse);
      if (config.errorCb) config.errorCb(m.view());
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
    chEvtRegister(&node->config.cand.error_event, &el, 0);

    while(not chThdShouldTerminateX()) {
      if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_INFINITE) != 0) {
	if (node->config.cand.fdcan->CCCR & FDCAN_CCCR_INIT) {
	  node->config.cand.fdcan->CCCR &= ~FDCAN_CCCR_INIT;
	  node->setCanStatus(NODE_OFFLINE);
	  StrCbHelper m("canErrorThdDispatch bus_off condition");
	  if (node->config.errorCb) node->config.errorCb(m.view());
	  chThdSleepMilliseconds(50);
	}
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
    
    switch (transfer_type) {
     case (CanardTransferTypeRequest): {
      if (auto sigItr = config.idToHandleRequest.find(data_type_id);
	  sigItr !=  config.idToHandleRequest.end()) {
	*out_data_type_signature = sigItr->second.signature;
	return true;
      }
      break;
     } 
       
    case (CanardTransferTypeResponse): {
      if (auto sigItr = config.idToHandleResponse.find(data_type_id);
	  sigItr !=  config.idToHandleResponse.end()) {
	*out_data_type_signature = sigItr->second.signature;
	return true;
      }
      break;
    } 
      
    case (CanardTransferTypeBroadcast): {
      if (auto sigItr = config.idToHandleBroadcast.find(data_type_id);
	  sigItr !=  config.idToHandleBroadcast.end()) {
	*out_data_type_signature = sigItr->second.signature;
	return true;
      }
      break;
    } 
      
    default:
      chSysHalt("data_type_id not handled");
    }

    StrCbHelper m("INFO: id %x not hardware filtered", data_type_id);
    if (config.errorCb) config.errorCb(m.view());

    return false;
  }
  
  void Node::onTransferReceived(CanardInstance *ins,
			  CanardRxTransfer *transfer)
  {
    (void) ins;
    switch (transfer->transfer_type) {
    case (CanardTransferTypeRequest): {
      if (auto sigItr = config.idToHandleRequest.find(transfer->data_type_id);
	  sigItr !=  config.idToHandleRequest.end()) {
	sigItr->second.cb(ins, transfer);
      } else {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->setCanStatus(REQUEST_UNHANDLED_ID);
	StrCbHelper m("INFO onTransferReceived Request id %u not handled",
		      transfer->data_type_id);
	if (node->config.errorCb) node->config.errorCb(m.view());
      }
      break;
    } 
      
    case (CanardTransferTypeResponse): {
      if (auto sigItr = config.idToHandleResponse.find(transfer->data_type_id);
	  sigItr !=  config.idToHandleResponse.end()) {
	sigItr->second.cb(ins, transfer);
      } else {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->setCanStatus(RESPONSE_UNHANDLED_ID);
	StrCbHelper m("INFO onTransferReceived Response id %u not handled",
		      transfer->data_type_id);
	if (node->config.errorCb) node->config.errorCb(m.view());
      }
      break;
    }
      
    case (CanardTransferTypeBroadcast): {
      if (auto sigItr = config.idToHandleBroadcast.find(transfer->data_type_id);
	  sigItr !=  config.idToHandleBroadcast.end()) {
	sigItr->second.cb(ins, transfer);
      } else {
	Node *node = static_cast<Node *>(canardGetUserReference(ins));
	node->setCanStatus(BROADCAST_UNHANDLED_ID);
	StrCbHelper m("INFO onTransferReceived Broadcast id %u not handled",
		      transfer->data_type_id);
	if (node->config.errorCb) node->config.errorCb(m.view());
      }
      
      break;
    }  
      
    default:
      chSysHalt("data_type_id not handled");
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
      // StrCbHelper m("INFO Receive Msg from 0x%x, length %u",
      //  		    uint32_t(rxmsg.ext.EID), uint32_t(rxmsg.DLC));
      // if (config.errorCb) config.errorCb(m.view());
      
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
	  StrCbHelper m("Warning : current time wraparound or "
			"message recieved in the future (unlikely)");
	  
	  if (config.errorCb) config.errorCb(m.view());
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
	// StrCbHelper m("INFO Transmit OK [%ld]", ++countOk);
	// if (config.errorCb) config.errorCb(m.view());
	break;
      }
      case MSG_TIMEOUT: {
	setCanStatus(TRANSMIT_TIMOUT);
	StrCbHelper m("Transmit MSG_TIMEOUT");
	if (config.errorCb) config.errorCb(m.view());
	break;
      }
      case MSG_RESET: {
	setCanStatus(TRANSMIT_RESET);
	canardPopTxQueue(&canard);
	StrCbHelper m("Transmit MSG_RESET");
	if (config.errorCb) config.errorCb(m.view());
	break;
      }
      default: {
	StrCbHelper m("ERROR canTransmit status %ld not handled in switch", tx_ok);
	if (config.errorCb) config.errorCb(m.view());
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

void Node::setStatus(uint8_t health, uint8_t mode,  specificStatusCode_t specific_code)
{
  node_status.health = health;
  node_status.mode = mode;
  node_status.vendor_specific_status_code = specific_code;
}


}
