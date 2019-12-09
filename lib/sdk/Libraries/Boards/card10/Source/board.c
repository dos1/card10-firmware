/*******************************************************************************
 * Copyright (C) 2015 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Date: 2018-12-03 19:30:05 +0000 (Mon, 03 Dec 2018) $
 * $Revision: 39561 $
 *
 ******************************************************************************/

#include <stdio.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "mxc_assert.h"
#include "board.h"
#include "uart.h"
#include "gpio.h"
#include "mxc_pins.h"
#include "pb.h"
#include "spixfc.h"

/***** Global Variables *****/
mxc_uart_regs_t * ConsoleUart = MXC_UART_GET_UART(CONSOLE_UART);
extern uint32_t SystemCoreClock;

const gpio_cfg_t pb_pin[] = {
    {PORT_0, PIN_20, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},   // Wristband GPIO1
    {PORT_0, PIN_20, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},   // Wristband GPIO1
    {PORT_0, PIN_23, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},   // TOP GPIO3 / Button 3
    {PORT_1, PIN_7, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},   // TOP GPIO5 / Button 4
};
const unsigned int num_pbs = (sizeof(pb_pin) / sizeof(gpio_cfg_t));

/***** File Scope Variables *****/
const uart_cfg_t uart_cfg = {
    .parity = UART_PARITY_DISABLE,
    .size   = UART_DATA_SIZE_8_BITS,
    .stop   = UART_STOP_1,
    .flow   = UART_FLOW_CTRL_DIS,
    .pol    = UART_FLOW_POL_DIS,
    .baud   = CONSOLE_BAUD,
    .clksel = UART_CLKSEL_SYSTEM
};

const sys_cfg_uart_t uart_sys_cfg = {
    .map = MAP_A,
    .flow = Disable
};

const sys_cfg_spixfc_t spixfc_sys_cfg = NULL;   // There is no special system configuration parameters for SPIXC

const spixfc_cfg_t mx25_spixfc_cfg = {
    0, //mode
    0, //ssel_pol
    10000000 //baud
};

/******************************************************************************/
void mxc_assert(const char *expr, const char *file, int line)
{
    printf("MXC_ASSERT %s #%d: (%s)\n", file, line, expr);
    while (1);
}

/******************************************************************************/
int Board_Init(void)
{
    int err;

    const gpio_cfg_t pins[] = {
        {PORT_0, PIN_8, GPIO_FUNC_OUT, GPIO_PAD_NONE},      // Motor
        {PORT_0, PIN_31, GPIO_FUNC_OUT, GPIO_PAD_NONE},     // ECG switch
    };
    const unsigned int num_pins = (sizeof(pins) / sizeof(gpio_cfg_t));
    unsigned int i;
    for (i = 0; i < num_pins; i++) {
        GPIO_OutClr(&pins[i]);
        GPIO_Config(&pins[i]);
    }

    if ((err = Console_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    return E_NO_ERROR;
}

/******************************************************************************/
int Console_Init(void)
{
    int err;

    if ((err = UART_Init(ConsoleUart, &uart_cfg, &uart_sys_cfg)) != E_NO_ERROR) {
        return err;
    }

    return E_NO_ERROR;
}

/******************************************************************************/
int Console_Shutdown(void)
{
    int err;

    if ((err = UART_Shutdown(ConsoleUart)) != E_NO_ERROR) {
        return err;
    }

    return E_NO_ERROR;
}

/******************************************************************************/
void NMI_Handler(void)
{
    __NOP();
}


/******************************************************************************/
int MX25_Board_Init(void)
{
  return SPIXFC_Init(MX25_SPI, &mx25_spixfc_cfg, &spixfc_sys_cfg);

}

/******************************************************************************/
int MX25_Board_Read(uint8_t* read, unsigned len, unsigned deassert, spixfc_width_t width)
{

    spixfc_req_t req = {MX25_SSEL,deassert,0,NULL,read, width,len,0,0,NULL};

    return SPIXFC_Trans(MX25_SPI, &req);
}

/******************************************************************************/
int MX25_Board_Write(const uint8_t* write, unsigned len, unsigned deassert, spixfc_width_t width)
{

    spixfc_req_t req = {MX25_SSEL,deassert,0,write,NULL, width,len,0,0,NULL};

    return SPIXFC_Trans(MX25_SPI, &req);
}

/******************************************************************************/
int MX25_Clock(unsigned len, unsigned deassert)
{
    return SPIXFC_Clocks(MX25_SPI, len, MX25_SSEL, deassert);
}
