#pragma once



/*
     aaaaa  
     f   b
     f   b
     ggggg
     e   c
     e   c
     ddddd h

 */

/*

   to display '1' display_segments (0b01100000);
   to display 'A.' display_segments (0b11101111);

   OR

   display_digit ('1', false);
   display_digit ('A', true);


   Si erreur : affichage erreur

   d : sD corrupted
   E : Empty -> pas de sdcard
   F : Full  -> sd full

   U : imU (+magnetometer)
   I : spI error
   II : I2c (symbol j)
   P : gPs error
   r : radio error
   t : telemetry error



   si point en + : autres erreurs

   Si pas d'erreur :
   0 à 9 sans point : niveau batterie


 */

typedef enum  {SEVSEG_SDCORRUPT=0, SEVSEG_NOSD,
	       SEVSEG_SDFULL, SEVSEG_IMU, SEVSEG_MAG, 
	       SEVSEG_BARO, SEVSEG_PITOT,
	       SEVSEG_SPI, SEVSEG_I2C,
	       SEVSEG_GPS, SEVSEG_RC, SEVSEG_TELEMETRY,
	       SEVSEG_RCINV,
	       SEVSEG_PWRSWITCH, SEVSEG_BAT, SEVSEG_ADC,
	       SEVSEG_GPIO_CONTINUITY, SEVSEG_GPIO_SHORT, 
	       SEVSEG_LSE, SEVSEG_HSE,
	       
	       SEVSEG_END } SevSegErrorState;


void sevseg_register_error (SevSegErrorState err);

void sevseg_unregister_error (SevSegErrorState err);

void sevseg_set_info_digit (uint8_t digit);

void sevseg_display_digit (const unsigned char c, const bool dp);

void sevseg_blink (uint32_t periodInMilliseconds);
