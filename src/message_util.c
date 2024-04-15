/*
 * Copyright (c) 2023-2024 i3 Product Development
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023-2024
 *
 ********************************************************************************************/

/**
 * @file      message_util.c
 * @brief     Provides functions that help you understand and 
 *            debug Reach message traffic. These utility
 *            functions print out Reach messages in a human
 *            readable format. They are not required and can
 *            be removed by #defining NO_REACH_LOGGING in
 *            reach-server.h. This message logging code takes
 *            about 20k in code space on an ARM.
 * @copyright (c) Copyright 2023-2024 i3 Product Development. 
 *            All Rights Reserved.
 */

#include <stdio.h>

#include "reach-server.h"
#include "message_util.h"
#include "i3_log.h"

// None of these message utilities are required without logging.
#ifndef NO_REACH_LOGGING

static void sLogNumberList(const uint32_t *entries, size_t num)
{
  int remaining_entries = num;
  size_t i=0;
  while (remaining_entries >= 8)
  {
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d, %d, %d, %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2], entries[i+3],
           entries[i+4], entries[i+5], entries[i+6], entries[i+7]);
    remaining_entries -= 8;
    i += 8;
  }

  switch (remaining_entries)
  {
  case 7:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d, %d, %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2], entries[i+3],
           entries[i+4], entries[i+5], entries[i+6]);
    break;
  case 6:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d, %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2], entries[i+3],
           entries[i+4], entries[i+5]);
    break;
  case 5:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2], entries[i+3],
           entries[i+4]);
    break;
  case 4:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2], entries[i+3]);
    break;
  case 3:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d, %d", i,
           entries[i+0], entries[i+1], entries[i+2]);
    break;
  case 2:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d, %d", i,
           entries[i+0], entries[i+1]);
    break;
  case 1:
    i3_log(LOG_MASK_REACH, "      idx %d:  %d", i,
           entries[i+0]);
    break;
  default:
    break;
  }
}

static void sByte_array_to_hex_string(const uint8_t *array, size_t array_size, char *hex_string) {
    size_t i;
    for (i = 0; i < array_size; i++) {
        sprintf(hex_string + (i * 2), "%02X", array[i]);
    }
}

static void sLogParameterValue(const cr_ParameterValue *param)
{
  // To match the apps and protobufs, must use _value_tags!
  // That's why the -3 when logging the type.
  switch (param->which_value)
  {
  case cr_ParameterValue_uint32_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   uint32: %u", param->parameter_id, param->value.uint32_value);
    break;
  case cr_ParameterValue_sint32_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   sint32: %d", param->parameter_id, param->value.sint32_value);
      break;
  case cr_ParameterValue_float32_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   float32: %.2f", param->parameter_id, param->value.float32_value);
      break;
  case cr_ParameterValue_uint64_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   uint64: %llu", param->parameter_id, param->value.uint64_value);
      break;
  case cr_ParameterValue_sint64_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   sint64: %lld", param->parameter_id, param->value.sint64_value);
      break;
  case cr_ParameterValue_float64_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   float64: %.3llf", param->parameter_id, param->value.float64_value);
      break;
  case cr_ParameterValue_bool_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   boolean: %s", param->parameter_id, param->value.bool_value? "true":"false");
      break;
  case cr_ParameterValue_string_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   string: %s", param->parameter_id, param->value.string_value);
      break;
  case cr_ParameterValue_enum_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   enum: %d", param->parameter_id, param->value.enum_value);
      break;
  case cr_ParameterValue_bitfield_value_tag:
    i3_log(LOG_MASK_REACH, "    id : %d.   bitfield: 0x%x", param->parameter_id, param->value.bitfield_value);
      break;
  case cr_ParameterValue_bytes_value_tag:
    {
      char hexStr[65];  // 32 entries plus null
      sByte_array_to_hex_string(param->value.bytes_value.bytes,
                                param->value.bytes_value.size,
                                hexStr);
      i3_log(LOG_MASK_REACH, "    id : %d.   bytes: %s", param->parameter_id, hexStr);
      break;
    }
  default:
      break;
  }
}

//-------------------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------------------

