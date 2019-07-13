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
 *  \brief Link layer (LL) master extended control interface implementation file.
 */
/*************************************************************************************************/

#include "lctr_api_adv_master_ae.h"
#include "lctr_api_init_master_ae.h"
#include "lmgr_api_adv_master_ae.h"
#include "ll_math.h"
#include "wsf_assert.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "util/bstream.h"

/*************************************************************************************************/
/*!
 *  \brief      Validate extended initiate parameters.
 *
 *  \param      pParam    Pointer to initiation parameters.
 *
 *  \return     TRUE if valid, FALSE otherwise.
 */
/*************************************************************************************************/
static bool_t llValidateInitiateParams(const LlExtInitParam_t *pParam)
{
  const uint8_t filtPolicyMax = ((lmgrCb.features & LL_FEAT_EXT_SCAN_FILT_POLICY) != 0) ? LL_SCAN_FILTER_WL_OR_RES_INIT : LL_SCAN_FILTER_WL_BIT;
  const uint8_t addrTypeMax = ((lmgrCb.features & LL_FEAT_PRIVACY) != 0) ? LL_ADDR_RANDOM_IDENTITY : LL_ADDR_RANDOM;

  if ((LL_API_PARAM_CHECK == 1) &&
     ((pParam->ownAddrType > addrTypeMax) ||
      (pParam->peerAddrType > addrTypeMax) ||
      (pParam->filterPolicy > filtPolicyMax) ||
      !LmgrIsAddressTypeAvailable(pParam->ownAddrType)))
  {
    return FALSE;
  }

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief      Validate extended initiate scan parameters.
 *
 *  \param      pParam    Pointer to scan parameters.
 *
 *  \return     TRUE if valid, FALSE otherwise.
 */
/*************************************************************************************************/
static bool_t llValidateInitiateScanParams(const LlExtInitScanParam_t *pParam)
{
  const uint16_t rangeMin = 0x0004;         /*  2.5 ms */
  /* const uint16_t rangeMax = 0xFFFF; */   /* 40.959375 ms */

  if ((LL_API_PARAM_CHECK == 1) &&
     ((pParam->scanInterval < pParam->scanWindow) || (pParam->scanWindow < rangeMin)))
  {
    return FALSE;
  }

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief      Extended create connection.
 *
 *  \param      pInitParam      Initiating parameters.
 *  \param      scanParam       Scan parameters table indexed by PHY.
 *  \param      connSpec        Connection specification table indexed by PHY.
 *
 *  \return     Status error code.
 *
 *  Create a connection to the specified peer address with the specified connection parameters.
 *  This function is only when operating in master role.
 */
/*************************************************************************************************/
uint8_t LlExtCreateConn(const LlExtInitParam_t *pInitParam, const LlExtInitScanParam_t scanParam[], const LlConnSpec_t connSpec[])
{
  unsigned int i;
  lctrExtInitiateMsg_t *pMsg;
  const uint8_t validInitPhys = LL_PHYS_LE_1M_BIT | LL_PHYS_LE_2M_BIT | LL_PHYS_LE_CODED_BIT;
  const uint8_t numInitPhyBits = LlMathGetNumBitsSet(pInitParam->initPhys);

  LL_TRACE_INFO1("### LlApi ###  LlExtCreateConn: initPhys=0x%02x", pInitParam->initPhys);

  WSF_ASSERT(pInitParam);   /* not NULL */

  if ((LL_API_PARAM_CHECK == 1) &&
      (!LmgrIsExtCommandAllowed() ||
      lmgrCb.numInitEnabled))
  {
    return LL_ERROR_CODE_CMD_DISALLOWED;
  }

  if ((LL_API_PARAM_CHECK == 1) &&
      ((pInitParam->initPhys & ~validInitPhys) || (pInitParam->initPhys == 0)))
  {
    return LL_ERROR_CODE_INVALID_HCI_CMD_PARAMS;
  }

  if ((LL_API_PARAM_CHECK == 1) &&
      (!llValidateInitiateParams(pInitParam)))
  {
    return LL_ERROR_CODE_INVALID_HCI_CMD_PARAMS;
  }

  if (LL_API_PARAM_CHECK == 1)
  {
    for (i = 0; i < numInitPhyBits; i++)
    {
      if (!llValidateInitiateScanParams(&scanParam[i]))
      {
        return LL_ERROR_CODE_INVALID_HCI_CMD_PARAMS;
      }
    }
  }

  lmgrCb.numExtScanPhys = 0;
  i = 0;

  if (pInitParam->initPhys & LL_PHYS_LE_1M_BIT)
  {
    LctrMstExtInitSetScanPhy(LCTR_SCAN_PHY_1M);
    lmgrCb.numExtScanPhys++;
    LctrMstExtInitParam(LCTR_SCAN_PHY_1M, &scanParam[i], &connSpec[i]);
    i++;
  }
  else
  {
    LctrMstExtInitClearScanPhy(LCTR_SCAN_PHY_1M);
    /* Provide default connSpec. */
    LctrMstExtInitParam(LCTR_SCAN_PHY_1M, NULL, &connSpec[0]);
  }

  if (pInitParam->initPhys & LL_PHYS_LE_2M_BIT)
  {
    /* Skip scanner settings, 2M PHY scanning not allowed; store only the 2M PHY connSpec. */
    LctrMstExtInitParam(LCTR_INIT_PHY_2M, NULL, &connSpec[i]);
    i++;
  }
  else
  {
    /* Provide default connSpec. */
    LctrMstExtInitParam(LCTR_INIT_PHY_2M, NULL, &connSpec[0]);
  }

  if (pInitParam->initPhys & LL_PHYS_LE_CODED_BIT)
  {
    LctrMstExtInitSetScanPhy(LCTR_SCAN_PHY_CODED);
    lmgrCb.numExtScanPhys++;
    LctrMstExtInitParam(LCTR_SCAN_PHY_CODED, &scanParam[i], &connSpec[i]);
    i++;
  }
  else
  {
    LctrMstExtInitClearScanPhy(LCTR_SCAN_PHY_CODED);
    /* Provide default connSpec. */
    LctrMstExtInitParam(LCTR_SCAN_PHY_CODED, NULL, &connSpec[0]);
  }

  if ((pMsg = WsfMsgAlloc(sizeof(*pMsg))) != NULL)
  {
    pMsg->hdr.dispId = LCTR_DISP_EXT_INIT;
    pMsg->hdr.event = LCTR_EXT_INIT_MSG_INITIATE;

    pMsg->filterPolicy = pInitParam->filterPolicy;
    pMsg->ownAddrType = pInitParam->ownAddrType;
    pMsg->peerAddrType = pInitParam->peerAddrType;
    pMsg->peerAddr = BstreamToBda64(pInitParam->pPeerAddr);
    pMsg->initPhys = pInitParam->initPhys;

    WsfMsgSend(lmgrPersistCb.handlerId, pMsg);
  }

  return LL_SUCCESS;
}
