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
 * @date    2023-08-22
 *
 ********************************************************************************************/


/**
 * @file      reach_decode.c
 * @brief     Support for protobuf decoding.
 * 
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 */

#include <pb_decode.h>

#include "reach-server.h"
#include "i3_log.h"
#include "message_util.h"
#include "reach_decode.h"

static uint32_t sDecodeReach_current_transaction = 0;

uint32_t cr_get_transaction_id()
{
    return sDecodeReach_current_transaction;
}

bool decode_reach_message(cr_ReachMessage *message,  // out: decoded
                          const uint8_t *buffer,     // in:  encoded
                          size_t size)               // in:  encoded size
{
    pb_istream_t is_stream = pb_istream_from_buffer(buffer, size);

    bool status = pb_decode(&is_stream, cr_ReachMessage_fields, (void *)message);
    if (status)
    {
        sDecodeReach_current_transaction = message->header.transaction_id;
        return true;
    }
    LOG_ERROR("Decoding failed: %s\n", PB_GET_ERROR(&is_stream));
    return false;
}


bool decode_reach_payload(cr_ReachMessageTypes message_type,     // in:  from the header
                          void *data,               // out: decode to here.
                          const uint8_t *buffer,    // in:  encoded from the header
                          size_t size)              // in:  encoded size
{

  /* Create a stream that reads from the buffer. */
  pb_istream_t is_stream = pb_istream_from_buffer(buffer, size);

  bool status = false;
  /* Now we are ready to decode the message. */
  switch (message_type) {
    case cr_ReachMessageTypes_GET_DEVICE_INFO:
        status = pb_decode(&is_stream, cr_DeviceInfoRequest_fields, data);
        if (status) {
          LOG_REACH(
              "Get device info request: \n%s\n",
              message_util_get_device_info_json());
        }
        break;
  case cr_ReachMessageTypes_PING:
      status = pb_decode(&is_stream, cr_PingRequest_fields, data);
      if (status) {
          LOG_REACH("Ping request: \n%s\n",
                    message_util_ping_json((cr_PingRequest *)data));
      } 
      break;
#ifdef INCLUDE_PARAMETER_SERVICE
  case cr_ReachMessageTypes_DISCOVER_PARAMETERS:
      status = pb_decode(&is_stream, cr_ParameterInfoRequest_fields, data);
      if (status) {
        LOG_REACH(
            "Parameter info request: \n%s\n",
            message_util_param_info_json((cr_ParameterInfoRequest *)data));
      }
      break;
  case cr_ReachMessageTypes_DISCOVER_PARAM_EX:
      status = pb_decode(&is_stream, cr_ParameterInfoRequest_fields, data);
      if (status) {
        LOG_REACH(
            "Parameter EX info request: \n%s\n",
            message_util_param_info_json((cr_ParameterInfoRequest *)data));
      }
      break;
  case cr_ReachMessageTypes_READ_PARAMETERS:
      status = pb_decode(&is_stream, cr_ParameterRead_fields, data);
      if (status) {
        LOG_REACH("Parameter read request: \n%s\n",
                  message_util_read_param_json((cr_ParameterRead *)data));
      }
      break;
  case cr_ReachMessageTypes_WRITE_PARAMETERS:
      status = pb_decode(&is_stream, cr_ParameterWrite_fields, data);
      if (status) {
        LOG_REACH("Parameter write request: \n%s\n",
                  message_util_write_param_json((cr_ParameterWrite *)data));
      }
      break;
    case cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY:
        status = pb_decode(&is_stream, cr_ParameterNotifyConfig_fields, data);
        if (status) {
          LOG_REACH("Parameter notify config: \n%s\n",
                    message_util_config_notify_param_json((cr_ParameterNotifyConfigResponse *)data));
        }
        break;

#endif  // def INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_FILE_SERVICE
  case cr_ReachMessageTypes_DISCOVER_FILES:
      status = pb_decode(&is_stream, cr_DiscoverFiles_fields, data);
      if (status) {
        LOG_REACH("Discover files request: \n%s\n",
                  message_util_discover_files_json());
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_INIT:
      status = pb_decode(&is_stream, cr_FileTransferInit_fields, data);
      if (status) {
        LOG_REACH("Transfer init request: \n%s\n",
                  message_util_transfer_init_json((cr_FileTransferInit *)data));
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA:
      status = pb_decode(&is_stream, cr_FileTransferData_fields, data);
      if (status) 
      {
          // cr_FileTransferData *ftd = (cr_FileTransferData*)data;
          LOG_REACH("Transfer data received: \n%s\n",
                    message_util_transfer_data_json((cr_FileTransferData *)data));
          // LOG_DUMP_MASK(LOG_MASK_REACH, "Data Decoded", 
          //                    ftd->message_data.bytes, ftd->message_data.size);
      }
      break;
  case cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION:
      status =
          pb_decode(&is_stream, cr_FileTransferDataNotification_fields, data);
      if (status) {
        LOG_REACH("Transfer data notification request: \n%s\n",
                  message_util_transfer_data_notification_json(
                      (cr_FileTransferDataNotification *)data));
      }
      break;
#endif // def INCLUDE_FILE_SERVICE

#ifdef INCLUDE_STREAM_SERVICE
  case cr_ReachMessageTypes_DISCOVER_STREAMS:
    //   status = pb_decode(&is_stream, cr_StreamsRequest_fields, data);
    //   if (status) {
    //     LOG_REACH(
    //         "Discover streams request: \n%s\n",
    //         message_util_discover_streams_json((cr_StreamsRequest *)data));
    //   }
    break;
#endif  // def INCLUDE_STREAM_SERVICE

#ifdef INCLUDE_COMMAND_SERVICE
  case cr_ReachMessageTypes_DISCOVER_COMMANDS:
      status = pb_decode(&is_stream, cr_DiscoverCommands_fields, data);
      if (status) {
        LOG_REACH(
            "Manage stream request: \n%s\n",
            message_util_discover_commands_json());
      }
      break;
  case cr_ReachMessageTypes_SEND_COMMAND:
      status = pb_decode(&is_stream, cr_SendCommand_fields, data);
      if (status) {
        LOG_REACH("Send command request: \n%s\n",
                  message_util_send_command_json((cr_SendCommand *)data));
      }
      break;
#endif  // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_CLI_SERVICE
  case cr_ReachMessageTypes_CLI_NOTIFICATION:
      status = pb_decode(&is_stream, cr_CLIData_fields, data);
      if (status) {
        LOG_REACH("CLI Notification request: \n%s\n",
                  message_util_cli_notification_json((cr_CLIData *)data));
      }
      break;
#endif  // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_TIME_SERVICE
  case cr_ReachMessageTypes_GET_TIME:
      status = pb_decode(&is_stream, cr_TimeGetRequest_fields, data);
      if (status) {
        LOG_REACH("Get Time request: \n%s\n",
                  message_util_time_get_request_json((cr_TimeGetRequest *)data));
      }
      break;
  case cr_ReachMessageTypes_SET_TIME:
      status = pb_decode(&is_stream, cr_TimeSetRequest_fields, data);
      if (status) {
        LOG_REACH("Set Time request: \n%s\n",
                  message_util_time_set_request_json((cr_TimeSetRequest *)data));
      }
      break;
#endif  // def INCLUDE_CLI_SERVICE

  default:
      break;
  }

  if (status)
      return true;

  LOG_ERROR("Decoding failed: %s\n", PB_GET_ERROR(&is_stream));
  return false;
}
