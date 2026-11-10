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
 * "cr_private.h" defines things internal to the Reach stack.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_private.h
 * @brief     defines things internal to the Reach stack. In a C++ system these 
 *            would be private members.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygnus Reach firmware stack is shared under an MIT license.
 */

#ifndef _CR_PRIVATE_H
#define _CR_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "reach-server.h"
#include "cr_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

    void pvtCrFile_reset_session(void);

/* The application allocates one of these for each Reach session. The stack
 * obtains the currently selected instance through cr_get_state(). */
typedef union {
    cr_PingResponse ping_response;
    cr_DeviceInfoResponse device_info_response;
#ifdef INCLUDE_PARAMETER_SERVICE
    cr_ParameterInfoResponse parameter_info_response;
    cr_ParamExInfoResponse parameter_ex_info_response;
    cr_ParameterReadResponse parameter_read_response;
    cr_ParameterWriteResponse parameter_write_response;
    cr_DiscoverParameterNotificationsResponse discover_notifications_response;
#if NUM_SUPPORTED_PARAM_NOTIFY != 0
    cr_ParameterNotifyConfigResponse notify_config_response;
#endif
#endif
#ifdef INCLUDE_FILE_SERVICE
    cr_DiscoverFilesResponse discover_files_response;
    cr_FileTransferResponse file_transfer_response;
    cr_FileTransferData file_transfer_data;
    cr_FileTransferDataNotification file_transfer_data_notification;
    cr_FileEraseResponse file_erase_response;
#endif
#ifdef INCLUDE_STREAM_SERVICE
    cr_DiscoverStreamsResponse discover_streams_response;
    cr_StreamResponse stream_response;
#endif
#ifdef INCLUDE_COMMAND_SERVICE
    cr_DiscoverCommandsResponse discover_commands_response;
    cr_SendCommandResponse send_command_response;
#endif
#ifdef INCLUDE_CLI_SERVICE
    cr_CLIData cli_data;
#endif
#ifdef INCLUDE_TIME_SERVICE
    cr_TimeSetResponse time_set_response;
    cr_TimeGetResponse time_get_response;
#endif
#ifdef INCLUDE_WIFI_SERVICE
    cr_DiscoverWiFiResponse discover_wifi_response;
    cr_WiFiConnectionResponse wifi_connection_response;
#endif
} cr_response_storage_t;

typedef struct {
    cr_FileTransferState state;
    uint32_t transfer_id;
    int32_t file_id;
    int32_t timeout_in_ms;
    uint32_t request_offset;
    uint32_t transfer_length;
    uint8_t read_write;
    uint32_t message_number;
    int32_t checksum;
    uint32_t messages_per_ack;
    uint32_t messages_until_ack;
    uint32_t bytes_transfered;
    bool use_checksum;
} cr_file_transfer_state_t;

