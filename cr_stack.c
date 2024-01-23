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
 * @brief     Contains the core of the Cygnus Reach firmware stack.
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

// H file provided by the app to configure the stack.
#include "reach-server.h"

#include "cr_stack.h"
#include "cr_private.h"
#include "i3_log.h"

#include "cJSON.h"
#include "pb_decode.h"
#include "pb_encode.h"

#include "message_util.h"
#include "reach_decode.h"
#include "reach_version.h"

//----------------------------------------------------------------------------
// static buffers used and reused by the reach stack.
// This is private data.
//----------------------------------------------------------------------------

// terminology
// A transaction is a series of messages.
// A message has a header and a payload.
// The prompt is a received payload.
// The response is a generated payload.
// A file "transfer" is a series of messages terminated by an ACK.

// the fully encoded message is received in the 
// sCr_encoded_message_buffer. 
static uint8_t sCr_encoded_message_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
static size_t  sCr_encoded_message_size = 0;

// The message header is decoded into this buffer containing an encoded payload buffer: 
static cr_ReachMessage sCr_uncoded_message_structure;

// The payload buffers are slightly smaller than the CR_CODED_BUFFER_SIZE
// so that the header can be added.
#define UNCODED_PAYLOAD_SIZE  (CR_CODED_BUFFER_SIZE-4)

// A decoded prompt payload.
// This can be reused from the encoded message buffer.
// static uint8_t sCr_decoded_prompt_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD;
static uint8_t *sCr_decoded_prompt_buffer = sCr_encoded_message_buffer;

// An uncoded response payload.
static uint8_t sCr_uncoded_response_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD;

// The response payload is encoded into sCr_encoded_payload_buffer[]. 
static uint8_t sCr_encoded_payload_buffer[UNCODED_PAYLOAD_SIZE] ALIGN_TO_WORD; 
static size_t sCr_encoded_payload_size; 
 
// The response payload is copied into the sCr_uncoded_message_structure

// The sCr_uncoded_message_structure is encoded into sCr_encoded_response_buffer[]  
static uint8_t sCr_encoded_response_buffer[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
static size_t  sCr_encoded_response_size = 0;


//----------------------------------------------------------------------------
// private but global variables shared with the Reach stack
//----------------------------------------------------------------------------
cr_ReachMessageTypes pvtCr_continued_message_type;
uint32_t pvtCr_num_continued_objects = 0;
uint32_t pvtCr_num_remaining_objects = 0;

//----------------------------------------------------------------------------
// static (private) "member" variables
//----------------------------------------------------------------------------
static int sCr_transaction_id = 0;
static bool sCR_error_reported = false;

//----------------------------------------------------------------------------
// static (private) "member" functions
//----------------------------------------------------------------------------

// <summary> Decodes and responds to the coded prompt provided
// to the Reach core. Calls handle_message() 
// </summary>
// <returns>0 on success or an error</returns>
static int handle_coded_prompt();

// <summary> Decodes the payload and calls the appropriate 
// handler function. 
// </summary>
// <param name="hdr"> Includes type of message</param>
// <param name="data">The actual message</param>
// <param name="size">in bytes</param>
// <returns>0 on success or an error</returns>
static int 
handle_message(const cr_ReachMessageHeader *hdr, const uint8_t *data, size_t size);

// <summary>
// Respond to a ping request
// </summary>
// <param name="request"> The triggering message</param>
// <param name="response">Response delivered here</param>
// <returns></returns>
static int handle_ping(const cr_PingRequest *request, cr_PingResponse *response);


// <summary>
// Respond to a request for device info
// </summary>
// <param name="request"> The triggering message</param>
// <param name="response">Response delivered here</param>
// <returns></returns>
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
                                        cr_DiscoverCommandsResult *);
    static int handle_send_command(const cr_SendCommand *, cr_SendCommandResult *);
#endif // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
    // CLI
    static int handle_cli_notification(const cr_CLIData *request, cr_CLIData *);
#endif // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_TIME_SERVICE
    static int handle_time_set(const cr_TimeSetRequest *request, 
                               cr_TimeSetResult *response);
    static int handle_time_get(const cr_TimeGetRequest *request, 
                               cr_TimeGetResult *response);
#endif  // def INCLUDE_TIME_SERVICE

// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static
int cr_encode_message(cr_ReachMessageTypes message_type,        // in
                      const void *payload,                      // in:  to be encoded
                      cr_ReachMessageHeader *hdr);              // in

