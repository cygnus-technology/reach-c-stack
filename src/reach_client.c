/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023
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
 *
 * @author  i3 Product Development
 *          Wei Zhou wei.zhou@i3pd.com
 *
 * @version 0.0.1
 * @date    2023-09-26
 * @brief   This file provides support for the socket based notification
 *          server running on the other end.  It primarily supports
 *          asyncrhonous CLI feedback via crcb_cli_respond().
 *          It duplicates the memory and functions used with the normal
 *          reach server.  A BLE link would be more efficient with memory.
 *
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Include this header for close function

#include "message_util.h"
#include "reach-server.h"
#include "cr_stack.h"
#include "i3_log.h"


#include "pb_encode.h"


// The device acts as a client for notifications.
// Notifications can be are asynchronous to server messages.
// Hence a separate set of buffers is used.
// The buffer usage is optimized for minimum memory using a ping/pong configuration.
// Possibly the ping pong buffers could be a bit smaller.

static uint8_t sRs_ping[CR_CODED_BUFFER_SIZE]    ALIGN_TO_WORD;
static uint8_t sRs_rawPong[CR_CODED_BUFFER_SIZE] ALIGN_TO_WORD;
// the "pong" buffer is set to be the uncoded data in the cr_ReachMessage structure.
static cr_ReachMessage *psRs_uncoded_message_structure = (cr_ReachMessage *)sRs_rawPong;
static uint8_t *psRs_pong = NULL;  // set below

static size_t sRs_encoded_payload_size;
static size_t sRs_encoded_response_size = 0;

static
bool encode_notification_payload(cr_ReachMessageTypes message_type,    // in
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
  case cr_ReachMessageTypes_ERROR_REPORT:
      status = pb_encode(&os_stream, cr_ErrorReport_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
        // cr_ErrorReport *er = (cr_ErrorReport *)data;
        // I3_LOG(LOG_MASK_REACH, "Error Report: %s", er->result_string);
      }
      break;
  case cr_ReachMessageTypes_DISCOVER_STREAMS:
    //  Stream support not yet coded
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
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
      status = pb_encode(&os_stream, cr_CLIData_fields, data);
      if (status) {
        *encode_size = os_stream.bytes_written;
      }
      break;
  case cr_ReachMessageTypes_PARAMETER_NOTIFICATION:
      status = pb_encode(&os_stream, cr_ParameterNotification_fields, data);
      if (status) {
          *encode_size = os_stream.bytes_written;
      }

    break;
  default:
      status = cr_ErrorCodes_NO_DATA;
      break;
  }

  if (status)
      return true;

  I3_LOG(LOG_MASK_ERROR, "%s: payload encoding failed: %s\n", __FUNCTION__, PB_GET_ERROR(&os_stream));
  return false;
}

static
bool encode_notification_message(const cr_ReachMessage *message,   // in:  message to be encoded
                          uint8_t *buffer,                  // out: Buffer to encode into
                          size_t buffer_size,               // in:  max size of encoded message
                          size_t *encode_size)              // out: actual size of encoded message.
{
  pb_ostream_t os_stream = pb_ostream_from_buffer(buffer, buffer_size);

  bool status = pb_encode(&os_stream, cr_ReachMessage_fields, (void *)message);
  if (status) {
    *encode_size = os_stream.bytes_written;
  } else {
    I3_LOG(LOG_MASK_ERROR, "%s: Message encoding failed: %s\n", __FUNCTION__, PB_GET_ERROR(&os_stream));
  }
  return status;
}

