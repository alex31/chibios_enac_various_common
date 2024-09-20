#include "UAVCan.hpp"

#include <ch.h>
#include <hal.h>
#include "stdutil++.hpp"
#include <cstring>

namespace UAVCAN
{
  template<typename MSG_T>
  void nullAppCb(CanardRxTransfer *t,
		 const MSG_T&)
  {
    DebugTrace("DBG default nullAppCb id = %u", t->data_type_id);
  }
  
  
  Node::Node(const Config &_config) : config(_config),
				      node_status{
			.uptime_sec = 0,
			.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
			.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION,
			.sub_mode = 0,
			.vendor_specific_status_code = SPECIFIC_OK}
    
  {
    chMtxObjectInit(&canard_mtx);
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
	UAVCAN::Node::broadcastMessageCb<nullAppCb<uavcan_protocol_NodeStatus>>
      };
    }
  }

  void Node::start()
  {
    canStart(&config.cand, &config.cancfg);
    canardInit(&canard, memory_pool, MEMORYPOOL_SIZE,
	       &onTransferReceivedDispatch,
	       &shouldAcceptTransferDispatch, this);
    canardSetLocalNodeID(&canard, config.nodeId);
    initNodesList();
    sender_thd = chThdCreateFromHeap(nullptr, 1024U, "sender_thd",
				      NORMALPRIO,
				      &senderThdDispatch, this);
    receiver_thd = chThdCreateFromHeap(nullptr, 2048U, "receiver_thd",
				      NORMALPRIO,
				      &receiverThdDispatch, this);
    heartbeat_thd = chThdCreateFromHeap(nullptr, 1024U, "heartbeat_thd",
				       NORMALPRIO,
				       &heartbeatThdDispatch, this);
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
	DebugTrace(
		   "onTransferReceived Request id %u not handled",
		   transfer->data_type_id);
      }
      break;
    } 
      
    case (CanardTransferTypeResponse): {
      if (auto sigItr = config.idToHandleResponse.find(transfer->data_type_id);
	  sigItr !=  config.idToHandleResponse.end()) {
	sigItr->second.cb(ins, transfer);
      } else {
	DebugTrace(
		   "onTransferReceived Response id %u not handled",
		   transfer->data_type_id);
      }
      break;
    }
      
    case (CanardTransferTypeBroadcast): {
      if (auto sigItr = config.idToHandleBroadcast.find(transfer->data_type_id);
	  sigItr !=  config.idToHandleBroadcast.end()) {
	sigItr->second.cb(ins, transfer);
      } else {
	DebugTrace(
		   "onTransferReceived Broadcast id %u not handled",
		   transfer->data_type_id);
      }
      
      break;
    }  
      
    default:
      chSysHalt("data_type_id not handled");
    }
  }
  
  void Node::senderStep()
  {
    chMtxLock(&canard_mtx);
    transmitQueue();
    chMtxUnlock(&canard_mtx);
  }
  
  void Node::receiverStep()
  {
    CANRxFrame rxmsg;
    while (canReceiveTimeout(&config.cand, CAN_ANY_MAILBOX, &rxmsg,
                             TIME_INFINITE) == MSG_OK) {
      // Traitement de la trame.
      DebugTrace("\033[3J\033[1H Receive Msg from 0x%x, length %u",
                 rxmsg.ext.EID, rxmsg.DLC);
      CanardCANFrame rx_frame_canard = chibiRx2canard(rxmsg);
      chMtxLock(&canard_mtx);
      /*
       * On reconstruit un message unique pour les transferts de plusieurs
       * trames et on les passe à travers shouldAcceptTransfer() puis
       * onTransferRecieved().
       */
      canardHandleRxFrame(&canard, &rx_frame_canard,
                          (uint64_t)chTimeI2US(chVTGetSystemTime()));
      chMtxUnlock(&canard_mtx);
    }
  }
  
  void Node::heartbeatStep()
  {
    // nettoyage de NodesList.
    const auto current_time = chTimeI2MS(chVTGetSystemTime());
    const auto current_time_us = chTimeI2US(current_time);
    const auto current_time_ms = chTimeI2MS(current_time);
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
	  DebugTrace("Warning : current time wraparound or "
		     "message recieved in the future (unlikely)")
	    }
      }
    }

    chMtxLock(&canard_mtx);
    // nettoyage recommandé par la librairie.
    canardCleanupStaleTransfers(&canard, current_time_us);
    // Génération de NodeStatus.
    sendNodeStatus();
    chMtxUnlock(&canard_mtx);

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
    sendRequest(req, CANARD_TRANSFER_PRIORITY_LOW, dest_node_id);
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
    node_status.uptime_sec = (uint32_t)chTimeI2S(chVTGetSystemTime());
    pkt.status = node_status;
    if (config.flagCb != nullptr) {
      pkt.software_version.optional_field_flags = config.flagCb();
    }
    
    getUniqueID((UniqId_t *)pkt.hardware_version.unique_id);
    Node::sendResponse(pkt, transfer);
  }

  void Node::transmitQueue()
  {
    static uint32_t countOk = 0;
    CANTxFrame tx_frame;
    for (const CanardCANFrame *canard_tx_frame = NULL;
	 (canard_tx_frame = canardPeekTxQueue(&canard)) != NULL;) {
      tx_frame = canard2chibiTx(canard_tx_frame);
      msg_t tx_ok =
        canTransmitTimeout(&config.cand, CAN_ANY_MAILBOX, &tx_frame, chTimeMS2I(100));
      switch (tx_ok) {
      case MSG_OK: {
	canardPopTxQueue(&canard);
	DebugTrace("MSG_OK [%ld]", ++countOk);
	break;
      }
      case MSG_TIMEOUT: {
	DebugTrace("MSG_TIMEOUT");
	break;
      }
      case MSG_RESET: {
	canardPopTxQueue(&canard);
	DebugTrace("MSG_RESET");
	break;
      }
      default: {
	DebugTrace("status %ld not handled in switch", tx_ok);
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
  node_status.uptime_sec = chTimeI2S(chVTGetSystemTime());
  broadcastMessage(node_status, CANARD_TRANSFER_PRIORITY_LOW);
}

void Node::setStatus(uint8_t health, uint8_t mode,  specificStatusCode specific_code)
{
  node_status.health = health;
  node_status.mode = mode;
  node_status.vendor_specific_status_code = specific_code;
}

  
}