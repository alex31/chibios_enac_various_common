/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_opamp_lld.c
 * @brief   STM32 OPAMP subsystem low level driver header.
 *
 * @addtogroup OPAMP
 * @{
 */

#include "hal_community.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   OPAMPD1 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP1 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD1;
#endif

/**
 * @brief   OPAMPD2 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP2 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD2;
#endif

/**
 * @brief   OPAMPD3 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP3 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD3;
#endif

/**
 * @brief   OPAMPD4 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP4 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD4;
#endif


/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level OPAMP driver initialization.
 *
 * @notapi
 */
void opamp_lld_init(void) {

#if STM32_OPAMP_USE_OPAMP1
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD1);
  OPAMPD1.opamp = OPAMP1;
#if STM32_OPAMP_USER_TRIM_ENABLED
  OPAMPD1.trim_n = OPAMPD1.trim_p = 0U;
#endif
#endif

#if STM32_OPAMP_USE_OPAMP2
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD2);
  OPAMPD2.opamp = OPAMP2;
#if STM32_OPAMP_USER_TRIM_ENABLED
  OPAMPD2.trim_n = OPAMPD2.trim_p = 0U;
#endif
#endif

#if STM32_OPAMP_USE_OPAMP3
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD3);
  OPAMPD3.opamp = OPAMP3;
# if STM32_OPAMP_USER_TRIM_ENABLED
  OPAMPD3.trim_n = OPAMPD3.trim_p = 0U;
# endif
#endif

#if STM32_OPAMP_USE_OPAMP4
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD4);
  OPAMPD4.opamp = OPAMP4;
#if STM32_OPAMP_USER_TRIM_ENABLED
  OPAMPD4.trim_n = OPAMPD4.trim_p = 0U;
#endif
#endif

#if STM32_OPAMP_USER_TRIM_ENABLED
  opamp_lld_calibrate();
#endif
}

/**
 * @brief   Configures and activates the OPAMP peripheral.
 * @note    Starting a driver that is already in the @p OPAMP_READY state
 *          disables output.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_start(OPAMPDriver *opampp) {
  opampp->opamp->CSR = opampp->config->csr;
#if STM32_OPAMP_USER_TRIM_ENABLED
  opampp->opamp->CSR |= OPAMP_CSR_USERTRIM;
  MODIFY_REG(opampp->opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	     opampp->trim_n<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
  MODIFY_REG(opampp->opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	     opampp->trim_p<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
}

/**
 * @brief   Deactivates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_stop(OPAMPDriver *opampp) {
  opampp->opamp->CSR = 0U;
}

/**
 * @brief   Enables an OPAMP 
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_enable(OPAMPDriver *opampp) {
  opampp->opamp->CSR |= OPAMP_CSR_OPAMPxEN;
}

/**
 * @brief   Disables an OPAMP 
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_disable(OPAMPDriver *opampp) {
  opampp->opamp->CSR &= ~OPAMP_CSR_OPAMPxEN;
}


/**
 * @brief   Calibrate all enabled OPAMPs
 * @pre     All enabled OPAMPs are calibrated in parallell
 *          process can take time (up to 30ms)
 * @notapi
 */
#if STM32_OPAMP_USER_TRIM_ENABLED
void opamp_lld_calibrate(void)
{
#if STM32_OPAMP_USE_OPAMP1
  uint32_t trimmingvaluen1 = 16U;
  uint32_t trimmingvaluep1 = 16U;
  OPAMPD1.state =  OPAMP_CALIBRATING;
#define CSRm OPAMPD1.opamp->CSR 
  /* Set Calibration mode */
  /* Non-inverting input connected to calibration reference voltage. */
  CSRm |= OPAMP_CSR_FORCEVP;
  /*  user trimming values are used for offset calibration */
  CSRm |= OPAMP_CSR_USERTRIM;
  /* Enable calibration */
  CSRm |= OPAMP_CSR_CALON;
  /* 1st calibration - N  Select 90U% VREF */
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_90);
  /* Enable the opamps */
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif
  
#if STM32_OPAMP_USE_OPAMP2
  uint32_t trimmingvaluen2 = 16U;
  uint32_t trimmingvaluep2 = 16U;
  OPAMPD2.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD2.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_90);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif
  
#if STM32_OPAMP_USE_OPAMP3
  uint32_t trimmingvaluen3 = 16U;
  uint32_t trimmingvaluep3 = 16U;
  OPAMPD3.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD3.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_90);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP4
  uint32_t trimmingvaluen4 = 16U;
  uint32_t trimmingvaluep4 = 16U;
  OPAMPD4.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD4.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_90);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

  chSysPolledDelayX(MS2ST(20));
  uint32_t delta = 8U; 
  
  while (delta != 0U) {
#if STM32_OPAMP_USE_OPAMP1
    MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen1<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP2
    MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen2<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP3
    MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen3<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP4
    MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen4<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif

    /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */ 
    /* Offset trim time: during calibration, minimum time needed between */
    /* two steps to have 1 mV accuracy */
    chSysPolledDelayX(MS2ST(2));

#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   { 
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluen1 += delta;
    } else  {
      /* OPAMP_CSR_OUTCAL is LOW try lower trimming */
      trimmingvaluen1 -= delta;
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen2 += delta;
    } else {
      trimmingvaluen2 -= delta;
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen3 += delta;
    } else {
      trimmingvaluen3 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen4 += delta;
    }  else {
      trimmingvaluen4 -= delta;
    }
