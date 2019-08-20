/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Application framework user interface.
 *
 *  Copyright (c) 2011-2018 Arm Ltd. All Rights Reserved.
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
#include "wsf_trace.h"
#include "app_ui.h"

/* card10:
 * copied from: lib/sdk/Libraries/BTLE/stack/ble-profiles/sources/apps/app/common/app_ui.c
 *
 * Reason: has several user interactions which we likley have to implement
 */

/* clang-format off */
/* clang-formet turned off for easier diffing against orginal file */
/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief Callback struct */
appUiCback_t appUiCbackTbl;


/*************************************************************************************************/
/*!
 *  \brief  card10 - Should disable encryption. MAXIM bug reported to us in current static library. Requires 
 *                   this to be called before the BTLE app starts making advertisements. Avoids encryption
 *                   rendering the frame unreadable.
 *
 *  \return None.
 */
/*************************************************************************************************/
void llc_api_crypto_disable_tx();


/*************************************************************************************************/
/*!
 *  \brief  Perform a user interface action based on the event value passed to the function.
 *
 *  \param  event   User interface event value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiAction(uint8_t event)
{
  switch (event)
  {
    case APP_UI_NONE:
      /* no action */
      break;

    case APP_UI_RESET_CMPL:
      APP_TRACE_INFO0(">>> Reset complete <<<");
      break;

    case APP_UI_ADV_START:
      llc_api_crypto_disable_tx();
      APP_TRACE_INFO0(">>> Advertising started <<<");
      break;

    case APP_UI_ADV_STOP:
      APP_TRACE_INFO0(">>> Advertising stopped <<<");
      break;

    case APP_UI_SCAN_START:
      APP_TRACE_INFO0(">>> Scanning started <<<");
      break;

    case APP_UI_SCAN_STOP:
      APP_TRACE_INFO0(">>> Scanning stopped <<<");
      break;

    case APP_UI_SCAN_REPORT:
      APP_TRACE_INFO0(">>> Scan data received from peer <<<");
      break;

    case APP_UI_CONN_OPEN:
      APP_TRACE_INFO0(">>> Connection opened <<<");
      break;

    case APP_UI_CONN_CLOSE:
      APP_TRACE_INFO0(">>> Connection closed <<<");
      break;

    case APP_UI_SEC_PAIR_CMPL:
      APP_TRACE_INFO0(">>> Pairing completed successfully <<<");
      break;

    case APP_UI_SEC_PAIR_FAIL:
      APP_TRACE_INFO0(">>> Pairing failed <<<");
      break;

    case APP_UI_SEC_ENCRYPT:
      APP_TRACE_INFO0(">>> Connection encrypted <<<");
      break;

    case APP_UI_SEC_ENCRYPT_FAIL:
      APP_TRACE_INFO0(">>> Encryption failed <<<");
      break;

    case APP_UI_PASSKEY_PROMPT:
      APP_TRACE_INFO0(">>> Prompt user to enter passkey <<<");
      break;

    case APP_UI_ALERT_CANCEL:
      APP_TRACE_INFO0(">>> Cancel a low or high alert <<<");
      break;

    case APP_UI_ALERT_LOW:
      APP_TRACE_INFO0(">>> Low alert <<<");
      break;

    case APP_UI_ALERT_HIGH:
      APP_TRACE_INFO0(">>> High alert <<<");
      break;

    case APP_UI_ADV_SET_START_IND:
      APP_TRACE_INFO0(">>> Advertising set(s) started <<<");
      break;

    case APP_UI_ADV_SET_STOP_IND:
      APP_TRACE_INFO0(">>> Advertising set(s) stopped <<<");
      break;

    case APP_UI_SCAN_REQ_RCVD_IND:
      APP_TRACE_INFO0(">>> Scan request received <<<");
      break;

    case APP_UI_EXT_SCAN_START_IND:
      APP_TRACE_INFO0(">>> Extended scanning started <<<");
      break;

    case APP_UI_EXT_SCAN_STOP_IND:
      APP_TRACE_INFO0(">>> Extended scanning stopped <<<");
      break;

    case APP_UI_PER_ADV_SET_START_IND:
      APP_TRACE_INFO0(">>> Periodic advertising set started <<<");
      break;

    case APP_UI_PER_ADV_SET_STOP_IND:
      APP_TRACE_INFO0(">>> Periodic advertising set stopped <<<");
      break;

    case APP_UI_PER_ADV_SYNC_EST_IND:
      APP_TRACE_INFO0(">>> Periodic advertising sync established <<<");
      break;

    case APP_UI_PER_ADV_SYNC_LOST_IND:
      APP_TRACE_INFO0(">>> Periodic advertising sync lost <<<");
      break;

    default:
      break;
  }

  if (appUiCbackTbl.actionCback)
  {
    (*appUiCbackTbl.actionCback)(event);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Display a passkey.
 *
 *  \param  passkey   Passkey to display.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiDisplayPasskey(uint32_t passkey)
{
  APP_TRACE_INFO1(">>> Passkey: %d <<<", passkey);
}

/*************************************************************************************************/
/*!
*  \brief  Display a confirmation value.
*
*  \param  confirm    Confirm value to display.
*
*  \return None.
*/
/*************************************************************************************************/
void AppUiDisplayConfirmValue(uint32_t confirm)
{
  APP_TRACE_INFO1(">>> Confirm Value: %d <<<", confirm);
}

/*************************************************************************************************/
/*!
 *  \brief  Display an RSSI value.
 *
 *  \param  rssi   Rssi value to display.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiDisplayRssi(int8_t rssi)
{
  APP_TRACE_INFO1(">>> RSSI: %d dBm<<<", rssi);
}

/*************************************************************************************************/
/*!
 *  \brief  Handle a UI timer expiration event.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appUiTimerExpired(wsfMsgHdr_t *pMsg)
{

}

/*************************************************************************************************/
/*!
 *  \brief  Perform button press polling.  This function is called to handle WSF
 *          message APP_BTN_POLL_IND.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appUiBtnPoll(void)
{
  if (appUiCbackTbl.btnPollCback)
  {
    (*appUiCbackTbl.btnPollCback)();
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle a hardware button press.  This function is called to handle WSF
 *          event APP_BTN_DOWN_EVT.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiBtnPressed(void)
{

}

/*************************************************************************************************/
/*!
 *  \brief  Register a callback function to receive application button press events.
 *
 *  \return None.
 *
 *  \note   Registered by application to receive button events
 */
/*************************************************************************************************/
void AppUiBtnRegister(appUiBtnCback_t btnCback)
{
  appUiCbackTbl.btnCback = btnCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register a callback function to receive action events.
 *
 *  \return None.
 *
 *  \note   Registered by platform
 */
/*************************************************************************************************/
void AppUiActionRegister(appUiActionCback_t actionCback)
{
  appUiCbackTbl.actionCback = actionCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register a callback function to receive APP_BTN_POLL_IND events.
 *
 *  \return None.
 *
 *  \note   Registered by platform
 */
/*************************************************************************************************/
void AppUiBtnPollRegister(appUiBtnPollCback_t btnPollCback)
{
  appUiCbackTbl.btnPollCback = btnPollCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Play a sound.
 *
 *  \param  pSound   Pointer to sound tone/duration array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiSoundPlay(const appUiSound_t *pSound)
{

}

/*************************************************************************************************/
/*!
 *  \brief  Stop the sound that is currently playing.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiSoundStop(void)
{

}

/*************************************************************************************************/
/*!
 *  \brief  Button test function-- for test purposes only.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiBtnTest(uint8_t btn)
{
  if (appUiCbackTbl.btnCback)
  {
    (*appUiCbackTbl.btnCback)(btn);
  }
}

/* clang-format on */
