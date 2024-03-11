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
/**
 * @file      message_util.h
 * @brief     Utility functions to print out Reach messages in a human readable 
 *            format.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 */

#ifndef __MESSAGE_UTIL_H__
#define __MESSAGE_UTIL_H__

    #include "reach.pb.h"
    #include "reach-server.h"

  // None of these message utilities are required without logging.
  #ifndef NO_REACH_LOGGING
    const char *msg_type_string(int32_t);

    char *message_util_get_null_value_json(const char *);
    char *message_util_get_device_info_json();
    char *message_util_get_device_info_response_json(const cr_DeviceInfoResponse *);

    char *message_util_ping_json(const cr_PingRequest *);
    char *message_util_ping_response_json(const cr_PingResponse *);

    #ifdef INCLUDE_PARAMETER_SERVICE
        char *message_util_param_info_json(const cr_ParameterInfoRequest *);
        char *message_util_param_info_response_json(const cr_ParameterInfoResponse *);
        char *message_util_param_info_ex_response_json(const cr_ParamExInfoResponse *);
        char *message_util_read_param_json(const cr_ParameterRead *);
        char *message_util_read_param_response_json(const cr_ParameterReadResult *);
        char *message_util_write_param_json(const cr_ParameterWrite *);
        char *message_util_write_param_response_json(const cr_ParameterWriteResult *);
        // char *message_util_write_param_request_json(const cr_ParameterWrite *);
        char *message_util_config_notify_param_json(const cr_ParameterNotifyConfigResponse *);
    #endif  // INCLUDE_PARAMETER_SERVICE

    #ifdef INCLUDE_FILE_SERVICE
        char *message_util_discover_files_json();
        char *message_util_discover_files_response_json(const cr_DiscoverFilesResponse *);
        char *message_util_transfer_init_json(const cr_FileTransferInit *);
        char *message_util_transfer_init_response_json(const cr_FileTransferInitResponse *);
        char *message_util_transfer_data_json(const cr_FileTransferData *);
        char *message_util_transfer_data_response_json(const cr_FileTransferData *);
        char *message_util_transfer_data_notification_json(
                const cr_FileTransferDataNotification *);
    #endif // def INCLUDE_FILE_SERVICE

    #ifdef INCLUDE_STREAM_SERVICE
        // Streams
        char *message_util_discover_streams_json();
    #endif // def INCLUDE_STREAM_SERVICE


    #ifdef INCLUDE_COMMAND_SERVICE
        // Commands
        char *message_util_discover_commands_json();
        char *
            message_util_discover_commands_response_json(const cr_DiscoverCommandsResponse *);
        char *message_util_send_command_json(const cr_SendCommand *);
        char *message_util_send_command_response_json(const cr_SendCommandResponse *);
    #endif // def INCLUDE_COMMAND_SERVICE

    #ifdef INCLUDE_CLI_SERVICE
        // CLI
        char *message_util_cli_notification_json(const cr_CLIData *);
    #endif // def INCLUDE_CLI_SERVICE

    #ifdef INCLUDE_TIME_SERVICE
        // Time
        char *message_util_time_set_response_json(const cr_TimeSetResponse *payload);
        char *message_util_time_get_response_json(const cr_TimeGetResponse *payload);
        char *message_util_time_set_request_json(const cr_TimeSetRequest *payload);
        char *message_util_time_get_request_json(const cr_TimeGetRequest *payload); 
    #endif // def INCLUDE_TIME_SERVICE

    #ifdef INCLUDE_WIFI_SERVICE
        char *message_util_discover_wifi_request_json(const cr_DiscoverWiFiRequest *payload);
        char *message_util_WiFi_connect_request_json(const cr_WiFiConnectionRequest *payload);
        char *message_util_discover_wifi_response_json(cr_DiscoverWiFiResponse *payload);
        char *message_util_WiFi_connect_response_json(cr_WiFiConnectionResponse *payload);
    #endif  // def INCLUDE_WIFI_SERVICE

  #endif // ndef NO_REACH_LOGGING
#endif /* __MESSAGE_UTIL_H__ */