#endif
                     
    delta >>= 1U;
    
  }

  /* Still need to check if righ calibration is current value or un step below */
  /* Indeed the first value that causes the OUTCAL bit to change from 1 to 0U */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	     trimmingvaluen1<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	     trimmingvaluen2<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	     trimmingvaluen3<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
	     trimmingvaluen4<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
#endif

  /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */ 
  /* Offset trim time: during calibration, minimum time needed between */
  /* two steps to have 1 mV accuracy */
  chSysPolledDelayX(MS2ST(2));
  
#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   { 
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluen1  += (trimmingvaluen1 != 31);
      MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
		 trimmingvaluen1<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen2  += (trimmingvaluen2 != 31);
      MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
		 trimmingvaluen2<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen3  += (trimmingvaluen3 != 31);
      MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
		 trimmingvaluen3<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluen4  += (trimmingvaluen4 != 31);
      MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
		 trimmingvaluen4<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    }
#endif

    /* 2nd calibration - P */
    /* Select 10U% VREF */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_10);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_10);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_10);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_CALSEL, OPAMP_CSR_OPAMPx_CALSEL_10);
#endif

  delta = 8U;


  while (delta != 0U)   {
#if STM32_OPAMP_USE_OPAMP1
    MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep1<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP2
    MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep2<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP3
    MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep3<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP4
    MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep4<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
  

    /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */ 
    /* Offset trim time: during calibration, minimum time needed between */
    /* two steps to have 1 mV accuracy */
    chSysPolledDelayX(MS2ST(2));
#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   { 
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluep1 += delta;
    } else  {
      /* OPAMP_CSR_OUTCAL is LOW try lower trimming */
      trimmingvaluep1 -= delta;
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep2 += delta;
    } else {
      trimmingvaluep2 -= delta;
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep3 += delta;
    } else {
      trimmingvaluep3 -= delta;
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep4 += delta;
    }  else {
      trimmingvaluep4 -= delta;
    }
#endif
    
    delta >>= 1U;
  }
  
  /* Still need to check if righ calibration is current value or un step below */
  /* Indeed the first value that causes the OUTCAL bit to change from 1 to 0U */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	     trimmingvaluep1<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	     trimmingvaluep2<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	     trimmingvaluep3<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
	     trimmingvaluep4<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
#endif

  /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */ 
  /* Offset trim time: during calibration, minimum time needed between */
  /* two steps to have 1 mV accuracy */
  chSysPolledDelayX(MS2ST(2));

#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   { 
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluep1 += (trimmingvaluep1 != 31);
      MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
		 trimmingvaluep1<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep2 += (trimmingvaluep2 != 31);
      MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
		 trimmingvaluep2<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    }
#endif
    
#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep3 += (trimmingvaluep3 != 31);
      MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
		 trimmingvaluep3<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    { 
      trimmingvaluep4 += (trimmingvaluep4 != 31);
      MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
		 trimmingvaluep4<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    }
#endif

#if STM32_OPAMP_USE_OPAMP1
#define CSRm OPAMPD1.opamp->CSR 
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD1.trim_n = trimmingvaluen1;
    /* Write calibration result P */
    OPAMPD1.trim_p = trimmingvaluep1;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen1<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep1<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    OPAMPD1.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP2
#define CSRm OPAMPD2.opamp->CSR 
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD2.trim_n = trimmingvaluen2;
    /* Write calibration result P */
    OPAMPD2.trim_p = trimmingvaluep2;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen2<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep2<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    OPAMPD2.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP3
#define CSRm OPAMPD3.opamp->CSR 
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD3.trim_n = trimmingvaluen3;
    /* Write calibration result P */
    OPAMPD3.trim_p = trimmingvaluep3;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen3<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep3<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    OPAMPD3.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP4
#define CSRm OPAMPD4.opamp->CSR 
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD4.trim_n = trimmingvaluen4;
    /* Write calibration result P */
    OPAMPD4.trim_p = trimmingvaluep4;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
	       trimmingvaluen4<<OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
	       trimmingvaluep4<<OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS);
    OPAMPD4.state = OPAMP_STOP;
#undef CSRm
#endif
}
#endif // STM32_OPAMP_USER_TRIM_ENABLED
#endif /* HAL_USE_OPAMP */

/** @} */
