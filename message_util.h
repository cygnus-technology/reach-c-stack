/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023
 *
 * @author  i3 Product Development
 *          Wei Zhou wei.zhou@i3pd.com
 *
 * @version 0.0.1
 * @date    2023-08-21
 *
 ********************************************************************************************/

#ifndef __MESSAGE_UTIL_H__
#define __MESSAGE_UTIL_H__

#include "reach.pb.h"

#define MAX_BUFFER_SIZE 256

extern const char *message_type_unknown;
extern const char *message_type_ping;
extern const char *message_type_get_device_info;
extern const char *message_type_discover_files;
extern const char *message_type_transfer_init;
extern const char *message_type_transfer_data;
extern const char *message_type_transfer_data_notification;
extern const char *message_type_read_file;
extern const char *message_type_write_file;
extern const char *message_type_param_info;
extern const char *message_type_read_param;
extern const char *message_type_write_param;

// Streams
extern const char *message_type_discover_streams;
extern const char *message_type_manage_stream;
extern const char *message_type_stream_data_notification;

// Commands
extern const char *message_type_discover_commands;
extern const char *message_type_send_command;

// CLI
extern const char *message_type_discover_cli;
extern const char *message_type_cli_notification;

// key types
extern const char *key_type_id;
extern const char *key_type_name;
extern const char *key_type_description;
extern const char *key_type_unit;
extern const char *key_type_data_type;
extern const char *key_type_access;
extern const char *key_type_storage_location;
extern const char *key_type_echo_data;
extern const char *key_type_signal_strength;

extern const char *key_type_timeout_in_ms;
extern const char *key_type_request_offset;
extern const char *key_type_transfer_length;
extern const char *key_type_has_result;
extern const char *key_type_result;
extern const char *key_type_result_value;
extern const char *key_type_result_string;
extern const char *key_type_transfer_id;
extern const char *key_type_max_bytes_per_message;
extern const char *key_type_is_complete;

extern const char *key_type_protocol_version;
extern const char *key_type_device_description;
extern const char *key_type_firmware_version;
extern const char *key_type_hash;

extern const char *key_type_file_id;
extern const char *key_type_file_name;
extern const char *key_type_file_access;
extern const char *key_type_file_storage_location;
extern const char *key_type_file_current_byte_length;
extern const char *key_type_file_max_byte_length;
extern const char *key_type_file_max_message_length;
extern const char *key_type_file_offset;
extern const char *key_type_file_checksum;
extern const char *key_type_file_data;
extern const char *key_type_file_transfer_type;
extern const char *key_type_file_message_number;
extern const char *key_type_file_message_data;
extern const char *key_type_file_messages_to_send;
extern const char *key_type_file_bytes_transfered;
extern const char *key_type_file_crc32;

extern const char *key_type_stream_id;
extern const char *key_type_stream_command;
extern const char *key_type_command;
extern const char *key_type_supported_cli_streams;

extern const char *key_type_message_data;

extern const char *key_type_param_id;
extern const char *key_type_param_name;
extern const char *key_type_param_data_type;
extern const char *key_type_param_size_in_bytes;
extern const char *key_type_param_description;
extern const char *key_type_param_unit;
extern const char *key_type_param_value_range;
extern const char *key_type_param_range_min;
extern const char *key_type_param_range_max;
extern const char *key_type_param_value;

extern const char *file_access_type_read;
extern const char *file_access_type_write;
extern const char *file_access_type_read_write;

const char *get_message_type(int32_t);

char *message_util_get_null_value_json(const char *);
char *message_util_get_device_info_json();
char *message_util_get_device_info_response_json(const cr_DeviceInfoResponse *);

char *message_util_ping_json(const cr_PingRequest *);
char *message_util_ping_response_json(const cr_PingResponse *);

char *message_util_discover_files_json();
char *message_util_discover_files_response_json(const cr_DiscoverFilesReply *);
char *message_util_transfer_init_json(const cr_FileTransferInit *);
char *
message_util_transfer_init_response_json(const cr_FileTransferInitReply *);
char *message_util_transfer_data_json(const cr_FileTransferData *);
char *message_util_transfer_data_response_json(const cr_FileTransferData *);
char *message_util_transfer_data_notification_json(
    const cr_FileTransferDataNotification *);

// Streams
char *message_util_discover_streams_json();

// Commands
char *message_util_discover_commands_json();
char *
message_util_discover_commands_response_json(const cr_DiscoverCommandsResult *);
char *message_util_send_command_json(const cr_SendCommand *);
char *message_util_send_command_response_json(const cr_SendCommandResult *);

// CLI
char *message_util_cli_notification_json(const cr_CLIData *);

char *message_util_param_info_json(const cr_ParameterInfoRequest *);
char *message_util_param_info_response_json(const cr_ParameterInfoResponse *);
char *message_util_param_info_ex_response_json(const cr_ParamExInfoResponse *);
char *message_util_read_param_json(const cr_ParameterRead *);
char *message_util_read_param_response_json(const cr_ParameterReadResult *);
char *message_util_write_param_json(const cr_ParameterWrite *);
char *message_util_write_param_response_json(const cr_ParameterWriteResult *);

char *message_util_write_param_request_json(const cr_ParameterWrite *);

#ifdef INCLUDE_TIME_SERVICE
    // Time
    char *message_util_time_set_response_json(const cr_TimeSetResult *payload);
    char *message_util_time_get_response_json(const cr_TimeGetResult *payload);
#endif // def INCLUDE_TIME_SERVICE

#endif /* __MESSAGE_UTIL_H__ */
