/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                        (c) Copyright 2023, i3 Product Development
 *
 * \brief reach_silabs.h/.c provides a SiLabs specific implementation of the required
 *      Reach functionality.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "reach_silabs.h"
#include "reach-server.h"
#include "cr_stack.h"
#include "i3_log.h"
#include "version.h"

#include "sl_bluetooth.h"
#include "em_common.h"
#include "app_assert.h"
#include "em_cmu.h"
#include "gatt_db.h"
#include "app.h"


static uint8_t  sRsl_ble_connection = 0;
static uint16_t sRsl_ble_characteristic = 0;
static bool     sRsl_ble_subscribed = false;

static uint32_t sNotifyCount = 0;
static uint32_t sNotifyDelay = 0;
static uint32_t sNotifyMaxDelay = 0;

void rsl_inform_connection(uint8_t connection, uint16_t characteristic)
{
  sRsl_ble_connection = connection;
  sRsl_ble_characteristic = characteristic;
  if (connection == 0)
      sRsl_ble_subscribed = false;
}

void rsl_inform_subscribed(bool subscribed)
{
    sRsl_ble_subscribed = subscribed;
}

int rsl_stats()
{
#if 0
    extern uint32_t gBytesWritten, gLastOffset, gWfPacketCount;
    extern char gRfLoop;

    int avg = sNotifyDelay/sNotifyCount;
    i3_log(LOG_MASK_BLE, "  wf: %d bytes, %d packets.", gBytesWritten, gWfPacketCount);
    i3_log(LOG_MASK_BLE, "  rf: %d notifications, in groups of %d, retries avg %d, max %d\n",
                 sNotifyCount, gRfLoop, avg, sNotifyMaxDelay);

    sNotifyCount = 0;
    sNotifyDelay = 0;
    sNotifyMaxDelay = 0;

    gBytesWritten = 0;
    gLastOffset = 0;
    gWfPacketCount = 0;
#endif
    return 0;
}

void rsl_init()
{
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!!");
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Cygnus Reach Protobuf Server");
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! (c) 2023 i3 Product Design, All Rights Reserved");
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Built %s, %s. Version %d.%d.%d", 
           __DATE__, __TIME__, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
  #ifdef ENABLE_REMOTE_CLI
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Remote CLI support built in.");
  #else
    i3_log(LOG_MASK_ALWAYS, TEXT_YELLOW "!!! Remote CLI support EXCLUDED from the build.");
  #endif
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!!");

    cr_test_sizes();

    // Local init to emulate a parameter respository
    extern void init_param_repo();
    init_param_repo();

    i3_log(LOG_MASK_ALWAYS, "Enter 'help' to see available commands.");
}


int crcb_cli_enter(const char *ins)
{
    bool handled = false;
    if (*ins == 0xA)
    {
        handled = true;
    }
    else if ((*ins == '?') || (!strncmp("help", ins, 4)))
    {
        i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Cygnus Reach Server, built %s, %s", __DATE__, __TIME__);
        i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Version %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "Commands:");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  ? or help to see this command.");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  lm <x> to change the log mask (hex).");
        handled = true;
    }
    else if (!strncmp("rcli", ins, 4))
    {
      #ifndef ENABLE_REMOTE_CLI
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI is not enabled at compile time.");
      #else
        if (!strncmp("rcli on", ins, 7))
        {
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Enabled remote CLI echo.");
            i3_log_set_remote_cli_enable(true);
        }
        else if (!strncmp("rcli off", ins, 7))
        {
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Disabled remote CLI echo.");
            i3_log_set_remote_cli_enable(false);
        }
        else
        {
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI echo is %s.",
                   i3_log_get_remote_cli_enable() ? "ENABLED" : "DISABLED");
            
        }
      #endif
        handled = true;
    }
    else if (!strncmp("lm", ins, 2))
    {
        unsigned int logMask = i3_log_get_mask();
        int numRead = sscanf(&ins[3], "%x", &logMask);
        if (numRead == 1)
        {
            i3_log_set_mask(logMask);
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "Log mask/Trace level set to 0x%x", logMask);
        }
        else
        {
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rx %s, numRead is %d", ins, numRead);
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "Log mask/Trace level is 0x%x", logMask);
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  ALWAYS, ERROR and WARN are enabled");
            if (logMask & LOG_MASK_WEAK) 
                i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  mask 0x%x: WEAK is enabled", LOG_MASK_WEAK);
            else
                i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: WEAK is disabled", LOG_MASK_WEAK);
            if (logMask & LOG_MASK_WIRE) 
                i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  mask 0x%x: WIRE is enabled", LOG_MASK_WIRE);
            else
                i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: WIRE is disabled", LOG_MASK_WIRE);
            if (logMask & LOG_MASK_REACH) 
                i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  mask 0x%x: REACH is enabled", LOG_MASK_REACH);
            else
                i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: REACH is disabled", LOG_MASK_REACH);
            if (logMask & LOG_MASK_PARAMS) 
                i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  mask 0x%x: PARAMS is enabled", LOG_MASK_PARAMS);
            else
                i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: PARAMS is disabled", LOG_MASK_PARAMS);
            if (logMask & LOG_MASK_FILES) 
                i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  mask 0x%x: FILES is enabled", LOG_MASK_FILES);
            else
                i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: FILES is disabled", LOG_MASK_FILES);
        }
        handled = true;
    }
    if (!handled)
        i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI 
               "CLI command '%s' not recognized (0x%x).", ins, *ins);
    i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI ">");
    return 0;
}

