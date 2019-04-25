/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Id: main.c 31505 2017-10-20 21:16:29Z zach.metzinger $
 *
 *******************************************************************************
 */

/**
 * @file    main.c
 * @brief   USB Mass Storage Class example
 * @details This project creates a mass storage device using either on-board RAM or 
 *          external SPI flash memory.  Load the project, connect a cable from the PC
 *          to the USB connector.  A new external drive should appear than can be read
 *          and written.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "mxc_config.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "flc.h"
#include "icc.h"
#include "crc.h"
#include "board.h"
#include "led.h"
#include "ff.h"
#include "crc16-ccitt.h"


#define GPIO_PORT_IN                PORT_1
#define GPIO_PIN_IN                 PIN_6


#define PARTITION_START (0x10000000 + 64 * 1024)
#define PARTITION_END (0x10000000 + 512 * 1024 - 1) /* TODO: check if 1 MB also works. Might have to enable the second bank */
//#define PARTITION_END (0x10000000 + 1024 * 1024 - 1)

extern void run_usbmsc(void);
DIR dir;                /* Directory object */
FATFS FatFs;

bool mount(void)
{
    FRESULT res;
    res=f_mount(&FatFs,"/",0);
    if(res != FR_OK) {
        printf("f_mount error %d\n", res);
        return false;
    }

    res = f_opendir(&dir, "0:");
    if(res != FR_OK) {
        printf("f_opendir error %d\n", res);
        return false;
    }

    return true;
}

bool check_integrity(void)
{
    FIL file;
    UINT readbytes;
    char *filename = "card10.bin";
    uint8_t data[512];
    FRESULT res;

    res=f_open(&file, filename, FA_OPEN_EXISTING|FA_READ);
    if(res != FR_OK) {
        printf("f_open error %d\n", res);
        return false;
    }

    uint16_t crcval = 0;
    do {
        res = f_read(&file, data, sizeof(data), &readbytes);
        if(res != FR_OK) {
            printf("f_read error %d\n", res);
            crcval = 1; // Make sure to fail the test
            break;
        }
        crcval = crc16_ccitt(crcval, data, readbytes);
    } while (readbytes == sizeof(data));

    f_close(&file);

    if(crcval == 0) {
        return true;
    } else {
        printf("CRC check failed. Final CRC: %d\n", crcval);
        return false;
    }
}

bool is_update_needed(void)
{
    FIL file;
    UINT readbytes;
    char *filename = "card10.bin";
    uint8_t data[512];
    FRESULT res;

    res=f_open(&file, filename, FA_OPEN_EXISTING|FA_READ);
    if(res != FR_OK) {
        printf("f_open error %d\n", res);
        return false;
    }

    uint8_t *partition = (uint8_t *)(intptr_t)PARTITION_START;
    bool different = false;
    do {
        res = f_read(&file, data, sizeof(data), &readbytes);
        if(res != FR_OK) {
            printf("f_read error %d\n", res);
            break; /* Going to return false, don't want to use this file */
        }
        if(memcmp(partition, data, readbytes)) {
            different = true;
            break;
        }
        partition += readbytes;
    } while (readbytes == sizeof(data));

    f_close(&file);

    //different = true;
    return different;
}

void erase_partition(void)
{
    int ret = FLC_MultiPageErase(PARTITION_START, PARTITION_END);
    if(ret != E_NO_ERROR) {
        printf("FLC_MultiPageErase failed with %d\n", ret);
        while(1);
    }
}

void flash_partition(void)
{
    FIL file;
    UINT readbytes;
    char *filename = "card10.bin";
    uint8_t data[512];
    FRESULT res;

    res=f_open(&file, filename, FA_OPEN_EXISTING|FA_READ);
    if(res != FR_OK) {
        printf("f_open error %d\n", res);
        while(1);
    }

    uint32_t partition = PARTITION_START;

    ICC_Disable();
    do {
        res = f_read(&file, data, sizeof(data), &readbytes);
        if(res != FR_OK) {
            printf("f_read error %d\n", res);
            break; /* Going to return false, don't want to use this file */
        }
        int ret = FLC_Write(partition, readbytes, (uint32_t *)data); /* wild cast. not sure if this works */
        if(ret != E_NO_ERROR) {
            printf("FLC_Write failed with %d\n", ret);
            while(1);
        }
        partition += readbytes;
    } while (readbytes == sizeof(data));
    ICC_Enable();

    f_close(&file);
}

#if 0
void test(void)
{
    FRESULT res;
    FIL file;
    UINT readbytes;
    char *filename = "card10.bin";
    int len = 512;
    char data[512];

    res=f_open(&file, filename, FA_OPEN_EXISTING|FA_READ);
    if(res != FR_OK) printf("f_open error %d\n", res);

    res = f_read(&file, data, len, &readbytes);
    if(res != FR_OK) printf("f_read error %d\n", res);

    f_close(&file);

    data[readbytes] = 0;

    printf("data: %s\n", data);
}
#endif

static inline void boot(const void * vtable){
    SCB->VTOR = (uintptr_t) vtable;

	// Reset stack pointer & branch to the new reset vector.
	__asm(  "mov r0, %0\n"
			"ldr sp, [r0]\n"
			"ldr r0, [r0, #4]\n"
			"bx r0\n"
			:
			: "r"(vtable)
			: "%sp", "r0");
};


/******************************************************************************/
int main(void)
{

    printf("\n\nBootloader\n");

    gpio_cfg_t gpio_in;
    gpio_in.port = GPIO_PORT_IN;
    gpio_in.mask = GPIO_PIN_IN;
    gpio_in.pad = GPIO_PAD_PULL_UP;
    gpio_in.func = GPIO_FUNC_IN;
    GPIO_Config(&gpio_in);

    // If the button is pressed, we go into MSC mode.
    if (!GPIO_InGet(&gpio_in)) {
        run_usbmsc();

        // If we return, don't try to boot. Maybe rather trigger a software reset.
        // Reason: Not sure in which state the USB peripheral is and what kind
        // of interrupts are active.
        while(1);
    }


    if(mount()) {
        if(check_integrity()) {
            if(is_update_needed()) {
                MXC_FLC0->clkdiv = 54;
                erase_partition();
                flash_partition();
            } else {
                printf("No update needed\n");
            }
        } else {
            printf("Integrity check failed\n");
        }
    }


    printf("Trying to boot\n");
    // boot partition
    boot((uintptr_t *) PARTITION_START);

    while (1) {
        // Should never be reached.
    }
}

/******************************************************************************/
void SysTick_Handler(void)
{
    mxc_delay_handler();
}
