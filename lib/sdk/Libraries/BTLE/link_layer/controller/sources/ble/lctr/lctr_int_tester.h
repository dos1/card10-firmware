/* Copyright (c) 2009-2019 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*************************************************************************************************/
/*!
 *  \brief Internal link layer controller connection interface file.
 */
/*************************************************************************************************/

#ifndef LCTR_INT_TESTER_H
#define LCTR_INT_TESTER_H

#include "ll_tester_api.h"
#include "wsf_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

void LctrForceTxAcl(uint8_t *pAclBuf);

/*************************************************************************************************/
/*!
 *  \brief      Force transmission of a data PDU.
 *
 *  \param      connHandle  Connection handle.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void lctrForceTxData(uint16_t connHandle);

#ifdef __cplusplus
};
#endif

#endif /* LCTR_INT_TESTER_H */