// msg_type_string(cr_ReachMessageTypes_GET_DEVICE_INFO)
const char *msg_type_string(int32_t message_type) {

  switch (message_type) {
  case cr_ReachMessageTypes_INVALID:
      return "Invalid";
  case cr_ReachMessageTypes_ERROR_REPORT:
      return "Error Report";
  case cr_ReachMessageTypes_PING:
    return "Ping";
  case cr_ReachMessageTypes_GET_DEVICE_INFO:
    return "Get Device Info";
  case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
    return "Discover Params";
  case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
    return "Discover Param EX";
  case cr_ReachMessageTypes_READ_PARAMETERS:
    return "Read Param";
  case cr_ReachMessageTypes_WRITE_PARAMETERS:
      return "Write Param";
  case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
      return "Config Param Notifiy";
  case cr_ReachMessageTypes_PARAMETER_NOTIFICATION:
      return "Param Notification";
  case cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS:
      return "Discover Notifications";
  case cr_ReachMessageTypes_DISCOVER_FILES:
    return "Discover Files";
  case cr_ReachMessageTypes_TRANSFER_INIT:
    return "Transfer Init";
  case cr_ReachMessageTypes_TRANSFER_DATA:
    return "Transfer Data";
  case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
    return "Transfer Data Notification";
  case cr_ReachMessageTypes_DISCOVER_COMMANDS:
    return "Discover Commands";
  case cr_ReachMessageTypes_SEND_COMMAND:
    return "Send Command";
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
      return "CLI Notification";
  case cr_ReachMessageTypes_DISCOVER_STREAMS:
    return "Discover Streams";
  case cr_ReachMessageTypes_OPEN_STREAM:
      return "Open Stream";
  case cr_ReachMessageTypes_CLOSE_STREAM:
      return "CLOSE Stream";
  case cr_ReachMessageTypes_STREAM_DATA_NOTIFICATION:
    return "Stream Data Notify";
  case cr_ReachMessageTypes_GET_TIME:
      return "Get Time";
  case cr_ReachMessageTypes_SET_TIME:
      return "Set Time";
  case cr_ReachMessageTypes_DISCOVER_WIFI:
      return "Discover WiFi";
  case cr_ReachMessageTypes_WIFI_CONNECT:
      return "WiFi Connect";
  default:
    break;
  }
  return "Unknown";
}

void message_util_log_device_info_request(cr_DeviceInfoRequest* data) {
  i3_log(LOG_MASK_REACH, "  Device Info Request:");
  i3_log(LOG_MASK_REACH, "    client_protocol_version: '%s'", data->client_protocol_version);
  if (data->has_challenge_key)
    i3_log(LOG_MASK_REACH, "    Challenge key '%s'", data->challenge_key);
  else
    i3_log(LOG_MASK_REACH, "    No challenge key");
}



