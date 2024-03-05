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
 *                          Copyright i3 Product Development 2023
 *
 *
 * @author  i3 Product Development
 *          Wei Zhou wei.zhou@i3pd.com
 *
 * @version 0.0.1
 * @date    2023-08-21
 *
 ********************************************************************************************/

/**
 * @file      message_util.c
 * @brief     Utility functions to print out Reach messages in a human readable 
 *            format.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 */

#include <stdio.h>

#include "reach-server.h"
#include "message_util.h"
#include "i3_log.h"
#include "cJSON.h"


// None of these message utilities are required without logging.
#ifndef NO_REACH_LOGGING

#if defined(INCLUDE_WIFI_SERVICE)
  #define MSG_BUFFER_LEN    256
  static char sMsgUtilBuffer[MSG_BUFFER_LEN];

#elif (defined(INCLUDE_PARAMETER_SERVICE) || defined(INCLUDE_TIME_SERVICE))
  #define MSG_BUFFER_LEN    40
  static char sMsgUtilBuffer[MSG_BUFFER_LEN];
#endif

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
  default:
    break;
  }
  return "Unknown";
}


char *message_util_get_null_value_json(const char *key) {

  cJSON *json = cJSON_CreateObject();
  cJSON_AddNullToObject(json, key);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_get_device_info_json() {
  return message_util_get_null_value_json(msg_type_string(cr_ReachMessageTypes_GET_DEVICE_INFO));
}

char *message_util_get_device_info_response_json(
    const cr_DeviceInfoResponse *response) {

    cJSON *json = cJSON_CreateObject();
    cJSON *json1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(json1, "protocol version", response->protocol_version);
    cJSON_AddStringToObject(json1, "name", response->device_name);
    cJSON_AddStringToObject(json1, "firmware version", response->firmware_version);
    cJSON_AddStringToObject(json1, "manufacturer", response->manufacturer);
    cJSON_AddStringToObject(json1, "device description", response->device_description);
    cJSON_AddNumberToObject(json1, "services", response->services);
    cJSON_AddNumberToObject(json1, "hash", response->parameter_metadata_hash);
    // not yet: application_identifier
    cJSON_AddNumberToObject(json1, "endpoints", response->endpoints);
    cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_GET_DEVICE_INFO), json1);

    // convert the cJSON object to a JSON string
    char *json_str = cJSON_Print(json);

    // free the JSON string and cJSON object
    cJSON_Delete(json);

    return json_str;
}

char *message_util_discover_files_json() {
  return message_util_get_null_value_json(msg_type_string(cr_ReachMessageTypes_DISCOVER_FILES));
}

char *message_util_discover_files_response_json(
    const cr_DiscoverFilesResponse *response) {

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();

  for (int i=0; i < response->file_infos_count; i++)
  {
    cJSON *json1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(json1, "id", response->file_infos[i].file_id);
    cJSON_AddStringToObject(json1, "name", response->file_infos[i].file_name);
    cJSON_AddNumberToObject(json1, "access", response->file_infos[i].access);
    cJSON_AddNumberToObject(json1, "current byte length", response->file_infos[i].current_size_bytes);
    cJSON_AddItemToArray(jsonArray, json1);
  }

  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_DISCOVER_FILES), jsonArray);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_init_json(const cr_FileTransferInit *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "id", request->file_id);
  cJSON_AddNumberToObject(json1, "read_write", request->read_write);
  cJSON_AddNumberToObject(json1, "request offset", request->request_offset);
  cJSON_AddNumberToObject(json1, "transfer length", request->transfer_length);
  cJSON_AddNumberToObject(json1, "transfer id", request->transfer_id);
  cJSON_AddNumberToObject(json1, "messages per ack", request->messages_per_ack);
  cJSON_AddNumberToObject(json1, "timeout", request->timeout_in_ms);

  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_TRANSFER_INIT), json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_init_response_json(
    const cr_FileTransferInitResponse *response) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "result", response->result);
  cJSON_AddNumberToObject(json1, "transfer_id", response->transfer_id);
  cJSON_AddNumberToObject(json1, "preferred_ack_rate", response->preferred_ack_rate);
  if (response->result != 0)
    cJSON_AddStringToObject(json1, "error_message", response->error_message);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_TRANSFER_INIT), json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_data_json(const cr_FileTransferData *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "transfer id", request->transfer_id);
  cJSON_AddNumberToObject(json1, "message number",  request->message_number);
  cJSON_AddNumberToObject(json1, "messsage size", request->message_data.size);
  // cJSON_AddRawToObject(json1, "messsage data", (char *)request->message_data.bytes);
  cJSON_AddNumberToObject(json1, "crc32", request->crc32);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_TRANSFER_DATA), json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *
