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
 * \brief "cr_stack.c" defines the API to the Cygnus Reach device stack
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_stack.c
 * @brief     Contains the core of the Cygnus Reach firmware 
 *            stack. The Reach stack depends on reach-server.h
 *            to control its configuration at build time.  The
 *            file reach-server.h should be part of the server
 *            application using the reach stack.
 * @brief TERMINOLOGY: 
 *      A transaction is a series of messages.
 *      A message has a header and a payload.
 *      The prompt is a received payload.
 *      The response is a generated payload.
 *      A file "transfer" is a series of messages terminated by
 *      an ACK.

 * @author    Chuck Peplinski
 * @date      2024-01-18
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

// from the applicaiton
#include "reach-server.h"

#include "cr_stack.h"
#include "cr_private.h"
#include "i3_log.h"

#include "pb_decode.h"
#include "pb_encode.h"

#include "message_util.h"
#include "reach_decode.h"
#include "reach_version.h"

//----------------------------------------------------------------------------
// static buffers used and reused by the reach stack.
// This is private data.
//----------------------------------------------------------------------------

// the fully encoded message is received in the sCr_encoded_message_buffer. 
/// @private
static uint8_t sCr_encoded_message_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;

/// @private
static size_t  sCr_encoded_message_size = 0;

// The message header is decoded into this buffer containing an encoded payload buffer: 
/// @private
static cr_ReachMessage sCr_uncoded_message_structure;

/// The payload buffers are slightly smaller than the CR_CODED_BUFFER_SIZE
/// so that the header can be added.
#define UNCODED_PAYLOAD_SIZE  (CR_CODED_BUFFER_SIZE-4)

// A decoded prompt payload. This can be reused from the encoded message buffer. 
// static uint8_t sCr_decoded_prompt_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD;
/// @private
static uint8_t *sCr_decoded_prompt_buffer = sCr_encoded_message_buffer;

// An uncoded response payload.
/// @private
static uint8_t sCr_uncoded_response_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD;

// The response payload is encoded into sCr_encoded_payload_buffer[]. 
/// @private
static uint8_t sCr_encoded_payload_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD; 
/// @private
static size_t sCr_encoded_payload_size; 
 
// The response payload is copied into the sCr_uncoded_message_structure

// The sCr_uncoded_message_structure is encoded into sCr_encoded_response_buffer[]  
/// @private
static uint8_t sCr_encoded_response_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
/// @private
static size_t  sCr_encoded_response_size = 0;


///----------------------------------------------------------------------------
/// private but global variables shared with the Reach stack
///----------------------------------------------------------------------------
cr_ReachMessageTypes pvtCr_continued_message_type;
uint32_t pvtCr_num_remaining_objects = 0;

///----------------------------------------------------------------------------
/// static (private) "member" variables
///----------------------------------------------------------------------------
static uint32_t sCr_transaction_id = 0;
static uint32_t sCr_client_id = 0;
static uint32_t sCr_endpoint_id = 0;
static bool sCr_error_reported = false;

///----------------------------------------------------------------------------
/// Forward declarations of static (private) "member" functions
///----------------------------------------------------------------------------

/// @private
static int handle_coded_prompt();

/// @private
static int 
handle_message(const cr_ReachMessageHeader *hdr, const uint8_t *data, size_t size);

/// <summary>
/// Respond to a ping request
/// </summary>
/// <param name="request"> The triggering message</param>
/// <param name="response">Response delivered here</param>
/// <returns></returns>
static int handle_ping(const cr_PingRequest *request, cr_PingResponse *response);


/// <summary>
/// Respond to a request for device info
/// </summary>
/// <param name="request"> The triggering message</param>
/// <param name="response">Response delivered here</param>
/// <returns></returns>
static int handle_get_device_info(const cr_DeviceInfoRequest *request,
                                      cr_DeviceInfoResponse *response);

#ifdef INCLUDE_STREAM_SERVICE
    // Streams
    static int handle_discover_streams(const cr_StreamsRequest *,
                                       cr_StreamsResponse *);

    static int handle_stream_data_notification(uint8_t *,
                                               pb_size_t, uint8_t *,
                                               size_t *);
#endif // def INCLUDE_STREAM_SERVICE

#ifdef INCLUDE_COMMAND_SERVICE
    // Commands 
    static int handle_discover_commands(const cr_DiscoverCommands *,
                                        cr_DiscoverCommandsResponse *);
    static int handle_send_command(const cr_SendCommand *, cr_SendCommandResponse *);

    static unsigned int sCr_requested_command_index = 0;
#endif // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
    // CLI
    static int handle_cli_notification(const cr_CLIData *request, cr_CLIData *);
#endif // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_TIME_SERVICE
    static int handle_time_set(const cr_TimeSetRequest *request, 
                               cr_TimeSetResponse *response);
    static int handle_time_get(const cr_TimeGetRequest *request, 
                               cr_TimeGetResponse *response);
#endif  // def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_WIFI_SERVICE
    static int handle_discover_wifi(const cr_DiscoverWiFiRequest *request, 
                                cr_DiscoverWiFiResponse *response);
    static int handle_wifi_connect(const cr_WiFiConnectionRequest *request, 
                                      cr_WiFiConnectionResponse *response);
#endif  // def INCLUDE_WIFI_SERVICE


// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static
int sCr_encode_message(cr_ReachMessageTypes message_type,       // in
                      const void *payload,                      // in:  to be encoded
                      cr_ReachMessageHeader *hdr);              // in



// Internal encode functions, to be deprecated.  Use sCr_encode_message.
static
bool encode_reach_payload(cr_ReachMessageTypes message_type,    // in
                          const void *data,                     // in:  data to be encoded
                          uint8_t *buffer,                      // out: Encode to here.
                          pb_size_t buffer_size,                // in:  max size of encoded data
                          size_t *encode_size);                 // out: encoded data size

static
bool encode_reach_message(const cr_ReachMessage *message,       // in:  message to be encoded
                          uint8_t *buffer,                      // out: Buffer to encode into
                          size_t buffer_size,                   // in:  max size of encoded message
                          size_t *encode_size);                 // out: actual size of encoded message.

