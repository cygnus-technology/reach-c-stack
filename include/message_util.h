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

    void message_util_log_device_info_request();
    void message_util_log_device_info_response(const cr_DeviceInfoResponse *);

    void message_util_log_ping_request(const cr_PingRequest *);
    void message_util_log_ping_response(const cr_PingResponse *);

    #ifdef INCLUDE_PARAMETER_SERVICE
        void message_util_log_param_info_request(const cr_ParameterInfoRequest *);
        void message_util_log_param_info_response(const cr_ParameterInfoResponse *);
        void message_util_log_param_info_ex_response(const cr_ParamExInfoResponse *);
        void message_util_log_read_param(const cr_ParameterRead *);
        void message_util_log_read_param_response(const cr_ParameterReadResponse *);
        void message_util_log_write_param(const cr_ParameterWrite *);
        void message_util_log_write_param_response(const cr_ParameterWriteResponse *);
        void message_util_log_config_notify_param(const cr_ParameterNotifyConfigResponse *);
        void message_util_log_notify_setup_request(const cr_DiscoverParameterNotifySetup *);
        void message_util_log_notify_setup_response(const cr_DiscoverParameterNotifySetupResponse *);
    #endif  // INCLUDE_PARAMETER_SERVICE

    #ifdef INCLUDE_FILE_SERVICE
        void message_util_log_discover_files();
        void message_util_log_discover_files_response(const cr_DiscoverFilesResponse *);
        void message_util_log_file_transfer_request(const cr_FileTransferRequest *);
        void message_util_log_file_transfer_response(const cr_FileTransferResponse *);
        void message_util_log_transfer_data(const cr_FileTransferData *);
        void message_util_log_transfer_data_response(const cr_FileTransferData *);
        void message_util_log_transfer_data_notification(bool is_request,
                const cr_FileTransferDataNotification *);
    #endif // def INCLUDE_FILE_SERVICE

    #ifdef INCLUDE_STREAM_SERVICE
        // Streams
        void message_util_log_discover_streams();
    #endif // def INCLUDE_STREAM_SERVICE


    #ifdef INCLUDE_COMMAND_SERVICE
        // Commands
        void message_util_log_discover_commands();
        void message_util_log_discover_commands_response(const cr_DiscoverCommandsResponse *);
        void message_util_log_send_command(const cr_SendCommand *);
        void message_util_log_command_response(const cr_SendCommandResponse *);
    #endif // def INCLUDE_COMMAND_SERVICE

    #ifdef INCLUDE_CLI_SERVICE
        // CLI
        void message_util_log_cli_notification(bool send, const cr_CLIData *);
    #endif // def INCLUDE_CLI_SERVICE

    #ifdef INCLUDE_TIME_SERVICE
        // Time
        void message_util_log_time_set_response(const cr_TimeSetResponse *payload);
        void message_util_log_time_get_response(const cr_TimeGetResponse *payload);
        void message_util_log_time_set_request(const cr_TimeSetRequest *payload);
        void message_util_log_time_get_request(const cr_TimeGetRequest *payload); 
    #endif // def INCLUDE_TIME_SERVICE

    #ifdef INCLUDE_WIFI_SERVICE
        void message_util_log_discover_wifi_request(const cr_DiscoverWiFiRequest *payload);
        void message_util_log_WiFi_connect_request(const cr_WiFiConnectionRequest *payload);
        void message_util_log_discover_wifi_response(cr_DiscoverWiFiResponse *payload);
        void message_util_log_WiFi_connect_response(cr_WiFiConnectionResponse *payload);
    #endif  // def INCLUDE_WIFI_SERVICE

  #endif // ndef NO_REACH_LOGGING
#endif /* __MESSAGE_UTIL_H__ */
