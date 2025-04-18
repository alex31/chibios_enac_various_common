#pragma once

#include <ch.h>
#include <hal.h>
#include <array>
#include <variant>
#include "fifoObject.hpp"


namespace {
/**
 * @brief   helper class used to generate dispatch thread name
 */
  struct InstCount {
    static uint8_t count;
  };

/**
 * @brief  concept that check if What type is in Args parameter pack
 */
  template<typename What, typename... Args>
  concept is_type_present = (std::is_same_v<What, Args> || ...);

/**
 * @brief  callback function type
 */
  template <typename T>
  using JobQueueCB = void(*)(T&);

/**
 * @brief  status returned but submitXXX methods
 */
  enum class JobQueueStatus {OK, TIMOUT, EMPTY};
}




/**
 * @brief	JobQueue main class
 * @tparam	QSZ	Fifo size
 * @tparam	MT...	Parameter pack of accepted type arguments
 */
template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
class JobQueue : private InstCount
{
private:
  /**
   * @brief	variant of fn callback + arg pair from parameter pack
   * @note	... expand the parameter pack
   */
  using JobFnAndArg = std::variant<std::pair<void(*)(MT&), MT>...>;

  /**
   * @brief	helper type to declare array of stack area
   * @note	one stack for each dispatch thread
   */
  template<size_t STS, size_t NBD>
  using StackArray =  std::array<std::array<stkalign_t,
					    THD_WORKING_AREA_SIZE(STS) / sizeof(stkalign_t)>,
				 NBD>;
  
  /**
   * @brief	encapsulate job
   * @note	copy constructor is deleted to avoid unwanted copies
   */
  struct Job {
    Job(const Job&) = delete;
    ~Job() = default;
    Job()= default;
    JobFnAndArg jfa;
  };
  
public:
 /**
   * @brief		submit a job from thread context
   * @param[in] cb	callback function, 
   * @param[in] t	argument for callback function, 
   * @param[in] timeout	optional timeout (TIME_INFINITE is the default)
   * @return    status  succes or timeout
   * @notes		argument type should be in the MT parameter pack list of
   *                    JobQueue possible type
   * @notes		cb must decay in function pointer : cannot be a capture lambda
   */
  template< typename T, std::invocable<T&> F>
  requires std::convertible_to<F, JobQueueCB<T>> &&
           is_type_present<T, MT...>
  JobQueueStatus submit(F cb, const T& t, sysinterval_t timeout=TIME_INFINITE);

 /**
   * @brief		submit a job from ISR context
   * @param[in] cb	callback function, 
   * @param[in] t	argument for callback function, 
   * @return    status  succes or fail because queue is full
   * @notes		argument type should be in the MT parameter pack list of
   *                    JobQueue possible type
   * @notes		cb must decay in function pointer : cannot be a capture lambda
   */
   template< typename T, std::invocable<T&> F>
  requires std::convertible_to<F, JobQueueCB<T>> &&
           is_type_present<T, MT...>
  JobQueueStatus submitI(F cb, const T& t);

/**
   * @brief		get instance of jobQueue
   * @tparam STS	optional stack size : default is 256
   * @tparam NDB	optional number of launched dispatch threads : default is 1
   * @return		jobQueue object
   * @notes		singleton pattern
   */
  template<size_t STS=256U, size_t NBD=1U>
  requires (NBD != 0)
    static JobQueue& instance();
  
private:
  JobQueue(const JobQueue&) = delete;
  JobQueue& operator=(const JobQueue &) = delete;
  JobQueue(JobQueue &&) = delete;
  JobQueue & operator=(JobQueue ) = delete;

  /**
   * @brief		internal job dispatch thread function
   */
  template <size_t SN, typename ST>
  JobQueue(std::array<ST, SN>& stacks); 
  ObjectFifo<Job, QSZ> fifo;
  [[noreturn]] static void jobDispatch(void *fifo);
};

template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
template <size_t SN, typename ST>
JobQueue<QSZ, MT...>::JobQueue(std::array<ST, SN>& stacks)
{
  for (auto& stack : stacks)
    chThdCreateStatic(stack.data(), sizeof(stack),
		      NORMALPRIO, &jobDispatch, &fifo);
}

template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
template< typename T, std::invocable<T&> F>
requires std::convertible_to<F, JobQueueCB<T>> &&
         is_type_present<T, MT...>
JobQueueStatus JobQueue<QSZ, MT...>::submit(F cb, const T& t, sysinterval_t timeout)
{
  auto [status, obj]  = fifo.takeObject(timeout);
  if (status == MSG_OK) {
    obj.jfa = std::pair(cb, t);
    fifo.sendObject(obj);
  }
  return status == MSG_OK ? JobQueueStatus::OK : JobQueueStatus::TIMOUT;
}


template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
template< typename T, std::invocable<T&> F>
requires std::convertible_to<F, JobQueueCB<T>> &&
         is_type_present<T, MT...>
JobQueueStatus JobQueue<QSZ, MT...>::submitI(F cb, const T& t)
{
  auto [status, obj] = fifo.takeObjectI();
  if (status == MSG_OK) {  
    obj.jfa = std::pair(cb, t);
    fifo.sendObjectI(obj);
  }
  return status == MSG_OK ? JobQueueStatus::OK : JobQueueStatus::EMPTY;
}

// STATIC Methods

template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
  template<size_t STS, size_t NBD>
  requires (NBD != 0)
  JobQueue<QSZ, MT...>& JobQueue<QSZ, MT...>::instance() {
#if PORT_ENABLE_GUARD_PAGES
  alignas(32)
#endif
    constinit static StackArray<STS, NBD> stacks __attribute__((section(FAST_SECTION "_clear")));
  static JobQueue jq(stacks);
  return jq;
}
 
template <uint16_t QSZ, typename... MT>
requires (QSZ != 0)
[[noreturn]]void JobQueue<QSZ, MT...>::jobDispatch(void *_fifo)
{
  char thdName[20];
  chsnprintf(thdName, sizeof(thdName), "dispatch S[%d]<%d>", count++, QSZ);
  chRegSetThreadName(thdName);
  
  auto fifo = static_cast<ObjectFifo<Job, QSZ> *>(_fifo);
  while (true) {
    auto [status, obj]  = fifo->receiveObject();
    if (status == MSG_OK) {
      std::visit([](auto&& fnarg) { // get the active variant function+argument pair
	auto&& [fn, arg] = fnarg;   // unpack pair
	fn(arg);                    // invoque callback with arg
      },
	obj.jfa);
      fifo->returnObject(obj);
    }
  }
}

// STATIC Members
uint8_t InstCount::count = 0;