// Internal encode functions, to be deprecated.  Use cr_encode_message.
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
        rval = pvtCrParam_read_param(NULL, (cr_ParameterReadResult *)sCr_uncoded_response_buffer);
        break;
    #endif  // def INCLUDE_PARAMETER_SERVICE

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
    msg_header.endpoint_id       = 0;
    msg_header.number_of_objects = pvtCr_num_continued_objects;
    msg_header.remaining_objects = pvtCr_num_remaining_objects;
    msg_header.transaction_id    = sCr_transaction_id;
    rval = cr_encode_message(encode_message_type,          // in
                             sCr_uncoded_response_buffer,  // in:  to be encoded
                             &msg_header);

    if (pvtCr_num_remaining_objects == 0)
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
    return rval;
}

static bool sCr_challenge_key_valid = true;
static bool test_challenge_key_is_valid(uint32_t challenge_key)
{
    #ifndef APP_REQUIRED_CHALLENGE_KEY 
        (void)challenge_key;
        return true;
        sCr_challenge_key_valid = true;
    #else
        if (challenge_key == APP_REQUIRED_CHALLENGE_KEY) 
        {
            sCr_challenge_key_valid = true;
            return true;
        }
        sCr_challenge_key_valid = false;
        cr_report_error(cr_ErrorCodes_CHALLENGE_FAILED, 
                        "Requred challenge key does not match.");
        return false;
    #endif
}

bool pvtCr_challenge_key_is_valid(void)
{
    #ifndef APP_REQUIRED_CHALLENGE_KEY 
        return true;
    #else
        if (!sCr_challenge_key_valid) {
            cr_report_error(cr_ErrorCodes_CHALLENGE_FAILED, 
                            "Challenge failed.");
        }
        return sCr_challenge_key_valid;
    #endif
}

/**
* @brief   cr_init
* @details To be called before starting the stack.
* @note    Not much happens here yet.
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error like cr_ErrorCodes_. 
*/
int cr_init() 
{
  // #define TEST_NOTIFICATION
  #ifdef TEST_NOTIFICATION
    // Test notification
    sCr_param_notify_list[0].parameter_id = 69;  // [11]
    sCr_param_notify_list[0].enabled = true;
    sCr_param_notify_list[0].minimum_notification_period = SYS_TICK_RATE;
    sCr_param_notify_list[0].maximum_notification_period = 100*SYS_TICK_RATE;
    sCr_param_notify_list[0].minimum_delta = 15.0;
    sCr_last_param_values[0].parameter_id = 69;
    sCr_last_param_values[0].timestamp = 0;
    sCr_last_param_values[0].which_value = cr_ParameterDataType_INT32;
    sCr_last_param_values[0].value.sint32_value = 1;
  #endif  // def TEST_NOTIFICATION

    return cr_ErrorCodes_NO_ERROR;
}