void message_util_log_device_info_response(const cr_DeviceInfoResponse *response) 
{
    i3_log(LOG_MASK_REACH, "  Device Info Response:");
    i3_log(LOG_MASK_REACH, "    name                  : %s", response->device_name);
    i3_log(LOG_MASK_REACH, "    protocol version      : %s", response->protocol_version_string);
    i3_log(LOG_MASK_REACH, "    firmware version      : %s", response->firmware_version);
    i3_log(LOG_MASK_REACH, "    manufacturer          : %s", response->manufacturer);
    i3_log(LOG_MASK_REACH, "    device description    : %s", response->device_description);
    i3_log(LOG_MASK_REACH, "    services              : 0x%x", response->services);
    i3_log(LOG_MASK_REACH, "    metadata hash         : 0x%x", response->parameter_metadata_hash);
    i3_log(LOG_MASK_REACH, "    endpoints             : 0x%x", response->endpoints);
    if (response->has_application_identifier)
    {
        char hex_string[48];
        // Convert byte array to hex string
        sByte_array_to_hex_string(response->application_identifier.bytes,
                                 response->application_identifier.size,
                                 hex_string);
        i3_log(LOG_MASK_REACH, "    application_identifier    : %s", hex_string);
    }
    i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_discover_files() 
{
  i3_log(LOG_MASK_REACH, "  Discover Files Request\r\n");
}

void message_util_log_discover_files_response(const cr_DiscoverFilesResponse *response)
{
  i3_log(LOG_MASK_REACH, "  Discover Files Response:");
  for (int i=0; i < response->file_infos_count; i++)
  {
    i3_log(LOG_MASK_REACH, "    [id                  : %d", response->file_infos[i].file_id);
    i3_log(LOG_MASK_REACH, "     name                : %s", response->file_infos[i].file_name);
    i3_log(LOG_MASK_REACH, "     access              : 0x%x", response->file_infos[i].access);
    i3_log(LOG_MASK_REACH, "     current byte length : %d", response->file_infos[i].current_size_bytes);
    i3_log(LOG_MASK_REACH, "    ]");
  }
  i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_file_transfer_request(const cr_FileTransferRequest *request)
{
  i3_log(LOG_MASK_REACH, "  File Transfer Init Request:");
  i3_log(LOG_MASK_REACH, "    id                 : %d", request->file_id);
  i3_log(LOG_MASK_REACH, "    read_write         : %d", request->read_write);
  i3_log(LOG_MASK_REACH, "    request offset     : %d", request->request_offset);
  i3_log(LOG_MASK_REACH, "    transfer length    : %d", request->transfer_length);
  i3_log(LOG_MASK_REACH, "    transfer id        : %d", request->transfer_id);
  i3_log(LOG_MASK_REACH, "    requested_ack_rate : %d", request->requested_ack_rate);
  i3_log(LOG_MASK_REACH, "    timeout            : %d\r\n", request->timeout_in_ms);
}

void message_util_log_file_transfer_response(const cr_FileTransferResponse *response)
{
  i3_log(LOG_MASK_REACH, "  File Transfer Init Response:");
  i3_log(LOG_MASK_REACH, "    result            :", response->result);
  i3_log(LOG_MASK_REACH, "    transfer_id       :", response->transfer_id);
  i3_log(LOG_MASK_REACH, "    ack_rate          :", response->ack_rate);
  if (response->has_result_message) {            
    i3_log(LOG_MASK_REACH, "    result_message  :", response->result_message);
  }
  i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_transfer_data(const cr_FileTransferData *request)
{
  i3_log(LOG_MASK_REACH, "  File Transfer Data Request:");
  i3_log(LOG_MASK_REACH, "    transfer id    : %d", request->transfer_id);
  i3_log(LOG_MASK_REACH, "    message number : %d", request->message_number);
  i3_log(LOG_MASK_REACH, "    messsage size  : %d", request->message_data.size);
  // don't print the data here.
  if (request->has_checksum)
  {
    i3_log(LOG_MASK_REACH, "    checksum       : 0x%x\r\n", request->checksum);
  }
  i3_log(LOG_MASK_REACH, "    No CRC\r\n");
}

/*
void
message_util_log_transfer_data_response(const cr_FileTransferData *request)
{
  i3_log(LOG_MASK_REACH, "  File Transfer Data Response:");
  i3_log(LOG_MASK_REACH, "    transfer id    : %d", request->transfer_id);
  i3_log(LOG_MASK_REACH, "    message number : %d", request->message_number);
  i3_log(LOG_MASK_REACH, "    messsage size  : %d", request->message_data.size);
  // don't print the data here.
  if (request->has_checksum)
  {
    i3_log(LOG_MASK_REACH, "    checksum       : 0x%x\r\n", request->checksum);
  }
  i3_log(LOG_MASK_REACH, "    No CRC\r\n");
} 
*/ 

void message_util_log_transfer_data_notification(bool is_request,
    const cr_FileTransferDataNotification *request)
{
  if (is_request)
    i3_log(LOG_MASK_REACH, "  Transfer Data Notification Request:");
  else
    i3_log(LOG_MASK_REACH, "  Transfer Data Notification Response:");
  i3_log(LOG_MASK_REACH, "    result       : %d", request->result);
  i3_log(LOG_MASK_REACH, "    transfer_id  : %d", request->transfer_id);
  i3_log(LOG_MASK_REACH, "    retry_offset : %d", request->retry_offset);
  if (request->has_result_message)
    i3_log(LOG_MASK_REACH, "    result_message: %s", request->result_message);
  if (request->is_complete)
    i3_log(LOG_MASK_REACH, "    Transfer Complete.\r\n");
  i3_log(LOG_MASK_REACH, "    Transfer NOT Complete.\r\n");
}

void message_util_log_param_info_request(const cr_ParameterInfoRequest *request)
{
  i3_log(LOG_MASK_REACH, "  Parameter Info Request:");
  if (request->parameter_ids_count == 0)
  {
    i3_log(LOG_MASK_REACH, "    Count zero means request all.\r\n");
    return;
  }
  i3_log(LOG_MASK_REACH, "    %d parameters requested:");
  sLogNumberList(request->parameter_ids, request->parameter_ids_count);
  i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_param_info_response(const cr_ParameterInfoResponse *response)
{
  i3_log(LOG_MASK_REACH, "  Parameter Info Response:");
  if (response->parameter_infos_count == 0) {
    i3_log(LOG_MASK_REACH, "    No Parameters\r\n");
    return;
  } 
  for (size_t i = 0; i < response->parameter_infos_count; i++) {
    i3_log(LOG_MASK_REACH, "    [id            : %d", response->parameter_infos[i].id);
    i3_log(LOG_MASK_REACH, "     data type     : %d", response->parameter_infos[i].data_type);
    i3_log(LOG_MASK_REACH, "     sz in bytes   : %d", response->parameter_infos[i].size_in_bytes);
    i3_log(LOG_MASK_REACH, "     name          : %s", response->parameter_infos[i].name);
    i3_log(LOG_MASK_REACH, "     access        : 0x%x", response->parameter_infos[i].access);
    if (response->parameter_infos[i].has_description)
        i3_log(LOG_MASK_REACH, "     description   : %s", response->parameter_infos[i].description);
    i3_log(LOG_MASK_REACH, "     units         : %s", response->parameter_infos[i].units);
    if (response->parameter_infos[i].has_range_min)
        i3_log(LOG_MASK_REACH, "     min           : %.1f", response->parameter_infos[i].range_min);
    if (response->parameter_infos[i].has_range_max)
        i3_log(LOG_MASK_REACH, "     max           : %.1f", response->parameter_infos[i].range_max);
    if (response->parameter_infos[i].has_default_value)
        i3_log(LOG_MASK_REACH, "     default       : %.1f", response->parameter_infos[i].default_value);
    i3_log(LOG_MASK_REACH, "     storage location: %d", response->parameter_infos[i].storage_location);
    i3_log(LOG_MASK_REACH, "    ]");
  }
  i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_param_info_ex_response(const cr_ParamExInfoResponse *response)
{
  i3_log(LOG_MASK_REACH, "  Parameter Info Ex Response:");
  i3_log(LOG_MASK_REACH, "    associated_pid     : %d", response->associated_pid);
  i3_log(LOG_MASK_REACH, "    data_type          : %d", response->data_type);
  i3_log(LOG_MASK_REACH, "    enumerations_count : %d", response->enumerations_count);
  for (size_t i = 0; i < response->enumerations_count; i++) 
  {
    i3_log(LOG_MASK_REACH, "    [id: %d.  name: %s]", 
           response->enumerations[i].id, response->enumerations[i].name);
  }
}

void message_util_log_discover_streams()
{
  i3_log(LOG_MASK_REACH, "  Discover Streams:");
}

/** Commands */
void message_util_log_discover_commands() {

  i3_log(LOG_MASK_REACH, "  Discover Commands:");
}

void message_util_log_discover_commands_response(
    const cr_DiscoverCommandsResponse *payload)
{
  i3_log(LOG_MASK_REACH, "  Discover Commands Response:");
  for (size_t i = 0; i < payload->available_commands_count; i++) {
    i3_log(LOG_MASK_REACH, "    [id         : %d", payload->available_commands[i].id);
    i3_log(LOG_MASK_REACH, "     name       : %s", payload->available_commands[i].name);
    if ( payload->available_commands[i].has_description ) 
      i3_log(LOG_MASK_REACH, "     description : %s", payload->available_commands[i].description);
    if ( payload->available_commands[i].has_timeout ) 
      i3_log(LOG_MASK_REACH, "     timeout     : %d", payload->available_commands[i].timeout);
    i3_log(LOG_MASK_REACH, "    ]");
  }
}

void message_util_log_send_command(const cr_SendCommand *payload)
{
  i3_log(LOG_MASK_REACH, "  Send Command %d\r\n", payload->command_id);
}

void message_util_log_command_response(const cr_SendCommandResponse *payload)
{
  i3_log(LOG_MASK_REACH, "  Send Command Response:");
  i3_log(LOG_MASK_REACH, "    result  : %d", payload->result);
  if (payload->has_result_message)
    i3_log(LOG_MASK_REACH, "    message : %s", payload->result_message);
  i3_log(LOG_MASK_REACH, "\n");
}

/** CLI */
void message_util_log_cli_notification(bool send, const cr_CLIData *payload)
{
  if (send)
    i3_log(LOG_MASK_REACH, "  Send CLI Notification:");
  else
    i3_log(LOG_MASK_REACH, "  Receive CLI Notification:");
  i3_log(LOG_MASK_REACH, "    %s\r\n", payload->message_data);
}

#ifdef INCLUDE_PARAMETER_SERVICE
/** Params */
void message_util_log_read_param(const cr_ParameterRead *request)
{
  i3_log(LOG_MASK_REACH, "  Read Parameter Request:");
  if (request->parameter_ids_count == 0)
  {
    i3_log(LOG_MASK_REACH, "  A count of zero means read all.\r\n");
    return;
  }
  sLogNumberList(request->parameter_ids, request->parameter_ids_count);
  i3_log(LOG_MASK_REACH, "\r\n");
}


void message_util_log_read_param_response(const cr_ParameterReadResponse *response)
{
    i3_log(LOG_MASK_REACH, "  Read Parameter Response:");
    for (size_t i = 0; i < response->values_count; i++) 
    {
      // This doesn't print the timestamp.
      sLogParameterValue(&response->values[i]);
    }
}

void message_util_log_write_param(const cr_ParameterWrite *payload)
{
  i3_log(LOG_MASK_REACH, "  Write Parameter Request:");
  for (size_t i = 0; i < payload->values_count; i++) 
  {
    sLogParameterValue(&payload->values[i]);
  }
}

void message_util_log_write_param_response(const cr_ParameterWriteResponse *payload)
{
  i3_log(LOG_MASK_REACH, "  write param response: %d\n", (int)payload->result);
  if (payload->has_result_message)
      i3_log(LOG_MASK_REACH, "    result message: %s", payload->result_message);
}

void message_util_log_config_notify_param(const cr_ParameterNotifyConfigResponse *payload)
{
  i3_log(LOG_MASK_REACH, "  write param response: %d\n", (int)payload->result);
  if (payload->has_result_message)
  {
      i3_log(LOG_MASK_REACH, "    result message: %s", payload->result_message);
  }
}

void message_util_log_notify_setup_request(const cr_DiscoverParameterNotifySetup *payload)
{
    uint32_t lm = i3_log_get_mask();
    if (0 ==(LOG_MASK_REACH & lm))
        return;

    i3_log(LOG_MASK_REACH, "  Notify Setup request: %d IDs\n", (int)payload->parameter_ids_count);
    i3_log(LOG_MASK_BARE, "    ");
    for (int i=0; i<payload->parameter_ids_count; i++)
    {
        i3_log(LOG_MASK_BARE, "%d ", payload->parameter_ids[i]);
        if (i== 16)
            i3_log(LOG_MASK_BARE, "\r\n    ");
    }
    i3_log(LOG_MASK_BARE, "\r\n");
}

void message_util_log_notify_setup_response(const cr_DiscoverParameterNotifySetupResponse *payload)
{
    i3_log(LOG_MASK_REACH, "  Notify Setup response: %d configs\n", (int)payload->configs_count);
    for (int i=0; i<payload->configs_count; i++)
    {
        if (!payload->configs[i].enabled)
        {
            i3_log(LOG_MASK_REACH, "    ID %d, not enabled", payload->configs[i].parameter_id);
            continue;
        }
        i3_log(LOG_MASK_REACH, "    ID %d, enabled, %.1f, %.1f, delta %.1f", 
               payload->configs[i].parameter_id, payload->configs[i].minimum_notification_period,
               payload->configs[i].maximum_notification_period, payload->configs[i].minimum_delta);
    }
}

#endif  // def INCLUDE_PARAMETER_SERVICE



void message_util_log_ping_request(const cr_PingRequest *payload)
{

  i3_log(LOG_MASK_REACH, "  Ping Request:");
  if (payload->echo_data.size == 0)
  {
    i3_log(LOG_MASK_REACH, "    No payload\r\n");
    return;
  }
  char hex_string[49];  // 2*24+1
  if (payload->echo_data.size > 24)
  {
    sByte_array_to_hex_string(payload->echo_data.bytes, 24, hex_string);
    i3_log(LOG_MASK_REACH, "    First 24 of %d payload bytes:", payload->echo_data.size);
    i3_log(LOG_MASK_REACH, "      %s\r\n", hex_string);
    return;
  }

  sByte_array_to_hex_string(payload->echo_data.bytes, payload->echo_data.size, hex_string);
  i3_log(LOG_MASK_REACH, "    %d payload bytes:", payload->echo_data.size);
  i3_log(LOG_MASK_REACH, "      %s\r\n", hex_string);
  return;
  i3_log(LOG_MASK_REACH, "\r\n");
}

void message_util_log_ping_response(const cr_PingResponse *payload)
{
  i3_log(LOG_MASK_REACH, "  Ping Response:");
  i3_log(LOG_MASK_REACH, "    signal strength : %d", payload->signal_strength);
  if (payload->echo_data.size == 0)
  {
    i3_log(LOG_MASK_REACH, "    No payload");
    return;
  }
  char hex_string[49];  // 2*24+1

  if (payload->echo_data.size > 24)
  {
    sByte_array_to_hex_string(payload->echo_data.bytes, 24, hex_string);
    i3_log(LOG_MASK_REACH, "    First 24 of %d payload bytes:", payload->echo_data.size);
    i3_log(LOG_MASK_REACH, "      %s\r\n", hex_string);
    return;
  }
  sByte_array_to_hex_string(payload->echo_data.bytes, payload->echo_data.size, hex_string);
  i3_log(LOG_MASK_REACH, "    %d payload bytes:", payload->echo_data.size);
  i3_log(LOG_MASK_REACH, "      %s\r\n", hex_string);
}

#ifdef INCLUDE_TIME_SERVICE
    void message_util_log_time_set_response(const cr_TimeSetResponse *payload) 
    {
        i3_log(LOG_MASK_REACH, "  Time set response result: %d", (int)payload->result);
        if (payload->has_result_message)
          i3_log(LOG_MASK_REACH, "    %s", payload->result_message);
        i3_log(LOG_MASK_REACH, "\r\n");
    }

    void message_util_log_time_get_response(const cr_TimeGetResponse *payload) 
    {
      i3_log(LOG_MASK_REACH, "  Time get response result: %d", (int)payload->result);
      i3_log(LOG_MASK_REACH, "    seconds_utc: %lld", payload->seconds_utc);
      if (payload->has_timezone)
        i3_log(LOG_MASK_REACH, "    timezone: %lld", payload->timezone);
      else
        i3_log(LOG_MASK_REACH, "    no timezone");
      if (payload->has_result_message)
        i3_log(LOG_MASK_REACH, "    %s", payload->result_message);
      i3_log(LOG_MASK_REACH, "\r\n");
    }

    void message_util_log_time_set_request(const cr_TimeSetRequest *payload) 
    {
      i3_log(LOG_MASK_REACH, "  Time set request:");
      i3_log(LOG_MASK_REACH, "    seconds_utc: %lld", payload->seconds_utc);
      if (payload->has_timezone)
        i3_log(LOG_MASK_REACH, "    timezone: %lld", payload->timezone);
      else
        i3_log(LOG_MASK_REACH, "    no timezone");
    }

    void message_util_log_time_get_request(const cr_TimeGetRequest *payload) 
    {
      (void)payload;
      i3_log(LOG_MASK_REACH, "  Time get request:\r\n");
    }
#endif  // def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_WIFI_SERVICE
    void message_util_log_discover_wifi_request(const cr_DiscoverWiFiRequest *payload)
    {
        if (payload->state == cr_WiFiState_CONNECTED)
            i3_log(LOG_MASK_REACH, "  WiFi Info request for connected link\r\n");
        else
            i3_log(LOG_MASK_REACH, "  WiFi Info request for unconnected link(s)\r\n");
    }

    void message_util_log_WiFi_connect_request(const cr_WiFiConnectionRequest *payload)
    {
        if (payload->action == cr_WiFiState_CONNECTED)
        {
            if (!payload->has_cd)
            {
                i3_log(LOG_MASK_REACH, "  WiFi connection request without description.\r\n");
                LOG_ERROR("WiFi connection request without description.\r\n");
                return;
            }
            i3_log(LOG_MASK_REACH, "  WiFi connect request for connection to:");
            i3_log(LOG_MASK_REACH, "    SSID : '%s'", payload->cd.ssid);
            if (payload->cd.has_signal_strength)
                i3_log(LOG_MASK_REACH, "    Signal Strength : %d", payload->cd.signal_strength);
            if (payload->cd.has_sec)
            {
                switch (payload->cd.sec)
                {
                case cr_WiFiSecurity_OPEN:
                    i3_log(LOG_MASK_REACH, "    Open connection\r\n");
                    break;
                case cr_WiFiSecurity_WEP:
                    i3_log(LOG_MASK_REACH, "    WEP security\r\n");
                    break;
                case cr_WiFiSecurity_WPA:
                    i3_log(LOG_MASK_REACH, "    WPA security\r\n");
                    break;
                case cr_WiFiSecurity_WPA2:
                    i3_log(LOG_MASK_REACH, "    WPA2 security\r\n");
                    break;
                case cr_WiFiSecurity_WPA3:
                    i3_log(LOG_MASK_REACH, "    WPA3 security\r\n");
                    break;
                default:
                    i3_log(LOG_MASK_REACH, "    Unknown security %d", payload->cd.sec);
                    break;
                }
            }
            else
                i3_log(LOG_MASK_REACH, "    No security specified");
            if (payload->cd.has_band)
                i3_log(LOG_MASK_REACH, "    Radio band %d", payload->cd.band);
            if (payload->has_autoconnect && payload->autoconnect)
                i3_log(LOG_MASK_REACH, "    Autoconnect requested");
            i3_log(LOG_MASK_REACH, "\r\n");
        }
        else
        {
            i3_log(LOG_MASK_REACH, "  WiFi connect request for disconnect.\r\n");
        }
    }

    void message_util_log_discover_wifi_response(cr_DiscoverWiFiResponse *payload)
    {
        i3_log(LOG_MASK_REACH, "  Discover WiFi Response:");
        if (payload->state == cr_WiFiState_CONNECTED)
            i3_log(LOG_MASK_REACH, "    Connected.\r\n");
        else 
            i3_log(LOG_MASK_REACH, "    Not Connected, connection ID %d.\r\n", payload->connectionId);
        if (payload->has_cd) 
            i3_log(LOG_MASK_REACH, "    SSID : '%s'.\r\n", payload->cd.ssid);
        if (payload->cd.has_signal_strength)
            i3_log(LOG_MASK_REACH, "    Signal strength %d\r\n", payload->cd.signal_strength);
        if (payload->cd.has_signal_strength)
            i3_log(LOG_MASK_REACH, "    Signal strength %d\r\n", payload->cd.signal_strength);
        if (payload->cd.has_band)
            i3_log(LOG_MASK_REACH, "    Radio band %d\r\n", payload->cd.band);
        i3_log(LOG_MASK_REACH, "\r\n");
    }

    void message_util_log_WiFi_connect_response(cr_WiFiConnectionResponse *payload)
    {
        i3_log(LOG_MASK_REACH, "  WiFi Connect Response:");
        i3_log(LOG_MASK_REACH, "    Connection Result %d\r\n", payload->result);
        if (payload->has_signal_strength)
            i3_log(LOG_MASK_REACH, "    Signal strength %d\r\n", payload->signal_strength);
        if (payload->has_error_message)
            i3_log(LOG_MASK_REACH, "    Message: '%s'\r\n", payload->error_message);
    }
  #endif  // def INCLUDE_WIFI_SERVICE


#else
    // When NO_REACH_LOGGING is defined we need empty implementations of these.
    // Defining NO_REACH_LOGGING saves about 19k of code space.
    const char *msg_type_string(int32_t) {return NULL;}
    void message_util_log_device_info_request(){}
    void message_util_log_device_info_response(const cr_DeviceInfoResponse *){}

    void message_util_log_ping_request(const cr_PingRequest *){}
    void message_util_log_ping_response(const cr_PingResponse *){}

    #ifdef INCLUDE_PARAMETER_SERVICE
        void message_util_log_param_info_request(const cr_ParameterInfoRequest *){}
        void message_util_log_param_info_response(const cr_ParameterInfoResponse *){}
        void message_util_log_param_info_ex_response(const cr_ParamExInfoResponse *){}
        void message_util_log_read_param(const cr_ParameterRead *){}
        void message_util_log_read_param_response(const cr_ParameterReadResponse *){}
        void message_util_log_write_param(const cr_ParameterWrite *){}
        void message_util_log_write_param_response(const cr_ParameterWriteResponse *){}
        void message_util_log_config_notify_param(const cr_ParameterNotifyConfigResponse *){}
        void message_util_log_notify_setup_request(const cr_DiscoverParameterNotifySetup *payload) {}
        void message_util_log_notify_setup_response(const cr_DiscoverParameterNotifySetupResponse *payload) {}
    #endif  // INCLUDE_PARAMETER_SERVICE

    #ifdef INCLUDE_FILE_SERVICE
        void message_util_log_discover_files(){}
        void message_util_log_discover_files_response(const cr_DiscoverFilesResponse *){}
        void message_util_log_file_transfer_request(const cr_FileTransferRequest *){}
        void message_util_log_file_transfer_response(const cr_FileTransferResponse *){}
        void message_util_log_transfer_data(const cr_FileTransferData *){}
        void message_util_log_transfer_data_response(const cr_FileTransferData *){}
        void message_util_log_transfer_data_notification(bool is_request,
                const cr_FileTransferDataNotification *){}
    #endif // def INCLUDE_FILE_SERVICE

    #ifdef INCLUDE_STREAM_SERVICE
        // Streams
        void message_util_log_discover_streams(){}
    #endif // def INCLUDE_STREAM_SERVICE


    #ifdef INCLUDE_COMMAND_SERVICE
        // Commands
        void message_util_log_discover_commands(){}
        void message_util_log_discover_commands_response(const cr_DiscoverCommandsResponse *){}
        void message_util_log_send_command(const cr_SendCommand *){}
        void message_util_log_command_response(const cr_SendCommandResponse *){}
    #endif // def INCLUDE_COMMAND_SERVICE

    #ifdef INCLUDE_CLI_SERVICE
        // CLI
        void message_util_log_cli_notification(bool send, const cr_CLIData *){}
    #endif // def INCLUDE_CLI_SERVICE

    #ifdef INCLUDE_TIME_SERVICE
        // Time
        void message_util_log_time_set_response(const cr_TimeSetResponse *payload){}
        void message_util_log_time_get_response(const cr_TimeGetResponse *payload){}
        void message_util_log_time_set_request(const cr_TimeSetRequest *payload){}
        void message_util_log_time_get_request(const cr_TimeGetRequest *payload){} 
    #endif // def INCLUDE_TIME_SERVICE

    #ifdef INCLUDE_WIFI_SERVICE
        void message_util_log_discover_wifi_request(const cr_DiscoverWiFiRequest *payload){}
        void message_util_log_WiFi_connect_request(const cr_WiFiConnectionRequest *payload){}
        void message_util_log_discover_wifi_response(cr_DiscoverWiFiResponse *payload){}
        void message_util_log_WiFi_connect_response(cr_WiFiConnectionResponse *payload);
    #endif  // def INCLUDE_WIFI_SERVICE

#endif  // ndef NO_REACH_LOGGING