typedef struct {
    uint8_t encoded_message_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
    size_t encoded_message_size;
    cr_ReachMessage uncoded_message_structure;
    cr_response_storage_t uncoded_response_buffer ALIGN_TO_WORD;
    uint8_t encoded_payload_buffer[CR_CODED_BUFFER_SIZE - 4] ALIGN_TO_WORD;
    size_t encoded_payload_size;
    uint8_t encoded_response_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
    size_t encoded_response_size;
    uint8_t raw_notification[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
    uint8_t coded_notification[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
    size_t encoded_notification_size;
    cr_ReachMessage notification_message;
    cr_ReachMessageTypes continued_message_type;
    uint32_t num_remaining_objects;
    uint32_t transaction_id, client_id, endpoint_id;
    bool error_reported;
    uint8_t requested_wifi_index;
    unsigned int requested_command_index;
    char advertised_name[APP_ADVERTISED_NAME_LENGTH];
    int call_count;
    uint32_t current_ticks;
    bool comm_link_is_connected;
    uint8_t short_error_buffer[16] ALIGN_TO_WORD;
    uint8_t async_error_buffer[CR_CODED_BUFFER_SIZE - 4] ALIGN_TO_WORD;
    size_t max_buffer_size;
    char stack_version[16], proto_version[16];
    uint8_t client_protocol_version[3];
    bool test_header;
    uint32_t decoded_transaction_id;

#ifdef INCLUDE_FILE_SERVICE
    cr_file_transfer_state_t file_transfer;
    bool watchdog_active;
    uint32_t watchdog_period, watchdog_target;
#endif
#ifdef INCLUDE_PARAMETER_SERVICE
    uint32_t num_ex_this_pid;
    int16_t requested_param_array[REACH_COUNT_PARAMS_IN_REQUEST];
    uint8_t requested_param_info_count, requested_param_index;
    uint8_t requested_notify_count, requested_param_read_count;
    bool discover_notify_all;
#if NUM_SUPPORTED_PARAM_NOTIFY != 0
    uint32_t notifications_sent;
    cr_ParameterNotifyConfig param_notify_list[NUM_SUPPORTED_PARAM_NOTIFY];
    cr_ParameterValue last_param_values[NUM_SUPPORTED_PARAM_NOTIFY];
    uint8_t requested_notify_index;
#endif
#endif

    uint32_t log_mask;
    bool ble_errors_detected;
#ifdef DEV_BUILD
    char fatal_error_buffer[256];
#endif
#ifdef INCLUDE_CLI_SERVICE
    char remote_cli_buffer[REACH_ERROR_BUFFER_LEN];
    bool remote_cli_enabled;
    const char *command_line;
#endif
} cr_state_t;

cr_state_t *cr_get_state(void);

    /// <summary>
    /// Returns the state of the challenge key which may block 
    /// access to the Reach interface 
    /// </summary>
    bool crcb_challenge_key_is_valid(void);

    ///  
    /// pvtCrFile_ functions support the (optional) files service. 
    ///  
    /// Private function to discover files 
    int pvtCrFile_discover(const cr_DiscoverFiles *request,
                              cr_DiscoverFilesResponse *response);
    /// Private function for file transfer init
    int pvtCrFile_transfer_init(const cr_FileTransferRequest *request,
                             cr_FileTransferResponse *response);
    /// Private function for file transfer data
    int pvtCrFile_transfer_data(const cr_FileTransferData *dataTransfer,
                             cr_FileTransferDataNotification *response);
    /// Private function for file transfer data notification
    int pvtCrFile_transfer_data_notification(const cr_FileTransferDataNotification *request,
                                             cr_FileTransferData *dataTransfer);
    /// Private function for file erase
    int pvtCrFile_erase_file(const cr_FileEraseRequest *request,
                             cr_FileEraseResponse *response);

    /// <summary>
    /// The file service includes a timeout Watchdog. 
    /// 0 ms disables watchdog. 
    /// </summary> 
    void pvtCr_watchdog_start_timeout(uint32_t msec, uint32_t ticks);

    /// resets the timeout period to original
    void pvtCr_watchdog_stroke_timeout(uint32_t ticks);

    /// disables the watchdog
    void pvtCr_watchdog_end_timeout(void);

    /// if active, compares ticks to expected timeout.
    /// return 1 if timeout occurred
    int pvtCr_watchdog_check_timeout(uint32_t ticks);

    ///  
    /// pvtCrParam_ functions support the (optional) parameters 
    /// service. 
    ///  

    ///  Private helper function to discover parameters
    int pvtCrParam_discover_parameters(const cr_ParameterInfoRequest *,
                                       cr_ParameterInfoResponse *);

    ///  Private helper function to discover extended parameters
    int pvtCrParam_discover_parameters_ex(const cr_ParameterInfoRequest *,
                                          cr_ParamExInfoResponse *);

    ///  Private helper function to discover current parameter
    ///  notifications
    int pvtCrParam_discover_notifications(const cr_DiscoverParameterNotifications *,
                                          cr_DiscoverParameterNotificationsResponse *);

    /**
    * @brief   cr_get_active_notify_count
    * @return  How many parameter notifications are active
    */
    size_t cr_get_active_notify_count(void);

    ///  Private helper function to read a parameter
    int pvtCrParam_read_param(const cr_ParameterRead *, 
                              cr_ParameterReadResponse *);
    ///  Private helper function to write a parameter
    int pvtCrParam_write_param(cr_ParameterWrite *, 
                               cr_ParameterWriteResponse *);

  #if NUM_SUPPORTED_PARAM_NOTIFY != 0
    ///  Private helper functions to configure parameter
    ///  notifications
    int pvtCrParam_param_enable_notify(const cr_ParameterEnableNotifications *,
                                       cr_ParameterNotifyConfigResponse *);
    int pvtCrParam_param_disable_notify(const cr_ParameterDisableNotifications *,
                                       cr_ParameterNotifyConfigResponse *);

    /**
    * @brief   pvtCr_notify_param
    * @details parameter notifications are handled by the Reach stack. The stack 
    * will use the read parameters to be notified on an appropriate timescale and 
    *          send notifications if enough changes. Signals the
    *          client that this parameter may have changed.
    * @param   param (input) pointer to the parameter data that has 
    *                changed.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration if the notification fails.
    */
    int pvtCr_notify_param(cr_ParameterValue *param);
  #endif // NUM_SUPPORTED_PARAM_NOTIFY != 0

    ///  Private helper function to check for parameter
    ///  notifications
    void pvtCrParam_check_for_notifications(void);

    /**
    * @brief   pvtCr_compare_proto_version 
    * @details Used to support backward compatibility.
    * @return  Returns 0 if the client's protocol version is equal 
    *          to the specified version.  A positive value means the
    *          client is greater (newer).  A negative value means
    *          the client version is older than the specified
    *          version.
    */
    int pvtCr_compare_proto_version(uint8_t major, uint8_t minor, uint8_t patch);



#ifdef INCLUDE_STREAM_SERVICE
    // Streams
    int pvtCr_discover_streams(const cr_DiscoverStreams *,
                                       cr_DiscoverStreamsResponse *);
    int pvtCr_open_stream(const cr_StreamOpen *,
                                       cr_StreamResponse *);
    int pvtCr_close_stream(const cr_StreamClose *,
                                       cr_StreamResponse *);
    // Write: The stream flows to the device.
    int pvtCr_stream_receive_notification(cr_StreamData *data);

    int pvtCr_stream_send_notification(cr_StreamData *data);

#endif // def INCLUDE_STREAM_SERVICE


    /**
    * @brief   pvtCr_cli_respond
    * @details When the device supports a CLI it is expected to share anything 
    *          printed to the CLI back to the stack for remote display using
    *          pvtCr_cli_respond(). The implementation can call this at any
    *          time to print to the remote CLI
    * @param   cli A string being sent back to the remote CLI.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
    *          cr_ErrorCodes_ enumeration.
    */
    int pvtCr_cli_respond(char *cli);

    /**
    * @brief   pvtCr_notify_error
    * @details Called by cr_report_error().  Can be called at any 
    *          point to send error messages to the client.
    * @param   err Pointer to a structure with a code and a string.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
    *          the cr_ErrorCodes_ enumeration
    */
    int pvtCr_notify_error(cr_ErrorReport *err);

    /**
    * @brief   pvtCr_encode_message
    * @details Takes a raw reach message and encodes it to protobuf 
    *          format. 
    * @param   err message_type : Type of message, from the enum
    * @param   payload : Pointer to the data to be encoded
    * @param   hdr : NULL for notifications. Otherwise the header to 
    *              be encoded.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
    *          the cr_ErrorCodes_ enumeration
    */
    int pvtCr_encode_message(cr_ReachMessageTypes message_type,
                             const void *payload,
                             cr_ReachMessageHeader *hdr);

    /**
    * @brief   pvtCr_get_raw_notification_buffer
    * @details For notifications, get the raw buffer into which the 
    *          data can be copied.
    * @param   pRaw: The buffer
    * @param   pSize: The size of the buffer
    */
    void pvtCr_get_raw_notification_buffer(uint8_t **pRaw, size_t *pSize);

    /**
    * @brief   pvtCr_get_coded_notification_buffers
    * @details For notifications, called after encoding to get the 
    *          coded buffer for transmission and its size.
    * @param   pRaw: The buffer
    * @param   pSize: The size of the coded data in the buffer
    */
    void pvtCr_get_coded_notification_buffers(uint8_t **pCoded, size_t *pSize);

    void pvtCr_sanitize_string_to_utf8(char *input);


#ifdef __cplusplus
}
#endif

#endif  // ndef _CR_PRIVATE_H

