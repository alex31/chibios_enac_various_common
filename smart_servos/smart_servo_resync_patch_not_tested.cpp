#include "smart_servo.h"
#include <ch.h>
#include <hal.h>
#include "string.h"


namespace {
  uint8_t compute_chk(servo_msg_t* msg);
  
  void set_chk(servo_msg_t* msg, uint8_t chk) {
    *((uint8_t*)msg + msg->len + 3) = chk;
  }
  
  void send_msg(SmartServo::SmartServoSio* sio, servo_msg_t* msg) {
    size_t len = msg->len + 4U;
    (void)sio->writeTimeout(reinterpret_cast<uint8_t *>(msg), len, TIME_INFINITE);
  }
  
  class ScopedPriorityElevator {
  public:
    explicit ScopedPriorityElevator(tprio_t new_prio) {
      old_priority_ = chThdGetPriorityX();
      chThdSetPriority(new_prio);
    }
    
    ~ScopedPriorityElevator() {
      chThdSetPriority(old_priority_);
    }
    
    // Non-copyable and non-movable
    ScopedPriorityElevator(const ScopedPriorityElevator&) = delete;
    ScopedPriorityElevator& operator=(const ScopedPriorityElevator&) = delete;
    
  private:
    tprio_t old_priority_;
  };

}

SmartServo::Status SmartServo::readStatus(size_t paramlen)
{
  const size_t payload_len = paramlen;
  const size_t frame_len = payload_len + 6U;
  const sysinterval_t timeout = TIME_US2I(600 + (frame_len * 40U));
  const systime_t start = chVTGetSystemTimeX();

  auto timeLeft = [&]() -> sysinterval_t {
    const systime_t now = chVTGetSystemTimeX();
    const sysinterval_t elapsed = chTimeDiffX(start, now);
    return (elapsed >= timeout) ? TIME_IMMEDIATE : static_cast<sysinterval_t>(timeout - elapsed);
  };

  // Resynchronize to packet start (0xFF 0xFF) to tolerate stale/misaligned bytes.
  uint8_t b = 0;
  bool seenFF = false;
  while (true) {
    const sysinterval_t left = timeLeft();
    if (left == TIME_IMMEDIATE) {
      return Status::STATUS_TIMEOUT;
    }
    if (sio->readTimeout(&b, 1U, left) != 1U) {
      return Status::STATUS_TIMEOUT;
    }
    if (!seenFF) {
      seenFF = (b == 0xFFU);
      continue;
    }
    if (b == 0xFFU) {
      break;
    }
    seenFF = false;
  }

  uint8_t *raw = reinterpret_cast<uint8_t *>(&servo_status);
  raw[0] = 0xFFU;
  raw[1] = 0xFFU;
  const size_t remaining = frame_len - 2U;
  if (sio->readTimeout(&raw[2], remaining, timeLeft()) != remaining) {
    return Status::STATUS_TIMEOUT;
  }

  if (servo_status.STX != 0xFFFFU) {
    DebugTrace("uartRec STX = 0x%x instead of 0xFFFF", servo_status.STX);
    return Status::STATUS_TIMEOUT;
  }
  if (servo_status.len != (payload_len + 2U)) {
    DebugTrace("SmartServo::readStatus invalid len=%u expected=%u",
	       servo_status.len, static_cast<unsigned>(payload_len + 2U));
    return Status::STATUS_TIMEOUT;
  }
  
  if(compute_chk(&servo_status) != servo_status.params_and_crc[servo_status.len - 2U]) {
    DebugTrace("SmartServo::readStatus CRC ERROR");
    return Status::CHECKSUM_ERROR;
  }

  if (servo_status.error != 0U) {
    return static_cast<Status>(servo_status.error);
  }

  return Status::OK;
}


void SmartServo::init()
{
  if (sio_cfg != nullptr) {
    sio->setConfig(*sio_cfg);
  }
  (void)sio->start();
}

uint32_t SmartServo::getSerialBaudrate()
{
  return sio_cfg ? sio_cfg->baud : 0U;
}

