/*******************************************************************************
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
 * $Date: 2019-10-25 14:21:06 -0500 (Fri, 25 Oct 2019) $
 * $Revision: 48094 $
 *
 ******************************************************************************/

#include "lp.h"
#include "icc.h"
#include "simo.h"
#include "board.h"
#include "wut_regs.h"
#include "wsf_types.h"
#include "wsf_timer.h"
#include "usbhs_regs.h"
#include "gcr_regs.h"
#include "mcr_regs.h"
#include "tmr_regs.h"
#include "wut.h"
#include "mxc_sys.h"
#include "bb_drv.h"
#include "wakeup.h"

uint32_t wut_count;
uint32_t wut_snapshot;
uint32_t vrego_d_setting;
uint32_t btleldocn_setting;

#define WAKEUP_32M_US           1105
#define MXC_R_SIR_SHR13         *((uint32_t*)(0x40000434))
#define MXC_R_SIR_SHR17         *((uint32_t*)(0x40000444))

/*************************************************/
void WUT_IRQHandler(void)
{
    WUT_IntClear();
    NVIC_ClearPendingIRQ(WUT_IRQn);
}

/*************************************************/
/* Arm WUT for wakeup from Sleep */
void WUT_SetInt(uint32_t sleep_time)
{
    /* arm WUT for wakeup */
    MXC_WUT->cmp = MXC_WUT->cnt + ((uint64_t)(sleep_time) * SYS_WUT_GetFreq() / BB_CLK_RATE_HZ);   
}

/*************************************************/
/* Arm WUT for wakeup from Sleep, save BB clock */
void WUT_SetWakeup(uint32_t sleep_time)
{
    /* save clocks */
    WUT_Edge();
    WUT_Store();
    /* arm WUT for wakeup */
    WUT_SetInt(sleep_time);
}

/*************************************************/
/*  This will calculate extra delay needed to power up
    after WAIT_TICKS long Sleep
    Input and output are in DBB_TICK units (0.625us per tick)
    This timing depends on how fast VT/RXIN voltage decay/restore,
    i.e. must be characterized per board design

    For now use data points for EvKit_V1 board:
      1) Sleep time ~125ms -> power up delay ~0.5ms
      2) Sleep time ~500ms -> power up delay ~1.5ms
      3) Sleep time ~2 sec -> power up delay ~3.5ms
      4) Sleep time ~8 sec -> power up delay ~5.5ms

    Adding time to power 32 MHz crystal
*/
uint32_t GetWakeDelay(uint32_t wait_ticks)
{
    uint32_t ret;
    if( wait_ticks < US_TO_BBTICKS(125000))
        ret = US_TO_BBTICKS(500);
    else  if( wait_ticks < US_TO_BBTICKS(500000))
        ret = US_TO_BBTICKS(1500);
    else  if( wait_ticks < US_TO_BBTICKS(2000000))
        ret = US_TO_BBTICKS(3500);
    else
        ret = US_TO_BBTICKS(5500);

    ret += US_TO_BBTICKS(WAKEUP_32M_US);

    return ret;
}

/*************************************************/
/* This will switch system clock to HIRC 60MHz source */
static void switchToHIRC(void)
{
    MXC_GCR->clkcn &= ~(MXC_S_GCR_CLKCN_PSC_DIV128);
    MXC_GCR->clkcn |= MXC_S_GCR_CLKCN_PSC_DIV4;
    MXC_GCR->clkcn |= MXC_F_GCR_CLKCN_HIRC_EN;
    MXC_SETFIELD(MXC_GCR->clkcn, MXC_F_GCR_CLKCN_CLKSEL, MXC_S_GCR_CLKCN_CLKSEL_HIRC);
    // Disable unused clocks
    while(!(MXC_GCR->clkcn & MXC_F_GCR_CLKCN_CKRDY)); // Wait for the switch to occur
    MXC_GCR->clkcn &= ~(MXC_F_GCR_CLKCN_HIRC96M_EN);
    SystemCoreClockUpdate();
}

/*************************************************/
/* This will switch system clock to 96MHz source */
static void switchToHIRC96M(void)
{
    MXC_GCR->clkcn &= ~(MXC_S_GCR_CLKCN_PSC_DIV128);
    MXC_GCR->clkcn |= MXC_S_GCR_CLKCN_PSC_DIV1;
    MXC_GCR->clkcn |= MXC_F_GCR_CLKCN_HIRC96M_EN;
    MXC_SETFIELD(MXC_GCR->clkcn, MXC_F_GCR_CLKCN_CLKSEL, MXC_S_GCR_CLKCN_CLKSEL_HIRC96);

    while(!(MXC_GCR->clkcn & MXC_F_GCR_CLKCN_CKRDY)); // Wait for the switch to occur

    // Disable unused clocks
    MXC_GCR->clkcn &= ~(MXC_F_GCR_CLKCN_HIRC_EN);
    SystemCoreClockUpdate();
}

