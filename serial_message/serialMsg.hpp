#pragma once
#include <array>
#include <iterator>
#include <stdint.h>
#include <algorithm>
#include <climits>
#include <cstring>
#include "serialMsg_conf.hpp"

/*
  BUGFIX:
  * SystemDependant_chibiosUart : 
    si on veut du high bandwith sans perte de messages dans le sens vers le MCU, il faut :

    * ne pas utiliser le driver UART
    * ne pas utiliser ni Pipe ni RingBuffer
    * utiliser le driver DMA en double buffer
    * configurer l'UART à la main (ou à l'aide du driver SERIAL en venant ecraser la partie ISR et DMA
    de la config après coup)
    * remplir le pipe sous ISR (du coup on doit utiliser les iqueue / iqueue standard de chibios) 
    au lieu du pipe


  TODO:
  * commenter
  * faire un .mk pour chibios
  * reflechir pour des messages à taille variable
  * faire un message qui permette de verifier que les 2 côtés causent la même langue :
    on verifie l'égalité de :
    * nombre de messages enregistés
    * checksum de la longeur de chaque message
  * remplacer la macros Derive_Msg_* par autre chose ?
  * faire un join propre sur le process avant exit, inutile pour MCU, mais 
    utile sous linux.


  OPTIM:
  * eviter la copie à la reception : 
    1/ - runOnRecept(const P& p) : evite la copie
       - virer populate devenu inutile
    2/ creer un class RPayloadMsg sans champ Payload
  * limiter le nombre d'appel systeme en lisant SYNC et LEN à la fois et en resynchronisant 
    dans le buffer ensuite.

 */


/*
#                  _____                                                         
#                 / ____|                                                        
#                | |        ___    _ __ ___    _ __ ___     ___    _ __          
#                | |       / _ \  | '_ ` _ \  | '_ ` _ \   / _ \  | '_ \         
#                | |____  | (_) | | | | | | | | | | | | | | (_) | | | | |        
#                 \_____|  \___/  |_| |_| |_| |_| |_| |_|  \___/  |_| |_|        
*/





// Deduce best type to fit serialMsg_conf.hpp configuration
using MessageId_t = std::conditional<nbMaxMessageIds < 255, uint8_t, uint16_t>::type;
using StartSync_t = std::remove_const<decltype(startSyncValue)>::type;
constexpr uint16_t maxMessageLen = maxPayloadLen +
				  sizeof( MessageId_t) + sizeof(SystemDependant::Checksum_t);
using MessageLen_t = std::conditional< maxMessageLen < 255, uint8_t, uint16_t>::type;
constexpr size_t fullFrameLen = sizeof(StartSync_t) + sizeof(MessageLen_t) + maxMessageLen;

static_assert(std::is_same<std::uint8_t, unsigned char>::value,
              "We require std::uint8_t to be implemented as unsigned char");

template <typename T>
const std::array<uint8_t, sizeof(T)> & ctoBytes(const T& native) {
  const auto nativePtr = &native;
  const auto bytesPtr = reinterpret_cast<const std::array<uint8_t, sizeof(T)> *>(nativePtr);
  return *bytesPtr;
}
   
template <typename T>
 std::array<uint8_t, sizeof(T)> & toBytes( T& native) {
   auto nativePtr = &native;
   auto bytesPtr = reinterpret_cast< std::array<uint8_t, sizeof(T)> *>(nativePtr);
  return *bytesPtr;
}
   

template <typename T>
const T& cfromBytes(const  std::array<uint8_t, maxMessageLen> &bytes) {
  const auto bytesPtr = &bytes;
  const auto nativePtr = reinterpret_cast<const T *>(bytesPtr);
  return *nativePtr;
}


template <typename T, typename E>
typename std::enable_if<!std::is_integral<T>::value>::type pushByte(T& array, const E& elem) {
  std::rotate(array.begin(), array.begin()+1, array.end());
  *(array.end()-1) = elem;
}



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

