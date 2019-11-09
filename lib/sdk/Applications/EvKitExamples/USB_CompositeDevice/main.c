/* *****************************************************************************
 * Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 * $Date: 2018-07-18 13:53:37 -0500 (Wed, 18 Jul 2018) $
 * $Revision: 36256 $
 *
 **************************************************************************** */

#include <stdio.h>
#include <stddef.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "board.h"
#include "led.h"
#include "pb.h"
#include "usb.h"
#include "usb_event.h"
#include "enumerate.h"
#include "hid_kbd.h"
#include "msc.h"
#include "descriptors.h"
#include "mscmem.h"

/* **** Definitions **** */
#define EVENT_ENUM_COMP     MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE   (EVENT_ENUM_COMP + 1)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/* **** Global Data **** */
volatile int configured;
volatile int suspended;
volatile unsigned int event_flags;
int remote_wake_en;

/* This EP assignment must match the Configuration Descriptor */
static msc_cfg_t msc_cfg = {
    1,                    /* EP OUT */
    MXC_USBHS_MAX_PACKET, /* OUT max packet size */
    2,                    /* EP IN */
    MXC_USBHS_MAX_PACKET, /* IN max packet size */
};

static const msc_idstrings_t ids = {
    "MAXIM",            /* Vendor string.  Maximum of 8 bytes */
    "MSC Example",      /* Product string.  Maximum of 16 bytes */
    "1.0"               /* Version string.  Maximum of 4 bytes */
};

/* Functions to control "disk" memory. See msc.h for definitions. */
static const msc_mem_t mem = {
    mscmem_init,
    mscmem_start,
    mscmem_stop,
    mscmem_ready,
    mscmem_size,
    mscmem_read,
    mscmem_write,
};

/* **** Function Prototypes **** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata);
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int event_callback(maxusb_event_t evt, void *data);
static void usb_app_sleep(void);
static void usb_app_wakeup(void);
static void button_callback(void *pb);

/**
 * @brief      @brief      Callback function for startup that calls the system startup for USB.
 *
 * @return     0 if successful
 */
int usb_startup_cb()
{
    const sys_cfg_usbhs_t sys_usbhs_cfg = NULL;
    return SYS_USBHS_Init(&sys_usbhs_cfg);
}

/**
 * @brief      Callback function for shutdown that calls the system shutdown for USB.
 *
 * @return     Shutdown status
 */
int usb_shutdown_cb()
{
    return SYS_USBHS_Shutdown();
}

/**
 * User-supplied function to delay usec micro-seconds
 *
 * @param[in]  usec  The usec time to delay.
 */
void delay_us(unsigned int usec)
{
    /* mxc_delay() takes unsigned long, so can't use it directly */
    mxc_delay(usec);
}

