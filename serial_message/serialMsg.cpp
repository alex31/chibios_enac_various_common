#include "serialMsg.hpp"
//#include <iostream>
#include <type_traits>
/*
#                 _ __            _   _    _                        _          
#                | '_ \          | | | |  | |                      | |         
#                | |_) |   __ _  | |_| |  | |    ___     __ _    __| |         
#                | .__/   / _` |  \__, |  | |   / _ \   / _` |  / _` |         
#                | |     | (_| |   __/ |  | |  | (_) | | (_| | | (_| |         
#                |_|      \__,_|  |___/   |_|   \___/   \__,_|  \__,_|         
#                 _            _   _                        
#                | |          | | | |                       
#                | |    __ _  | |_| |    ___   _ __         
#                | |   / _` |  \__, |   / _ \ | '__|        
#                | |  | (_| |   __/ |  |  __/ | |           
#                |_|   \__,_|  |___/    \___| |_|           
*/

size_t			MsgRegistry::messageIdIdx = 0;
MsgRegistryFn_t	MsgRegistry::factoryFnArray[nbMaxMessageIds] = {nullptr};

bool
MsgRegistry::createAndRunInstance(const MessageId_t msgId,
				  const std::array<uint8_t, maxMessageLen> &rawPayload,
				  const size_t len)
{
  if ((msgId < messageIdIdx) && (factoryFnArray[msgId] != nullptr)) {
    return factoryFnArray[msgId](rawPayload, len);
  } 
  return false;
}

void FrameMsgReceive::millFrame(void)
{
  StartSync_t startSync = {0};
  SystemDependant::Checksum_t distantCrc=0;

  struct Frame {
    MessageLen_t len;
    struct __attribute__((packed)) {
      MessageId_t msgId;
      std::array<uint8_t, maxMessageLen> rawPayload;
    } idPayload;
  } __attribute__((packed)) frame;

  // wait for sync
  do {
    std::array<uint8_t, 1> newByte;
    SystemDependant::read(newByte);
    pushByte(startSync, newByte[0]);
  } while (startSync != startSyncValue);
  // std::cout << "SYNC IS DONE\n";
  
  // get payload len
  
  SystemDependant::read(toBytes(frame.len));
  // std::cout << "GET LEN = " << frame.len+0 << std::endl;


  // get payload and crc
  // not very readable, but read payload and checksum in one system call instead of two
  // because system call are usually costly
  auto& idPayloadBytes = toBytes(frame.idPayload);
  SystemDependant::read(idPayloadBytes, frame.len + sizeof(SystemDependant::Checksum_t));
  std::copy(idPayloadBytes.begin() + frame.len,
	    idPayloadBytes.begin() + frame.len + sizeof(SystemDependant::Checksum_t),
	    toBytes(distantCrc).begin());
    

  const SystemDependant::Checksum_t localCrc =
    SystemDependant::checksum(ctoBytes(frame), frame.len + sizeof(MessageLen_t));
  if (localCrc == distantCrc) {
    // std::cout << "CRC OK\n";
  } else {
    // std::cout << "CRC DIFFER "  << std::hex << "0x" << localCrc << " <> " <<
    //      "0x" << distantCrc << std::endl;
    return;
  }

  if (not MsgRegistry::createAndRunInstance(frame.idPayload.msgId,
					    frame.idPayload.rawPayload, frame.len)) {
    // std::cout << "POPULATE ERROR\n";
  }
  
}


void FrameMsgReceive::launchMillFrameThread(void)
{
  SystemDependant::launch(&millFrameThread);
}

[[noreturn]]
void FrameMsgReceive::millFrameThread(void)
{
  while (true) {
    millFrame();
  };
}