// template parameter : P : Payload, PM : PayloadMessage

// macro which ensures correct SERIALMSG usage
#define Derive_Msg(Type) class Msg_ ## Type: \
    public PayloadMsg<Type> { \
  public: \
    Msg_ ## Type(const Type &p) : PayloadMsg<Type>(p) {}; \
    Msg_ ## Type() : PayloadMsg<Type>() {}; 

static_assert(std::is_unsigned<MessageId_t>::value,
	      "MessageId_t need to be an unsigned type");
static_assert(sizeof(MessageId_t) <= 4,
	      "sizeof(MessageId_t) need to be <= 4");




class BaseMsg {
public:
  BaseMsg() = default;
  virtual ~BaseMsg() {};
  virtual size_t getPayloadSize(void) const = 0;
  virtual void populatePayload(const std::array<uint8_t, maxMessageLen> &bytes) = 0;
  virtual void runOnRecept(void) const = 0;
};


template<typename P>
class PayloadMsg : public BaseMsg {
public:
  using PType = P;
  PayloadMsg(const P &p) {memcpy(&idPayload.payload, &p, sizeof(P));};
  //PayloadMsg(const P &p) {idPayload.payload = p;};
  PayloadMsg() = default;
  virtual ~PayloadMsg() {};
  virtual void   runOnRecept(void) const {};
  static constexpr size_t PSIZE =  sizeof(MessageId_t)+sizeof(P);
  virtual void populatePayload(const std::array<uint8_t, maxMessageLen> &bytes) final;
  static  void setMsgId(MessageId_t msgId) {PmsgId = msgId;};
  const   std::array<uint8_t, PSIZE>& getPayloadBuffer(void) const {return ctoBytes(idPayload);};
  size_t getPayloadSize(void)  const final {return PSIZE;};
protected:
  static  MessageId_t getMsgId(void) {return PmsgId;};
private:
  static MessageId_t PmsgId;
  struct __attribute__((packed)) {
    const MessageId_t msgId=PmsgId;
    P payload;
  } idPayload;
public:
  const P* const data = &idPayload.payload;
};

using MsgRegistryFn_t = bool (*)(const std::array<uint8_t,
				 maxMessageLen> &rawPayload, const size_t len);

class MsgRegistry {
public:

  template <class PM>
  static bool
  registerFactoryFunction(const MsgRegistryFn_t classFactoryFunction);
  
  static bool
  createAndRunInstance(const MessageId_t msgId,
		       const std::array<uint8_t, maxMessageLen> &rawPayload,
		       const size_t len);
  
private:
  MsgRegistry(void) = default;
  static size_t			messageIdIdx;
  static MsgRegistryFn_t	factoryFnArray[nbMaxMessageIds];
};

template <class PM>
bool msgRegisterCB(const std::array<uint8_t, maxMessageLen> &rawPayload,
				  const size_t len)
{
  if (PM::PSIZE == len) {
    PM msg;
    msg.populatePayload(rawPayload);
    msg.runOnRecept();
    return true;
  } else {
    return false;
  }
}

template<class PM>
void  msgRegister(void)
{
  static_assert(PM::PSIZE <= (maxPayloadLen + sizeof(MessageId_t)),
		"maxPayloadLen to small");
  if (not MsgRegistry::registerFactoryFunction<PM> (msgRegisterCB<PM>)) {
    SystemDependant::abort("registerFactoryFunction failed");
  }
}



template <class PM>
bool
MsgRegistry::registerFactoryFunction(const MsgRegistryFn_t classFactoryFunction)
{
  if ((messageIdIdx < nbMaxMessageIds) && (factoryFnArray[messageIdIdx] == nullptr)) {
    factoryFnArray[messageIdIdx] = classFactoryFunction;
    // here verify that the message length is less than
    // (maxPayloadLen - sizeof(messId_t)
    if (messageIdIdx < (nbMaxMessageIds -1)) {
      PM::setMsgId(messageIdIdx++);
    }
    return true;
  } else {
    return false;
  }
}

template <class P>
MessageId_t PayloadMsg<P>::PmsgId = 0;

template <class P>
void PayloadMsg<P>::populatePayload(const std::array<uint8_t, maxMessageLen> &bytes)
{
  (void) bytes;
  //  idPayload.payload = cfromBytes<P>(bytes);
  memcpy(&idPayload.payload, bytes.data(), sizeof(idPayload.payload));
}

/*
#                  __                                         _          
#                 / _|                                       | |         
#                | |_   _ __    __ _   _ __ ___     ___    __| |         
#                |  _| | '__|  / _` | | '_ ` _ \   / _ \  / _` |         
#                | |   | |    | (_| | | | | | | | |  __/ | (_| |         
#                |_|   |_|     \__,_| |_| |_| |_|  \___|  \__,_|         
#                 _            _   _                        
#                | |          | | | |                       
#                | |    __ _  | |_| |    ___   _ __         
#                | |   / _` |  \__, |   / _ \ | '__|        
#                | |  | (_| |   __/ |  |  __/ | |           
#                |_|   \__,_|  |___/    \___| |_|           
*/





/*
#                  ____    _         _                             _          
#                 / __ \  | |       (_)                           | |         
#                | |  | | | |__     | |         ___    _ __     __| |         
#                | |  | | | '_ \    | |        / __|  | '_ \   / _` |         
#                | |__| | | |_) |  _/ |        \__ \  | | | | | (_| |         
#                 \____/  |_.__/  |__/         |___/  |_| |_|  \__,_|         
*/
template <class PM>
class FrameMsgSendObject {
public:
  FrameMsgSendObject() {};
  void send(const PM& _pm);
  static void send (const typename PM::PType& _m);
private:
  static constexpr size_t frameLen =  sizeof(StartSync_t) +  sizeof(MessageLen_t) +
				      PM::PSIZE + sizeof(SystemDependant::Checksum_t);
  // member method
  void calcCrc();

  // member variable
  
  struct {
    const StartSync_t startSync = startSyncValue;
    struct {
      const MessageLen_t len =  PM::PSIZE;
      std::array<uint8_t, PM::PSIZE> payload;
    } __attribute__((packed)) lenAndPayload;
    typename SystemDependant::Checksum_t crc;
  } __attribute__((packed)) frame;
};


template <class PM>
void FrameMsgSendObject<PM>::send(const PM& pm) {
  const auto& source = pm.getPayloadBuffer();
  std::copy(source.cbegin(), source.cend(),
	    frame.lenAndPayload.payload.begin());
  calcCrc();
  SystemDependant::lock();
  SystemDependant::write(ctoBytes(frame));
  SystemDependant::unlock();
};

template <class PM>
void FrameMsgSendObject<PM>::send(const typename PM::PType& s) {
  PM msg(s);
  FrameMsgSendObject<PM> fms;
  fms.send(msg);
};

template <class PM>
void FrameMsgSendObject<PM>::calcCrc() {
    frame.crc = SystemDependant::checksum(ctoBytes(frame.lenAndPayload));
};

/*
#                  ____    _         _                                     
#                 / __ \  | |       (_)                                    
#                | |  | | | |__     | |         _ __    ___    ___         
#                | |  | | | '_ \    | |        | '__|  / _ \  / __|        
#                | |__| | | |_) |  _/ |        | |    |  __/ | (__         
#                 \____/  |_.__/  |__/         |_|     \___|  \___|        
*/
class FrameMsgReceive {
public:
  // using NewMsgCallBack_t = void (*)(const uint8_t *buffer, const size_t len,
  // 				    void * const userData);
  // using ChkErrCallBack_t = void (*)(const uint32_t recCrc, const uint32_t calcCrc);
  
  static void launchMillFrameThread(void);

private:
  FrameMsgReceive () = delete;

  // member method
  static void millFrame(void);

  [[noreturn]]
  static void millFrameThread(void);
};