/* ************************************************************************** */
int main(void)
{
    maxusb_cfg_options_t usb_opts;
    
    printf("\n\n***** " TOSTRING(TARGET) " USB Composite Device (Keyboard and Mass Storage) Example *****\n");
    printf("Waiting for VBUS...\n");

    /* Initialize state */
    configured = 0;
    suspended = 0;
    event_flags = 0;
    remote_wake_en = 0;

    /* Start out in full speed */
    usb_opts.enable_hs = 0;							/* 0:Full Speed		1:High Speed */
    usb_opts.delay_us = delay_us; /* Function which will be used for delays */
    usb_opts.init_callback = usb_startup_cb;
    usb_opts.shutdown_callback = usb_shutdown_cb;

    /* Initialize the usb module */
    if (usb_init(&usb_opts) != 0) {
        printf("usb_init() failed\n");
        while (1);
    }

    /* Initialize the enumeration module */
    if (enum_init() != 0) {
        printf("enum_init() failed\n");
        while (1);
    }

    /* Register enumeration data */
    enum_register_descriptor(ENUM_DESC_DEVICE, (uint8_t*)&composite_device_descriptor, 0);
    enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t*)&composite_config_descriptor, 0);
    if (usb_opts.enable_hs) {
	/* Two additional descriptors needed for high-speed operation */
	enum_register_descriptor(ENUM_DESC_OTHER_SPEED, (uint8_t*)&composite_config_descriptor_hs, 0);
	enum_register_descriptor(ENUM_DESC_QUAL, (uint8_t*)&composite_device_qualifier_descriptor, 0);
    }
    enum_register_descriptor(ENUM_DESC_STRING, lang_id_desc, 0);
    enum_register_descriptor(ENUM_DESC_STRING, mfg_id_desc, 1);
    enum_register_descriptor(ENUM_DESC_STRING, prod_id_desc, 2);
    enum_register_descriptor(ENUM_DESC_STRING, serial_id_desc, 3);
    enum_register_descriptor(ENUM_DESC_STRING, hidkbd_func_desc, 4);
    enum_register_descriptor(ENUM_DESC_STRING, msc_func_desc, 5);

    /* Handle configuration */
    enum_register_callback(ENUM_SETCONFIG, setconfig_callback, NULL);

    /* Handle feature set/clear */
    enum_register_callback(ENUM_SETFEATURE, setfeature_callback, NULL);
    enum_register_callback(ENUM_CLRFEATURE, clrfeature_callback, NULL);

    /* Initialize the class driver */
    if (hidkbd_init(&composite_config_descriptor.hid_interface_descriptor,
		    &composite_config_descriptor.hid_descriptor,
		    report_descriptor) != 0) {
        printf("hidkbd_init() failed\n");
        while (1);
    }

    /* Initialize the class driver */
    if (msc_init(&composite_config_descriptor.msc_interface_descriptor, &ids, &mem) != 0) {
        printf("msc_init() failed\n");
        while (1);
    }

    /* Register callbacks */
    usb_event_enable(MAXUSB_EVENT_NOVBUS, event_callback, NULL);
    usb_event_enable(MAXUSB_EVENT_VBUS, event_callback, NULL);

    /* Register callback for keyboard events */
    if (PB_RegisterCallback(0, button_callback) != E_NO_ERROR) {
        printf("PB_RegisterCallback() failed\n");
        while (1);
    }

    /* Start with USB in low power mode */
    usb_app_sleep();
    NVIC_EnableIRQ(USB_IRQn);

    /* Wait for events */
    while (1) {

        if (suspended || !configured) {
            LED_Off(0);
        } else {
            LED_On(0);
        }

        if (event_flags) {
            /* Display events */
            if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_NOVBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_NOVBUS);
                printf("VBUS Disconnect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_VBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_VBUS);
                printf("VBUS Connect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_BRST)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_BRST);
                printf("Bus Reset\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_BRSTDN)) {										///
        		MXC_CLRBIT(&event_flags, MAXUSB_EVENT_BRSTDN);
        		printf("Bus Reset Done: %s speed\n", (usb_get_status() & MAXUSB_STATUS_HIGH_SPEED) ? "High" : "Full");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_SUSP)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_SUSP);
                printf("Suspended\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_DPACT)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_DPACT);
                printf("Resume\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_ENUM_COMP)) {
                MXC_CLRBIT(&event_flags, EVENT_ENUM_COMP);
                printf("Enumeration complete. Press SW2 to send character.\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_REMOTE_WAKE)) {
                MXC_CLRBIT(&event_flags, EVENT_REMOTE_WAKE);
                printf("Remote Wakeup\n");
            }
        }
    }
}

/* ************************************************************************** */

static int setconfig_callback(usb_setup_pkt *sud, void *cbdata)
{
    /* Confirm the configuration value */
    if (sud->wValue == composite_config_descriptor.config_descriptor.bConfigurationValue) {
        configured = 1;
        MXC_SETBIT(&event_flags, EVENT_ENUM_COMP);
    	if (usb_get_status() & MAXUSB_STATUS_HIGH_SPEED) {													///
    	    msc_cfg.out_ep = composite_config_descriptor_hs.endpoint_descriptor_1.bEndpointAddress & 0x7;
    	    msc_cfg.out_maxpacket = composite_config_descriptor_hs.endpoint_descriptor_1.wMaxPacketSize;
    	    msc_cfg.in_ep = composite_config_descriptor_hs.endpoint_descriptor_2.bEndpointAddress & 0x7;
    	    msc_cfg.in_maxpacket = composite_config_descriptor_hs.endpoint_descriptor_2.wMaxPacketSize;
    	} else {
			msc_cfg.out_ep = composite_config_descriptor.endpoint_descriptor_1.bEndpointAddress & 0x7;
			msc_cfg.out_maxpacket = composite_config_descriptor.endpoint_descriptor_1.wMaxPacketSize;
			msc_cfg.in_ep = composite_config_descriptor.endpoint_descriptor_2.bEndpointAddress & 0x7;
			msc_cfg.in_maxpacket = composite_config_descriptor.endpoint_descriptor_2.wMaxPacketSize;
    	}
        msc_configure(&msc_cfg);
        return hidkbd_configure(composite_config_descriptor.endpoint_descriptor_3.bEndpointAddress & USB_EP_NUM_MASK);
    } else if (sud->wValue == 0) {
        configured = 0;
        msc_deconfigure();
        return hidkbd_deconfigure();
    }

    return -1;
}

/* ************************************************************************** */

static int setfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if (sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 1;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */

static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if (sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 0;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */

static void usb_app_sleep(void)
{
    /* TODO: Place low-power code here */
    suspended = 1;
}

/* ************************************************************************** */

static void usb_app_wakeup(void)
{
    /* TODO: Place low-power code here */
    suspended = 0;
}

/* ************************************************************************** */

static int event_callback(maxusb_event_t evt, void *data)
{
    /* Set event flag */
    MXC_SETBIT(&event_flags, evt);

    switch (evt) {
        case MAXUSB_EVENT_NOVBUS:
            usb_event_disable(MAXUSB_EVENT_BRST);
            usb_event_disable(MAXUSB_EVENT_SUSP);
            usb_event_disable(MAXUSB_EVENT_DPACT);
            usb_disconnect();
            configured = 0;
            enum_clearconfig();
            hidkbd_deconfigure();
            msc_deconfigure();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_VBUS:
            usb_event_clear(MAXUSB_EVENT_BRST);
            usb_event_enable(MAXUSB_EVENT_BRST, event_callback, NULL);
    	    usb_event_clear(MAXUSB_EVENT_BRSTDN);												///
    	    usb_event_enable(MAXUSB_EVENT_BRSTDN, event_callback, NULL);						///
            usb_event_clear(MAXUSB_EVENT_SUSP);
            usb_event_enable(MAXUSB_EVENT_SUSP, event_callback, NULL);
            usb_connect();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_BRST:
            usb_app_wakeup();
            enum_clearconfig();
            hidkbd_deconfigure();
            msc_deconfigure();
            configured = 0;
            suspended = 0;
            break;
    	case MAXUSB_EVENT_BRSTDN:																///
    	    if (usb_get_status() & MAXUSB_STATUS_HIGH_SPEED) {
				enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t*)&composite_config_descriptor_hs, 0);
				enum_register_descriptor(ENUM_DESC_OTHER_SPEED, (uint8_t*)&composite_config_descriptor, 0);
    	    } else {
				enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t*)&composite_config_descriptor, 0);
				enum_register_descriptor(ENUM_DESC_OTHER_SPEED, (uint8_t*)&composite_config_descriptor_hs, 0);
    	    }
    	    break;
        case MAXUSB_EVENT_SUSP:
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_DPACT:
            usb_app_wakeup();
            break;
        default:
            break;
    }

    return 0;
}

/* ************************************************************************** */

void button_callback(void *pb)
{
    static const uint8_t chars[] = "Maxim Integrated\n";
    static int i = 0;
    int count = 0;
    int button_pressed = 0;
    
    //determine if interrupt triggered by bounce or a true button press
    while (PB_Get(0) && !button_pressed) {
        count++;
        if (count > 1000) {
            button_pressed = 1;
        }
    }

    if (button_pressed) {
        LED_Toggle(1);
        if (configured) {
            if (suspended && remote_wake_en) {
                /* The bus is suspended. Wake up the host */
                suspended = 0;
                usb_app_wakeup();
                usb_remote_wakeup();
                MXC_SETBIT(&event_flags, EVENT_REMOTE_WAKE);
            } else {
                if (i >= (sizeof(chars) - 1)) {
                    i = 0;
                }
                hidkbd_keypress(chars[i++]);
            }
        }
    }
}

/* ************************************************************************** */

void USB_IRQHandler(void)
{
    usb_event_handler();
}
