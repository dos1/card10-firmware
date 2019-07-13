/*******************************************************************************
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
 * $Id: max32665_sdma.h 41998 2019-03-29 14:54:55Z kevin.gillespie $
 *
 *******************************************************************************
 */

/**************************************************************************************************
  Macros
**************************************************************************************************/
extern unsigned SystemCoreClock;

/*************************************************************************************************/
/*!
 *  \fn     __enable_irq
 *
 *  \brief  Globally enables all interrupts.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
void __enable_irq(void);

/*************************************************************************************************/
/*!
 *  \fn     __disable_irq
 *
 *  \brief  Globally enables all interrupts.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
void __disable_irq(void);


/*************************************************************************************************/
/*!
 *  \fn     interrupt_arm
 *
 *  \brief  Interrupt ARM core.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
void interrupt_ARM(void);

/*************************************************************************************************/
/*!
 *  \fn     NVIC_EnableIRQ
 *
 *  \brief  Enables given interrupt.
 *
 *  \param  irq Interrupt number. 
 *
 *  \return None.
 */
/*************************************************************************************************/
void NVIC_EnableIRQ(int irq);

/*************************************************************************************************/
/*!
 *  \fn     NVIC_DisableIRQ
 *
 *  \brief  Disables given interrupt.
 *
 *  \param  irq Interrupt number. 
 *
 *  \return None.
 */
/*************************************************************************************************/
void NVIC_DisableIRQ(int irq);

/*************************************************************************************************/
/*!
 *  \fn     NVIC_ClearPendingIRQ
 *
 *  \brief  Clears given interrupt.
 *
 *  \param  irq Interrupt number. 
 *
 *  \return None.
 */
/*************************************************************************************************/
void NVIC_ClearPendingIRQ(int irq);