int rsl_notify_client(uint8_t *data, size_t len)
{
    sl_status_t rval;

    i3_log(LOG_MASK_BLE, "%s(%d)", __FUNCTION__, len);
    sNotifyCount++;
    uint32_t loopCount = 0;
    do
    {
        rval = sl_bt_gatt_server_send_notification(sRsl_ble_connection,
                                                   sRsl_ble_characteristic,
                                                   len,
                                                   (uint8_t*)data);
        loopCount++;
        if (loopCount > 3000)
            break;
    }
    while (rval != SL_STATUS_OK);

    sNotifyDelay += loopCount;
    if (loopCount > sNotifyMaxDelay)
        sNotifyMaxDelay = loopCount;

    return (int)rval;
}

// The cr_process function calls crcb_send_coded_response() to send responses to the client.
// Returns zero on success.
int crcb_send_coded_response(const uint8_t *respBuf, size_t respSize)
{
    if (respSize == 0)
    {
        i3_log(LOG_MASK_REACH, "%s: No bytes to send.  ", __FUNCTION__);
        return CR_ERROR_NONE;
    }
    int rval = 0;
    i3_log(LOG_MASK_REACH, TEXT_GREEN "%s: send %d bytes.", __FUNCTION__, respSize);

    if (sRsl_ble_subscribed)
    {   int s1;
        i3_log(LOG_MASK_BLE, "%s: call rsl_notify_client() with %d bytes", __FUNCTION__, respSize);
        s1 = rsl_notify_client((uint8_t*)respBuf, respSize);
        switch (s1)
        {
        case SL_STATUS_OK:  // 0
            i3_log(LOG_MASK_BLE, "Sent notification %d bytes, OK.", respSize);
            rval = 0;
            break;
        case SL_STATUS_COMMAND_TOO_LONG:
            LOG_ERROR("Response of %d bytes is too long. Return %d", 
                      respSize, CR_ERROR_BUFFER_TOO_SMALL);
            rval = CR_ERROR_BUFFER_TOO_SMALL;
            break;
        case SL_STATUS_INVALID_HANDLE:
            LOG_ERROR("Invalid Handle. Return %d", CR_ERROR_INVALID_PARAMETER);
            rval = CR_ERROR_INVALID_PARAMETER;
            break;
        default:
            LOG_ERROR("Response of %d bytes failed with status 0x%x.\n", 
                             respSize, s1);
            rval = CR_ERROR_INVALID_PARAMETER;
        }
        return rval;
    }
    return CR_ERROR_NONE;
}

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

#define UPDATE_APP_TIMER_MS 50
uint32_t time_since_startup = 0;

sl_sleeptimer_timer_handle_t sleepTimerHandle;
void app_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
    (void)handle;
    (void)data;
    //Code executed when the timer expires.
    time_since_startup++;
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void rsl_app_init(void)
{
    sl_status_t status;

#if 1
    // this clears a line between runs.
    // init_cygnus_cli() prints a banner.
    i3_log(LOG_MASK_ALWAYS, "\n\n**\n");
#else
    i3_log(LOG_MASK_ALWAYS, "\n\n**");
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "Cygnus Reach Device v%d.%d.%d",
           MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "Built %s, %s", __DATE__, __TIME__);
    i3_log(LOG_MASK_ALWAYS, "**\n");
