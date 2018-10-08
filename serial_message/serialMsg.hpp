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
  * remplacer la macros Derive_Msg_* par autre chose ?
  * faire un join propre sur le process avant exit, inutile pour MCU, mais 
    utile sous linux.


  OPTIM:
  * eviter une copie à l'envoi : la classe PayloadMsg devrait avoir une reference (ou un pointeur)
       sur P au lieu d'un P

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

#define Derive_DynMsg(Type) class Msg_ ## Type:	\
    public PayloadDynMsg<Type> { \
  public: \
    Msg_ ## Type(const Type &p) : PayloadDynMsg<Type>(p) {}; \
    Msg_ ## Type() : PayloadDynMsg<Type>() {}; 

static_assert(std::is_unsigned<MessageId_t>::value,
	      "MessageId_t need to be an unsigned type");
static_assert(sizeof(MessageId_t) <= 4,
	      "sizeof(MessageId_t) need to be <= 4");




class BaseMsg {
public:
  BaseMsg() = default;
  virtual ~BaseMsg() {};
  virtual size_t getPayloadSize(void) const = 0;
  virtual void populatePayload(const std::array<uint8_t, maxMessageLen> &bytes,
			       size_t len = 0) = 0;
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
  virtual void   runOnRecept(void) const override {};
  virtual void populatePayload(const std::array<uint8_t, maxMessageLen> &bytes, size_t len = 0) final;
  static  void setMsgId(MessageId_t msgId) {PmsgId = msgId;};
  static constexpr size_t PSIZE =  sizeof(MessageId_t)+sizeof(P);
  const   std::array<uint8_t, PSIZE>& getPayloadBuffer(void) const {return ctoBytes(idPayload);};
  virtual size_t getPayloadSize(void)  const  override {return PSIZE;};
  static  MessageId_t getMsgId(void) {return PmsgId;};
protected:
  struct __attribute__((packed)) {
    const MessageId_t msgId=PmsgId;
    P payload;
  } idPayload;
private:
  static MessageId_t PmsgId;
public:
  const P* const data = &idPayload.payload;
};



template<typename P>
class PayloadDynMsg : public PayloadMsg<P> {
public:
  PayloadDynMsg(const P &p) : PayloadMsg<P>(p) {};
  PayloadDynMsg() = default;
  virtual ~PayloadDynMsg() {};
  virtual void   runOnRecept(void) const {};
  static constexpr size_t PSIZEMAX =  PayloadMsg<P>::PSIZE;
  virtual size_t getPayloadSize(void)  const final;
};





using MsgRegistryFn_t = bool (*)(const std::array<uint8_t,
				 maxMessageLen> &rawPayload, const size_t len);

struct __attribute__((packed)) ProtocolVersion {
  uint32_t nbMessages;
  uint32_t sumOfBytes;
  uint16_t checksum;
}; 
class MsgRegistry {
public:

  template <class PM>
  static bool
  registerFactoryFunction(const MsgRegistryFn_t classFactoryFunction);
  
  static bool
  createAndRunInstance(const MessageId_t msgId,
		       const std::array<uint8_t, maxMessageLen> &rawPayload,
		       const size_t len);
  
  static ProtocolVersion pv;

private:
  MsgRegistry(void) = default;
  static size_t			messageIdIdx;
  static MsgRegistryFn_t	factoryFnArray[nbMaxMessageIds];
};

template <class PM>
bool msgRegisterCB(const std::array<uint8_t, maxMessageLen> &rawPayload,
				  const size_t len)
{
  if (len <= PM::PSIZE) {
    PM msg;
    msg.populatePayload(rawPayload, len);
    if ((len == PM::PSIZE) or (len == msg.getPayloadSize())) {
      msg.runOnRecept();
      return true;
    } 
  }
  return false;
}

template<class PM>
void  msgRegister(void)
{
  const std::array<uint8_t, 4> crcBuf = {PM::getMsgId(), PM::PSIZE,
					 static_cast<uint8_t>(MsgRegistry::pv.checksum & 0xff),
					 static_cast<uint8_t>((MsgRegistry::pv.checksum & 0xff00) >> 8)};
  static_assert(PM::PSIZE <= (maxPayloadLen + sizeof(MessageId_t)),
		"maxPayloadLen to small");
  if (not MsgRegistry::registerFactoryFunction<PM> (msgRegisterCB<PM>)) {
    SystemDependant::abort("registerFactoryFunction failed");
  }
  MsgRegistry::pv.nbMessages = PM::getMsgId();
  MsgRegistry::pv.sumOfBytes += PM::PSIZE;
  MsgRegistry::pv.checksum = fletcher16(crcBuf);
}



template <class PM>
bool
MsgRegistry::registerFactoryFunction(const MsgRegistryFn_t classFactoryFunction)
{
  if ((messageIdIdx < nbMaxMessageIds) && (factoryFnArray[messageIdIdx] == nullptr)) {
    factoryFnArray[messageIdIdx] = classFactoryFunction;
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
void PayloadMsg<P>::populatePayload(const std::array<uint8_t, maxMessageLen> &bytes, size_t len)
{
  len = (len == 0) ?  sizeof(idPayload.payload) : std::min(len, sizeof(idPayload.payload));
  memcpy(&idPayload.payload, bytes.data(), len);
}

template <class P>
size_t PayloadDynMsg<P>::getPayloadSize(void)  const 
{
  const auto& payload = PayloadMsg<P>::idPayload.payload;
  const size_t notUsedElem = payload.ASize - payload.dynSize;
  const size_t notUsedBytes = notUsedElem * sizeof(typename P::AType);
  return PSIZEMAX - notUsedBytes;
};



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
  //  static constexpr size_t frameLen =  sizeof(StartSync_t) +  sizeof(MessageLen_t) +
  //				      PM::PSIZE + sizeof(SystemDependant::Checksum_t);
  // member method
  void calcCrc();

  // member variable
  
  struct {
    const StartSync_t startSync = startSyncValue;
    struct {
      MessageLen_t len =  0;
      std::array<uint8_t, PM::PSIZE+sizeof(SystemDependant::Checksum_t)> payloadAndCrc;
    } __attribute__((packed)) lenAndPayloadAndCrc;
  } __attribute__((packed)) frame;
};


template <class PM>
void FrameMsgSendObject<PM>::send(const PM& pm) {
  frame.lenAndPayloadAndCrc.len = pm.getPayloadSize();
  const auto& source = pm.getPayloadBuffer();
  std::copy(source.cbegin(), source.cbegin()+frame.lenAndPayloadAndCrc.len,
	    frame.lenAndPayloadAndCrc.payloadAndCrc.begin());
  calcCrc();
  SystemDependant::lock();
  SystemDependant::write(ctoBytes(frame), sizeof(StartSync_t) + sizeof(MessageLen_t) +
			 frame.lenAndPayloadAndCrc.len + sizeof(SystemDependant::Checksum_t));
  // printf ("DBG, sumSize=%d, frameSize=%d",
  // 	  frame.lenAndPayloadAndCrc.len +
  // 	  sizeof(StartSync_t) + sizeof(MessageLen_t) + sizeof(SystemDependant::Checksum_t),
  // 	  sizeof(frame));
  SystemDependant::unlock();
};

template <class PM>
void FrameMsgSendObject<PM>::send(const typename PM::PType& s) {
  PM msg(s);
  FrameMsgSendObject<PM> fms;
  fms.send(msg);
};

// constexpr OutputIt copy( InputIt first, InputIt last, OutputIt d_first );
template <class PM>
void FrameMsgSendObject<PM>::calcCrc() {
  typename SystemDependant::Checksum_t crc = SystemDependant::checksum(ctoBytes(frame.lenAndPayloadAndCrc),
								       frame.lenAndPayloadAndCrc.len +
								       sizeof(MessageLen_t));
  
  //  printf("send crc [%d bytes] = 0x%x\n", frame.lenAndPayloadAndCrc.len + sizeof(MessageLen_t), crc);
  const auto from_b = toBytes(crc).cbegin();
  const auto from_e = from_b + sizeof(SystemDependant::Checksum_t);
  const auto to_b = frame.lenAndPayloadAndCrc.payloadAndCrc.begin() + frame.lenAndPayloadAndCrc.len;
  std::copy(from_b, from_e, to_b);
};

/*
#                  ____    _         _                                     
#                 / __ \  | |       (_)                                    
#                | |  | | | |__     | |         _ __    ___    ___         
#                | |  | | | '_ \    | |        | '__|  / _ \  / __|        
#                | |__| | | |_) |  _/ |        | |    |  __/ | (__         
#                 \____/  |_.__/  |__/         |_|     \___|  \___|        
*/
Derive_Msg(ProtocolVersion) 
  void  runOnRecept(void) const final {
  if (data->nbMessages != MsgRegistry::pv.nbMessages) {
    // send back a message to make the other side fail on the same field
    MsgRegistry::pv.nbMessages = 0;
    FrameMsgSendObject<Msg_ProtocolVersion>::send(MsgRegistry::pv);
    SystemDependant::abort("protocol error on nbMessages");
  }
  if (data->sumOfBytes != MsgRegistry::pv.sumOfBytes) {
    // send back a message to make the other side fail on the same field
    MsgRegistry::pv.sumOfBytes = 0;
    FrameMsgSendObject<Msg_ProtocolVersion>::send(MsgRegistry::pv);
    SystemDependant::abort("protocol error on sumOfBytes");
  }
  if (data->checksum != MsgRegistry::pv.checksum) {
    // send back a message to make the other side fail on the same field
    MsgRegistry::pv.checksum++;
    FrameMsgSendObject<Msg_ProtocolVersion>::send(MsgRegistry::pv);
    SystemDependant::abort("protocol error on checksum");
  }
}
};


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
  static void checkProtocolVersion(void);

  [[noreturn]]
  static void millFrameThread(void);
};
