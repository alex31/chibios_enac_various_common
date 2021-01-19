#pragma once

#include "ch.h"
#include "hal.h"

typedef void (*benchmarkFn_t) (void * userData);

typedef struct {
  uint32_t totalMicroSeconds;
  uint32_t meanMicroSeconds;
  uint32_t minMicroSeconds;
  uint32_t maxMicroSeconds;  
} benchResults;

/**
 * @brief   measure the execution time for a function
 *
 * @param[in] fn        pointer to the finction to be benchmarked
 * @param[in] iter      number of time the finction will be called
 * @return              The benchmark result.
 * @api
 */
benchResults doBench(const benchmarkFn_t fn, const size_t iter,
		 void *userData);
