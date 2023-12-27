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
#include "nvm3_default.h"
#include "app.h"

static uint8_t  sRsl_ble_connection = 0;
static uint16_t sRsl_ble_characteristic = 0;
static bool     sRsl_ble_subscribed = false;
static int8_t   sRsl_rssi = 0;

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

uint8_t rsl_get_connection(void)
{
    return sRsl_ble_connection; 
}

int rsl_get_rssi(void)
{
    return (int)sRsl_rssi;
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
    I3_LOG(LOG_MASK_BLE, "  wf: %d bytes, %d packets.", gBytesWritten, gWfPacketCount);
    I3_LOG(LOG_MASK_BLE, "  rf: %d notifications, in groups of %d, retries avg %d, max %d\n",
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

// This CLI handler for remote access mirrors that found in silabs_cli.c for local access.
// It calls the same functions but uses a different parser.
int crcb_cli_enter(const char *ins)
{
    if (*ins == 0xA)
    {
        i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI ">");
        return 0;
    }

    if ((*ins == '?') || (!strncmp("help", ins, 4)) )
    {
        i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Cygnus Reach Server, built %s, %s", __DATE__, __TIME__);
        i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "!!! Version %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "Commands:");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  ver : Print versions");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  /   : Display status");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  lm  : Log Mask: lm 0xXXX");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  rcli: Remote CLI <on|off>");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  phy : phy <1|2> BLE PHY 1M or 2M");
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "  nvm : nvm <?|clear|init>");
        i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI ">");
        return 0;
    }

    crcb_set_command_line(ins);
    // step through remote_command_table and execute if matching
    if (!strncmp("ver", ins, 3))
        print_versions();
    else if (!strncmp("/", ins, 1))
        slash(NULL);
    else if (!strncmp("lm", ins, 2))
        cli_lm(NULL);
    else if (!strncmp("rcli", ins, 4))
        cli_rcli(NULL);
    else if (!strncmp("phy", ins, 3))
        cli_phy(NULL);
    else if (!strncmp("nvm", ins, 3))
        cli_nvm(NULL);
    else if (!strncmp("sn", ins, 2))
        cli_sn(NULL);
    else
        i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI 
               "CLI command '%s' not recognized (0x%x).", ins, *ins);

    i3_log(LOG_MASK_ALWAYS | LOG_MASK_BARE, TEXT_CLI ">");
    return 0;
}

int rsl_notify_client(uint8_t *data, size_t len)
{
    sl_status_t rval;

    I3_LOG(LOG_MASK_BLE, "%s(%d)", __FUNCTION__, len);
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
        I3_LOG(LOG_MASK_REACH, "%s: No bytes to send.  ", __FUNCTION__);
        return cr_ErrorCodes_NO_ERROR;
    }
    int rval = 0;
    I3_LOG(LOG_MASK_REACH, TEXT_GREEN "%s: send %d bytes.", __FUNCTION__, respSize);

    if (sRsl_ble_subscribed)
    {   int s1;
        I3_LOG(LOG_MASK_BLE, "%s: call rsl_notify_client() with %d bytes", __FUNCTION__, respSize);
        s1 = rsl_notify_client((uint8_t*)respBuf, respSize);
        switch (s1)
        {
        case SL_STATUS_OK:  // 0
            I3_LOG(LOG_MASK_BLE, "Sent notification %d bytes, OK.", respSize);
            rval = 0;
            break;
        case SL_STATUS_COMMAND_TOO_LONG:
            LOG_ERROR("Response of %d bytes is too long. Return %d", 
                      respSize, cr_ErrorCodes_BUFFER_TOO_SMALL);
            rval = cr_ErrorCodes_BUFFER_TOO_SMALL;
            break;
        case SL_STATUS_INVALID_HANDLE:
            LOG_ERROR("Invalid Handle. Return %d", cr_ErrorCodes_INVALID_PARAMETER);
            rval = cr_ErrorCodes_INVALID_PARAMETER;
            break;
        default:
            LOG_ERROR("Response of %d bytes failed with status 0x%x.\n", 
                             respSize, s1);
            rval = cr_ErrorCodes_INVALID_PARAMETER;
        }
        return rval;
    }
    return cr_ErrorCodes_NO_ERROR;
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

    // this clears a line between runs.
    // init_cygnus_cli() prints a banner.
    i3_log(LOG_MASK_ALWAYS, "\n\n**\n");

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
    extern void generate_data_for_notify(uint32_t timestamp);

    uint32_t timestamp = time_since_startup * UPDATE_APP_TIMER_MS;
    generate_data_for_notify(timestamp);
    // process reach stack
    cr_process(timestamp);
}

extern sli_bt_gattdb_attribute_chrvalue_t gattdb_attribute_field_10;

