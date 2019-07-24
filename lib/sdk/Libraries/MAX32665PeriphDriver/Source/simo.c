/**
 * @file       simo.c
 * @brief      This file contains the function implementations for the
 *             SIMO peripheral module.
 */

/* *****************************************************************************
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Date: 2018-08-28 17:03:02 -0500 (Tue, 28 Aug 2018) $
 * $Revision: 37424 $
 *
 **************************************************************************** */

/* **** Includes **** */
#include <string.h>
#include <stdio.h>
#include "mxc_config.h"
#include "mxc_assert.h"
#include "mxc_sys.h"
#include "simo.h"

/**
 * @ingroup simo
 * @{
 */

/* **** Definitions **** */

/* **** Globals **** */

/* **** Functions **** */
// All Voltages are in mV

void SIMO_setVregO_A(uint32_t voltage)
{
	uint32_t base_voltage = 0;
	if(MXC_SIMO->vrego_a & MXC_F_SIMO_VREGO_A_RANGEA) {
		base_voltage = VREGO_HIGH_RANGE_BASE;
	} else {
		base_voltage = VREGO_LOW_RANGE_BASE;
	}

	uint32_t setpoint = (voltage-base_voltage)/10;
	uint32_t value = (MXC_SIMO->vrego_a & ~MXC_F_SIMO_VREGO_A_VSETA) | (setpoint & MXC_F_SIMO_VREGO_A_VSETA);

	// Write the SIMO Registers twice due to clock glitch
	MXC_SIMO->vrego_a = value;
	MXC_SIMO->vrego_a = value;

}

void SIMO_setVregO_B(uint32_t voltage)
{
	uint32_t base_voltage = 0;
	if(MXC_SIMO->vrego_b & MXC_F_SIMO_VREGO_B_RANGEB) {
		base_voltage = VREGO_HIGH_RANGE_BASE;
	} else {
		base_voltage = VREGO_LOW_RANGE_BASE;
	}

	uint32_t setpoint = (voltage-base_voltage)/10;
	uint32_t value = (MXC_SIMO->vrego_b & ~MXC_F_SIMO_VREGO_B_VSETB) | (setpoint & MXC_F_SIMO_VREGO_B_VSETB);

	// Write the SIMO Registers twice due to clock glitch
	MXC_SIMO->vrego_b = value;
	MXC_SIMO->vrego_b = value;

}

void SIMO_setVregO_C(uint32_t voltage)
{
	uint32_t base_voltage = 0;
	if(MXC_SIMO->vrego_c & MXC_F_SIMO_VREGO_C_RANGEC) {
		base_voltage = VREGO_HIGH_RANGE_BASE;
	} else {
		base_voltage = VREGO_LOW_RANGE_BASE;
	}
	
	uint32_t setpoint = (voltage-base_voltage)/10;
	uint32_t value = (MXC_SIMO->vrego_c & ~MXC_F_SIMO_VREGO_C_VSETC) | (setpoint & MXC_F_SIMO_VREGO_C_VSETC);

	// Write the SIMO Registers twice due to clock glitch
	MXC_SIMO->vrego_c = value;
	MXC_SIMO->vrego_c = value;
}

void SIMO_setVregO_D(uint32_t voltage)
{
	uint32_t base_voltage = 0;
	if(MXC_SIMO->vrego_d & MXC_F_SIMO_VREGO_D_RANGED) {
		base_voltage = VREGO_HIGH_RANGE_BASE;
	} else {
		base_voltage = VREGO_LOW_RANGE_BASE;
	}

	uint32_t setpoint = (voltage-base_voltage)/10;
	uint32_t value = (MXC_SIMO->vrego_d & ~MXC_F_SIMO_VREGO_D_VSETD) | (setpoint & MXC_F_SIMO_VREGO_D_VSETD);

	// Write the SIMO Registers twice due to clock glitch
	MXC_SIMO->vrego_d = value;
	MXC_SIMO->vrego_d = value;
}

void SIMO_setIpkA(uint32_t peak_current)
{
	// For now, do nothing
	// We don't know how to go from current->reg value
}
void SIMO_setIpkB(uint32_t peak_current)
{
	// For now, do nothing
	// We don't know how to go from current->reg vale
}
void SIMO_setIpkC(uint32_t peak_current)
{
	// For now, do nothing
	// We don't know how to go from current->reg vale
}
void SIMO_setIpkD(uint32_t peak_current)
{
	// For now, do nothing
	// We don't know how to go from current->reg vale
}

void SIMO_setMaxTon(uint32_t ontime)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}

void SIMO_setAlertThresholdA(uint32_t threshold)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setAlertThresholdB(uint32_t threshold)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setAlertThresholdC(uint32_t threshold)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setAlertThresholdD(uint32_t threshold)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}

void SIMO_setZeroCrossCalA(uint32_t zerocross)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setZeroCrossCalB(uint32_t zerocross)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setZeroCrossCalC(uint32_t zerocross)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}
void SIMO_setZeroCrossCalD(uint32_t zerocross)
{
	// Do nothing for now
	// We don't know how to calculate correct values
}

uint32_t SIMO_getOutReadyA(void)
{
	return (MXC_SIMO->buck_out_ready & MXC_F_SIMO_BUCK_OUT_READY_BUCKOUTRDYA) ? 
			E_NO_ERROR : E_BAD_STATE;
}

uint32_t SIMO_getOutReadyB(void)
{
	return (MXC_SIMO->buck_out_ready & MXC_F_SIMO_BUCK_OUT_READY_BUCKOUTRDYB) ? 
			E_NO_ERROR : E_BAD_STATE;
}

uint32_t SIMO_getOutReadyC(void)
{
	return (MXC_SIMO->buck_out_ready & MXC_F_SIMO_BUCK_OUT_READY_BUCKOUTRDYC) ? 
			E_NO_ERROR : E_BAD_STATE;
}

uint32_t SIMO_getOutReadyD(void)
{
	return (MXC_SIMO->buck_out_ready & MXC_F_SIMO_BUCK_OUT_READY_BUCKOUTRDYD) ? 
			E_NO_ERROR : E_BAD_STATE;
}

/**@} end of group simo */