/*************************************************/
/* This will power off unused hardware */
void DisableUnused(void)
{
   // Allow the USB Switch to be turned off in deepsleep and backup modes
   LP_USBSWLPDisable();

   // Shut down all unneeded power gates
   LP_ICacheXIPShutdown();
   LP_CryptoShutdown();
   LP_SysCacheShutdown();
   LP_USBFIFOShutdown();
   LP_ROMShutdown();
   LP_ROM1Shutdown();
   LP_ICache1Shutdown();
}

/*************************************************/
/* This will enter DeepSleep Mode */
void EnterDeepsleep(void)
{
    MXC_GCR->pm &= ~ ( MXC_F_GCR_PM_GPIOWKEN
                       | MXC_F_GCR_PM_RTCWKEN
                       | MXC_F_GCR_PM_USBWKEN
                       | MXC_F_GCR_PM_SDMAWKEN );    // disable other wakeups
    MXC_GCR->pm |= MXC_F_GCR_PM_WUTWKEN;    //  enable WUT wakeup

    //Shutdown unused power domains
    MXC_PWRSEQ->lpcn |= MXC_F_PWRSEQ_LPCN_BGOFF;
    MXC_PWRSEQ->lpcn |= MXC_F_PWRSEQ_LPCN_FWKM;

    if(MXC_GCR->revision == 0xA2) {
        MXC_R_SIR_SHR13 = 0x0;
        MXC_R_SIR_SHR17 &= ~(0xC0);
    }

    ICC_Disable();
    LP_ICacheShutdown();

    LP_VDD3PowerDown();
    LP_VDD4PowerDown();


    /* power down BLE VREGOD and LDOs */
    MXC_PWRSEQ->lpvddpd |= MXC_F_PWRSEQ_LPVDDPD_VREGODPD;
    vrego_d_setting = MXC_SIMO->vrego_d;
    btleldocn_setting = MXC_GCR->btleldocn;
    MXC_GCR->btleldocn = 0x66;
    MXC_SIMO->vrego_d = 0;

    BbDrvDisable();

    // Retain all SRAM
    MXC_PWRSEQ->lpcn |= (MXC_S_PWRSEQ_LPCN_RAMRET_EN3);

    MXC_MCR->ctrl |= MXC_F_MCR_CTRL_VDDCSWEN;
    switchToHIRC();
    SIMO_setVregO_A(1800);
    SIMO_setVregO_B(810);
    SIMO_setVregO_C(810);

    LP_EnterDeepSleepMode();
}

/*************************************************/
/* this will restore from DeepSleep Mode */
void ExitDeepsleep(void)
{
    SIMO_setVregO_B(1100);
    SIMO_setVregO_C(1100);

    MXC_WUT->intr = 1;
    NVIC_ClearPendingIRQ(WUT_IRQn);

    MXC_PWRSEQ->lpvddpd &= ~MXC_F_PWRSEQ_LPVDDPD_VREGODPD;
    MXC_SIMO->vrego_d = vrego_d_setting;
    while(!(MXC_SIMO->buck_out_ready & MXC_F_SIMO_BUCK_OUT_READY_BUCKOUTRDYD)) {}

    MXC_PWRSEQ->lpcn &= ~0x3FE00000; // Disable voltage Monitors for unused rails

    MXC_GCR->pm |= MXC_F_GCR_PM_WUTWKEN;

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    MXC_PWRSEQ->lpcn |= MXC_F_PWRSEQ_LPCN_BCKGRND;
    LP_ICacheWakeup();
    ICC_Enable();

    /* Enable peripheral clocks */
    MXC_GCR->perckcn0 &= ~(MXC_F_GCR_PERCKCN0_GPIO0D | MXC_F_GCR_PERCKCN0_GPIO1D);
    MXC_GCR->perckcn1 &= ~(MXC_F_GCR_PERCKCN1_TRNGD);

    // Restore BTLELDOCN setting
    MXC_GCR->btleldocn = btleldocn_setting;

    // Power up the 32MHz XO
    MXC_GCR->clkcn |= MXC_F_GCR_CLKCN_X32M_EN;

    // Wait for crystal warmup
    WUT_SetInt(US_TO_BBTICKS(WAKEUP_32M_US));
    LP_EnterSleepMode();

    while(!(MXC_GCR->clkcn & MXC_F_GCR_CLKCN_X32M_RDY)) {}

    /* setup the systick */
    SysTick->LOAD = (SystemCoreClock / 1000) * WSF_MS_PER_TICK;
    SysTick->VAL = 0;
    SysTick->CTRL |= (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

    switchToHIRC96M();

    /* enable UART */
    Console_Init();
}