void SmartServo::setSerialBaudrate(uint32_t baudrate)
{
  if (sio_cfg == nullptr) {
    return;
  }
  sio->stop();
  sio_cfg->baud = baudrate;
  sio->setConfig(*sio_cfg);
  (void)sio->start();
}
/*
PING	0
READ	2
WRITE	1 + N (dépend de ce que tu écris)
REG_WRITE	idem que WRITE
ACTION	0
RESET	0 ou quelques options selon modèle
 */
SmartServo::Status SmartServo::ping(uint8_t id)
{
  servo_msg.STX = 0xFFFF;
  servo_msg.id = id;
  servo_msg.len = 2;
  servo_msg.instruction = Instruction::SMART_SERVO_PING;
  set_chk(&servo_msg, compute_chk(&servo_msg));

  ScopedPriorityElevator guard(HIGHPRIO); // avoid context change
  send_msg(sio, &servo_msg);
  if (const Status st = readStatus(0U); st != Status::OK) {
    return st;
  }

  if (id == BROADCAST_ID) {
    // A valid broadcast response must come from a real servo ID, not 254.
    return servo_status.id == BROADCAST_ID ? Status::STATUS_TIMEOUT : Status::OK;
  }

  if (servo_status.id != id) {
    DebugTrace("SmartServo::ping id mismatch tx=%u rx=%u", id, servo_status.id);
    return Status::STATUS_TIMEOUT;
  }

  return Status::OK;
}

SmartServo::Status SmartServo::read(record_t *record) {
  servo_msg.STX = 0xFFFF;
  servo_msg.id = record->id;
  servo_msg.len = 2U + 2U;
  servo_msg.instruction = Instruction::SMART_SERVO_READ;
  servo_msg.params_and_crc[0] = record->reg;
  servo_msg.params_and_crc[1] = record->len;
  set_chk(&servo_msg, compute_chk(&servo_msg));

  ScopedPriorityElevator guard(HIGHPRIO); // avoid context change
  send_msg(sio, &servo_msg);
  SmartServo::Status status = readStatus(record->len);
  memcpy(&record->data, servo_status.params_and_crc, record->len);

  return status;
}

SmartServo::Status SmartServo::write(record_t *record, bool is_reg_write) {
  servo_msg.STX = 0xFFFF;
  servo_msg.id = record->id;
  servo_msg.len = record->len + 3U;
  if(is_reg_write) {
    servo_msg.instruction = Instruction::SMART_SERVO_REG_WRITE;
  } else {
    servo_msg.instruction = Instruction::SMART_SERVO_WRITE;
  }
  servo_msg.params_and_crc[0] = record->reg;
  memcpy(&servo_msg.params_and_crc[1], record->data, record->len);
  set_chk(&servo_msg, compute_chk(&servo_msg));

  ScopedPriorityElevator guard(HIGHPRIO); // avoid context change
  send_msg(sio, &servo_msg);

  return record->id != BROADCAST_ID && response_level == RL_NORMAL ? readStatus(0): Status::OK;
}

SmartServo::Status SmartServo::action(uint8_t id) {
  servo_msg.STX = 0xFFFF;
  servo_msg.id = id;
  servo_msg.len = 2;
  servo_msg.instruction = Instruction::SMART_SERVO_ACTION;
  set_chk(&servo_msg, compute_chk(&servo_msg));

  ScopedPriorityElevator guard(HIGHPRIO); // avoid context change
  send_msg(sio, &servo_msg);
  
  return id != BROADCAST_ID && response_level == RL_NORMAL ? readStatus(0): Status::OK;
}

SmartServo::Status SmartServo::reset(uint8_t id) {
  if(id == BROADCAST_ID) {
    // Broadcast ID cannot be use for reset.
    return Status::INVALID_PARAMS;
  }

  servo_msg.STX = 0xFFFF;
  servo_msg.id = id;
  servo_msg.len = 2;
  servo_msg.instruction = Instruction::SMART_SERVO_RESET;
  set_chk(&servo_msg, compute_chk(&servo_msg));

  ScopedPriorityElevator guard(HIGHPRIO); // avoid context change
  send_msg(sio, &servo_msg);

  return response_level == RL_NORMAL ? readStatus(0): Status::OK;
}



