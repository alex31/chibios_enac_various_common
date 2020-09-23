/**
 * @file    usb_msd.h
 * @brief   USB mass storage driver and functions
 */

#ifndef _USB_MSD_H_
#define _USB_MSD_H_

#include "ch.h"
#include "hal.h"
#include "chdebug.h"

#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

/* endpoint index */
#define USB_MS_DATA_EP 1



#define USBD USBD1

#define SEM_TAKEN      
#define SEM_RELEASED   

#define         EVT_USB_RESET                           (1 << 0)
#define         EVT_BOT_RESET                           (1 << 1)
#define         EVT_SEM_TAKEN                           (1 << 2)
#define         EVT_SEM_RELEASED                        (1 << 3)
#define         EVT_USB_CONFIGURED                      (1 << 4)
#define         EVT_SCSI_REQ_TEST_UNIT_READY            (1 << 5)
#define         EVT_SCSI_REQ_READ_FMT_CAP               (1 << 6)
#define         EVT_SCSI_REQ_SENSE6                     (1 << 7)
#define         EVT_SCSI_REQ_SENSE10                    (1 << 8)
#define         EVT_WAIT_FOR_COMMAND_BLOCK              (1 << 9)
#define         EVT_SCSI_REQ_SEND_DIAGNOSTIC            (1 << 10)
#define         EVT_SCSI_REQ_READ_CAP10                 (1 << 11)
#define         EVT_SCSI_PROC_INQ                       (1 << 12)

/**
 * @brief Command Block Wrapper structure
 */
PACK_STRUCT_BEGIN typedef struct {
	uint32_t signature;
	uint32_t tag;
	uint32_t data_len;
	uint8_t flags;
	uint8_t lun;
	uint8_t scsi_cmd_len;
	uint8_t scsi_cmd_data[16];
} PACK_STRUCT_STRUCT msd_cbw_t PACK_STRUCT_END;

/**
 * @brief Command Status Wrapper structure
 */
PACK_STRUCT_BEGIN typedef struct {
	uint32_t signature;
	uint32_t tag;
	uint32_t data_residue;
	uint8_t status;
} PACK_STRUCT_STRUCT msd_csw_t PACK_STRUCT_END;

/**
 * @brief Structure holding sense data (status/error information)
 */
PACK_STRUCT_BEGIN typedef struct {
		uint8_t byte[18];
} PACK_STRUCT_STRUCT msd_scsi_sense_response_t PACK_STRUCT_END;

/**
 * @brief structure holding the data to reply to an INQUIRY SCSI command
 */
PACK_STRUCT_BEGIN typedef struct
{
    uint8_t peripheral;
    uint8_t removable;
    uint8_t version;
    uint8_t response_data_format;
    uint8_t additional_length;
    uint8_t sccstp;
    uint8_t bqueetc;
    uint8_t cmdque;
    uint8_t vendor_id[8];
    uint8_t product_id[16];
    uint8_t product_rev[4];
} PACK_STRUCT_STRUCT msd_scsi_inquiry_response_t PACK_STRUCT_END;

/**
 * @brief Possible states for the USB mass storage driver
 */
typedef enum {
    MSD_IDLE,
    MSD_READ_COMMAND_BLOCK,
    MSD_EJECTED,
    MSD_BOT_RESET
} msd_state_t;

/**
 * @brief Driver configuration structure
 */
typedef struct {
    /**
    * @brief USB driver to use for communication
    */
    USBDriver *usbp;

    /**
    * @brief Block device to use for storage
    */
    BaseBlockDevice *bbdp;

    /**
    * @brief Index of the USB endpoint to use for transfers
    */
    usbep_t bulk_ep;

    /**
    * @brief Optional callback that will be called whenever there is
    *        read/write activity
    * @note  The callback is called with argument true when activity starts,
    *        and false when activity stops.
    */
    void (*rw_activity_callback)(bool);

    /**
    * @brief Short vendor identification
    * @note  ASCII characters only, maximum 8 characters (pad with zeroes).
    */
    uint8_t short_vendor_id[8];

    /**
    * @brief Short product identification
    * @note  ASCII characters only, maximum 16 characters (pad with zeroes).
    */
    uint8_t short_product_id[16];

    /**
    * @brief Short product revision
    * @note  ASCII characters only, maximum 4 characters (pad with zeroes).
    */
    uint8_t short_product_version[4];

} USBMassStorageConfig;

/**
 * @brief   USB mass storage driver structure.
 * @details This structure holds all the states and members of a USB mass
 *          storage driver.
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the Mass Storage over USB driver can use
 *          this function as requests hook in the USB configuration.
 *          The following requests are emulated:
 *          - MSD_REQ_RESET.
 *          - MSD_GET_MAX_LUN.
 *          .
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The hook status.
 * @retval true         Message handled internally.
 * @retval false        Message not handled.
 */
bool msdRequestsHook(USBDriver *usbp);


/**
 * @brief   start usb storage
 */
void init_msd_driver(void *dbgThreadPtr, USBMassStorageConfig *msdConfig);


 /**
 * @brief   stop usb storage
 */
void deinit_msd_driver(void);

/**
 * @brief   register connected event source in local event mask
 * @details This function is a stub to  chEvtRegisterMask
 */
void msd_register_evt_connected (event_listener_t *elp, eventmask_t mask);

/**
 * @brief   register ejected event source in local event mask
 * @details This function is a stub to  chEvtRegisterMask
 *          ejected event is a logical event : when host unmount the filesystem,
 *          not a physical event (event is not sent in case of unplugged usb wire)
 */
void msd_register_evt_ejected (event_listener_t *elp, eventmask_t mask);

#ifdef __cplusplus
}
#endif



#endif /* _USB_MSD_H_ */