#endif
    // Add the CLI commands implemented in cli_functions.c
    extern void init_cygnus_cli();
    init_cygnus_cli();

    // Add a timer for Reach
    sl_sleeptimer_init();
    uint32_t timer_timeout = UPDATE_APP_TIMER_MS;
    status = sl_sleeptimer_start_periodic_timer_ms(&sleepTimerHandle,
                                                   timer_timeout,
                                                   app_timer_callback,
                                                   (void *)NULL, 0, 0);
    app_assert_status(status);
    CMU_ClockEnable(cmuClock_RTCC, true);
    
    rsl_init();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void rsl_app_process_action(void)
{
    // process reach stack
    cr_process(time_since_startup * UPDATE_APP_TIMER_MS);
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 * 
 * Reach expects a Reach service advertising UUID
 * edd59269-79b3-4ec2-a6a2-89bfb640f930 and a REACH
 * characteristic with UUID
 * d42d1039-1d11-4f10-bae6-5f3b44cf6439.
 * Support Read, Write, Write without response and notify.
 * Do not support Reliable write or indicate.
 * Maximum length is 255.
 *****************************************************************************/
void rsl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;


    switch (SL_BT_MSG_ID(evt->header))
    {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
        {
            extern char gPhy;

            sl_bt_connection_set_preferred_phy(evt->data.evt_connection_opened.connection, gPhy, gPhy);
            i3_log(LOG_MASK_ALWAYS, "Device connected to BLE with connection ID %u. %dM Phy requested",
                   evt->data.evt_connection_opened.connection, gPhy);

            rsl_inform_connection(evt->data.evt_connection_opened.connection, REACH_BLE_CHARICTERISTIC_ID);
            break;
        }

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      i3_log(LOG_MASK_ALWAYS, "Device disconnected from BLE");
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);

      rsl_inform_connection(0, REACH_BLE_CHARICTERISTIC_ID);

      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_gatt_server_user_read_request_id:
        {
            sl_bt_evt_gatt_server_user_read_request_t *data = &evt->data.evt_gatt_server_user_read_request;
            i3_log(LOG_MASK_BLE, "sl_bt_evt_gatt_server_user_read_request_id %d", data->characteristic);

            if (data->characteristic == REACH_BLE_CHARICTERISTIC_ID)
            {
                uint8_t *resp_buf;
                size_t resp_len;

                cr_get_coded_response_buffer(&resp_buf, &resp_len);
                i3_log(LOG_MASK_BLE, "Read request for reach. %d.", resp_len);

                int rval = sl_bt_gatt_server_send_user_read_response(data->connection,
                                                                     data->characteristic,
                                                                     0, // uint8_t att_errorcode,
                                                                     resp_len, // size_t value_len,
                                                                     resp_buf, // const uint8_t* value,
                                                                     NULL);
                if (rval != 0)
                {
                    i3_log(LOG_MASK_ERROR, "Reach user read responded. rval 0x%x.", rval);
                }
                break;
            }
            i3_log(LOG_MASK_ERROR, "parameter read request not for Reach.");
            break;
        }

    case sl_bt_evt_gatt_server_characteristic_status_id:
        {
            sl_bt_evt_gatt_server_characteristic_status_t *data = &evt->data.evt_gatt_server_characteristic_status;

            if (data->characteristic == REACH_BLE_CHARICTERISTIC_ID)
            {
                if (data->status_flags == gatt_server_client_config)
                {
                    if (data->client_config_flags == gatt_notification)
                    {
                        i3_log(LOG_MASK_ALWAYS, "Subscribed to REACH notifications");
                        rsl_inform_subscribed(true);
                    }
                    else
                    {
                        i3_log(LOG_MASK_BLE, "sl_bt_evt_gatt_server_characteristic_status_id: Unsubscribed from REACH notifications");
                        rsl_inform_subscribed(false);
                    }
                }
                break;
            }
            i3_log(LOG_MASK_ERROR, "Failed to handle non-REACH BLE configuration change");
            break;
        }

    case sl_bt_evt_gatt_server_user_write_request_id:
        {
            sl_bt_evt_gatt_server_attribute_value_t *data = &evt->data.evt_gatt_server_attribute_value;

            if (data->attribute == REACH_BLE_CHARICTERISTIC_ID)  // reach
            {
                i3_log(LOG_MASK_ALWAYS, "Write to reach.  Len %d", data->value.len);
                cr_store_coded_prompt(data->value.data, data->value.len);
                break;
            }

            i3_log(LOG_MASK_ERROR, "Failed to handle non-REACH parameter write request (0x%x)");
            break;
        }
    case sl_bt_evt_connection_parameters_id:
        {
            sl_bt_evt_connection_parameters_t *data = &evt->data.evt_connection_parameters;

            i3_log(LOG_MASK_BLE, "connection %d. interval %d. latency %d. timeout %d. secure %d, txsize %d",
                   data->connection,    /**< Connection handle */
                   data->interval,      /**< Connection interval. Time = Value x 1.25 ms */
                   data->latency,       /**< Peripheral latency (how many connection intervals the peripheral can skip) */
                   data->timeout,       /**< Supervision timeout. Time = Value x 10 ms */
                   data->security_mode, /** Security mode. Values:
                                           (0x0): No security
                                           (0x1): Unauthenticated pairing with encryption
                                           (0x2): Authenticated pairing with encryption
                                           (0x3): Authenticated Secure Connections pairing with 128-bit encryption */
                   data->txsize);       /**< Maximum Data Channel PDU Payload size that the controller can send in an air packet */

            break;
        }

    case sl_bt_evt_connection_phy_status_id:
        {
            sl_bt_evt_connection_phy_status_t *data = &evt->data.evt_connection_phy_status;
            switch (data->phy)
            {   /**< Current active PHY. See values from @ref sl_bt_connection_set_preferred_phy command. */
            case 1:
                i3_log(LOG_MASK_ALWAYS, TEXT_YELLOW "Slow 1M PHY" TEXT_RESET);
                break;
            case 2:
                i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "Fast 2M PHY" TEXT_RESET);
                break;
            case 4:
                i3_log(LOG_MASK_ALWAYS, TEXT_YELLOW "Slow 125k coded PHY" TEXT_RESET);
                break;
            case 8:
                i3_log(LOG_MASK_ALWAYS, TEXT_YELLOW "Slow 500k coded PHY" TEXT_RESET);
                break;
            default:
                i3_log(LOG_MASK_ERROR, "Undefined BLE PHY 0x%x", data->phy);
                break;
            }
            break;
        }
    case sl_bt_evt_connection_remote_used_features_id:
        // i3_log(LOG_MASK_BLE, "sl_bt_evt_connection_remote_used_features_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
        // i3_log(LOG_MASK_BLE, "sl_bt_evt_gatt_mtu_exchanged_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;

    case sl_bt_evt_gatt_server_attribute_value_id:
        {
            sl_bt_evt_gatt_server_attribute_value_t *data = &evt->data.evt_gatt_server_attribute_value;
            if (data->attribute == REACH_BLE_CHARICTERISTIC_ID)  // reach
            {
                i3_log(LOG_MASK_BLE, "Attribute Write to reach.  Len %d", data->value.len);
                cr_store_coded_prompt(data->value.data, data->value.len);
                break;
            }
            i3_log(LOG_MASK_ALWAYS, "attribute 0x%x.  Len %d", data->attribute, data->value.len);
            break;
        }
    case sl_bt_evt_gatt_server_execute_write_completed_id:
        {
            sl_bt_evt_gatt_server_execute_write_completed_t *data = &evt->data.evt_gatt_server_execute_write_completed;
            i3_log(LOG_MASK_WARN, "write completed result 0x%x.", data->result);
            break;
        }
  #if 1
    case sl_bt_evt_gatt_procedure_completed_id:
        i3_log(LOG_MASK_BLE, "sl_bt_evt_gatt_procedure_completed_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_evt_user_message_to_host_id:
        i3_log(LOG_MASK_BLE, "sl_bt_evt_user_message_to_host_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_cmd_gatt_server_send_notification_id:
        i3_log(LOG_MASK_BLE, "sl_bt_cmd_gatt_server_send_notification_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
  #endif
        // -------------------------------
        // Default event handler.

    case sl_bt_evt_system_resource_exhausted_id:
    {
        // I hit this when doing a read very quickly.
        // We have buffer errors.  I believe they are benign, just causing some delay.
        // The comment suggests changing SL_BT_CONFIG_BUFFER_SIZE, but that didn't help.
        sl_bt_evt_system_resource_exhausted_t *data = &evt->data.evt_system_resource_exhausted;
        i3_log(LOG_MASK_WARN, "evt_system_resource_exhausted: disc %d, buf %d, heap %d.",
               data ->num_buffers_discarded, 
               data->num_buffer_allocation_failures, 
               data->num_heap_allocation_failures);
        break;
    }
    case sl_bt_evt_connection_data_length_id:
        // normal when phy changes to 2M.
        break;

    default:
        i3_log(LOG_MASK_ERROR, "Event 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    }
}

