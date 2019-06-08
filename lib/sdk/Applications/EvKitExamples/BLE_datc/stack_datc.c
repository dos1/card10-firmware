/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Stack initialization for datc.
 *
 *  Copyright (c) 2016-2017 ARM Ltd. All Rights Reserved.
 *  ARM Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include "wsf_types.h"
#include "wsf_os.h"
#include "util/bstream.h"

#include "datc/datc_api.h"

#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "svc_dis.h"
#include "svc_core.h"
#include "sec_api.h"

/*************************************************************************************************/
/*!
 *  \brief      Initialize stack.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void StackInitDatc(void)
{
  wsfHandlerId_t handlerId;
  uint8_t features[sizeof(uint64_t)];
  uint8_t mask[sizeof(uint64_t)];

  SecInit();
  SecAesInit();
  SecCmacInit();
  SecEccInit();

  // Only use legacy API.
  Uint64ToBstream(features, 0);
  Uint64ToBstream(mask, HCI_LE_SUP_FEAT_LE_EXT_ADV);
  HciVsSetFeatures(features, mask);

  handlerId = WsfOsSetNextHandler(HciHandler);
  HciHandlerInit(handlerId);

  handlerId = WsfOsSetNextHandler(DmHandler);
  DmDevVsInit(0);
  DmDevPrivInit();
  DmScanInit();
  DmConnInit();
  DmConnMasterInit();
  DmSecInit();
  DmSecLescInit();
  DmPrivInit();
  DmHandlerInit(handlerId);

  L2cInit();
  L2cMasterInit();

  handlerId = WsfOsSetNextHandler(AttHandler);
  AttHandlerInit(handlerId);
  AttsInit();
  AttsIndInit();
  AttcInit();

  handlerId = WsfOsSetNextHandler(SmpHandler);
  SmpHandlerInit(handlerId);
  SmpiInit();
  SmpiScInit();
  HciSetMaxRxAclLen(100);

  handlerId = WsfOsSetNextHandler(AppHandler);
  AppHandlerInit(handlerId);

  handlerId = WsfOsSetNextHandler(DatcHandler);
  DatcHandlerInit(handlerId);
}
