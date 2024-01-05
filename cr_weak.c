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
 * \brief "cr_weak.c" contains the weak implementations of functions the user of
 * the Cygnus Reach device stack should override.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

// #include <stdio.h>
#include <stdint.h>
// #include <string.h>
// #include <assert.h>
#include <stdbool.h>

#include "cr_stack.h"
#include "i3_log.h"

// The Reach stack relies on these callback functions being implemented by the device.
// The gcc supported "weak" keyword allows us to build without having real implementations.
 
 
// The cr_process function calls crcb_get_coded_prompt() 
// to get any available prompt in coded format.  
// The implementation copies the data into the provided buffer and sets the size.
// cr_store_coded_prompt() can be used to push data into the stack.
// Then  crcb_get_coded_prompt() can remain in its weak state, as the data 
//  is already where it belongs.
// The len variable must specify the number of valid bytes in the supplied prompt.
// crcb_get_coded_prompt() must not block as that would disable any notifications.
// Return cr_ErrorCodes_NO_ERROR if prompt data is supplied.
// Return cr_ErrorCodes_NO_DATA if no data is available.
// A message larger than CR_CODED_BUFFER_SIZE is fatal.
int __attribute__((weak)) crcb_get_coded_prompt(uint8_t *prompt, size_t *len)
{
    // The weak implementation assumes that cr_store_coded_prompt() was called 
    // and there is nothing to be done.
    (void)prompt;
    affirm(*len <= CR_CODED_BUFFER_SIZE);
    // I3_LOG(LOG_MASK_WEAK, "%s: weak default assumes cr_store_coded_prompt().\n", __FUNCTION__);
    if (*len != 0)
        return cr_ErrorCodes_NO_ERROR;
    return cr_ErrorCodes_NO_DATA;
}


// The cr_process function calls crcb_send_coded_response() to send responses to the client.
// Returns zero on success.
int __attribute__((weak)) crcb_send_coded_response(const uint8_t *response, size_t len)
{
  #if 1
    (void)response;
    (void)len;
    printf("%s: weak default.\n", __FUNCTION__);
    return 0;
  #else
    (void)response;
    (void)len;
    printf("%s: weak default, %d bytes.\n", __FUNCTION__, (int)len);

    for (size_t i = 0; i < len; i++)
    {
      printf("0x%02X, ", (unsigned char)response[i]);

      if ((i > 0) && (((i + 1) % 12) == 0))
      {
        printf("\n");
      }
    }
    printf("\n\n");

    return cr_ErrorCodes_NO_DATA;
  #endif
}


#define CR_PROTOCOL_VERSION    1

// Async error report
int __attribute__((weak)) crcb_notify_error(cr_ErrorReport *err)
{
    (void)err;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}



//*************************************************************************
//  Device Service
//*************************************************************************


