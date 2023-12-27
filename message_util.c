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
#include <stdio.h>

#include "message_util.h"
#include "i3_log.h"
#include "cJSON.h"

/** Message Types */
const char *message_type_unknown = "unknown";
const char *message_type_ping = "Ping";
const char *message_type_get_device_info = "Get Device Info";
const char *message_type_discover_files = "Discover Files";
const char *message_type_transfer_init = "Transfer Init";
const char *message_type_transfer_data = "Transfer Data";
const char *message_type_transfer_data_notification =
    "Transfer Data Notification";
const char *message_type_read_file = "rf";
const char *message_type_read_file_response = "rf~";
const char *message_type_write_file = "wf";
const char *message_type_write_file_response = "wf~";
const char *message_type_param_info = "Discover Parameter";
const char *message_type_read_param = "Read Parameter";
const char *message_type_write_param = "Write Parameter";

// Streams
const char *message_type_discover_streams = "Discover Streams";
const char *message_type_manage_stream = "Manage Stream";
const char *message_type_stream_data_notification = "Stream Data Notification";

// Commands
const char *message_type_discover_commands = "Discover Commands";
const char *message_type_send_command = "Send Command";

// CLI
const char *message_type_discover_cli = "Discover CLI";
const char *message_type_cli_notification = "CLI Notification";

/** Key Types */
const char *key_type_id = "id";
const char *key_type_name = "name";
const char *key_type_description = "description";
const char *key_type_storage_location = "storage location";
const char *key_type_access = "access";
const char *key_type_echo_data = "echo data";
const char *key_type_signal_strength = "signal strength";

const char *key_type_timeout_in_ms = "timeout";
const char *key_type_request_offset = "request offset";
const char *key_type_transfer_length = "transfer length";
const char *key_type_has_result = "has result";
const char *key_type_result = "result";
const char *key_type_result_value = "value";
const char *key_type_result_string = "string";
const char *key_type_transfer_id = "transfer id";
const char *key_type_max_bytes_per_message = "max bytes per message";
const char *key_type_file_messages_per_ack = "messages per ack";
const char *key_type_is_complete = "is complete";

const char *key_type_protocol_version = "protocol version";
const char *key_type_device_description = "device description";
const char *key_type_firmware_version = "firmware version";
const char *key_type_hash = "hash";
const char *key_type_manufacturer = "manufacturer";
const char *key_type_services = "services";
    // not yet: application_identifier
const char *key_type_endpoints = "endpoints";
const char *key_type_parameter_buffer_count = "parameter_buffer_count";
const char *key_type_num_medium_structs_in_msg = "num_medium_structs_in_msg";
const char *key_type_big_data_buffer_size = "big_data_buffer_size";


const char *key_type_file_id = "id";
const char *key_type_file_read_write = "read_write";
const char *key_type_file_name = "n";
const char *key_type_file_access = "access";
const char *key_type_file_storage_location = "storage location";
const char *key_type_file_current_byte_length = "current byte length";
const char *key_type_file_max_byte_length = "max byte length";
const char *key_type_file_max_message_length = "max message length";
const char *key_type_file_offset = "o";
const char *key_type_file_checksum = "c";
const char *key_type_file_data = "d";
const char *key_type_file_transfer_type = "transfer type";
const char *key_type_file_message_number = "message number";
const char *key_type_file_message_size = "messsage size";
const char *key_type_file_message_data = "messsage data";
const char *key_type_file_messages_to_send = "messages to send";
const char *key_type_file_bytes_transfered = "bytes transfered";
const char *key_type_file_crc32 = "crc32";

const char *key_type_stream_id = "stream id";
const char *key_type_stream_command = "stream command";
const char *key_type_command = "command";
const char *key_type_supported_cli_streams = "supported cli streams";

const char *key_type_message_data = "message data";

const char *key_type_param_id = "id";
const char *key_type_param_name = "name";
const char *key_type_param_data_type = "data type";
const char *key_type_param_size_in_bytes = "l";
const char *key_type_param_description = "d";
const char *key_type_param_unit = "units";
const char *key_type_param_value_range = "r";
const char *key_type_param_access = "a";
const char *key_type_param_range_min = "min";
const char *key_type_param_range_max = "max";
const char *key_type_param_storage_location = "s";
const char *key_type_param_value = "value";

