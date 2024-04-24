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
 * The Cygngus Reach firmware stack is shared under an MIT license.
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

    /// <summary>
    /// Private variables controlling the sort of continuing 
    /// transactions used by file transfers, etc. 
    /// </summary>

    /// The type of the current continued message
    extern cr_ReachMessageTypes pvtCr_continued_message_type;

    /// The number of continued objects (remaining)
    extern uint32_t             pvtCr_num_remaining_objects;

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
    void pvtCr_watchdog_end_timeout();

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


#ifdef __cplusplus
}
#endif

#endif  // ndef _CR_PRIVATE_H