/**
* @brief   cr_set_advertised_name
* @details Set the name of the device that should be advertised before 
*          connecting.  Used in BLE.
* @note    Just store the string up to length REACH_SHORT_STRING_LEN.  The code 
*          setting up the link can retrieve this using cr_get_advertised_name()
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/
char sCr_advertised_name[REACH_SHORT_STRING_LEN];
int cr_set_advertised_name(char *name, int length)
{
    strncpy(sCr_advertised_name, name, REACH_SHORT_STRING_LEN);
    if (length >= REACH_SHORT_STRING_LEN) 
        return REACH_SHORT_STRING_LEN;
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
* @brief   cr_set_advertised_name
* @details Set the name of the device that should be advertised before 
*          connecting.  Used in BLE.
* @note    Just store the string up to length REACH_SHORT_STRING_LEN.  The code 
*          setting up the link can retrieve this using cr_get_advertised_name()
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/

// allows the application to store the prompt where the Reach stack can see it.
int cr_store_coded_prompt(uint8_t *data, size_t len)
{
    affirm(len <= sizeof(sCr_encoded_message_buffer));

    memcpy(sCr_encoded_message_buffer, data, len);
    sCr_encoded_message_size = len;
    return 0;
}

int cr_get_coded_response_buffer(uint8_t **pResponse, size_t *len)
{
    *pResponse = sCr_encoded_response_buffer;
    *len = sCr_encoded_response_size;
    if (sCr_encoded_response_size == 0)
        return cr_ErrorCodes_NO_DATA;
    sCr_encoded_response_size = 0;
    return cr_ErrorCodes_NO_ERROR;
}

// static uint32_t lastTick = 0;
static int sCallCount = 0;
static uint32_t sCurrentTicks = 0;
// The application must call cr_process() regularly.
// ticks tells it approximately how many  milliseconds have passed since
// the system started.  This allows it to perform timing related tasks.
int cr_process(uint32_t ticks) 
{
    sCurrentTicks = ticks;   // store it so others can use it.
    sCallCount++;

    if (!cr_get_ble_connected())
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

  // #define TEST_ERROR_REPORT
  #ifdef TEST_ERROR_REPORT
    if (sCallCount == 500)
    {
        i3_log(LOG_MASK_ALWAYS, "On tick 500, test the error report using Future Legend:");
        // Tick 2 error test: Send 190 characters to test too long string.
        cr_report_error(0, "And in the death, as the last few corpses "
                        "lay rotting on the slimy thoroughfare, "
                        "Fleas the size of rats sucked on rats the size of cats "
                        "and ten thousand peoploids split into small tribes...\n");
        // Credit to David Bowie for the text...
        crcb_send_coded_response(sCr_encoded_response_buffer, sCr_encoded_response_size);
        return 0;
    }
    if (sCallCount > 5000) sCallCount = 5000;
  #endif  // def TEST_ERROR_REPORT

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

        if (rval && !sCR_error_reported)
        {
            // The functions called here must report their errors
            // and return the error code.  This is a backup.
            cr_report_error(rval, "Otherwise unreported error");
        }
        sCR_error_reported = false;
    }
    crcb_send_coded_response(sCr_encoded_response_buffer, sCr_encoded_response_size);

    return cr_ErrorCodes_NO_ERROR;
}

// <summary>
// The tick count is passed in to cr_process().  This function 
// gives other Reach functions access to that value. 
// </summary>
// <returns> the 32 bit tick count passed in to 
// cr_process() </returns> 
uint32_t cr_get_current_ticks()
{
    // a 32 bit number will roll over in 49 days at 1kHz.
    return sCurrentTicks;
}

 // <summary>
 //  The BLE stack must inform the Reach stack of the status of
 //  the BLE connection.  The Reach loop only runs when BLE is
 //  connected.  All parameter notifications are cleared when
 //  a BLE connection is established. The client must reenable
 //  notifications on each connection.
 // </summary>
 static bool sCr_ble_is_connected = false;
 void cr_set_ble_connected(bool connected)
 { 
   if (!sCr_ble_is_connected && connected)
   {
       // we are newly connected, so clear any stale data.
       pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
       pvtCr_num_continued_objects = 0; 
       pvtCr_num_remaining_objects = 0;
       pvtCrParam_clear_notifications();
     #ifdef APP_REQUIRED_CHALLENGE_KEY
       sCr_challenge_key_valid = false;
     #endif 
     #ifdef APP_REQUIRED_PARAMETER_KEY
       sCr_parameter_key_valid = false;
     #endif
   }
   sCr_ble_is_connected = connected;
 } 

 bool cr_get_ble_connected(void)
 { 
   return sCr_ble_is_connected;
 } 

void cr_report_error(int error_code, const char *fmt, ...)
{
    va_list args;
    cr_ErrorReport *err = (cr_ErrorReport *)sCr_uncoded_response_buffer;
    err->result_value = error_code;

    va_start(args, fmt);
    int ptr = snprintf(err->result_string,
                       REACH_BYTES_IN_A_FILE_PACKET,
                       "Error %d: ", error_code);
    vsnprintf(&err->result_string[ptr],
              REACH_BYTES_IN_A_FILE_PACKET-ptr, fmt, args);
    // force termination
    err->result_string[REACH_BYTES_IN_A_FILE_PACKET-1] = 0;
    va_end(args);
    // i3_log(LOG_MASK_WARN, "error string %d char", strlen(err->result_string));

  #define ASYNC_ERROR_NOTIFCATION
  #ifdef ASYNC_ERROR_NOTIFCATION
    crcb_notify_error(err);
    i3_log(LOG_MASK_WARN, "Logged Error report:");
    i3_log(LOG_MASK_ERROR, "%s", err->result_string);
  #else
    cr_ReachMessageHeader msg_header;
    msg_header.message_type      = cr_ReachMessageTypes_ERROR_REPORT;
    msg_header.number_of_objects = 0;
    msg_header.remaining_objects = 0;
    msg_header.transaction_id    = 0;
    int rval = cr_encode_message(cr_ReachMessageTypes_ERROR_REPORT, // in
                                 sCr_uncoded_response_buffer,       // in:  to be encoded
                                 &msg_header);                      // in
    if (0 != rval)
    {
        i3_log(LOG_MASK_ERROR, "Error Encoding an error report:\r\n"
                               "%s", err->result_string);
        return;
    }
    i3_log(LOG_MASK_WARN, "Logged Error report:");
    i3_log(LOG_MASK_ERROR, "%s", err->result_string);
    sCR_error_reported = true;
  #endif
}

// 
// Static functions 
//

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

    I3_LOG(LOG_MASK_REACH, "Message type: \t%s",
           msg_type_string(msgPtr->header.message_type));
    LOG_DUMP_WIRE("handle_coded_prompt (message): ",
                       msgPtr->payload.bytes, msgPtr->payload.size);
    I3_LOG(LOG_MASK_REACH, "Prompt Payload size: %d. Transaction ID %d", 
           msgPtr->payload.size, sCr_transaction_id);

    // further decode and process the message
    // The result will be fully encoded at sCr_encoded_response_buffer[]
    // in case of a non-zero return there will be an encoded error report.
    return handle_message(hdr, coded_data, msgPtr->payload.size);
}


// #define VERBOSE_SIZES
static size_t sMaxBufferSize = 0;
static int checkSize(size_t test, size_t limit, char *name)
{
    if (test > sMaxBufferSize)
        sMaxBufferSize = test;

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

// The test harness checks that buffer sizes are not too big.
// These buffer sizes are adjusted in reach.options.
void cr_test_sizes()
{
    int rval = 0;
    #define MAX_BLE_SZ  CR_CODED_BUFFER_SIZE

  #ifdef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, "Verbose buffer size report:" TEXT_RESET);
  #else
    i3_log(LOG_MASK_ALWAYS, "Silent buffer size check:  " TEXT_RESET);
  #endif
    // reach.pb.h gives us some "Maximum encoded size of messages".
    // Check these.
    rval += checkSize(cr_CLIData_size, MAX_BLE_SZ, "cr_CLIData_size");
    rval += checkSize(cr_CommandInfo_size, MAX_BLE_SZ, "cr_CommandInfo_size");
    rval += checkSize(cr_DeviceInfoResponse_size, MAX_BLE_SZ, "cr_DeviceInfoResponse_size");
    rval += checkSize(cr_DiscoverCommandsResult_size, MAX_BLE_SZ, "cr_DiscoverCommandsResult_size");
    rval += checkSize(cr_DiscoverFilesReply_size, MAX_BLE_SZ, "cr_DiscoverFilesReply_size");
    rval += checkSize(cr_DiscoverStreamsResponse_size, MAX_BLE_SZ, "cr_DiscoverStreamsResponse_size");
    rval += checkSize(cr_ParamExKey_size, MAX_BLE_SZ, "cr_ParamExKey_size");
    rval += checkSize(cr_ErrorReport_size, MAX_BLE_SZ, "cr_ErrorReport_size");
    rval += checkSize(cr_FileInfo_size, MAX_BLE_SZ, "cr_FileInfo_size");
    rval += checkSize(cr_FileTransferDataNotification_size, MAX_BLE_SZ, "cr_FileTransferDataNotification_size");
    rval += checkSize(cr_FileTransferData_size, MAX_BLE_SZ, "cr_FileTransferData_size");
    rval += checkSize(cr_FileTransferInitReply_size, MAX_BLE_SZ, "cr_FileTransferInitReply_size");
    rval += checkSize(cr_FileTransferInit_size, MAX_BLE_SZ, "cr_FileTransferInit_size");
    rval += checkSize(cr_ParamExInfoResponse_size, MAX_BLE_SZ, "cr_ParamExInfoResponse_size");
    rval += checkSize(cr_ParameterInfoRequest_size, MAX_BLE_SZ, "cr_ParameterInfoRequest_size");
    rval += checkSize(cr_ParameterInfoResponse_size, MAX_BLE_SZ, "cr_ParameterInfoResponse_size");
    rval += checkSize(cr_ParameterInfo_size, MAX_BLE_SZ, "cr_ParameterInfo_size");
    rval += checkSize(cr_ParameterNotification_size, MAX_BLE_SZ, "cr_ParameterNotification_size");
    rval += checkSize(cr_ParameterNotifyConfigResult_size, MAX_BLE_SZ, "cr_ParameterNotifyConfigResult_size");
    rval += checkSize(cr_ParameterNotifyConfig_size, MAX_BLE_SZ, "cr_ParameterNotifyConfig_size");
    rval += checkSize(cr_ParameterReadResult_size, MAX_BLE_SZ, "cr_ParameterReadResult_size");
    rval += checkSize(cr_ParameterRead_size, MAX_BLE_SZ, "cr_ParameterRead_size");
    rval += checkSize(cr_ParameterValue_size, MAX_BLE_SZ, "cr_ParameterValue_size");
    rval += checkSize(cr_ParameterWrite_size, MAX_BLE_SZ, "cr_ParameterWrite_size");
    rval += checkSize(cr_PingRequest_size, MAX_BLE_SZ, "cr_PingRequest_size");
    rval += checkSize(cr_PingResponse_size, MAX_BLE_SZ, "cr_PingResponse_size");
    rval += checkSize(cr_ReachMessageHeader_size, MAX_BLE_SZ, "cr_ReachMessageHeader_size");
    rval += checkSize(cr_ReachMessage_size, MAX_BLE_SZ, "cr_ReachMessage_size");
    rval += checkSize(cr_SendCommandResult_size, MAX_BLE_SZ, "cr_SendCommandResult_size");
    rval += checkSize(cr_StreamData_size, MAX_BLE_SZ, "cr_StreamData_size");
    rval += checkSize(cr_StreamInfo_size, MAX_BLE_SZ, "cr_StreamInfo_size");


    // check the unencoded structure sizes.
    #define MAX_RAW_SZ  CR_DECODED_BUFFER_SIZE

    rval += checkSize(sizeof(cr_StreamInfo), MAX_RAW_SZ, "sizeof(cr_StreamInfo)");
    rval += checkSize(sizeof(cr_ErrorReport), MAX_RAW_SZ, "sizeof(cr_ErrorReport)");
    rval += checkSize(sizeof(cr_ParameterReadResult), MAX_RAW_SZ, "sizeof(cr_ParameterReadResult)");
    rval += checkSize(sizeof(cr_ParameterWrite), MAX_RAW_SZ, "sizeof(cr_ParameterWrite)");
    rval += checkSize(sizeof(cr_ParameterNotifyConfig), MAX_RAW_SZ, "sizeof(cr_ParameterNotifyConfig)");
    rval += checkSize(sizeof(cr_ParameterNotification), MAX_RAW_SZ, "sizeof(cr_ParameterNotification)");
    rval += checkSize(sizeof(cr_ParameterNotifyConfigResult), MAX_RAW_SZ, "sizeof(cr_ParameterNotifyConfigResult)");
    rval += checkSize(sizeof(cr_ParameterValue), MAX_RAW_SZ, "sizeof(cr_ParameterValue)");
    rval += checkSize(sizeof(cr_CLIData), MAX_RAW_SZ, "sizeof(cr_CLI_Data)");
    rval += checkSize(sizeof(cr_ParameterRead), MAX_RAW_SZ, "sizeof(cr_ParameterRead)");
    rval += checkSize(sizeof(cr_FileTransferData), MAX_RAW_SZ, "sizeof(cr_FileTransferData)");
    rval += checkSize(sizeof(cr_ParameterInfo), MAX_RAW_SZ, "sizeof(cr_ParameterInfo)");

    // If these don't match, check the structures associated with them
    affirm(sizeof(reach_sizes_t) == REACH_SIZE_STRUCT_SIZE);
    affirm(REACH_MAX_RESPONSE_SIZE == CR_CODED_BUFFER_SIZE);

  #ifdef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, "\n");
  #endif  // def VERBOSE_SIZES

    affirm(rval == 0);     // halt if failure

   #ifndef VERBOSE_SIZES
    i3_log(LOG_MASK_ALWAYS, TEXT_GREEN "     Size tests all pass.");
  #endif  // def VERBOSE_SIZES

}

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

    if (!decode_reach_payload(message_type,
                              sCr_decoded_prompt_buffer,
                              coded_data, size))
    {
        cr_report_error(cr_ErrorCodes_DECODING_FAILED, "%s: decode payload %d failed.", 
                        __FUNCTION__, message_type);
        return cr_ErrorCodes_DECODING_FAILED;
    }

    pvtCr_num_continued_objects = 0;  // default
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
                          (cr_ParameterReadResult *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_WRITE_PARAMETERS:
        rval = pvtCrParam_write_param((cr_ParameterWrite *)sCr_decoded_prompt_buffer,
                           (cr_ParameterWriteResult *)sCr_uncoded_response_buffer);
        break;

    #if NUM_SUPPORTED_PARAM_NOTIFY != 0
    case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
        rval = pvtCrParam_config_param_notify((cr_ParameterNotifyConfig *)sCr_decoded_prompt_buffer,
                           (cr_ParameterNotifyConfigResult *)sCr_uncoded_response_buffer);
        break;
    #endif
  #endif // def INCLUDE_PARAMETER_SERVICE

  #ifdef INCLUDE_FILE_SERVICE
    case cr_ReachMessageTypes_DISCOVER_FILES:
        rval = pvtCrFile_discover((cr_DiscoverFiles *)sCr_decoded_prompt_buffer,
                              (cr_DiscoverFilesReply *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_TRANSFER_INIT:
        rval = pvtCrFile_transfer_init((cr_FileTransferInit *)sCr_decoded_prompt_buffer,
                             (cr_FileTransferInitReply *)sCr_uncoded_response_buffer);
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
                                 (cr_DiscoverCommandsResult *)sCr_uncoded_response_buffer);
        break;

    case cr_ReachMessageTypes_SEND_COMMAND:
        rval = handle_send_command((cr_SendCommand *)sCr_decoded_prompt_buffer,
                            (cr_SendCommandResult *)sCr_uncoded_response_buffer);
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
                                (cr_TimeSetResult *)sCr_uncoded_response_buffer);
        break;
    case cr_ReachMessageTypes_GET_TIME:
        rval = handle_time_get((cr_TimeGetRequest *)sCr_decoded_prompt_buffer, 
                                (cr_TimeGetResult *)sCr_uncoded_response_buffer);
        break;
  #endif  // def INCLUDE_TIME_SERVICE

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
    msg_header.number_of_objects = pvtCr_num_continued_objects;
    msg_header.remaining_objects = pvtCr_num_remaining_objects;
    msg_header.transaction_id    = sCr_transaction_id;
    rval = cr_encode_message(encode_message_type,
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

    if (!pvtCr_challenge_key_is_valid()) {
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
static void populate_device_info_sizes(cr_DeviceInfoResponse *dir)
{
    reach_sizes_t sizes_struct; 

    sizes_struct.max_message_size             = 244;  // biggest BLE message
    sizes_struct.big_data_buffer_size         = REACH_BIG_DATA_BUFFER_LEN;
    sizes_struct.parameter_buffer_count       = REACH_COUNT_PARAM_IDS;
    sizes_struct.num_params_in_response       = REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE;
    sizes_struct.num_descriptors_in_response  = REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE;
    sizes_struct.device_description_len       = REACH_LONG_STRING_LEN;
    sizes_struct.max_param_bytes              = REACH_NUM_PARAM_BYTES;
    sizes_struct.param_info_description_len   = REACH_PARAM_INFO_DESCRIPTION_LEN;
    sizes_struct.medium_string_len            = REACH_MEDIUM_STRING_LEN;
    sizes_struct.short_string_len             = REACH_SHORT_STRING_LEN;
    sizes_struct.num_param_notifications      = NUM_SUPPORTED_PARAM_NOTIFY;
    sizes_struct.num_commands_in_response     = REACH_NUM_COMMANDS_IN_RESPONSE;
    sizes_struct.num_param_desc_in_response   = REACH_COUNT_PARAM_DESC_IN_RESPONSE;
    dir->sizes_struct.size = sizeof(reach_sizes_t);
    memcpy(dir->sizes_struct.bytes, &sizes_struct,  sizeof(reach_sizes_t));
}

// <summary>
// In response to a request for device info, get the raw data 
// from the app via a crcb function, then update the provided 
// pointer with the latest accurate data. 
// </summary>
// <param name="request"> A pointer to the request 
// which may include a challenge key.</param> 
// <param name="response">A pointer to memory where the 
// complete device info response will be composed.</param> 
// <returns>0 on success.  cr_ErrorCodes_NO_DATA if 
// the device was built to require a challenge key 
// and the provided key does not match.</returns> 
static int 
handle_get_device_info(const cr_DeviceInfoRequest *request,  // in
                       cr_DeviceInfoResponse     *response)  // out
{
    (void)request;

    memset(response, 0, sizeof(cr_DeviceInfoResponse));

    if (!test_challenge_key_is_valid(request->challenge_key)) {
        return cr_ErrorCodes_NO_DATA;
    }

    crcb_device_get_info(response);
#ifdef INCLUDE_PARAMETER_SERVICE
    response->parameter_metadata_hash = crcb_compute_parameter_hash();
#endif  // def INCLUDE_PARAMETER_SERVICE

    response->protocol_version = cr_ReachProtoVersion_CURRENT_VERSION;
    populate_device_info_sizes(response);
    return 0;
}

const char *cr_get_reach_version()
{
    return REACH_VERSION_STRING;
}

#ifdef INCLUDE_COMMAND_SERVICE
static int 
handle_discover_commands(const cr_DiscoverCommands *request,
                         cr_DiscoverCommandsResult *response)
{
    if (!pvtCr_challenge_key_is_valid()) {
        pvtCr_num_remaining_objects = 0;
        pvtCr_num_continued_objects = 0;
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    (void)request;
    int num_commands = crcb_get_command_count();
    int rval;
    if (num_commands >= REACH_NUM_COMMANDS_IN_RESPONSE)
    {
        // this is just because I'm lazy.  Write it if you need it.
        i3_log(LOG_MASK_WARN, "%s: Found %d commands.  Will only report first %d.", 
                  __FUNCTION__, num_commands, REACH_NUM_COMMANDS_IN_RESPONSE);
        num_commands = 6;
    }
    crcb_command_discover_reset(0);  // index, not really cid.
    response->available_commands_count = num_commands;
    for (int i=0; i<REACH_NUM_COMMANDS_IN_RESPONSE; i++)
    {
        rval = crcb_command_discover_next(&response->available_commands[i]);
        if (rval != 0)
        {
            if (i == 0)
            {
                LOG_ERROR("Discover commands found nothing.");
                pvtCr_num_remaining_objects = 0;
                pvtCr_num_continued_objects = 0;
                return 0;
            }
            break;
        }
    }
    pvtCr_num_remaining_objects = 0;
    pvtCr_num_continued_objects = 0;
    return 0;
}

static int handle_send_command(const cr_SendCommand *request,
                                   cr_SendCommandResult *response) 
{
    if (!pvtCr_challenge_key_is_valid()) {
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
        crcb_cli_enter(request->message_data);
        return cr_ErrorCodes_NO_RESPONSE;
    }
#endif // def INCLUDE_CLI_SERVICE


#ifdef INCLUDE_TIME_SERVICE
    static int handle_time_set(const cr_TimeSetRequest *request, 
                               cr_TimeSetResult *response)
    {
        if (!pvtCr_challenge_key_is_valid()) {
            response->result = cr_ErrorCodes_CHALLENGE_FAILED;
            return 0;
        }

        response->result = crcb_time_set(request->seconds_utc);
        response->result_message[0] = 0;

        return 0;
    }

    static int handle_time_get(const cr_TimeGetRequest *request, 
                               cr_TimeGetResult *response)
    {
        (void)request;
        if (!pvtCr_challenge_key_is_valid()) {
            response->result = cr_ErrorCodes_CHALLENGE_FAILED;
            return 0;
        }
        response->result = crcb_time_get(&response->seconds_utc);
        return 0;
    }

#endif  // def INCLUDE_TIME_SERVICE


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
      status = pb_encode(&os_stream, cr_DeviceInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Get device info response: \n%s\n",
                  message_util_get_device_info_response_json(
                      (cr_DeviceInfoResponse *)data));
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
      status = pb_encode(&os_stream, cr_PingResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Ping response: \n%s\n",
                  message_util_ping_response_json((cr_PingResponse *)data));
      }
      break;

#ifdef INCLUDE_PARAMETER_SERVICE
  case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Discover parameter response: \n%s\n",
                  message_util_param_info_response_json(
                      (cr_ParameterInfoResponse *)data));
      }
      break;
  case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
      status = pb_encode(&os_stream, cr_ParamExInfoResponse_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Discover parameter EX response: \n%s\n",
                  message_util_param_info_ex_response_json(
                      (cr_ParamExInfoResponse *)data));
      }
      break;
  case cr_ReachMessageTypes_READ_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterReadResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Read parameter response: \n%s\n",
                  message_util_read_param_response_json(
                      (cr_ParameterReadResult *)data));
      }
      break;
  case cr_ReachMessageTypes_WRITE_PARAMETERS:
      status = pb_encode(&os_stream, cr_ParameterWriteResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Write parameter response: \n%s\n",
                  message_util_write_param_response_json(
                      (cr_ParameterWriteResult *)data));
      }
      else
      {
          LOG_ERROR("pb_encode() failed for write params.");
      }
      break;

  #if NUM_SUPPORTED_PARAM_NOTIFY != 0
  case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
      status = pb_encode(&os_stream, cr_ParameterNotifyConfigResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("parameter notify config response: \n%s\n",
                  message_util_config_notify_param_json(
                      (cr_ParameterNotifyConfigResult *)data));
      }
      break;
  #endif
#endif // def INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_FILE_SERVICE
  case cr_ReachMessageTypes_DISCOVER_FILES:
      status = pb_encode(&os_stream, cr_DiscoverFilesReply_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Discover files response: \n%s\n",
                  message_util_discover_files_response_json(
                      (cr_DiscoverFilesReply *)data));
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_INIT:

      status = pb_encode(&os_stream, cr_FileTransferInitReply_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Transfer init response: \n%s\n",
                  message_util_transfer_init_response_json(
                      (cr_FileTransferInitReply *)data));
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA:
      status = pb_encode(&os_stream, cr_FileTransferData_fields, data);
      if (status) {
          *encode_size = os_stream.bytes_written;
          cr_FileTransferData *td = (cr_FileTransferData *)data;
          LOG_REACH(" Transfer Data encoded: \n%s\n", message_util_transfer_data_json(td));
          LOG_DUMP_MASK(LOG_MASK_REACH, "Data Sent", 
                        td->message_data.bytes, td->message_data.size);
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
      status =
          pb_encode(&os_stream, cr_FileTransferDataNotification_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Transfer data notification response: \n%s\n",
                  message_util_transfer_data_notification_json(
                      (cr_FileTransferDataNotification *)data));
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
    //         message_util_discover_streams_json((cr_StreamsRequest *)data));
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
      status = pb_encode(&os_stream, cr_DiscoverCommandsResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Discover commands response: \n%s\n",
                  message_util_discover_commands_response_json(
                      (cr_DiscoverCommandsResult *)data));
      }
      break;
  case cr_ReachMessageTypes_SEND_COMMAND:
      status = pb_encode(&os_stream, cr_SendCommandResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Send Command response: \n%s\n",
                  message_util_send_command_response_json(
                      (cr_SendCommandResult *)data));
      }
      break;
#endif  // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
      status = pb_encode(&os_stream, cr_CLIData_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("CLI Notification response: \n%s\n",
                  message_util_cli_notification_json((cr_CLIData *)data));
      }
      break;
#endif  // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_TIME_SERVICE
  case cr_ReachMessageTypes_SET_TIME:
      status = pb_encode(&os_stream, cr_TimeSetResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Time set response: \n%s\n",
                  message_util_time_set_response_json(
                      (cr_TimeSetResult *)data));
      }
      break;
  case cr_ReachMessageTypes_GET_TIME:
      status = pb_encode(&os_stream, cr_TimeGetResult_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        LOG_REACH("Time set response: \n%s\n",
                  message_util_time_get_response_json(
                      (cr_TimeGetResult *)data));
      }
      break;
#endif  // def INCLUDE_TIME_SERVICE

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

// encodes message to sCr_encoded_response_buffer.
// The caller must populate the header
static int cr_encode_message(cr_ReachMessageTypes message_type,    // in
                             const void *payload,                  // in:  to be encoded
                             cr_ReachMessageHeader *hdr)           // in
{
    // I3_LOG(LOG_MASK_REACH, "%s(): hdr: type %d, num_obj %d, remain %d, trans_id %d.", __FUNCTION__,
    //        hdr->message_type, hdr->number_of_objects, hdr->remaining_objects, hdr->transaction_id);

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

    I3_LOG(LOG_MASK_REACH, "%s(): type %d, num_obj %d, remain %d, trans_id %d.", __FUNCTION__,
           sCr_uncoded_message_structure.header.message_type, 
           sCr_uncoded_message_structure.header.number_of_objects, 
           sCr_uncoded_message_structure.header.remaining_objects, 
           sCr_uncoded_message_structure.header.transaction_id);

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