const char *file_access_type_read = "r";
const char *file_access_type_write = "w";
const char *file_access_type_read_write = "rw";

const char *get_message_type(int32_t message_type) {

  switch (message_type) {
  case cr_ReachMessageTypes_PING:
    return message_type_ping;
  case cr_ReachMessageTypes_GET_DEVICE_INFO:
    return message_type_get_device_info;
  case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
    return message_type_param_info;
  case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
    return "Discover Param EX";
  case cr_ReachMessageTypes_READ_PARAMETERS:
    return message_type_read_param;
  case cr_ReachMessageTypes_WRITE_PARAMETERS:
    return message_type_write_param;
  case cr_ReachMessageTypes_DISCOVER_FILES:
    return message_type_discover_files;
  case cr_ReachMessageTypes_TRANSFER_INIT:
    return message_type_transfer_init;
  case cr_ReachMessageTypes_TRANSFER_DATA:
    return message_type_transfer_data;
  case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
    return message_type_transfer_data_notification;
  case cr_ReachMessageTypes_DISCOVER_STREAMS:
    return message_type_discover_streams;
  case cr_ReachMessageTypes_STREAM_DATA_NOTIFICATION:
    return message_type_stream_data_notification;
  case cr_ReachMessageTypes_DISCOVER_COMMANDS:
    return message_type_discover_commands;
  case cr_ReachMessageTypes_SEND_COMMAND:
    return message_type_send_command;
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
    return message_type_cli_notification;
  default:
    break;
  }
  return message_type_unknown;
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
  return message_util_get_null_value_json(message_type_get_device_info);
}

char *message_util_get_device_info_response_json(
    const cr_DeviceInfoResponse *response) {

    cJSON *json = cJSON_CreateObject();
    cJSON *json1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(json1, key_type_protocol_version, response->protocol_version);
    cJSON_AddStringToObject(json1, key_type_name, response->device_name);
    cJSON_AddStringToObject(json1, key_type_firmware_version, response->firmware_version);
    cJSON_AddStringToObject(json1, key_type_manufacturer, response->manufacturer);
    cJSON_AddStringToObject(json1, key_type_device_description, response->device_description);
    cJSON_AddNumberToObject(json1, key_type_services, response->services);
    cJSON_AddNumberToObject(json1, key_type_hash, response->parameter_metadata_hash);
    // not yet: application_identifier
    cJSON_AddNumberToObject(json1, key_type_endpoints, response->endpoints);
    cJSON_AddItemToObject(json, message_type_get_device_info, json1);

    // convert the cJSON object to a JSON string
    char *json_str = cJSON_Print(json);

    // free the JSON string and cJSON object
    cJSON_Delete(json);

    return json_str;
}

char *message_util_discover_files_json() {
  return message_util_get_null_value_json(message_type_discover_files);
}