// encodes message to sRs_ping.  payload is expected to be sRs_ping as well.
// The caller must populate the header
static int 
rs_notification_message(cr_ReachMessageTypes message_type,    // in
                        const void *payload)                  // in:  to be encoded
{
    // I3_LOG(LOG_MASK_REACH, "%s(): hdr: type %d, remain %d, trans_id %d.", __FUNCTION__,
    //        hdr->message_type, hdr->remaining_objects, hdr->transaction_id);

   if (psRs_pong == NULL)
   {
       psRs_pong = (uint8_t *)psRs_uncoded_message_structure->payload.bytes;
   }

    if (!encode_notification_payload(message_type, payload,
                                     psRs_pong, REACH_MESSAGE_PAYLOAD_MAX,
                                     &sRs_encoded_payload_size))
    {
        cr_report_error(cr_ErrorCodes_ENCODING_FAILED, "encode payload %d failed.", message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }

    // build the message envelope
    psRs_uncoded_message_structure->header.message_type = message_type;
    psRs_uncoded_message_structure->header.remaining_objects = 0;
    // The transaction ID should always be 0 for the app
    // To Do:  Get the endpoint and client ID's from cr_stack.c
    psRs_uncoded_message_structure->header.client_id      = 0;
    psRs_uncoded_message_structure->header.endpoint_id    = 0;
    psRs_uncoded_message_structure->header.transaction_id = 0;
    psRs_uncoded_message_structure->has_header = true;
    psRs_uncoded_message_structure->payload.size = sRs_encoded_payload_size;
    // psRs_uncoded_message_structure->payload.bytes  is already sRs_pong

    // encode the wrapped message back into ping
    if (!encode_notification_message(psRs_uncoded_message_structure,
                                     sRs_ping, sizeof(sRs_ping),
                                     &sRs_encoded_response_size))
    {
        I3_LOG(LOG_MASK_ERROR, "%s: encode message %d failed.", __FUNCTION__, message_type);
        return cr_ErrorCodes_ENCODING_FAILED;
    }
    return 0;
}

// When the device supports a CLI it is expected to share anything printed 
// to the CLI back to the stack for remote display using crcb_cli_respond()
int crcb_cli_respond(char *cli)
{
    if (!cr_get_comm_link_connected())
        return 0;
    I3_LOG(LOG_MASK_WIRE, TEXT_GREEN "%s for CLI" TEXT_RESET, __FUNCTION__);
    cr_CLIData *cld = (cr_CLIData*)sRs_ping;
    strncpy(cld->message_data, cli, REACH_ERROR_BUFFER_LEN);
    cld->is_complete = true;

    rs_notification_message(cr_ReachMessageTypes_CLI_NOTIFICATION, sRs_ping);

    LOG_DUMP_WIRE("CLI", sRs_ping, sRs_encoded_response_size);
    crcb_send_coded_response(sRs_ping, sRs_encoded_response_size);

    return 0;
}

// When the device supports a CLI it is expected to share anything printed 
// to the CLI back to the stack for remote display using crcb_cli_respond()
int crcb_notify_error(cr_ErrorReport *err)
{
    cr_ErrorReport *errRep = (cr_ErrorReport*)sRs_ping;
    memcpy(errRep, err, sizeof(cr_ErrorReport));

    rs_notification_message(cr_ReachMessageTypes_ERROR_REPORT, sRs_ping);

    LOG_DUMP_WIRE("error report", sRs_ping, sRs_encoded_response_size);
    crcb_send_coded_response(sRs_ping, sRs_encoded_response_size);

    return 0;
}

#if NUM_SUPPORTED_PARAM_NOTIFY >= 0
// When the device supports a CLI it is expected to share anything printed
// to the CLI back to the stack for remote display using crcb_cli_respond()
int crcb_notify_param(cr_ParameterValue *param)
{
    cr_ParameterNotification *note = (cr_ParameterNotification*)sRs_ping;
    note->values_count = 1;
    memcpy(&note->values[0], param, sizeof(cr_ParameterValue));

    rs_notification_message(cr_ReachMessageTypes_PARAMETER_NOTIFICATION,
                            sRs_ping);

    i3_log(LOG_MASK_PARAMS, TEXT_MAGENTA "Notify PID %d" TEXT_RESET, param->parameter_id);
    LOG_DUMP_WIRE("notification", sRs_ping, sRs_encoded_response_size);
    crcb_send_coded_response(sRs_ping, sRs_encoded_response_size);

    return 0;
}
#endif // NUM_SUPPORTED_PARAM_NOTIFY >= 0