SmartServo::Status SmartServo::detectBaudrateBroadcast(etl::span<const uint32_t> baudrates)
{
  for(auto baud: baudrates) {
    setSerialBaudrate(baud);
    if(ping(BROADCAST_ID) == SmartServo::OK) {
      return SmartServo::OK;
    }
  }
  return Status::STATUS_TIMEOUT;
}

SmartServo::Status SmartServo::detectBaudrateUnicast(etl::span<const uint32_t> baudrates)
{
  uint32_t firstMatchingBaudrate = 0;
  for(auto baud: baudrates) {
    setSerialBaudrate(baud);
    for(uint8_t id = 1U; id < BROADCAST_ID; id++) {
      if(ping(id) == SmartServo::OK) {
	if (firstMatchingBaudrate == 0) {
	  firstMatchingBaudrate = baud;
	  break; // exit inner examine id loop
	} else {
	  return Status::HETEROGENEOUS_BAUDRATES;
	}
      }
    }
  }
  return firstMatchingBaudrate == 0 ? Status::STATUS_TIMEOUT : SmartServo::OK;
}

SmartServo::Status SmartServo::detectBaudrate(etl::span<const uint32_t> baudrates)
{
  if (detectBaudrateBroadcast(baudrates) == SmartServo::OK) {
    return SmartServo::OK;
  }
  return detectBaudrateUnicast(baudrates);
}


SmartServo::Status SmartServo::detectBaudrate(std::initializer_list<uint32_t> baudrates)
{
    return detectBaudrate(etl::span<const uint32_t>{baudrates.begin(), baudrates.size()});
}


SmartServo::Status SmartServo::sync_write(record_t *records, size_t nb_records) {
  if(nb_records < 1) {
    return Status::INVALID_PARAMS;
  }

    

  uint8_t records_reg = records[0].reg;
  uint8_t records_len = records[0].len;
    
  servo_msg.STX = 0xFFFF;
  servo_msg.id = BROADCAST_ID;
  servo_msg.len = nb_records * (records_len+1) + 4;
  if(servo_msg.len > TX_BUF_LEN) {
    return Status::TX_BUFFER_OVERFLOW;
  }
  servo_msg.instruction = Instruction::SMART_SERVO_SYNC_WRITE;
  servo_msg.params_and_crc[0] = records_reg;
  servo_msg.params_and_crc[1] = records_len;

  for(size_t i=0; i<nb_records; i++) {
    if(records[i].len != records_len || records[i].reg != records_reg) {
      // len and reg must be the same in all records
      return Status::INVALID_PARAMS;
    }
    uint8_t* rec_id_p = &servo_msg.params_and_crc[2] + i * (records_len+1);
    uint8_t* data_p = rec_id_p + 1;
    *rec_id_p = records[i].id;
    memcpy(data_p, records[i].data, records_len);
  }

  set_chk(&servo_msg, compute_chk(&servo_msg));

  send_msg(sio, &servo_msg);

  // do the servos answer with status packet ???
  return Status::OK;
}

SmartServo::Status SmartServo::writeRegister(uint8_t id, uint8_t reg, uint8_t value)
{
  SmartServo::record_t rec = {
    .id = id,
    .reg = reg,
    .len = 1,
    .data = {value}
  };

  return write(&rec);
}



servo_msg_t IN_DMA_SECTION_NOINIT(SmartServo::servo_msg);
servo_msg_t IN_DMA_SECTION_NOINIT(SmartServo::servo_status);

namespace {
  uint8_t compute_chk(servo_msg_t* msg) {
    uint8_t chk = 0;
    size_t chk_len = msg->len + 1;
    uint8_t* data = (uint8_t*) &msg->id;
    for(size_t i=0; i<chk_len; i++) {
      chk += data[i];
    }
    chk = ~chk;
    return chk;
  }
}