char *message_util_discover_files_response_json(
    const cr_DiscoverFilesReply *response) {

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();

  for (int i=0; i < response->file_infos_count; i++)
  {
    cJSON *json1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(json1, key_type_file_id,
                            response->file_infos[i].file_id);
    cJSON_AddStringToObject(json1, key_type_name,
                            response->file_infos[i].file_name);
    cJSON_AddNumberToObject(json1, key_type_file_access,
                            response->file_infos[i].access);
    cJSON_AddNumberToObject(json1, key_type_file_current_byte_length,
                            response->file_infos[i].current_size_bytes);
    cJSON_AddItemToArray(jsonArray, json1);
  }

  cJSON_AddItemToObject(json, message_type_discover_files, jsonArray);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_init_json(const cr_FileTransferInit *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, key_type_file_id, request->file_id);
  cJSON_AddNumberToObject(json1, key_type_file_read_write, request->read_write);
  cJSON_AddNumberToObject(json1, key_type_request_offset,
                          request->request_offset);
  cJSON_AddNumberToObject(json1, key_type_transfer_length, request->transfer_length);
  cJSON_AddNumberToObject(json1, key_type_transfer_id, request->transfer_id);
  cJSON_AddNumberToObject(json1, key_type_file_messages_per_ack, 
                          request->messages_per_ack);
  cJSON_AddNumberToObject(json1, key_type_timeout_in_ms,
                          request->timeout_in_ms);

  cJSON_AddItemToObject(json, message_type_transfer_init, json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_init_response_json(
    const cr_FileTransferInitReply *response) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, "result", response->result);
  cJSON_AddNumberToObject(json1, "transfer_id", response->transfer_id);
  cJSON_AddNumberToObject(json1, "preferred_ack_rate", response->preferred_ack_rate);
  if (response->result != 0)
    cJSON_AddStringToObject(json1, "error_message", response->error_message);
  cJSON_AddItemToObject(json, message_type_transfer_init, json1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_transfer_data_json(const cr_FileTransferData *request) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json1, key_type_transfer_id, request->transfer_id);
  cJSON_AddNumberToObject(json1, key_type_file_message_number,  request->message_number);
  cJSON_AddNumberToObject(json1, key_type_file_message_size, request->message_data.size);
  // cJSON_AddRawToObject(json1, key_type_file_message_data, (char *)request->message_data.bytes);
  cJSON_AddNumberToObject(json1, key_type_file_crc32, request->crc32);
  cJSON_AddItemToObject(json, message_type_transfer_data, json1);

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
  cJSON_AddNumberToObject(json1, key_type_file_id, request->transfer_id);
  cJSON_AddNumberToObject(json1, key_type_file_message_number,
                          request->message_number);
  // cJSON_AddRawToObject(json1, key_type_file_message_data, (char *)request->message_data.bytes);
  cJSON_AddNumberToObject(json1, key_type_file_crc32, request->crc32);
  cJSON_AddItemToObject(json, message_type_transfer_data, json1);

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
  cJSON_AddItemToObject(json, message_type_transfer_data_notification, json1);

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
  cJSON_AddItemToObject(json, message_type_param_info, jsonArray);
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

      cJSON_AddNumberToObject(json_1, key_type_param_id,
                              response->parameter_infos[i].id);
      cJSON_AddNumberToObject(json_1, key_type_param_data_type,
                              response->parameter_infos[i].data_type);
      cJSON_AddNumberToObject(json_1, key_type_param_size_in_bytes,
                              response->parameter_infos[i].size_in_bytes);
      cJSON_AddStringToObject(json_1, key_type_name,
                              response->parameter_infos[i].name);
      cJSON_AddStringToObject(json_1, key_type_description,
                              response->parameter_infos[i].description);
      cJSON_AddStringToObject(json_1, key_type_param_unit,
                              response->parameter_infos[i].units);
      cJSON_AddNumberToObject(json_1, key_type_param_range_min,
                              response->parameter_infos[i].range_min);
      cJSON_AddNumberToObject(json_1, key_type_param_range_max,
                              response->parameter_infos[i].range_max);
      cJSON_AddNumberToObject(json_1, key_type_access,
                              response->parameter_infos[i].access);
      cJSON_AddNumberToObject(json_1, key_type_storage_location,
                              response->parameter_infos[i].storage_location);

      cJSON_AddItemToArray(jsonArray, json_1);
    }
    cJSON_AddItemToObject(json, message_type_param_info, jsonArray);
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

  cJSON_AddNumberToObject(json, "associated_pid",
                          response->associated_pid);
  cJSON_AddNumberToObject(json, "data_type",
                          response->data_type);
  cJSON_AddNumberToObject(json, "enumerations_count",
                          response->enumerations_count);
  if (response->enumerations_count > 0) 
  {
    cJSON *jsonArray2 = cJSON_CreateArray();
    for (size_t i = 0; i < response->enumerations_count; i++) 
    {
      cJSON *json1 = cJSON_CreateObject();
      cJSON_AddNumberToObject(json1, "id",
                              response->enumerations[i].id);
      cJSON_AddStringToObject(json1, "name",
                              response->enumerations[i].name);
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
  return message_util_get_null_value_json(message_type_discover_streams);
}

/** Commands */
char *message_util_discover_commands_json() {
  return message_util_get_null_value_json(message_type_discover_commands);
}

char *message_util_discover_commands_response_json(
    const cr_DiscoverCommandsResult *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();
  if (payload->available_commands_count > 0) {
    cJSON *json_1 = cJSON_CreateObject();
    for (size_t i = 0; i < payload->available_commands_count; i++) {

      cJSON_AddNumberToObject(json_1, key_type_id,
                              payload->available_commands[i].id);
      cJSON_AddStringToObject(json_1, key_type_name,
                              payload->available_commands[i].name);
    }
    cJSON_AddItemToArray(jsonArray, json_1);
  }

  cJSON_AddItemToObject(json, message_type_discover_commands, jsonArray);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_send_command_json(const cr_SendCommand *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_1, key_type_command, payload->command_id);
  cJSON_AddItemToObject(json, message_type_send_command, json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *
message_util_send_command_response_json(const cr_SendCommandResult *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_1, key_type_result, payload->result);
  cJSON_AddItemToObject(json, message_type_send_command, json_1);

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

  cJSON_AddStringToObject(json_1, key_type_message_data,
                          payload->message_data);

  cJSON_AddItemToObject(json, message_type_cli_notification, json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

/** Params */
char *message_util_read_param_json(const cr_ParameterRead *request) {

  // // I3_LOG(LOG_MASK_REACH, "message_util_read_param_json\n");

  cJSON *json = cJSON_CreateObject();
  cJSON *jsonArray = cJSON_CreateArray();
  if (request->parameter_ids_count > 0) {
    for (size_t i = 0; i < request->parameter_ids_count; i++) {
      cJSON_AddItemToArray(jsonArray,
                           cJSON_CreateNumber(request->parameter_ids[i]));
    }
  }
  cJSON_AddItemToObject(json, message_type_read_param, jsonArray);

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
      cJSON_AddNumberToObject(json_1, key_type_param_id,
                              response->values[i].parameter_id);

      char typeStr[16];
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
    cJSON_AddItemToObject(json, message_type_read_param, jsonArray);
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
      cJSON_AddNumberToObject(json_1, key_type_param_id,
                              payload->values[i].parameter_id);
      switch (payload->values[i].which_value)
      {
      case cr_ParameterValue_uint32_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.uint32_value);
          break;
      case cr_ParameterValue_sint32_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.sint32_value);
          break;
      case cr_ParameterValue_float32_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.float32_value);
          break;
      case cr_ParameterValue_uint64_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.uint64_value);
          break;
      case cr_ParameterValue_sint64_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.sint64_value);
          break;
      case cr_ParameterValue_float64_value_tag:
          cJSON_AddNumberToObject(json_1, key_type_param_value,
                                  payload->values[i].value.float64_value);
          break;
      case cr_ParameterValue_bool_value_tag:
          cJSON_AddBoolToObject(json_1, key_type_param_value,
                                payload->values[i].value.bool_value);
          break;
      case cr_ParameterValue_string_value_tag:
          cJSON_AddStringToObject(json_1, key_type_param_value,
                                  payload->values[i].value.string_value);
          break;
      default:
          break;
      }

      cJSON_AddItemToArray(jsonArray, json_1);
    }
    cJSON_AddItemToObject(json, message_type_write_param, jsonArray);
  }

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

static char sBuffer[32];
char *
message_util_write_param_response_json(const cr_ParameterWriteResult *payload) {
#if 1
  sprintf(sBuffer, "  write param response: %d", (int)payload->result);
  return sBuffer;
#else

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_1, key_type_result, payload->result);
  cJSON_AddItemToObject(json, message_type_write_param, json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
#endif
}

char *message_util_ping_json(const cr_PingRequest *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddRawToObject(json_1, key_type_echo_data,
                       (char *)payload->echo_data.bytes);
  cJSON_AddItemToObject(json, message_type_ping, json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}

char *message_util_ping_response_json(const cr_PingResponse *payload) {

  cJSON *json = cJSON_CreateObject();
  cJSON *json_1 = cJSON_CreateObject();
  cJSON_AddRawToObject(json_1, key_type_echo_data,
                       (char *)payload->echo_data.bytes);
  cJSON_AddNumberToObject(json_1, key_type_signal_strength,
                          payload->signal_strength);
  cJSON_AddItemToObject(json, message_type_ping, json_1);

  // convert the cJSON object to a JSON string
  char *json_str = cJSON_Print(json);

  // free the JSON string and cJSON object
  cJSON_Delete(json);

  return json_str;
}