// The stack will call this function.  The device must override the weak 
// implementation to provide a valid device info structure to the stack.
// response message members like hash and services are computed by the 
// reach stack.
int __attribute__((weak)) crcb_device_get_info(cr_DeviceInfoResponse *pDi)
{
    (void)pDi;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


//*************************************************************************
//  Link (ping) Service
//*************************************************************************
int __attribute__((weak)) crcb_ping_get_signal_strength(int8_t *rssi)
{
    (void)rssi;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

#ifdef INCLUDE_CLI_SERVICE
//*************************************************************************
//  CLI Service
//*************************************************************************

// When the CLI service is active, the stack can provide the device with 
// remotely entered CLI input by calling crcb_cli_enter().
int __attribute__((weak)) crcb_cli_enter(const char *cli)
{
    (void)cli;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// When the device supports a CLI it is expected to share anything printed 
// to the CLI back to the stack for remote display using crcb_cli_respond()
int __attribute__((weak)) crcb_cli_respond(char *cli)
{
    (void)cli;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// store the command line to be parsed elsewhere
const char *sSaveIns = NULL;
void __attribute__((weak)) crcb_set_command_line(const char *ins)
{
    sSaveIns = ins;
}

const char *__attribute__((weak)) crcb_get_command_line()
{
    return sSaveIns;
}
#endif  // def INCLUDE_CLI_SERVICE

//*************************************************************************
//  Parameter Service
//*************************************************************************

// Gets the parameter description for the next parameter.
// Allows the stack to iterate through the parameter list.
// Implies an order in the parameter list that is known by the application,
// but not directly by the stack.  
// Parameter ID's need not be continuous or in order.
// The double pointer allows the app to provide a pointer to flash memory.
// Returns zero if the description is valid.
int __attribute__((weak)) crcb_parameter_discover_next(cr_ParameterInfo **desc)
{
    (void)desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// returns the number of parameters supported by the system.
int __attribute__((weak)) crcb_parameter_get_count()
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}

// Resets the application's pointer into the parameter table such that 
// the next call to crcb_parameter_discover_next() will return the
// description of this parameter.
int __attribute__((weak)) crcb_parameter_discover_reset(const uint32_t pid)
{
    (void)pid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// In parallel to the parameter discovery, use this to find out 
// about enumerations and bitfields
int __attribute__((weak)) crcb_parameter_ex_discover_next(cr_ParamExInfoResponse *pDesc)
{
    (void)pDesc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// returns the number of extension parameters supported by the pid.
// a negative pid gets all.
int __attribute__((weak)) crcb_parameter_ex_get_count(const int32_t pid)
{
    (void)pid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}

// Resets the application's pointer into the parameter table such that 
// the next call to crcb_parameter_discover_next() will return the
// description of this parameter.
int __attribute__((weak)) crcb_parameter_ex_discover_reset(const int32_t pid)
{
    (void)pid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// These read and write functions allow the stack to remotely access the
// parameter repository of the device.  The parameter descripion of this pid
// specifying the size and type of the data is known both by the app and the 
// stack.
// The data is copied into a structure provided by the app.
// Might return cr_ErrorCodes_READ_FAILED or cr_ErrorCodes_PERMISSION_DENIED
int __attribute__((weak)) crcb_parameter_read(const uint32_t pid, cr_ParameterValue *data)
{
    (void)pid;
    (void)data;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// Might return cr_ErrorCodes_WRITE_FAILED
int __attribute__((weak)) crcb_parameter_write(const uint32_t pid, const cr_ParameterValue *data)
{
    (void)pid;
    (void)data;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// return a number that changes if the parameter descriptions have changed.
uint32_t __attribute__((weak)) crcb_compute_parameter_hash(void)
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}


// parameter notifications are handled by the Reach stack.
// The stack will use the read parameters to be notified on an appropriate
// timescale and send notifications if enough changes.
// We're considering using a feature of the parameter repository for notifications.
                                                       

//*************************************************************************
//  Command Service
//*************************************************************************

int __attribute__((weak)) crcb_file_get_command_count()
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}

int __attribute__((weak)) crcb_command_discover_next(cr_CommandInfo *cmd_desc)
{
    (void)cmd_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_command_discover_reset(const uint32_t pid)
{
    (void)pid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// actually execute the command
int __attribute__((weak)) crcb_command_execute(const uint8_t cid)
{
    (void)cid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

//*************************************************************************
//  File Service
//*************************************************************************
int __attribute__((weak)) crcb_file_get_file_count()
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}

int __attribute__((weak)) crcb_file_discover_next(cr_FileInfo *file_desc)
{
    (void)file_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_file_discover_reset(const uint8_t fid)
{
    (void)fid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_file_get_description(uint32_t fid, 
                                                  cr_FileInfo *file_desc)
{
    (void)fid;
    (void)file_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

// Could override the requested ack rate using this.
// 0 does not override.
int __attribute__((weak)) crcb_file_get_preferred_ack_rate(int is_write)
{
    (void)is_write;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return 0;
}


// Attempts to read the specified file.
// returns zero or an error code
int __attribute__((weak)) crcb_read_file(const uint32_t fid,            // which file
                 const int offset,              // offset, negative value specifies current location.
                 const size_t bytes_requested,  // how many bytes to read
                 uint8_t *pData,                // where the data goes
                 int *bytes_read)               // bytes actually read, negative for errors.
{
    (void)fid;
    (void)offset;
    (void)bytes_requested;
    (void)pData,
    (void)bytes_read;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// returns zero or an error code
int __attribute__((weak)) crcb_write_file(const uint32_t fid,   // which file
                 const int offset,      // offset, negative value specifies current location.
                 const size_t bytes,    // how many bytes to write
                 const uint8_t *pData)  // where to get the data from
{
    (void)fid;
    (void)offset;
    (void)bytes;
    (void)pData;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// returns zero or an error code
int __attribute__((weak)) crcb_erase_file(const uint32_t fid)
{
    (void)fid;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


//*************************************************************************
//  OTA Service
//*************************************************************************
int __attribute__((weak)) crcb_OTA_discover_next(cr_OTA_s *OTA_desc)
{
    (void)OTA_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_OTA_discover_reset(uint8_t OTA_id)
{
    (void)OTA_id;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}



//*************************************************************************
//  Log Service
//*************************************************************************

int __attribute__((weak)) crcb_log_discover_next(cr_log_s *log_desc)
{
    (void)log_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_log_discover_reset(uint8_t  log_id)
{
    (void)log_id;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


//*************************************************************************
//  Time Service
//*************************************************************************
// Time is specified in UTC Epoch format, seconds since 1970.
// More than 32 bits are required to remain valid past 2030.
int __attribute__((weak)) crcb_time_get(uint64_t *utc_seconds)
{
    (void)utc_seconds;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_time_set(uint64_t utc_seconds)
{
    (void)utc_seconds;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}



//*************************************************************************
//  Stream Service
//  Unlike the other services, the spec says that streams can go in 
// either direction.  
// Here I only consider streaming from the device.
//*************************************************************************

int __attribute__((weak)) crcb_stream_discover_next(cr_stream_s *stream_desc)
{
    (void)stream_desc;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

int __attribute__((weak)) crcb_stream_discover_reset(uint8_t  stream_id)
{
    (void)stream_id;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}


// A stream is sent as an array of records.
// The inclusion of message pack format records allows for arbitrarily
// complex records in the stream.
int __attribute__((weak)) crcb_stream_send_packet(const uint8_t stream_id,
                          void *data,
                          size_t num_packets)
{
    (void)stream_id;
    (void)data;
    (void)num_packets;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}