static int handle_continued_transactions()
{
    int rval = 0;

    if (pvtCr_continued_message_type == cr_ReachMessageTypes_INVALID)
    {
        // I3_LOG(LOG_MASK_REACH, "%s(): No continued transactions.", __FUNCTION__);
        return cr_ErrorCodes_NO_DATA;  // no continued transaction.
    }

    cr_ReachMessageTypes encode_message_type = pvtCr_continued_message_type;
    switch (pvtCr_continued_message_type)
    {
    #ifdef INCLUDE_PARAMETER_SERVICE
    case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued dp.", __FUNCTION__);
        rval = 
            pvtCrParam_discover_parameters(NULL, 
                                       (cr_ParameterInfoResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued dpx.", __FUNCTION__);
        rval = 
            pvtCrParam_discover_parameters_ex(NULL, 
                                       (cr_ParamExInfoResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_READ_PARAMETERS:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued rp.", __FUNCTION__);
        rval = pvtCrParam_read_param(NULL, (cr_ParameterReadResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS:
        rval = pvtCrParam_discover_notifications(NULL, (cr_DiscoverParameterNotificationsResponse *)sCr_uncoded_response_buffer);
        break;
    #endif  // def INCLUDE_PARAMETER_SERVICE

    #ifdef INCLUDE_COMMAND_SERVICE
    case cr_ReachMessageTypes_DISCOVER_COMMANDS:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued disc cmds.", __FUNCTION__);
        rval = 
            handle_discover_commands(NULL,
                                     (cr_DiscoverCommandsResponse *)sCr_uncoded_response_buffer);
        break;
    #endif  // def INCLUDE_COMMAND_SERVICE

    #ifdef INCLUDE_WIFI_SERVICE
    case cr_ReachMessageTypes_DISCOVER_WIFI:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued disc wifi.", __FUNCTION__);
        rval = 
            handle_discover_wifi(NULL,
                                     (cr_DiscoverWiFiResponse *)sCr_uncoded_response_buffer);
        break;
    #endif  // def INCLUDE_WIFI_SERVICE

    #ifdef INCLUDE_FILE_SERVICE
    case cr_ReachMessageTypes_TRANSFER_DATA:
        I3_LOG(LOG_MASK_REACH, "%s(): Continued rf.", __FUNCTION__);
        rval = pvtCrFile_transfer_data_notification(NULL, (cr_FileTransferData *)sCr_uncoded_response_buffer);
        encode_message_type = cr_ReachMessageTypes_TRANSFER_DATA;
        break;
    #endif // def INCLUDE_FILE_SERVICE

    default:
        LOG_ERROR("Continued type %d not written.", pvtCr_continued_message_type);
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA;
    }

    if (rval != 0)
        return rval;

    cr_ReachMessageHeader msg_header;
    memset(&msg_header, 0, sizeof(msg_header));
    msg_header.message_type      = encode_message_type;
    msg_header.endpoint_id       = sCr_endpoint_id;
    msg_header.client_id         = sCr_client_id;
    msg_header.transaction_id    = sCr_transaction_id;
    msg_header.remaining_objects = pvtCr_num_remaining_objects;

    rval = sCr_encode_message(encode_message_type,         // in
                             sCr_uncoded_response_buffer,  // in:  to be encoded
                             &msg_header);

    if (pvtCr_num_remaining_objects == 0)
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
    return rval;
}

/**
* @brief   cr_init
* @details To be called before starting the stack.
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error like cr_ErrorCodes_. 
*/
int cr_init() 
{
    return cr_ErrorCodes_NO_ERROR;
}

#ifndef APP_ADVERTISED_NAME_LENGTH
    /// APP_ADVERTISED_NAME_LENGTH is the length of the string
    /// holding the advertised name.  It can be set by the app in
    /// reach-server.h
  #define APP_ADVERTISED_NAME_LENGTH    REACH_SHORT_STRING_LEN
#endif

/// <summary>
/// sCr_advertised_name holds the name that will be advertised 
/// (via BLE).  This can be set programmatically. 
/// </summary>
char sCr_advertised_name[APP_ADVERTISED_NAME_LENGTH];


/**
* @brief   cr_set_advertised_name
* @details Sets the name of the device that should be advertised before 
*          connecting.  Used in BLE.  The length of the string is set by
*          APP_ADVERTISED_NAME_LENGTH which can be define in the application.
*          See reach-server.h.  Uses REACH_SHORT_STRING_LEN when
*          APP_ADVERTISED_NAME_LENGTH is not defined.  The code setting up the
*          communication link can retrieve this using cr_get_advertised_name().
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/
int cr_set_advertised_name(char *name, int length)
{
    strncpy(sCr_advertised_name, name, APP_ADVERTISED_NAME_LENGTH);
    if (length >= APP_ADVERTISED_NAME_LENGTH) 
        return APP_ADVERTISED_NAME_LENGTH;
    return cr_ErrorCodes_NO_ERROR;
}

/**
* @brief   cr_get_advertised_name
* @details Retrieves the name stored by cr_set_advertised_name().
* @return  pointer to a string of length REACH_SHORT_STRING_LEN.
*/
const char *cr_get_advertised_name()
{
    return (const char *)sCr_advertised_name;
}

/**
* @brief   cr_store_coded_prompt
* @details allows the application to store the prompt where the Reach stack can 
*          see it.  The byte data and length are copied into private storage.
*          This data is retrieved using crcb_get_coded_prompt().
* @param   data: The coded prompt to be stored. 
* @param   len : number of bytes to be stored. 
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/
int cr_store_coded_prompt(uint8_t *data, size_t len)
{
    affirm(len <= sizeof(sCr_encoded_message_buffer));

    memcpy(sCr_encoded_message_buffer, data, len);
    sCr_encoded_message_size = len;
    return cr_ErrorCodes_NO_ERROR;
}

/**
* @brief   cr_get_coded_response_buffer
* @details Retrieve the adress of the "coded response buffer".  This buffer 
*          contains the response to a prompt, coded according to protobuf specs,
*          to be transmitted to the client.  The stored coded length is zeroed
*          by this call.
* @param   ppResponse: Pointer to pointer to bytes.
* @param   pLen : pointer to the number of bytes for transmission.
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/
int cr_get_coded_response_buffer(uint8_t **ppResponse, size_t *pLen)
{
    *ppResponse = sCr_encoded_response_buffer;
    *pLen = sCr_encoded_response_size;
    if (sCr_encoded_response_size == 0)
        return cr_ErrorCodes_NO_DATA;
    sCr_encoded_response_size = 0;
    return cr_ErrorCodes_NO_ERROR;
}

// static uint32_t lastTick = 0;
static int sCr_CallCount = 0;
static uint32_t sCr_currentTicks = 0;

/**
* @brief   cr_process
* @details The application must call cr_process() regularly as it does most of 
*          the work required of Reach. The ticks parameter is expected to be a
*          monotonically increasing value representing the time since the system
*          started. This allows it to perform timing related tasks such as
*          notifications. cr_process() returns immediately if the device is not
*          connected to BLE.
* @param   ticks: A measure of time passed, typically milliseconds, but the 
*               units are not specified.
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code, however these are 
*          indicative only.  The non-zero returns indicate normal conditions.
*/
int cr_process(uint32_t ticks) 
{
    sCr_currentTicks = ticks;   // store it so others can use it.
    sCr_CallCount++;

    if (!cr_get_comm_link_connected())
        return cr_ErrorCodes_NO_ERROR;

  #ifdef INCLUDE_FILE_SERVICE
    int timeout = pvtCr_watchdog_check_timeout(ticks);
    if (timeout) {
        i3_log(LOG_MASK_ERROR, "Timeout watchdog expired.");
        pvtCr_watchdog_end_timeout();
    }
  #endif // def INCLUDE_FILE_SERVICE

    /*if (ticks - lastTick > 10001)
    {
        i3_log(LOG_MASK_ALWAYS, "\r\n--------%s: tick %d", __FUNCTION__, ticks);
        lastTick = ticks;
    }*/

    // clear buffers of previous data
    memset(&sCr_uncoded_message_structure,  0, sizeof(cr_ReachMessage));
    memset(sCr_uncoded_response_buffer,     0, sizeof(sCr_uncoded_response_buffer));
    memset(sCr_encoded_payload_buffer,      0, sizeof(sCr_encoded_payload_buffer));
    // memset(sCr_encoded_response_buffer,     0, sizeof(sCr_encoded_response_buffer));

    // Support for continued transactions:
    //   zero indicates valid data was produced.
    //   cr_ErrorCodes_NO_DATA indicates no data was produced.
    //   Other non-zero values indicate an error report was produced.
    int rval = handle_continued_transactions();
    if (rval == cr_ErrorCodes_NO_DATA)
    {
        // Gets the encoded buffer from the app.
        rval = crcb_get_coded_prompt(sCr_encoded_message_buffer, &sCr_encoded_message_size);
        if (rval == cr_ErrorCodes_NO_DATA)
        {
            sCr_encoded_message_size = 0;

            // check notifications when nothing else is happening.
            pvtCrParam_check_for_notifications();

            return cr_ErrorCodes_NO_DATA;
        }


        I3_LOG(LOG_MASK_REACH, TEXT_MAGENTA "Got a new prompt" TEXT_RESET);
        LOG_DUMP_WIRE("Rcvd prompt", sCr_encoded_message_buffer, sCr_encoded_message_size);
        rval = handle_coded_prompt(); // in case of error the reply is the error report
        sCr_encoded_message_size = 0;

        // these two cases require no response/reply
        if ((rval == cr_ErrorCodes_NO_DATA) || (rval == cr_ErrorCodes_NO_RESPONSE))
            return rval;

        if (rval && !sCr_error_reported)
        {
            // The functions called here must report their errors
            // and return the error code.  This is a backup.
            cr_report_error(rval, "Otherwise unreported error");
        }
        sCr_error_reported = false;
    }
    crcb_send_coded_response(sCr_encoded_response_buffer, sCr_encoded_response_size);

    return cr_ErrorCodes_NO_ERROR;
}


/**
* @brief   cr_get_current_ticks
* @details The tick count is passed in to cr_process(). This function gives 
*          other Reach functions access to that value. 
* @return  The same tick count passed into cr_process().
*/
uint32_t cr_get_current_ticks()
{
    // a 32 bit number will roll over in 49 days at 1kHz.
    return sCr_currentTicks;
}

static bool sCr_comm_link_is_connected = false;
/**
* @brief   cr_set_comm_link_connected
* @details The communication stack must inform the Reach stack of the status of 
*          the communication link. The integration must inform Reach when
*          the connection status changes. The Reach loop only runs when the
*          connection is valid. All parameter notifications are cleared when a
*          connection is established. The client must reenable notifications on
*          each connection.
* @param   connected true if connected.
*/
void cr_set_comm_link_connected(bool connected)
{ 
   if (!sCr_comm_link_is_connected && connected)
   {
       // we are newly connected, so clear any stale data.
       pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
       pvtCr_num_remaining_objects = 0;
       cr_clear_param_notifications();
       crcb_invalidate_challenge_key();
   }
   sCr_comm_link_is_connected = connected;
} 

/**
* @brief   cr_get_comm_link_connected
* @details Returns what was set using cr_set_comm_link_connected().
* @return  true if the communication link is connected.
*/
bool cr_get_comm_link_connected(void)
{ 
   return sCr_comm_link_is_connected;
} 

/**
* @brief   cr_report_error
* @details The system can report errors to the client.  This 
*          takes some memory and so users are given the option
*          to configure this with ERROR_REPORT_FORMAT defined in
*          reach-server.h.  ERROR_FORMAT_FULL causes another 240
*          byte buffer to be allocated so that the error string
*          can be forwarded to the client at any time without
*          sharing buffers.  ERROR_FORMAT_SHORT causes only the
*          error code to be passed to the client.
*          ERROR_FORMAT_LOG_ONLY disables the remote
*          notification and only prints the error code in the
*          local log.
* @param error_code : Use of the cr_ErrorCodes_ enum is encouraged but not 
*                   required.
* @param fmt : A printf-like string with variables. 
*/

#ifndef ERROR_REPORT_FORMAT
    #warning  ERROR_REPORT_FORMAT should be defined in reach-server.h
    void cr_report_error(int error_code, const char *fmt, ...)
    {
        (void)fmt;
        i3_log(LOG_MASK_ERROR, "cr_report_error(%d) is disabled", error_code);
    }
#else
  #if (ERROR_REPORT_FORMAT == ERROR_FORMAT_LOG_ONLY)
    void cr_report_error(int error_code, const char *fmt, ...)
    {
        (void)fmt;
        i3_log(LOG_MASK_ERROR, "cr_report_error(%d) to log only", err);
    }
  #elif (ERROR_REPORT_FORMAT == ERROR_FORMAT_SHORT)
    #define SHORT_ERROR_BUF_LEN  16
    // The asynchronous version requires a buffer.
    /// @private
    static uint8_t sCr_short_error_buffer[SHORT_ERROR_BUF_LEN] ALIGN_TO_WORD;

    void cr_report_error(int error_code, const char *fmt, ...)
    {
        (void)fmt;

        cr_ErrorReport *err = (cr_ErrorReport *)sCr_short_error_buffer;
        memset(err, 0, SHORT_ERROR_BUF_LEN);
        err->result_value = error_code;
        int ptr = snprintf(err->result_string,
                           SHORT_ERROR_BUF_LEN,
                           "Error %d.", error_code);
        crcb_notify_error(err);
        i3_log(LOG_MASK_ERROR, "Logged short error report, code %d:", error_code);
    }
  #else  // (ERROR_REPORT_FORMAT == ERROR_FORMAT_FULL)
    // The asynchronous version requires a buffer.
    /// @private
    static uint8_t sCr_async_error_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD;

    void cr_report_error(int error_code, const char *fmt, ...)
    {
        va_list args;

        cr_ErrorReport *err = (cr_ErrorReport *)sCr_async_error_buffer;
        err->result = error_code;

        va_start(args, fmt);
        int ptr = snprintf(err->result_message,
                           REACH_BYTES_IN_A_FILE_PACKET,
                           "Error %d: ", error_code);
        vsnprintf(&err->result_message[ptr],
                  REACH_BYTES_IN_A_FILE_PACKET-ptr, fmt, args);
        // force termination
        err->result_message[REACH_BYTES_IN_A_FILE_PACKET-1] = 0;
        va_end(args);
        // i3_log(LOG_MASK_WARN, "error string %d char", strlen(err->result_string));

        crcb_notify_error(err);
        i3_log(LOG_MASK_WARN, "Logged Error full report:");
        i3_log(LOG_MASK_ERROR, "%s", err->result_message);
    }
  #endif  // ERROR_REPORT_FORMAT ==
#endif // #ifndef ERROR_REPORT_FORMAT


// 
// Static functions 
//

#ifdef AHSOKA_HEADER
/*
 * Ahsoka Serial Header Format
 *
 * |-------------------------|-----------------|-------------------------|
 * |  Header Size (2-bytes)  |  Ahsoka Header  |  Message Payload Bytes  |
 * |-------------------------|-----------------|-------------------------|
 *
 * While the full packet could be stored in a payload, because we know the
 * first two bytes are exclusively going to be used to determine the size
 * of the header,
*/

static bool sClassic_header_format = true;

/*
* @brief   handle_coded_classic_prompt
* @details A static (private) function to decode the wrapper and
*          dispatch the appropriate message handlers.
* @return Zero on success or an error code.
*/
/// @private
static int handle_coded_classic_prompt() 
{
    // sCr_uncoded_message_structure will hold the decoded message.
    cr_ReachMessage *msgPtr = &sCr_uncoded_message_structure;
    memset(msgPtr, 0, sizeof(cr_ReachMessage));
    if(!decode_reach_message(msgPtr, 
                             (pb_byte_t *)sCr_encoded_message_buffer, 
                             sCr_encoded_message_size))
    {
        cr_report_error(cr_ErrorCodes_DECODING_FAILED, "%s:Reach header Decode failed", __FUNCTION__);
        return cr_ErrorCodes_DECODING_FAILED;
    }

    cr_ReachMessageHeader *hdr = &msgPtr->header;
    uint8_t *coded_data = (uint8_t *)msgPtr->payload.bytes;
    sCr_transaction_id = hdr->transaction_id;
    sCr_endpoint_id    = hdr->endpoint_id;
    sCr_client_id      = hdr->client_id;

    I3_LOG(LOG_MASK_REACH, "Message type: \t%s", msg_type_string(msgPtr->header.message_type));
    LOG_DUMP_WIRE("handle_coded_prompt (message): ",
                       msgPtr->payload.bytes, msgPtr->payload.size);
    I3_LOG(LOG_MASK_REACH, "Prompt Payload size: %d. Transaction ID %d, client_id %d, endpoint %d.", 
           msgPtr->payload.size, sCr_transaction_id, sCr_client_id, sCr_endpoint_id);

    // further decode and process the message
    // The result will be fully encoded at sCr_encoded_response_buffer[]
    // in case of a non-zero return there will be an encoded error report.
    return handle_message(hdr, coded_data, msgPtr->payload.size);
}

/*
* @brief   handle_coded_prompt
* @details A static (private) function to decode the wrapper and
*          dispatch the appropriate message handlers.
* @return Zero on success or an error code.
*/
/// @private
static int handle_coded_prompt() // ahsoka version
{
    // Is this a classic Reach header or an Ahsoka header?
    // Classic reach witll start with 0x0A
    if ((sCr_encoded_message_buffer[0] == 0x0A) && (sCr_encoded_message_buffer[1] != 0x0))
    {
        sClassic_header_format = true;
        return handle_coded_classic_prompt(); 
    }
    sClassic_header_format = false;
    i3_log(LOG_MASK_REACH, TEXT_MAGENTA "Decode Ahsoka header:");

    // We will first decode the header, skipping over the two byte 
    // packet size at the front. This can go on the stack as it's not large and 
    // we won't need it after this function.  The key parts are saved to statics.
    cr_AhsokaMessageHeader header;

    // Store the size of message is in the first two bytes.
    // endian?
    uint16_t coded_header_size = *(uint16_t*)&sCr_encoded_message_buffer;

    // feed the header into the stream buffer, skipping the leading size
    pb_istream_t is_stream = 
        pb_istream_from_buffer(((pb_byte_t *)&sCr_encoded_message_buffer) + 2, 
                               coded_header_size);

    // Decode the header from the incoming buffer.
    memset(&header, 0, sizeof(cr_AhsokaMessageHeader));
    bool status = pb_decode(&is_stream, cr_AhsokaMessageHeader_fields, (void *)&header);
    if (!status)
    {
        LOG_ERROR("Ahsoka Header Decoding failed: %s\n", PB_GET_ERROR(&is_stream));
        cr_report_error(cr_ErrorCodes_DECODING_FAILED, 
                        "%s: Ahsoka header Decode failed", __FUNCTION__);
        return cr_ErrorCodes_DECODING_FAILED;
    }

    // save the things we need out of the header.
    sCr_transaction_id = header.client_message_id;
    sCr_endpoint_id    = header.endpoint_id;
    memcpy(&sCr_client_id, header.client_id.bytes, header.client_id.size);
    pvtCr_num_remaining_objects = header.message_size;

    // The coded data begins after the header
    uint8_t *coded_data = (uint8_t *)((unsigned int)(&sCr_encoded_message_buffer)
                                      + 2 + coded_header_size);
    uint16_t message_size = sCr_encoded_message_size - 2 - coded_header_size;
    uint8_t *coded_payload = (uint8_t *)&sCr_uncoded_message_structure;
    memcpy(coded_payload, coded_data, message_size);

    I3_LOG(LOG_MASK_REACH, "Message type: \t%s",
           msg_type_string(header.transport_id));

    // I don't see how to get the size without decoding.
    LOG_DUMP_WIRE("handle_coded_prompt ahsoka payload: ",
                       coded_payload, message_size);
    I3_LOG(LOG_MASK_REACH, "Prompt Payload: size: %d, Transaction ID %d, client_id %d, endpoint_id %d.", 
           message_size, sCr_transaction_id, sCr_client_id, sCr_endpoint_id);

    // further decode and process the message
    // The result will be fully encoded at sCr_encoded_response_buffer[]
    // in case of a non-zero return there will be an encoded error report.
    cr_ReachMessageHeader rhdr = {0};
    rhdr.message_type = header.transport_id;
    return handle_message(&rhdr, coded_payload, message_size);
}

#else // classic reach header

/*
* @brief   handle_coded_prompt
* @details A static (private) function to decode the wrapper and
*          dispatch the appropriate message handlers.
* @return Zero on success or an error code.
*/
/// @private
static int handle_coded_prompt() 
{
    // sCr_uncoded_message_structure will hold the decoded message.
    cr_ReachMessage *msgPtr = &sCr_uncoded_message_structure;
    memset(msgPtr, 0, sizeof(cr_ReachMessage));
    if(!decode_reach_message(msgPtr, 
                             (pb_byte_t *)sCr_encoded_message_buffer, 
                             sCr_encoded_message_size))
    {
        cr_report_error(cr_ErrorCodes_DECODING_FAILED, "%s:Reach header Decode failed", __FUNCTION__);
        return cr_ErrorCodes_DECODING_FAILED;
    }

    cr_ReachMessageHeader *hdr = &msgPtr->header;
    uint8_t *coded_data = (uint8_t *)msgPtr->payload.bytes;
    sCr_transaction_id = hdr->transaction_id;
    sCr_endpoint_id    = hdr->endpoint_id;
    sCr_client_id      = hdr->client_id;

    I3_LOG(LOG_MASK_REACH, "Message type: \t%s",
           msg_type_string(msgPtr->header.message_type));
    LOG_DUMP_WIRE("handle_coded_prompt (message): ",
                       msgPtr->payload.bytes, msgPtr->payload.size);
    I3_LOG(LOG_MASK_REACH, "Prompt Payload size: %d. Transaction ID %d, client_id %d, endpoint %d.", 
           msgPtr->payload.size, sCr_transaction_id, sCr_client_id, sCr_endpoint_id);

    // further decode and process the message
    // The result will be fully encoded at sCr_encoded_response_buffer[]
    // in case of a non-zero return there will be an encoded error report.
    return handle_message(hdr, coded_data, msgPtr->payload.size);
}

#endif  // def AHSOKA_HEADER


static size_t sCr_MaxBufferSize = 0;
static int sCr_checkSize(size_t test, size_t limit, char *name)
{
    if (test > sCr_MaxBufferSize)
        sCr_MaxBufferSize = test;

    if (test > limit)
    {
        i3_log(LOG_MASK_ALWAYS, TEXT_RED "  %s = %d, TOO BIG." TEXT_RESET, name, test);
        return 1;
    }
  #ifdef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "  %s = %d, OK." TEXT_RESET, name, test);
  #endif  // def VERBOSE_SIZES
    return 0;
}

/// cr_test_sizes(): The test harness checks that buffer sizes
/// are not too big. These buffer sizes are adjusted via 
/// reach.options.prototype and update_proto.bat 
void cr_test_sizes()
{
    int rval = 0;

    /// MAX_BLE_SZ is used to check the encoded structure
    /// sizes. 
    #define MAX_BLE_SZ  CR_CODED_BUFFER_SIZE

  #ifdef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, "Verbose buffer size report:" TEXT_RESET);
  #else
    i3_log(LOG_MASK_ALWAYS, "Silent buffer size check:  " TEXT_RESET);
  #endif
    /// reach.pb.h gives us some "Maximum encoded size of messages".
    /// Check these against MAX_BLE_SZ, the limit of an encoded 
    /// buffer. 
    rval += sCr_checkSize(cr_CLIData_size, MAX_BLE_SZ, "cr_CLIData_size");
    rval += sCr_checkSize(cr_CommandInfo_size, MAX_BLE_SZ, "cr_CommandInfo_size");
    rval += sCr_checkSize(cr_DeviceInfoResponse_size, MAX_BLE_SZ, "cr_DeviceInfoResponse_size");
    rval += sCr_checkSize(cr_DiscoverCommandsResponse_size, MAX_BLE_SZ, "cr_DiscoverCommandsResponse_size");
    rval += sCr_checkSize(cr_DiscoverFilesResponse_size, MAX_BLE_SZ, "cr_DiscoverFilesResponse_size");
    rval += sCr_checkSize(cr_DiscoverStreamsResponse_size, MAX_BLE_SZ, "cr_DiscoverStreamsResponse_size");
    rval += sCr_checkSize(cr_ParamExKey_size, MAX_BLE_SZ, "cr_ParamExKey_size");
    rval += sCr_checkSize(cr_ErrorReport_size, MAX_BLE_SZ, "cr_ErrorReport_size");
    rval += sCr_checkSize(cr_FileInfo_size, MAX_BLE_SZ, "cr_FileInfo_size");
    rval += sCr_checkSize(cr_FileTransferDataNotification_size, MAX_BLE_SZ, "cr_FileTransferDataNotification_size");
    rval += sCr_checkSize(cr_FileTransferData_size, MAX_BLE_SZ, "cr_FileTransferData_size");
    rval += sCr_checkSize(cr_FileTransferResponse_size, MAX_BLE_SZ, "cr_FileTransferResponse_size");
    rval += sCr_checkSize(cr_FileTransferRequest_size, MAX_BLE_SZ, "cr_FileTransferRequest_size");
    rval += sCr_checkSize(cr_ParamExInfoResponse_size, MAX_BLE_SZ, "cr_ParamExInfoResponse_size");
    rval += sCr_checkSize(cr_ParameterInfoRequest_size, MAX_BLE_SZ, "cr_ParameterInfoRequest_size");
    rval += sCr_checkSize(cr_ParameterInfoResponse_size, MAX_BLE_SZ, "cr_ParameterInfoResponse_size");
    rval += sCr_checkSize(cr_ParameterInfo_size, MAX_BLE_SZ, "cr_ParameterInfo_size");
    rval += sCr_checkSize(cr_ParameterNotification_size, MAX_BLE_SZ, "cr_ParameterNotification_size");
    rval += sCr_checkSize(cr_ParameterNotifyConfigResponse_size, MAX_BLE_SZ, "cr_ParameterNotifyConfigResponse_size");
    rval += sCr_checkSize(cr_ParameterNotifyConfig_size, MAX_BLE_SZ, "cr_ParameterNotifyConfig_size");
    rval += sCr_checkSize(cr_ParameterReadResponse_size, MAX_BLE_SZ, "cr_ParameterReadResponse_size");
    rval += sCr_checkSize(cr_ParameterRead_size, MAX_BLE_SZ, "cr_ParameterRead_size");
    rval += sCr_checkSize(cr_ParameterValue_size, MAX_BLE_SZ, "cr_ParameterValue_size");
    rval += sCr_checkSize(cr_ParameterWrite_size, MAX_BLE_SZ, "cr_ParameterWrite_size");
    rval += sCr_checkSize(cr_ParameterWriteResponse_size, MAX_BLE_SZ, "cr_ParameterWriteResponse_size");
    rval += sCr_checkSize(cr_PingRequest_size, MAX_BLE_SZ, "cr_PingRequest_size");
    rval += sCr_checkSize(cr_PingResponse_size, MAX_BLE_SZ, "cr_PingResponse_size");
    rval += sCr_checkSize(cr_ReachMessageHeader_size, MAX_BLE_SZ, "cr_ReachMessageHeader_size");
    rval += sCr_checkSize(cr_ReachMessage_size, MAX_BLE_SZ, "cr_ReachMessage_size");
    rval += sCr_checkSize(cr_SendCommandResponse_size, MAX_BLE_SZ, "cr_SendCommandResponse_size");
    rval += sCr_checkSize(cr_StreamData_size, MAX_BLE_SZ, "cr_StreamData_size");
    rval += sCr_checkSize(cr_StreamInfo_size, MAX_BLE_SZ, "cr_StreamInfo_size");

    rval += sCr_checkSize(cr_WiFiConnectionRequest_size,    MAX_BLE_SZ, "cr_WiFiConnectionRequest_size");
    rval += sCr_checkSize(cr_WiFiConnectionResponse_size,   MAX_BLE_SZ, "cr_WiFiConnectionResponse_size");
    rval += sCr_checkSize(cr_DiscoverWiFiResponse_size,          MAX_BLE_SZ, "cr_DiscoverWiFiResponse_size");
    rval += sCr_checkSize(cr_DiscoverWiFiRequest_size,          MAX_BLE_SZ, "cr_DiscoverWiFiRequest_size");



    /// <summary>
    /// MAX_RAW_SZ is used to check the unencoded structure sizes. 
    /// </summary>
    #define MAX_RAW_SZ  CR_DECODED_BUFFER_SIZE

    rval += sCr_checkSize(sizeof(cr_StreamInfo), MAX_RAW_SZ, "sizeof(cr_StreamInfo)");
    rval += sCr_checkSize(sizeof(cr_ErrorReport), MAX_RAW_SZ, "sizeof(cr_ErrorReport)");
    rval += sCr_checkSize(sizeof(cr_ParameterReadResponse), MAX_RAW_SZ, "sizeof(cr_ParameterReadResponse)");
    rval += sCr_checkSize(sizeof(cr_ParameterWrite), MAX_RAW_SZ, "sizeof(cr_ParameterWrite)");
    rval += sCr_checkSize(sizeof(cr_ParameterNotifyConfig), MAX_RAW_SZ, "sizeof(cr_ParameterNotifyConfig)");
    rval += sCr_checkSize(sizeof(cr_ParameterNotification), MAX_RAW_SZ, "sizeof(cr_ParameterNotification)");
    rval += sCr_checkSize(sizeof(cr_ParameterNotifyConfigResponse), MAX_RAW_SZ, "sizeof(cr_ParameterNotifyConfigResponse)");
    rval += sCr_checkSize(sizeof(cr_ParameterValue), MAX_RAW_SZ, "sizeof(cr_ParameterValue)");
    rval += sCr_checkSize(sizeof(cr_CLIData), MAX_RAW_SZ, "sizeof(cr_CLI_Data)");
    rval += sCr_checkSize(sizeof(cr_ParameterRead), MAX_RAW_SZ, "sizeof(cr_ParameterRead)");
    rval += sCr_checkSize(sizeof(cr_FileTransferData), MAX_RAW_SZ, "sizeof(cr_FileTransferData)");
    rval += sCr_checkSize(sizeof(cr_ParameterInfo), MAX_RAW_SZ, "sizeof(cr_ParameterInfo)");


    rval += sCr_checkSize(sizeof(cr_WiFiConnectionRequest),         MAX_RAW_SZ, "cr_WiFiConnectionRequest");
    rval += sCr_checkSize(sizeof(cr_WiFiConnectionResponse_size),   MAX_RAW_SZ, "cr_WiFiConnectionResponse");
    rval += sCr_checkSize(sizeof(cr_DiscoverWiFiResponse_size),          MAX_RAW_SZ, "cr_DiscoverWiFiResponse");
    rval += sCr_checkSize(sizeof(cr_DiscoverWiFiRequest_size),          MAX_RAW_SZ, "cr_DiscoverWiFiRequest");


    // If these don't match, check the structures associated with them
    affirm(sizeof(reach_sizes_t) == REACH_SIZE_STRUCT_SIZE);
    affirm(REACH_MAX_RESPONSE_SIZE == CR_CODED_BUFFER_SIZE);

  #ifdef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, "\n");
  #endif  // def VERBOSE_SIZES

    // affirm(rval == 0);     // halt if failure

   #ifndef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "     Size tests all pass.");
  #endif  // def VERBOSE_SIZES

}

// <summary> Decodes the payload and calls the appropriate 
// handler function. 
// </summary>
// <param name="hdr"> Includes type of message</param>
// <param name="data">The actual message</param>
// <param name="size">in bytes</param>
// <returns>0 on success or an error</returns>

/*********************************************************************************
  * The caller separated the wrapper into header and coded_data.
  * The header and coded_data point into sCr_uncoded_message_structure.
  * In this function: 
  *   The prompt is decoded into sCr_decoded_prompt_buffer and handled.
  *   The result is coded into sCr_encoded_response_buffer with length.
  *   First the payload is determined.  Then it is encoded.
  *   Finally the encoded payload is added to a message which is encoded.
  */
static int
handle_message(const cr_ReachMessageHeader *hdr, const uint8_t *coded_data, size_t size)
{
    affirm(hdr);
    affirm(coded_data);

    cr_ReachMessageTypes message_type = (cr_ReachMessageTypes)hdr->message_type;
    cr_ReachMessageTypes encode_message_type = message_type; // default 

    //LOG_REACH("call decode_reach_payload(from 0x%x, to 0x%x, sz %d)", 
    //          coded_data, sCr_decoded_prompt_buffer, size);

    if (!decode_reach_payload(message_type,
                              sCr_decoded_prompt_buffer,
                              coded_data, size))
    {
        cr_report_error(cr_ErrorCodes_DECODING_FAILED, "%s: decode payload %d failed.", 
                        __FUNCTION__, message_type);
        return cr_ErrorCodes_DECODING_FAILED;
    }

    pvtCr_num_remaining_objects = 0;  // default

    int rval = 0;
    switch (message_type)
    {
    case cr_ReachMessageTypes_PING:
        rval = handle_ping((cr_PingRequest *)sCr_decoded_prompt_buffer,
                    (cr_PingResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_GET_DEVICE_INFO:
        rval = handle_get_device_info((cr_DeviceInfoRequest *)sCr_decoded_prompt_buffer,
                               (cr_DeviceInfoResponse *)sCr_uncoded_response_buffer);
        break;
  #ifdef INCLUDE_PARAMETER_SERVICE
    case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
        rval = pvtCrParam_discover_parameters((cr_ParameterInfoRequest *)sCr_decoded_prompt_buffer,
                                   (cr_ParameterInfoResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
        rval = pvtCrParam_discover_parameters_ex((cr_ParameterInfoRequest *)sCr_decoded_prompt_buffer,
                                   (cr_ParamExInfoResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_READ_PARAMETERS:
        rval = pvtCrParam_read_param((cr_ParameterRead *)sCr_decoded_prompt_buffer,
                          (cr_ParameterReadResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_WRITE_PARAMETERS:
        rval = pvtCrParam_write_param((cr_ParameterWrite *)sCr_decoded_prompt_buffer,
                           (cr_ParameterWriteResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS:
        rval = pvtCrParam_discover_notifications((cr_DiscoverParameterNotifications *)sCr_decoded_prompt_buffer,
                           (cr_DiscoverParameterNotificationsResponse *)sCr_uncoded_response_buffer);
        break;

    #if NUM_SUPPORTED_PARAM_NOTIFY != 0
    case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
        rval = pvtCrParam_config_param_notify((cr_ParameterNotifyConfig *)sCr_decoded_prompt_buffer,
                           (cr_ParameterNotifyConfigResponse *)sCr_uncoded_response_buffer);
        break;
    #endif
  #endif // def INCLUDE_PARAMETER_SERVICE

  #ifdef INCLUDE_FILE_SERVICE
    case cr_ReachMessageTypes_DISCOVER_FILES:
        rval = pvtCrFile_discover((cr_DiscoverFiles *)sCr_decoded_prompt_buffer,
                              (cr_DiscoverFilesResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_TRANSFER_INIT:
        rval = pvtCrFile_transfer_init((cr_FileTransferRequest *)sCr_decoded_prompt_buffer,
                             (cr_FileTransferResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_TRANSFER_DATA:
        // file write:
        rval = pvtCrFile_transfer_data((cr_FileTransferData *)sCr_decoded_prompt_buffer,
                             (cr_FileTransferDataNotification *)sCr_uncoded_response_buffer);
        // returns cr_ErrorCodes_NO_RESPONSE and hence no error when no response is desired.
        // when zero is returned we need to ack with a notification.
        if (rval == cr_ErrorCodes_NO_ERROR)
            encode_message_type = cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION;
        break;

    case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
    {
        cr_FileTransferDataNotification *request = 
            (cr_FileTransferDataNotification *)sCr_decoded_prompt_buffer;
        rval = pvtCrFile_transfer_data_notification(request,
                                          (cr_FileTransferData *)sCr_uncoded_response_buffer);
        // for continuing transactions we need more data.
        if (!request->is_complete)
            encode_message_type = cr_ReachMessageTypes_TRANSFER_DATA;
        break;
    }
  #endif // def INCLUDE_FILE_SERVICE

  #ifdef INCLUDE_STREAM_SERVICE
    case cr_ReachMessageTypes_DISCOVER_STREAMS:
        // rval = handle_discover_streams( (cr_StreamsRequest *)sCr_decoded_prompt_buffer,
        //                        (cr_StreamsResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_OPEN_STREAM:
        // rval = handle_open_stream( (cr_StreamsRequest *)sCr_decoded_prompt_buffer,
        //                            (cr_StreamsResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_CLOSE_STREAM:
        // rval = handle_close_stream( (cr_StreamsRequest *)sCr_decoded_prompt_buffer,
        //                            (cr_StreamsResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_STREAM_DATA_NOTIFICATION:
        // rval = handle_streams_notification( (cr_StreamsRequest *)sCr_decoded_prompt_buffer,
        //                            (cr_StreamsResponse *)sCr_uncoded_response_buffer);
        break;
  #endif // def INCLUDE_STREAM_SERVICE

  #ifdef INCLUDE_COMMAND_SERVICE
    case cr_ReachMessageTypes_DISCOVER_COMMANDS:
        rval = handle_discover_commands((cr_DiscoverCommands *)sCr_decoded_prompt_buffer,
                                 (cr_DiscoverCommandsResponse *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_SEND_COMMAND:
        rval = handle_send_command((cr_SendCommand *)sCr_decoded_prompt_buffer,
                            (cr_SendCommandResponse *)sCr_uncoded_response_buffer);
        break;
  #endif  // def INCLUDE_COMMAND_SERVICE

  #ifdef INCLUDE_CLI_SERVICE
    case cr_ReachMessageTypes_CLI_NOTIFICATION:
        rval = handle_cli_notification((cr_CLIData *)sCr_decoded_prompt_buffer,
                                 (cr_CLIData *)sCr_uncoded_response_buffer);
        break;
  #endif // def INCLUDE_CLI_SERVICE

  #ifdef INCLUDE_TIME_SERVICE
    case cr_ReachMessageTypes_SET_TIME:
        rval = handle_time_set((cr_TimeSetRequest *)sCr_decoded_prompt_buffer, 
                                (cr_TimeSetResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_GET_TIME:
        rval = handle_time_get((cr_TimeGetRequest *)sCr_decoded_prompt_buffer, 
                                (cr_TimeGetResponse *)sCr_uncoded_response_buffer);
        break;
  #endif  // def INCLUDE_TIME_SERVICE

  #ifdef INCLUDE_WIFI_SERVICE
    case cr_ReachMessageTypes_DISCOVER_WIFI:
        rval = handle_discover_wifi((cr_DiscoverWiFiRequest *)sCr_decoded_prompt_buffer, 
                                (cr_DiscoverWiFiResponse *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_WIFI_CONNECT:
        rval = handle_wifi_connect((cr_WiFiConnectionRequest *)sCr_decoded_prompt_buffer, 
                                   (cr_WiFiConnectionResponse *)sCr_uncoded_response_buffer);
        break;
  #endif  // def INCLUDE_WIFI_SERVICE

    default:
        cr_report_error(cr_ErrorCodes_NOT_IMPLEMENTED, "Unhandled message type %d.", message_type);
        LOG_ERROR("Unhandled message type %d.", message_type);
        rval = cr_ErrorCodes_NOT_IMPLEMENTED; 
        break;
    }
    if (rval != 0)
        return rval;

    cr_ReachMessageHeader msg_header;
    msg_header.message_type      = encode_message_type;
    msg_header.remaining_objects = pvtCr_num_remaining_objects;
    msg_header.transaction_id    = sCr_transaction_id;
    msg_header.endpoint_id       = sCr_endpoint_id;
    msg_header.client_id         = sCr_client_id;

    rval = sCr_encode_message(encode_message_type,
                              sCr_uncoded_response_buffer,
                              &msg_header);
    if (rval != 0)
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "Reach encode failed (%d).", rval);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    return 0;
}

// handle_ping attempts to reuse the encoded prompt
static int handle_ping(const cr_PingRequest *request, cr_PingResponse *response) 
{
    int8_t rssi;

    if (!crcb_challenge_key_is_valid()) {
        return cr_ErrorCodes_NO_DATA;
    }

    if (request->echo_data.size > 0) {
        response->echo_data.size = request->echo_data.size;
        i3_log(LOG_MASK_ALWAYS, "ping data size %d", request->echo_data.size);
        memcpy(response->echo_data.bytes, request->echo_data.bytes,
               request->echo_data.size);
    }
    crcb_ping_get_signal_strength(&rssi);
    response->signal_strength = rssi;
    return 0;
}


// <summary>
// Fill in the structure that communicates buffer sizes to the 
// Reach client.  Comments on the BufferSizes message in the
// reach.proto file give more description of the meaning of 
// each entry. 
// </summary>
// <param name="dir">Pointer to a device info response 
// structure into which the sizes data will be copied</param> 
static void sCr_populate_device_info_sizes(cr_DeviceInfoResponse *dir)
{
    reach_sizes_t sizes_struct; 

    sizes_struct.max_message_size             = 244;  // biggest BLE message
    sizes_struct.big_data_buffer_size         = REACH_BIG_DATA_BUFFER_LEN;
    sizes_struct.parameter_buffer_count       = REACH_COUNT_PARAM_IDS;
    sizes_struct.num_params_in_response       = REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE;
    sizes_struct.description_len              = REACH_DESCRIPTION_LEN;
    sizes_struct.max_param_bytes              = REACH_NUM_PARAM_BYTES;
    sizes_struct.param_info_description_len   = REACH_PARAM_INFO_DESCRIPTION_LEN;
    sizes_struct.medium_string_len            = REACH_MEDIUM_STRING_LEN;
    sizes_struct.short_string_len             = REACH_SHORT_STRING_LEN;
    sizes_struct.param_info_enum_count        = REACH_PARAM_INFO_ENUM_COUNT;
    sizes_struct.num_descriptors_in_response  = REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE;
    sizes_struct.num_param_notifications      = NUM_SUPPORTED_PARAM_NOTIFY;
    sizes_struct.num_commands_in_response     = REACH_NUM_COMMANDS_IN_RESPONSE;
    sizes_struct.num_param_desc_in_response   = REACH_COUNT_PARAM_DESC_IN_RESPONSE;
    dir->sizes_struct.size = sizeof(reach_sizes_t);
    memcpy(dir->sizes_struct.bytes, &sizes_struct,  sizeof(reach_sizes_t));
}

uint8_t sClientProtocolVersion[3];
/**
 * @brief   handle_get_device_info 
 * @details In response to a request for device info, get the 
 *         raw data from the app via a crcb function, then
 *         update the provided pointer with the latest accurate
 *         data.
 *@param   request : A pointer to the request which may include  
 *                 a challenge key.
 *@param   response : A pointer to memory where the complete  
 *                  device info response will be composed.
 *@return  0 on success.  cr_ErrorCodes_NO_DATA if the device 
 *          was built to require a challenge key and the
 *          provided key does not match.
*/
static int 
handle_get_device_info(const cr_DeviceInfoRequest *request,  // in
                       cr_DeviceInfoResponse     *response)  // out
{
    int major = 0, minor = 0, patch = 0;
    memset(sClientProtocolVersion, 0, 3);
    memset(response, 0, sizeof(cr_DeviceInfoResponse));
    crcb_device_get_info(request, response);
    crcb_configure_access_control(request, response);
    if (response->services &  cr_ServiceIds_PARAMETER_REPO)
        response->parameter_metadata_hash = crcb_compute_parameter_hash();

    // Store the client's protocol version to be used in compatibility checks.
    int numRead = sscanf(request->client_protocol_version, "%d.%d.%d", &major, &minor, &patch);
    if (numRead != 3)
    {
        // the client protocol version is added at 0.1.3.
        I3_LOG(LOG_MASK_WARN, "  Invalid client protocol version (%d), %d.%d.%d replaced with 0.0.21\n",
               numRead, major, minor, patch);
        sClientProtocolVersion[0] = 0;
        sClientProtocolVersion[1] = 0;
        sClientProtocolVersion[2] = 21;
    }
    sClientProtocolVersion[0] = 0xFF & major;
    sClientProtocolVersion[1] = 0xFF & minor;
    sClientProtocolVersion[2] = 0xFF & patch;

    response->protocol_version = cr_ReachProtoVersion_CURRENT_VERSION;
    snprintf(response->protocol_version_string, CR_STACK_VERSION_LEN, cr_get_proto_version());
    sCr_populate_device_info_sizes(response);
    return 0;
}


/**
* @brief   cr_get_reach_version 
* @details The version is in semantic version format: 
*          MAJOR.MINOR.PATCH
*          with an optional string appended. 
* @return  Returns a pointer to a null terminated string 
*          containing the C stack version.
*/
const char *cr_get_reach_version()
{
    static char sCr_c_stack_version[CR_STACK_VERSION_LEN];
    snprintf(sCr_c_stack_version, CR_STACK_VERSION_LEN, "%u.%u.%u",
            REACH_C_MAJOR_VERSION, REACH_C_MINOR_VERSION, REACH_C_PATCH_VERSION);
     return sCr_c_stack_version;
}


/**
* @brief   cr_get_proto_version 
* @details The version is in semantic version format: 
*          MAJOR.MINOR.PATCH
*          with an optional string appended. 
* @return  Returns a pointer to a null terminated string 
*          containing the C protobuf version.
*/
const char *cr_get_proto_version()
{
    static char sCr_proto_version[CR_STACK_VERSION_LEN];

    snprintf(sCr_proto_version, CR_STACK_VERSION_LEN, "%u.%u.%u",
             cr_ReachProto_MAJOR_Version_MAJOR_VERSION,
             cr_ReachProto_MINOR_Version_MINOR_VERSION,
             cr_ReachProto_PATCH_Version_PATCH_VERSION);
     return sCr_proto_version;
}

 /**
 * @brief   pvtCr_compare_proto_version 
 * @details Used to support backward compatibility.
 * @return  Returns 0 if the client's protocol version is equal 
 *          to the specified version.  A positive value means the
 *          client is greater (newer).  A negative value means
 *          the client version is older than the specified
 *          version.
 */
int pvtCr_compare_proto_version(uint8_t major, uint8_t minor, uint8_t patch)
{
    if ((sClientProtocolVersion[0] == major)
        && (sClientProtocolVersion[1] == minor) 
        && (sClientProtocolVersion[2] == patch))
    {
        return 0;
    }
    if (sClientProtocolVersion[0] > major)
        return 1;
    if (sClientProtocolVersion[0] < major)
        return -1;
    // major must now be equal.
    if (sClientProtocolVersion[1] > minor)
        return 1;
    if (sClientProtocolVersion[1] < minor)
        return -1;
    // minor must now be equal.
    if (sClientProtocolVersion[2] > patch)
        return 1;
    if (sClientProtocolVersion[2] < patch)
        return -1;
    return -2;  // cannot get here.
}

#ifdef INCLUDE_COMMAND_SERVICE
static int 
handle_discover_commands(const cr_DiscoverCommands *request,
                         cr_DiscoverCommandsResponse *response)
{
    int rval;
    int num_commands;

    if (!crcb_challenge_key_is_valid()) {
        pvtCr_num_remaining_objects = 0;
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    if (request != NULL) 
    {
        // request will be null on repeated calls.
        // Here implies we are responding to the initial request.
        num_commands = crcb_get_command_count();
        sCr_requested_command_index = 0;
        I3_LOG(LOG_MASK_DEBUG, "%s: first request, num_commands %d", 
               __FUNCTION__, num_commands);
    }
    else  // here the request was null and we are continuing the previous request;
    {
        num_commands = pvtCr_num_remaining_objects;
        I3_LOG(LOG_MASK_DEBUG, "%s: continued request, num_commands %d from %d", 
               __FUNCTION__, num_commands, sCr_requested_command_index);

    }
    crcb_command_discover_reset(sCr_requested_command_index);  // index, not really cid.

    for (int i=0; i<REACH_NUM_COMMANDS_IN_RESPONSE; i++)
    {
        rval = crcb_command_discover_next(&response->available_commands[i]);
        if (rval != 0)
        {
            if (i == 0)
            {
                LOG_ERROR("Discover commands found nothing.");
                pvtCr_num_remaining_objects = 0;
                return 0;
            }
            break;
        }
        else
            sCr_requested_command_index++;
    }
    if (num_commands <= REACH_NUM_COMMANDS_IN_RESPONSE)
    {
        response->available_commands_count = num_commands;
        // they all fit in one response.
        pvtCr_num_remaining_objects = 0;
        I3_LOG(LOG_MASK_DEBUG, "%s: Completed with %d", __FUNCTION__, num_commands);
        return 0;
        // and we're done.
    } 
    // otherwise there are more so set up for continued commands
    response->available_commands_count = REACH_NUM_COMMANDS_IN_RESPONSE;
    pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_COMMANDS;
    pvtCr_num_remaining_objects = num_commands - REACH_NUM_COMMANDS_IN_RESPONSE;
    I3_LOG(LOG_MASK_DEBUG, "%s: Setup continuing with %d", __FUNCTION__, pvtCr_num_remaining_objects);
    return 0;
}

static int handle_send_command(const cr_SendCommand *request,
                                   cr_SendCommandResponse *response) 
{
    if (!crcb_challenge_key_is_valid()) {
        response->result = cr_ErrorCodes_CHALLENGE_FAILED;
        return 0;
    }

    response->result = crcb_command_execute(request->command_id);
    return 0;
}
#endif // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
    //*************************************************************************
    //  CLI Service
    //*************************************************************************
    static int handle_cli_notification(const cr_CLIData *request, 
                                        cr_CLIData *response)
    {
        (void)response;
        i3_log(LOG_MASK_ALWAYS, "Remote command: '%s'", request->message_data);
        if (!i3_log_get_remote_cli_enable()) 
            i3_log(LOG_MASK_WARN, "  -> Command received, remote CLI response is disabled.");
        crcb_cli_enter(request->message_data);
        return cr_ErrorCodes_NO_RESPONSE;
    }
#endif // def INCLUDE_CLI_SERVICE


#ifdef INCLUDE_TIME_SERVICE
    static int handle_time_set(const cr_TimeSetRequest *request, 
                               cr_TimeSetResponse *response)
    {
        if (!crcb_challenge_key_is_valid()) {
            response->result = cr_ErrorCodes_CHALLENGE_FAILED;
            return 0;
        }

        response->result = crcb_time_set(request);
        response->result_message[0] = 0;

        return 0;
    }

    static int handle_time_get(const cr_TimeGetRequest *request, 
                               cr_TimeGetResponse *response)
    {
        (void)request;
        if (!crcb_challenge_key_is_valid()) {
            response->result = cr_ErrorCodes_CHALLENGE_FAILED;
            return 0;
        }
        response->result = crcb_time_get(response);
        return 0;
    }

#endif  // def INCLUDE_TIME_SERVICE


#ifdef INCLUDE_WIFI_SERVICE
    static int handle_discover_wifi(const cr_DiscoverWiFiRequest *request, 
                                    cr_DiscoverWiFiResponse *response)
    {
        int rval;
        int num_ap;

        if (!crcb_challenge_key_is_valid()) {
            pvtCr_num_remaining_objects = 0;
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA; 
        }

        if (request != NULL) 
        {
            // request will be null on repeated calls.
            // Here implies we are responding to the initial request.
            num_ap = crcb_get_wifi_count();
            sCr_requested_command_index = 0;
            I3_LOG(LOG_MASK_DEBUG, "%s: first request, num_ap %d", 
                   __FUNCTION__, num_ap);
        }
        else  // here the request was null and we are continuing the previous request;
        {
            num_ap = pvtCr_num_remaining_objects;
            I3_LOG(LOG_MASK_DEBUG, "%s: continued request, num_ap %d from %d", 
                   __FUNCTION__, num_ap, sCr_requested_command_index);

        }
        crcb_wifi_discover_reset(sCr_requested_command_index);  // index, not really cid.

        rval = crcb_wifi_discover_next(&response->cd);
        if (rval != 0)
        {
            if (request != NULL)
            {
                LOG_ERROR("Discover wifi found nothing.");
                response->result = cr_ErrorCodes_NO_SERVICE;
            }
            pvtCr_num_remaining_objects = 0;
            return 0;
        }
        if (num_ap == 1)
        {
            response->result = 0;
            // One and done
            pvtCr_num_remaining_objects = 0;
            I3_LOG(LOG_MASK_DEBUG, "%s: Completed with %d", __FUNCTION__, num_ap);
            return 0;
            // and we're done.
        } 
        // otherwise there are more so set up for continued commands
        pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_WIFI;
        pvtCr_num_remaining_objects = num_ap - 1;
        I3_LOG(LOG_MASK_DEBUG, "%s: continuing with %d", __FUNCTION__, pvtCr_num_remaining_objects);
        return 0;
    }

    static int handle_wifi_connect(const cr_WiFiConnectionRequest *request, 
                                      cr_WiFiConnectionResponse *response)
    {
        (void)request;
        if (!crcb_challenge_key_is_valid()) {
            response->result = cr_ErrorCodes_CHALLENGE_FAILED;
            return 0;
        }
        crcb_wifi_connection(request, response);
        return 0;
    }

#endif  // def INCLUDE_WIFI_SERVICE

// int32_t handle_discover_streams(const cr_StreamsRequest *,
//                                 cr_StreamsResponse *) {

//   // cr_FileInfo info = cr_FileInfo_init_zero;
//   // info.file_id = 1;
//   // strcpy(info.file_name, "Sensor Data");
//   // info.access = cr_AccessLevel_write;
//   // info.current_size_bytes = 244;
//   // response.file_infos[0] = info;
//   // response.file_infos_count = 1;

//   return 0;
// }



/*
 *  Encode Section
 */

static bool sTestHeader = false;

static
bool encode_reach_payload(cr_ReachMessageTypes message_type,    // in
                          const void *data,                     // in:  data to be encoded
                          uint8_t *buffer,                      // out: Encode to here.
                          pb_size_t buffer_size,                // in:  max size of encoded data
                          size_t *encode_size)                  // out: encoded data size
{

  /* Create a stream that writes to the buffer. */
  pb_ostream_t os_stream = pb_ostream_from_buffer(buffer, buffer_size);
  bool status = false;

  /* Now we are ready to encode the message. */
  switch (message_type) 
  {

  case cr_ReachMessageTypes_GET_DEVICE_INFO:
      if (sTestHeader)
      {
          // encode GDI, not response.
          status = pb_encode(&os_stream, cr_DeviceInfoRequest_fields, data);
          if (status) {
            *encode_size = os_stream.bytes_written;
            LOG_REACH("Encoded get device info request: \n%s\n");
          }
          break;
      }
      status = pb_encode(&os_stream, cr_DeviceInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_device_info_response((cr_DeviceInfoResponse *)data);
      }
      break;

  case cr_ReachMessageTypes_ERROR_REPORT:
      status = pb_encode(&os_stream, cr_ErrorReport_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        // cr_ErrorReport *er = (cr_ErrorReport *)data;
        // I3_LOG(LOG_MASK_REACH, "Error Report: %s", er->result_string);
      }
      break;
  case cr_ReachMessageTypes_PING:
      if (sTestHeader)
      {
          status = pb_encode(&os_stream, cr_PingRequest_fields, data);
          if (status) {
            *encode_size = os_stream.bytes_written;
            LOG_REACH("Encoded Ping request\n");
          }
          break;
      }
      status = pb_encode(&os_stream, cr_PingResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_ping_response((cr_PingResponse *)data);
      }
      break;

#ifdef INCLUDE_PARAMETER_SERVICE
  case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_param_info_response((cr_ParameterInfoResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
      status = pb_encode(&os_stream, cr_ParamExInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_param_info_ex_response((cr_ParamExInfoResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_READ_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterReadResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_read_param_response((cr_ParameterReadResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_WRITE_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterWriteResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_write_param_response((cr_ParameterWriteResponse *)data);
      }
      else
      {
          LOG_ERROR("pb_encode() failed for write params.");
      }
      break;

  #if NUM_SUPPORTED_PARAM_NOTIFY != 0
  case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
      status = pb_encode(&os_stream, cr_ParameterNotifyConfigResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_config_notify_param((cr_ParameterNotifyConfigResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS:
      status = pb_encode(&os_stream, cr_DiscoverParameterNotificationsResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_discover_notifications_response((cr_DiscoverParameterNotificationsResponse *)data);
      }
      break;
  #endif
#endif // def INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_FILE_SERVICE
  case cr_ReachMessageTypes_DISCOVER_FILES:
      status = pb_encode(&os_stream, cr_DiscoverFilesResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_discover_files_response((cr_DiscoverFilesResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_INIT:

      status = pb_encode(&os_stream, cr_FileTransferResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_file_transfer_response((cr_FileTransferResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA:
      status = pb_encode(&os_stream, cr_FileTransferData_fields, data);
      if (status) {
          *encode_size = os_stream.bytes_written;
          cr_FileTransferData *td = (cr_FileTransferData *)data;
          message_util_log_transfer_data(td);
          LOG_DUMP_MASK(LOG_MASK_REACH, "Data Sent", 
                        td->message_data.bytes, td->message_data.size);
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
      status =
          pb_encode(&os_stream, cr_FileTransferDataNotification_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_transfer_data_notification(false,
                      (cr_FileTransferDataNotification *)data);
      }
      break;
#endif // def INCLUDE_FILE_SERVICE

#ifdef INCLUDE_STREAM_SERVICE
  case cr_ReachMessageTypes_DISCOVER_STREAMS:
    // if (request) {
    //   status = pb_encode(&os_stream, cr_StreamsRequest_fields, data);
    //   if (status) {
    //     *encode_size = os_stream.bytes_written;
    //     LOG_REACH(
    //         "Discover streams request: \n%s\n",
    //         message_util_log_discover_streams((cr_StreamsRequest *)data));
    //   }
    // } else {
    //   status = pb_encode(&os_stream, cr_StreamsResponse_fields, data);
    //   if (status) {
    //     *encode_size = os_stream.bytes_written;
    //     LOG_REACH("Discover streams response: \n%s\n",
    //               message_util_discover_streams_response_json(
    //                   (cr_StreamsResponse *)data));
    //   }
    // }
    break;
#endif // def INCLUDE_STREAM_SERVICE 

#ifdef INCLUDE_COMMAND_SERVICE
  case cr_ReachMessageTypes_DISCOVER_COMMANDS:
      status = pb_encode(&os_stream, cr_DiscoverCommandsResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_discover_commands_response((cr_DiscoverCommandsResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_SEND_COMMAND:
      status = pb_encode(&os_stream, cr_SendCommandResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_command_response((cr_SendCommandResponse *)data);
      }
      break;
#endif  // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
      status = pb_encode(&os_stream, cr_CLIData_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_cli_notification(true, (cr_CLIData *)data);
      }
      break;
#endif  // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_TIME_SERVICE
  case cr_ReachMessageTypes_SET_TIME:
      status = pb_encode(&os_stream, cr_TimeSetResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_time_set_response((cr_TimeSetResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_GET_TIME:
      status = pb_encode(&os_stream, cr_TimeGetResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        message_util_log_time_get_response((cr_TimeGetResponse *)data);
      }
      break;
#endif  // def INCLUDE_TIME_SERVICE


#ifdef INCLUDE_WIFI_SERVICE
  case cr_ReachMessageTypes_DISCOVER_WIFI:
      status = pb_encode(&os_stream, cr_DiscoverWiFiResponse_fields, data);
      if (status) {
        message_util_log_discover_wifi_response((cr_DiscoverWiFiResponse *)data);
      }
      break;
  case cr_ReachMessageTypes_WIFI_CONNECT:
      status = pb_encode(&os_stream, cr_WiFiConnectionResponse_fields, data);
      if (status) {
        message_util_log_WiFi_connect_request((cr_WiFiConnectionRequest *)data);
      }
      break;
#endif  // def INCLUDE_WIFI_SERVICE

  default:
      LOG_ERROR("No encoder for %d", message_type);
      status = cr_ErrorCodes_NO_DATA;
      break;
  }

  // but there must also be room for the header.
  affirm (*encode_size < buffer_size);

  if (status)
      return true;

  LOG_ERROR("Encoding failed: %s\n", PB_GET_ERROR(&os_stream));
  return false;
}

static
bool encode_reach_message(const cr_ReachMessage *message,   // in:  message to be encoded
                          uint8_t *buffer,                  // out: Buffer to encode into
                          size_t buffer_size,               // in:  max size of encoded message
                          size_t *encode_size)              // out: actual size of encoded message.
{
  pb_ostream_t os_stream = pb_ostream_from_buffer(buffer, buffer_size);

  bool status = pb_encode(&os_stream, cr_ReachMessage_fields, (void *)message);
  if (status) {
    *encode_size = os_stream.bytes_written;
  } else {
    LOG_ERROR("Encoding failed: %s\n", PB_GET_ERROR(&os_stream));
  }

  LOG_DUMP_WIRE("The encoded message", buffer, *encode_size);

  return status;
}

#ifdef AHSOKA_HEADER


// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static int sCr_encode_classic_message(cr_ReachMessageTypes message_type,   // in
                             const void *payload,                  // in:  to be encoded
                             cr_ReachMessageHeader *hdr)           // in
{
    // I3_LOG(LOG_MASK_REACH, "%s(): hdr: type %d, remain %d, trans_id %d.", __FUNCTION__,
    //        hdr->message_type, hdr->remaining_objects, hdr->transaction_id);

    if (!encode_reach_payload(message_type, payload,
                              sCr_encoded_payload_buffer,
                              sizeof(sCr_encoded_payload_buffer),
                              &sCr_encoded_payload_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode payload %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    i3_log(LOG_MASK_REACH, TEXT_MAGENTA "Encode classic header:");

    // build the message envelope
    sCr_uncoded_message_structure.header     = *hdr;
    sCr_uncoded_message_structure.has_header = true;
    memcpy(sCr_uncoded_message_structure.payload.bytes, 
           sCr_encoded_payload_buffer, 
           sCr_encoded_payload_size);
    sCr_uncoded_message_structure.payload.size = sCr_encoded_payload_size;  

    I3_LOG(LOG_MASK_REACH, "%s(): type %d, remain %d, trans_id %d, client %d, ep %d.", 
           __FUNCTION__,
           sCr_uncoded_message_structure.header.message_type, 
           sCr_uncoded_message_structure.header.remaining_objects, 
           sCr_uncoded_message_structure.header.transaction_id,
           sCr_uncoded_message_structure.header.client_id,
           sCr_uncoded_message_structure.header.endpoint_id);

    // encode the wrapped message
    if (!encode_reach_message(&sCr_uncoded_message_structure,
                              sCr_encoded_response_buffer,
                              sizeof(sCr_encoded_response_buffer),
                              &sCr_encoded_response_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode message %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    return 0;
}

static
bool encode_ahsoka_header(const cr_AhsokaMessageHeader *header,  // in:  message to be encoded
                          uint8_t *buffer,                      // out: Buffer to encode into
                          size_t buffer_size,                   // in:  max size of encoded message
                          size_t *encode_size)                  // out: actual size of encoded message.
{
  pb_ostream_t os_stream = pb_ostream_from_buffer(buffer, buffer_size);

  bool status = pb_encode(&os_stream, cr_AhsokaMessageHeader_fields, (void *)header);
  if (status) {
     *encode_size = os_stream.bytes_written;
  } else {
    LOG_ERROR("Encoding ahsoka header failed: %s\n", PB_GET_ERROR(&os_stream));
  }

  LOG_DUMP_WIRE("The encoded ahsoka header", buffer, *encode_size);

  return status;
}

// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static int sCr_encode_message(cr_ReachMessageTypes message_type,   // in
                             const void *payload,                  // in:  to be encoded
                             cr_ReachMessageHeader *hdr)           // in
{   // Ahsoka version
    if (sClassic_header_format)
    {
        return sCr_encode_classic_message(message_type, payload, hdr);
    }

    LOG_REACH("Encode Ahsoka message:");
    cr_AhsokaMessageHeader ahdr;
    ahdr.transport_id          = message_type;
    ahdr.client_id.size        = sizeof(hdr->client_id);
    memcpy(ahdr.client_id.bytes, &hdr->client_id, sizeof(hdr->client_id));
    ahdr.endpoint_id           = hdr->endpoint_id;
    ahdr.client_message_id     = hdr->transaction_id;
    ahdr.message_size          = hdr->remaining_objects;
    ahdr.is_message_compressed = false;

    // I3_LOG(LOG_MASK_REACH, "%s(): hdr: type %d, remain %d, trans_id %d.", __FUNCTION__,
    //        hdr->message_type, hdr->remaining_objects, hdr->transaction_id);

    uint8_t *encBuffer = (uint8_t *)sCr_encoded_response_buffer;
    // encode the header
    if (!encode_ahsoka_header(&ahdr,
                              &encBuffer[2],
                              sizeof(sCr_encoded_response_buffer) - 2,
                              &sCr_encoded_response_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode ahsoka header %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    // copy the size to the start of the buffer
    uint16_t header_size = sCr_encoded_response_size;
    *(uint16_t *)encBuffer = header_size;
    // memcpy(encBuffer, &header_size, sizeof(header_size));
    I3_LOG(LOG_MASK_AHSOKA, "Place header_size %d at head of buffer.", header_size);
    LOG_DUMP_MASK(LOG_MASK_AHSOKA, "ahsoka header with size prefix: ",
                  encBuffer, header_size+2);

    // encode the payload
    if (!encode_reach_payload(message_type, payload,
                              &encBuffer[header_size+2],
                              sizeof(sCr_encoded_response_buffer) - 2 - header_size,
                              &sCr_encoded_payload_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode payload %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    sCr_encoded_response_size = sCr_encoded_payload_size + header_size + 2;
    LOG_DUMP_MASK(LOG_MASK_AHSOKA, "ahsoka message complete: ",
                  encBuffer, sCr_encoded_response_size);
    return 0;
}

  #ifdef TEST_HEADER
    void pvtCr_test_ahsoka_header()
    {
        // create an ahsoka format GDI, and decode it.
        sClassic_header_format = false;

        // make up payload of type cr_DeviceInfoRequest
        cr_PingRequest payload = {0};
        payload.echo_data.size = sprintf((char*)payload.echo_data.bytes, "aaa bbb ccc");
        // encodes to sCr_encoded_response_buffer 
        cr_ReachMessageHeader hdr;
        hdr.message_type = cr_ReachMessageTypes_PING;
        hdr.endpoint_id = 1;
        hdr.client_id = 2;
        hdr.remaining_objects = 3;
        hdr.transaction_id = 4;
        // encodes message to sCr_encoded_response_buffer.
        sTestHeader = true; // tell encode to use request, not response
        int rval = sCr_encode_message(cr_ReachMessageTypes_PING, &payload, &hdr);
        if (rval != 0)
        {
            LOG_ERROR("sCr_encode_message (ahsoka) failed with %d", rval);
            return;
        }
        sTestHeader = false;

        LOG_REACH("Copy %d bytes from response to encoded", sCr_encoded_response_size);
        sCr_encoded_message_size =  sCr_encoded_response_size;
        memcpy(sCr_encoded_message_buffer, sCr_encoded_response_buffer, sCr_encoded_message_size);
        LOG_DUMP_WIRE("ahsoka encoded: ",
                      sCr_encoded_message_buffer, sCr_encoded_message_size);

        LOG_REACH("Decode");
        handle_coded_prompt();
        LOG_REACH("done");
        sClassic_header_format = false;
    }
  #endif // TEST_HEADER
#else
// Classic version
// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static int sCr_encode_message(cr_ReachMessageTypes message_type,   // in
                             const void *payload,                  // in:  to be encoded
                             cr_ReachMessageHeader *hdr)           // in
{
    // I3_LOG(LOG_MASK_REACH, "%s(): hdr: type %d, remain %d, trans_id %d.", __FUNCTION__,
    //        hdr->message_type, hdr->remaining_objects, hdr->transaction_id);

    if (!encode_reach_payload(message_type, payload,
                              sCr_encoded_payload_buffer,
                              sizeof(sCr_encoded_payload_buffer),
                              &sCr_encoded_payload_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode payload %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }

    // build the message envelope
    sCr_uncoded_message_structure.header     = *hdr;
    sCr_uncoded_message_structure.has_header = true;
    memcpy(sCr_uncoded_message_structure.payload.bytes, 
           sCr_encoded_payload_buffer, 
           sCr_encoded_payload_size);
    sCr_uncoded_message_structure.payload.size = sCr_encoded_payload_size;  

    I3_LOG(LOG_MASK_REACH, "%s(): type %d, remain %d, trans_id %d, client %d, ep %d.", 
           __FUNCTION__,
           sCr_uncoded_message_structure.header.message_type, 
           sCr_uncoded_message_structure.header.remaining_objects, 
           sCr_uncoded_message_structure.header.transaction_id,
           sCr_uncoded_message_structure.header.client_id,
           sCr_uncoded_message_structure.header.endpoint_id);

    // encode the wrapped message
    if (!encode_reach_message(&sCr_uncoded_message_structure,
                              sCr_encoded_response_buffer,
                              sizeof(sCr_encoded_response_buffer),
                              &sCr_encoded_response_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode message %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    return 0;
}
#endif  // def AHSOKA_HEADER


