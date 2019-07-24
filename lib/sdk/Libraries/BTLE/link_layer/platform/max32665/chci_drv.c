/*******************************************************************************
 * Copyright (C) 2019 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Date: 2019-06-05 10:22:48 -0500 (Wed, 05 Jun 2019) $
 * $Revision: 43683 $
 *
 ******************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief Controller HCI interface to the Host
 */
/*************************************************************************************************/

#include "hci_defs.h"
#include "chci_api.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
extern void hciTrSerialRxIncoming(uint8_t *pBuf, uint8_t len);

/*************************************************************************************************/
void chciDrvInit(void)
{

}

/*************************************************************************************************/
uint16_t chciDrvWrite(uint8_t prot, uint8_t type, uint16_t len, uint8_t *pData)
{
    // Convert CHCI_TR_TYPE to HCI_TYPE
    uint8_t hciType = (type == CHCI_TR_TYPE_EVT) ? HCI_EVT_TYPE : HCI_ACL_TYPE;

    // Write to Host HCI handler
    hciTrSerialRxIncoming(&hciType, 1);
    hciTrSerialRxIncoming(pData, len);
    return len;
}