const char *rsl_get_advertised_name() 
{
    return (char*)(gattdb_attribute_field_10.data);
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
    {
        bd_addr address;
        uint8_t address_type;
        uint8_t sysId[8];

        // I3_LOG(LOG_MASK_BLE, "sl_bt_evt_system_boot_id 0x%x", SL_BT_MSG_ID(evt->header));
        // Extract unique ID from BT Address.
        sc = sl_bt_system_get_identity_address(&address, &address_type);
        app_assert_status(sc);

        // Pad and reverse unique ID to get System ID.
        sysId[0] = address.addr[5];
        sysId[1] = address.addr[4];
        sysId[2] = address.addr[3];
        sysId[3] = 0xFF;
        sysId[4] = 0xFE;
        sysId[5] = address.addr[2];
        sysId[6] = address.addr[1];
        sysId[7] = address.addr[0];

        sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id, 0,
                                                     sizeof(sysId),
                                                     sysId);
        app_assert_status(sc);
        I3_LOG(LOG_MASK_BLE, "BLE system ID %02X:%02X:%02X:%02X:%02X:%02X",
               sysId[0], sysId[1], sysId[2], sysId[5], sysId[6], sysId[7]);


        memset(gattdb_attribute_field_10.data, 0, gattdb_attribute_field_10.max_len);
        unsigned int sn = 0;
        int rval = rsl_read_serial_number(&sn);
        if (rval != 0) {
            gattdb_attribute_field_10.len =
                snprintf((char*)(gattdb_attribute_field_10.data),
                         gattdb_attribute_field_10.max_len,
                         "Reacher %02X:%02X:%02X", sysId[5], sysId[6], sysId[7]);
        }
        else {
            gattdb_attribute_field_10.len =
                snprintf((char*)(gattdb_attribute_field_10.data),
                         gattdb_attribute_field_10.max_len,
                         "Reacher SN-%u", sn);
        }
        i3_log(LOG_MASK_ALWAYS, TEXT_YELLOW "Advertise non-extended name %s, len %d of %d", 
               gattdb_attribute_field_10.data, 
               gattdb_attribute_field_10.len, 
               gattdb_attribute_field_10.max_len);

        sc = sl_bt_advertiser_create_set(&advertising_set_handle);
        app_assert_status(sc);

        // Generate data for advertising
        sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                   sl_bt_advertiser_general_discoverable);
        app_assert_status(sc);

        // Set advertising interval to 100ms.
        sc = sl_bt_advertiser_set_timing(advertising_set_handle, 
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
    }

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
    {
        extern char gPhy;

        sl_bt_connection_set_preferred_phy(evt->data.evt_connection_opened.connection, gPhy, gPhy);
        i3_log(LOG_MASK_ALWAYS, "Device connected to BLE with connection ID %u. %dM Phy requested",
               evt->data.evt_connection_opened.connection, gPhy);

        rsl_inform_connection(evt->data.evt_connection_opened.connection, REACH_BLE_CHARICTERISTIC_ID);
        sl_bt_connection_get_rssi(evt->data.evt_connection_opened.connection);
        break;
    }

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      i3_log(LOG_MASK_ALWAYS, "Device disconnected from BLE, restart advertising");

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 advertiser_general_discoverable);
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
            I3_LOG(LOG_MASK_BLE, "sl_bt_evt_gatt_server_user_read_request_id %d", data->characteristic);

            if (data->characteristic == REACH_BLE_CHARICTERISTIC_ID)
            {
                uint8_t *resp_buf;
                size_t resp_len;

                cr_get_coded_response_buffer(&resp_buf, &resp_len);
                I3_LOG(LOG_MASK_BLE, "Read request for reach. %d.", resp_len);

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
                        I3_LOG(LOG_MASK_BLE, "sl_bt_evt_gatt_server_characteristic_status_id: Unsubscribed from REACH notifications");
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

            I3_LOG(LOG_MASK_BLE, "connection %d. interval %d. latency %d. timeout %d. secure %d, txsize %d",
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
        // I3_LOG(LOG_MASK_BLE, "sl_bt_evt_connection_remote_used_features_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
        // I3_LOG(LOG_MASK_BLE, "sl_bt_evt_gatt_mtu_exchanged_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;

    case sl_bt_evt_gatt_server_attribute_value_id:
        {
            sl_bt_evt_gatt_server_attribute_value_t *data = &evt->data.evt_gatt_server_attribute_value;
            if (data->attribute == REACH_BLE_CHARICTERISTIC_ID)  // reach
            {
                I3_LOG(LOG_MASK_BLE, "Attribute Write to reach.  Len %d", data->value.len);
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
        I3_LOG(LOG_MASK_BLE, "sl_bt_evt_gatt_procedure_completed_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_evt_user_message_to_host_id:
        I3_LOG(LOG_MASK_BLE, "sl_bt_evt_user_message_to_host_id 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    case sl_bt_cmd_gatt_server_send_notification_id:
        I3_LOG(LOG_MASK_BLE, "sl_bt_cmd_gatt_server_send_notification_id 0x%x", SL_BT_MSG_ID(evt->header));
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

    case sl_bt_evt_connection_rssi_id: 
    {
        // RSSI was requested, this is the response
        sl_bt_evt_connection_rssi_t *data =  &evt->data.evt_connection_rssi;
        // i3_log(LOG_MASK_WARN, "RSSI reports %d.",  data->rssi);
        sRsl_rssi = data->rssi;   // overwrite the previous value
        break;
    }

    default:
        i3_log(LOG_MASK_ERROR, "Event 0x%x", SL_BT_MSG_ID(evt->header));
        break;
    }
}

int rsl_read_serial_number(unsigned int *sn)
{
    size_t dataLen;
    uint32_t objectType;
    Ecode_t eCode;

    nvm3_getObjectInfo(nvm3_defaultHandle, REACH_SN_KEY, &objectType, &dataLen);
    if (objectType != NVM3_OBJECTTYPE_DATA) {
        i3_log(LOG_MASK_WARN, "NVM object type of SN key 0x%x failed, SN not read.", REACH_SN_KEY);
        return -1;
    }
    if (dataLen != sizeof(unsigned int)) {
        i3_log(LOG_MASK_WARN, "dataLen of SN is %d, not %d.", dataLen, sizeof(unsigned int));
    }
    eCode = nvm3_readData(nvm3_defaultHandle, REACH_SN_KEY, (uint8_t *)sn, sizeof(unsigned int));
    if (ECODE_NVM3_OK != eCode) {
        i3_log(LOG_MASK_ERROR, "NVM Read of SN key 0x%x failed with 0x%x.", 
           REACH_SN_KEY, eCode);
        return -2;
    }
    return 0;

}