message_util_transfer_data_response_json(const cr_FileTransferData *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "id", request->transfer_id);
  cJSON_AddNumberToObject(json1, "message number", request->message_number);
  // cJSON_AddRawToObject(json1, "messsage data", (char *)request->message_data.bytes);
  cJSON_AddNumberToObject(json1, "crc32", request->crc32);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_TRANSFER_DATA), json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_data_notification_json(
    const cr_FileTransferDataNotification *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "result", request->result);
  cJSON_AddBoolToObject(json1, "is_complete", request->is_complete);
  cJSON_AddNumberToObject(json1, "transfer_id", request->transfer_id);
  if (request->result != 0)
  {
    cJSON_AddNumberToObject(json1, "retry_offset", request->retry_offset);
    cJSON_AddStringToObject(json1, "error_message", request->error_message);
  }
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION), json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_param_info_json(const cr_ParameterInfoRequest *request) {

  // I3_LOG(LOG_MASK_REACH, "message_util_param_info_json\n");

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();
  if (request->parameter_ids_count > 0) {

    for (size_t i = 0; i < request->parameter_ids_count; i++) {
      cJSON_AddItemToArray(jsonArray,
                           cJSON_CreateNumber(request->parameter_ids[i]));
    }
  }
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_DISCOVER_PARAMETERS), jsonArray);
  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_param_info_response_json(
    const cr_ParameterInfoResponse *response) {

  cJSON *json = cJSON_CreateObject();
  if (response->parameter_infos_count > 0) {
    cJSON *jsonArray = cJSON_CreateArray();
    for (size_t i = 0; i < response->parameter_infos_count; i++) {
      cJSON *json_1 = cJSON_CreateObject();

      cJSON_AddNumberToObject(json_1, "id", response->parameter_infos[i].id);
      cJSON_AddNumberToObject(json_1, "data type", response->parameter_infos[i].data_type);
      cJSON_AddNumberToObject(json_1, "sz in bytes", response->parameter_infos[i].size_in_bytes);
      cJSON_AddStringToObject(json_1, "name", response->parameter_infos[i].name);
      cJSON_AddStringToObject(json_1, "description", response->parameter_infos[i].description);
      cJSON_AddStringToObject(json_1, "units", response->parameter_infos[i].units);
      cJSON_AddNumberToObject(json_1, "min", response->parameter_infos[i].range_min);
      cJSON_AddNumberToObject(json_1, "max", response->parameter_infos[i].range_max);
      cJSON_AddNumberToObject(json_1, "access", response->parameter_infos[i].access);
      cJSON_AddNumberToObject(json_1, "storage location", response->parameter_infos[i].storage_location);

      cJSON_AddItemToArray(jsonArray, json_1);
    }
    cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_DISCOVER_PARAMETERS), jsonArray);
  }

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_param_info_ex_response_json(
    const cr_ParamExInfoResponse *response) {

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();

  cJSON_AddNumberToObject(json, "associated_pid", response->associated_pid);
  cJSON_AddNumberToObject(json, "data_type", response->data_type);
  cJSON_AddNumberToObject(json, "enumerations_count", response->enumerations_count);
  if (response->enumerations_count > 0) 
  {
    cJSON *jsonArray2 = cJSON_CreateArray();
    for (size_t i = 0; i < response->enumerations_count; i++) 
    {
      cJSON *json1 = cJSON_CreateObject();
      cJSON_AddNumberToObject(json1, "id", response->enumerations[i].id);
      cJSON_AddStringToObject(json1, "name", response->enumerations[i].name);
      cJSON_AddItemToArray(jsonArray2, json1);
    }
    cJSON_AddItemToObject(json, "enumerations", jsonArray2);
  }

  // To Do:  The title comes at t the end, but it should be at the beginning.

  cJSON_AddItemToObject(json, "Parameter Info EX Response", jsonArray);
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_discover_streams_json() {
  return message_util_get_null_value_json(msg_type_string(cr_ReachMessageTypes_DISCOVER_STREAMS));
}

/** Commands */
char *message_util_discover_commands_json() {
  return message_util_get_null_value_json(msg_type_string(cr_ReachMessageTypes_DISCOVER_COMMANDS));
}

char *message_util_discover_commands_response_json(
    const cr_DiscoverCommandsResponse *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();
  if (payload->available_commands_count > 0) {
    cJSON *json_1 = cJSON_CreateObject();
    for (size_t i = 0; i < payload->available_commands_count; i++) {

      cJSON_AddNumberToObject(json_1, "id", payload->available_commands[i].id);
      cJSON_AddStringToObject(json_1, "name", payload->available_commands[i].name);
    }
    cJSON_AddItemToArray(jsonArray, json_1);
  }

  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_DISCOVER_COMMANDS), jsonArray);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_send_command_json(const cr_SendCommand *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_1, "command", payload->command_id);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_SEND_COMMAND), json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *
message_util_send_command_response_json(const cr_SendCommandResponse *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_1, "result", payload->result);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_SEND_COMMAND), json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

/** CLI */
char * message_util_cli_notification_json(const cr_CLIData *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();

  cJSON_AddStringToObject(json_1, "message data", payload->message_data);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_CLI_NOTIFICATION), json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

#ifdef INCLUDE_PARAMETER_SERVICE
/** Params */
char *message_util_read_param_json(const cr_ParameterRead *request) {

  // // I3_LOG(LOG_MASK_REACH, "message_util_read_param_json\n");

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();
  if (request->parameter_ids_count > 0) {
    for (size_t i = 0; i < request->parameter_ids_count; i++) {
      cJSON_AddItemToArray(jsonArray, cJSON_CreateNumber(request->parameter_ids[i]));
    }
  }
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_READ_PARAMETERS), jsonArray);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *
message_util_read_param_response_json(const cr_ParameterReadResult *response) {

  // // I3_LOG(LOG_MASK_REACH,
  // "message_util_read_param_response_json\n");

  cJSON *json = cJSON_CreateObject();
  if (response->values_count > 0) {
    cJSON *jsonArray = cJSON_CreateArray();
    for (size_t i = 0; i < response->values_count; i++) {
      cJSON *json_1 = cJSON_CreateObject();
      cJSON_AddNumberToObject(json_1, "id",
                              response->values[i].parameter_id);

      char typeStr[16];
      // To match the apps and protobufs, must use _value_tags!
      // That's why the -3 here.
      sprintf(typeStr, "value (%d)", response->values[i].which_value-3);

      switch (response->values[i].which_value)
      {
      case cr_ParameterValue_uint32_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.uint32_value);
          break;
      case cr_ParameterValue_sint32_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.sint32_value);
          break;
      case cr_ParameterValue_float32_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.float32_value);
          break;
      case cr_ParameterValue_uint64_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.uint64_value);
          break;
      case cr_ParameterValue_sint64_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.sint64_value);
          break;
      case cr_ParameterValue_float64_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.float64_value);
          break;
      case cr_ParameterValue_bool_value_tag:
          cJSON_AddBoolToObject(json_1, typeStr,
                                response->values[i].value.bool_value);
          break;
      case cr_ParameterValue_string_value_tag:
          cJSON_AddStringToObject(json_1, typeStr,
                                  response->values[i].value.string_value);
          break;
      case cr_ParameterValue_enum_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.enum_value);
          break;
      case cr_ParameterValue_bitfield_value_tag:
          cJSON_AddNumberToObject(json_1, typeStr,
                                  response->values[i].value.bitfield_value);
          break;
      case cr_ParameterValue_bytes_value_tag:
          {
              int numBytes = response->values[i].value.bytes_value.size < 32 ?
                  response->values[i].value.bytes_value.size : 32;
              cJSON_AddNumberToObject(json_1, typeStr,
                                      response->values[i].value.bytes_value.size);
              I3_LOG(LOG_MASK_REACH, "JSON shows number of bytes as %d.", response->values[i].value.bytes_value.size);
              LOG_DUMP_MASK(LOG_MASK_REACH, "bytes", response->values[i].value.bytes_value.bytes, numBytes);
          }
          break;
      default:
          break;
      }
      // }

      cJSON_AddItemToArray(jsonArray, json_1);
    }
    cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_READ_PARAMETERS), jsonArray);
  }

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_write_param_json(const cr_ParameterWrite *payload) {

  // I3_LOG(LOG_MASK_REACH, "message_util_write_param_json\n");

  cJSON *json = cJSON_CreateObject();
  if (payload->values_count > 0) {
    cJSON *jsonArray = cJSON_CreateArray();
    for (size_t i = 0; i < payload->values_count; i++) {
      cJSON *json_1 = cJSON_CreateObject();
      cJSON_AddNumberToObject(json_1, "id",
                              payload->values[i].parameter_id);
      switch (payload->values[i].which_value)
      {
      case cr_ParameterValue_uint32_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.uint32_value);
          break;
      case cr_ParameterValue_sint32_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.sint32_value);
          break;
      case cr_ParameterValue_float32_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.float32_value);
          break;
      case cr_ParameterValue_uint64_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.uint64_value);
          break;
      case cr_ParameterValue_sint64_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.sint64_value);
          break;
      case cr_ParameterValue_float64_value_tag:
          cJSON_AddNumberToObject(json_1, "value",
                                  payload->values[i].value.float64_value);
          break;
      case cr_ParameterValue_bool_value_tag:
          cJSON_AddBoolToObject(json_1, "value",
                                payload->values[i].value.bool_value);
          break;
      case cr_ParameterValue_string_value_tag:
          cJSON_AddStringToObject(json_1, "value",
                                  payload->values[i].value.string_value);
          break;
      default:
          break;
      }

      cJSON_AddItemToArray(jsonArray, json_1);
    }
    cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_WRITE_PARAMETERS), jsonArray);
  }

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *
message_util_write_param_response_json(
    const cr_ParameterWriteResult *payload)
{
  sprintf(sMsgUtilBuffer, "  write param response: %d", (int)payload->result);
  return sMsgUtilBuffer;
}

