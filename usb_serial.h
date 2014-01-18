#ifndef __USB_SERIAL_H__
#define __USB_SERIAL_H__
#include <hal.h>


#if HAL_USE_SERIAL_USB == TRUE

void usbSerialInit(SerialUSBDriver *sdu, USBDriver *usbDriver) ;
void usbSerialReset(SerialUSBDriver *sdu) ;
USBDriver *usbGetDriver (void);
bool_t isUsbConnected(void);

#endif // HAL_USE_SERIAL_USB

#endif // __USB_SERIAL_H__
