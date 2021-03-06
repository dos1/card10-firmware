/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  GATT profile.
 *
 *  Copyright (c) 2011-2018 Arm Ltd. All Rights Reserved.
 *  Arm Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact Arm Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/
#ifndef GATT_API_H
#define GATT_API_H

#include "att_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup GATT_PROFILE
 *  \{ */

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief Enumeration of handle indexes of characteristics to be discovered */
enum
{
  GATT_SC_HDL_IDX,          /*!< \brief Service changed */
  GATT_SC_CCC_HDL_IDX,      /*!< \brief Service changed client characteristic configuration descriptor */
  GATT_HDL_LIST_LEN         /*!< \brief Handle list length */
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Perform service and characteristic discovery for GATT service.  Note that pHdlList
 *          must point to an array of handles of length \ref GATT_HDL_LIST_LEN.  If discovery is
 *          successful the handles of discovered characteristics and descriptors will be set
 *          in pHdlList.
 *
 *  \param  connId    Connection identifier.
 *  \param  pHdlList  Characteristic handle list.
 *
 *  \return None.
 */
/*************************************************************************************************/
void GattDiscover(dmConnId_t connId, uint16_t *pHdlList);

/*************************************************************************************************/
/*!
 *  \brief  Process a value received in an ATT read response, notification, or indication
 *          message.  Parameter pHdlList must point to an array of length \ref GATT_HDL_LIST_LEN.
 *          If the attribute handle of the message matches a handle in the handle list the value
 *          is processed, otherwise it is ignored.
 *
 *  \param  pHdlList  Characteristic handle list.
 *  \param  pMsg      ATT callback message.
 *
 *  \return \ref ATT_SUCCESS if handle is found, \ref ATT_ERR_NOT_FOUND otherwise.
 */
/*************************************************************************************************/
uint8_t GattValueUpdate(uint16_t *pHdlList, attEvt_t *pMsg);

/*! \} */    /* GATT_PROFILE */

#ifdef __cplusplus
};
#endif

#endif /* GATT_API_H */