char *message_util_config_notify_param_json(
    const cr_ParameterNotifyConfigResponse *payload)
{
  sprintf(sMsgUtilBuffer, "  write param response: %d", (int)payload->result);
  return sMsgUtilBuffer;
}
#endif  // def INCLUDE_PARAMETER_SERVICE



char *message_util_ping_json(const cr_PingRequest *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddRawToObject(json_1, "echo data", (char *)payload->echo_data.bytes);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_PING), json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_ping_response_json(const cr_PingResponse *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddRawToObject(json_1, "echo data", (char *)payload->echo_data.bytes);
  cJSON_AddNumberToObject(json_1, "signal strength", payload->signal_strength);
  cJSON_AddItemToObject(json, msg_type_string(cr_ReachMessageTypes_PING), json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

#ifdef INCLUDE_TIME_SERVICE
    char *message_util_time_set_response_json(const cr_TimeSetResponse *payload) 
    {
        sprintf(sMsgUtilBuffer, "  Time set response result: %d", (int)payload->result);
        return sMsgUtilBuffer;
    }

    char *message_util_time_get_response_json(const cr_TimeGetResponse *payload) 
    {
        int ptr = 0;
        ptr += sprintf(&sMsgUtilBuffer[ptr], "  Time get response result: %d\r\n", (int)payload->result);
        ptr += sprintf(&sMsgUtilBuffer[ptr], "  Time get response seconds: %lu\r\n", (long unsigned int)payload->seconds_utc);
        return sMsgUtilBuffer;
    }

    char *message_util_time_set_request_json(const cr_TimeSetRequest *payload) 
    {
        sprintf(sMsgUtilBuffer, "  Time get request seconds: %lu\r\n", (long unsigned int)payload->seconds_utc);
        return sMsgUtilBuffer;
    }

    char *message_util_time_get_request_json(const cr_TimeGetRequest *payload) 
    {
      (void)payload;
        int ptr = 0;
        ptr += sprintf(&sMsgUtilBuffer[ptr], "  Time get request\r\n");
        return sMsgUtilBuffer;
    }
#endif  // def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_WIFI_SERVICE
    char *message_util_discover_wifi_request_json(const cr_DiscoverWiFiRequest *payload)
    {
        if (payload->state == cr_WiFiState_CONNECTED)
            sprintf(&sMsgUtilBuffer[0], "  WiFi Info request for connected link\r\n");
        else
            sprintf(&sMsgUtilBuffer[0], "  WiFi Info request for unconnected link(s)\r\n");
        return sMsgUtilBuffer;
    }

    char *message_util_WiFi_connect_request_json(const cr_WiFiConnectionRequest *payload)
    {
        int ptr = 0;
        // longest text is 234 bytes
        if (payload->action == cr_WiFiState_CONNECTED)
        {
            if (!payload->has_cd)
            {
                sprintf(&sMsgUtilBuffer[ptr], "  WiFi connection request without description.\r\n");
                LOG_ERROR("WiFi connection request without description.\r\n");
                return sMsgUtilBuffer;
            }
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  WiFi connect request for connection to:\r\n");
            ptr += sprintf(&sMsgUtilBuffer[ptr], "    SSID : '%s'\r\n", payload->cd.ssid);
            if (payload->cd.has_signal_strength)
                ptr += sprintf(&sMsgUtilBuffer[ptr], "    Signal Strength : %d\r\n", payload->cd.signal_strength);
            if (payload->cd.has_sec)
            {
                switch (payload->cd.sec)
                {
                case cr_WiFiSecurity_OPEN:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    Open connection\r\n");
                    break;
                case cr_WiFiSecurity_WEP:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    WEP security\r\n");
                    break;
                case cr_WiFiSecurity_WPA:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    WPA security\r\n");
                    break;
                case cr_WiFiSecurity_WPA2:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    WPA2 security\r\n");
                    break;
                case cr_WiFiSecurity_WPA3:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    WPA3 security\r\n");
                    break;
                default:
                    ptr += sprintf(&sMsgUtilBuffer[ptr], "    Unknown security %d\r\n", payload->cd.sec);
                    break;
                }
            }
            else
                ptr += sprintf(&sMsgUtilBuffer[ptr], "    No security specified\r\n");
            if (payload->cd.has_band)
                ptr += sprintf(&sMsgUtilBuffer[ptr], "    Radio band %d\r\n", payload->cd.band);
            if (payload->has_autoconnect && payload->autoconnect)
                ptr += sprintf(&sMsgUtilBuffer[ptr], "    Autoconnect requested\r\n");
        }
        else
        {
            sprintf(&sMsgUtilBuffer[ptr], "  WiFi connect request for disconnect.\r\n");
        }
        return sMsgUtilBuffer;
    }

    char *message_util_discover_wifi_response_json(cr_DiscoverWiFiResponse *payload)
    {
        int ptr = 0;
        if (payload->state == cr_WiFiState_CONNECTED)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Connected.\r\n");
        else 
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Not Connected, connection ID %d.\r\n", payload->connectionId);
        if (payload->has_cd) 
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  SSID : '%s'.\r\n", payload->cd.ssid);
        if (payload->cd.has_signal_strength)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Signal strength %d\r\n", payload->cd.signal_strength);
        if (payload->cd.has_signal_strength)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Signal strength %d\r\n", payload->cd.signal_strength);
        if (payload->cd.has_band)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "    Radio band %d\r\n", payload->cd.band);

        return sMsgUtilBuffer;
    }

    char *message_util_WiFi_connect_response_json(cr_WiFiConnectionResponse *payload)
    {
        int ptr = 0;
        ptr += sprintf(&sMsgUtilBuffer[ptr], "    Connection Result %d\r\n", payload->result);
        if (payload->has_signal_strength)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Signal strength %d\r\n", payload->signal_strength);
        if (payload->has_error_message)
            ptr += sprintf(&sMsgUtilBuffer[ptr], "  Message: '%s'\r\n", payload->error_message);
        return sMsgUtilBuffer;
    }
  #endif  // def INCLUDE_WIFI_SERVICE



#endif  // ndef NO_REACH_LOGGING
