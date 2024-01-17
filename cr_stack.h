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
 * \brief "cr_stack.h" defines the API to the Cygnus Reach device stack
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

#ifndef _CR_STACK_H
#define _CR_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// reach-server.h defines the build configuration of the Reach stack.
// This includes which services, and the basic size of structures appropriate for the 
// current transport means.  For example, BLE.
#include "reach-server.h"

// reach.pb.h is generated by nanopb based on the protobuf file reach.proto.
#include "reach.pb.h"
#include "reach_ble_proto_sizes.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Conventions:
//----------------------------------------------------------------------------
// Naming convention:  
//  cr_ prefix (for Cygnus Reach), then service name, then verb.
// crcb_ prefix for the "weak" callback functions required.
// 
// All API functions return zero on success or an identifiable error code.
//
// There are a small number of API functions that are to be called by
// the hosting application.  Most of the functions in the API are to be 
// provided by the application.  The reach stack provides "weak" declared
// versions so the application provide only what is required.
// The stack can be built to report errors if required weak functions are 
// not overridden.
//
// The configuration structures that must be passed in to the stack to 
// describe  the system are designed to be constant and stored in flash 
// if the application desires.  This allows for a minimum use of RAM.
//
// Lists of items such as parameters, command, files, etc are handled using
// an iterator pattern.  A function gets the next item and a reset function
// is available.
// In all cases, resetting to zero will select the lowest available object.
//

//----------------------------------------------------------------------------
// API
//----------------------------------------------------------------------------

// Error codes are defined in the proto file so that the client can see them.
    
// an arbitary minimum size for the buffers
// #define CR_MINIMUM_MESSAGE_BUFFER_SIZE  72

#define REACH_TIMEOUT   5000   // 5 seconds in ms
/*
//----------------------------------------------------------------------------
// static buffers used and reused by the reach stack.
//----------------------------------------------------------------------------

// Terminology
//   A transaction is a series of messages.
//   A message has a header and a payload.
//   The prompt is a received payload.
//   The response is a generated payload.
 
*/

#define ALIGN_TO_WORD   __attribute__((aligned(4)))

//----------------------------------------------------------------------------
// To be called once before cr_process is called.
int cr_init();

// The application should set the name of the device to be advertised.
// If the length exceeds the maximum, the max length will be returned.
int cr_set_advertised_name(char *name, int length);

// to be used by the transport code to get the name.
const char *cr_get_advertised_name();

// The application must call cr_process() regularly.  
// ticks tells it approximately how many  milliseconds have passed since 
// the system started.  This allows it to perform timing related tasks.  
// Most  
int cr_process(uint32_t ticks);


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
int crcb_get_coded_prompt(uint8_t *prompt, size_t *len);

// allows the app to store the coded prompt in the memory held by the stack.
int cr_store_coded_prompt(uint8_t *data, size_t len);


// The cr_process function calls crcb_send_coded_response() to send responses to the client.
// Returns zero on success.
int crcb_send_coded_response(const uint8_t *response, size_t len);
int cr_get_coded_response_buffer(uint8_t **pResponse, size_t *len);

// you can get more useful error reports if you provide ~128 bytes here.
void cr_provide_debug_buffer(char *buf, size_t len);

void cr_get_reply_buffer_and_size(char **ptr, size_t *sz);

// error handling is always present
void cr_report_error(int error_code, const char *fmt, ...);
int crcb_notify_error(cr_ErrorReport *err);

void cr_set_ble_connected(bool connected);
bool cr_get_ble_connected(void);

uint32_t cr_get_current_ticks();

void cr_test_sizes();



const char *cr_get_reach_version();

//*************************************************************************
//  Device Service (required)
//*************************************************************************

// The stack will call this function.  The device must override the weak 
// implementation to provide a valid device info structure to the stack.
// response message members like hash and services are computed by the 
// reach stack.
int crcb_device_get_info(cr_DeviceInfoResponse *pDi);


//*************************************************************************
//  Link (ping) Service (required)
//*************************************************************************
int crcb_ping_get_signal_strength(int8_t *rssi);

#ifdef INCLUDE_CLI_SERVICE
    //*************************************************************************
    //  CLI Service
    //*************************************************************************

    // When the CLI service is active, the stack can provide the device with 
    // remotely entered CLI input by calling crcb_cli_enter().
    int crcb_cli_enter(const char *cli);

    // When the device supports a CLI it is expected to share anything printed 
    // to the CLI back to the stack for remote display using crcb_cli_respond()
    int crcb_cli_respond(char *cli);

    // store the command line to be parsed elsewhere
    void crcb_set_command_line(const char *ins);
    const char *crcb_get_command_line();
#endif // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_PARAMETER_SERVICE
    //*************************************************************************
    //  Parameter Service
    //*************************************************************************

    #define MAX_NUM_PARAM_ID                32767 // 16 bits


    // Gets the parameter description for the next parameter.
    // Allows the stack to iterate through the parameter list.
    // Implies an order in the parameter list that is known by the application,
    // but not directly by the stack.  
    // Parameter ID's need not be continuous or in order.
    // The double pointer allows the app to provide a pointer to flash memory.
    // Returns zero if the description is valid.
    int crcb_parameter_discover_next(cr_ParameterInfo *desc);

    // returns the number of parameters supported by the system.
    int crcb_parameter_get_count();

    // Resets the application's pointer into the parameter table such that 
    // the next call to crcb_parameter_discover_next() will return the
    // description of this parameter.
    int crcb_parameter_discover_reset(const uint32_t pid);

    // In parallel to the parameter discovery, use this to find out 
    // about enumerations and bitfields
    // Only a single pointer since this gets the response directly.
    int crcb_parameter_ex_discover_next(cr_ParamExInfoResponse *pDesc);
    // negative pid specifes all pids.
    int crcb_parameter_ex_get_count(const int32_t pid);
    int crcb_parameter_ex_discover_reset(const int32_t pid);


    // These read and write functions allow the stack to remotely access the
    // parameter repository of the device.  The parameter descripion of this pid
    // specifying the size and type of the data is known both by the app and the 
    // stack.
    // The data is copied into a structure provided by the app.
    // Might return cr_ErrorCodes_READ_FAILED or cr_ErrorCodes_PERMISSION_DENIED
    int crcb_parameter_read(const uint32_t pid, cr_ParameterValue *data);

    // Might return cr_ErrorCodes_WRITE_FAILED
    int crcb_parameter_write(const uint32_t pid, const cr_ParameterValue *data); 

    // return a number that changes if the parameter descriptions have changed.
    uint32_t crcb_compute_parameter_hash(void);

    // parameter notifications are handled by the Reach stack.
    // The stack will use the read parameters to be notified on an appropriate
    // timescale and send notifications if enough changes.
    int crcb_notify_param(cr_ParameterValue *param);
#endif // def INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_COMMAND_SERVICE
    //*************************************************************************
    //  Command Service
    //*************************************************************************
    int crcb_get_command_count();
    int crcb_command_discover_next(cr_CommandInfo *cmd_desc);
    int crcb_command_discover_reset(const uint32_t pid);

    // actually execute the command
    int crcb_command_execute(const uint8_t cid);
#endif // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_FILE_SERVICE
    //*************************************************************************
    //  File Service
    //*************************************************************************
    int crcb_file_discover_next(cr_FileInfo *file_desc);
    int crcb_file_discover_reset(const uint8_t fid);
    int crcb_file_get_file_count();
    int crcb_file_get_description(uint32_t fid, cr_FileInfo *file_desc);
    int crcb_file_get_preferred_ack_rate(bool is_write);

    // Attempts to read the specified file.
    // returns zero or an error code
    int crcb_read_file(const uint32_t fid,            // which file
                     const int offset,              // offset, negative value specifies current location.
                     const size_t bytes_requested,  // how many bytes to read
                     uint8_t *pData,                // where the data goes
                     int *bytes_read);              // bytes actually read, negative for errors.

    // returns zero or an error code
    int crcb_write_file(const uint32_t fid,   // which file
                     const int offset,      // offset, negative value specifies current location.
                     const size_t bytes,    // how many bytes to write
                     const uint8_t *pData); // where to get the data from

    // returns zero or an error code
    int crcb_erase_file(const uint32_t fid);
#endif // def INCLUDE_FILE_SERVICE

#ifdef INCLUDE_OTA_SERVICE
    //*************************************************************************
    //  OTA Service
    //*************************************************************************
    typedef struct
    {
        uint8_t     OTA_id;             // ID of this OTA object
        uint8_t     OTA_file_id;        // Which file stores the OTA data
        uint8_t     OTA_command_id;     // Which command triggers the OTA sequence
    } cr_OTA_s;

    int crcb_OTA_discover_next(cr_OTA_s *OTA_desc);
    int crcb_OTA_discover_reset(uint8_t OTA_id);
#endif // def INCLUDE_OTA_SERVICE

#ifdef INCLUDE_TIME_SERVICE
    //*************************************************************************
    //  Time Service
    //*************************************************************************
    // Time is specified in UTC Epoch format, seconds since 1970.
    // More than 32 bits are required to remain valid past 2030.
    int crcb_time_get(uint64_t *utc_seconds);
    int crcb_time_set(uint64_t utc_seconds);
#endif // def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_STREAM_SERVICE
    //*************************************************************************
    //  Stream Service
    //  Unlike the other services, the spec says that streams can go in 
    // either direction.  
    // Here I only consider streaming from the device.
    //*************************************************************************

    typedef enum {
        str_number  = 0,  // Array of numbers of the same type
        str_record  = 1,  // Array of objects, may include a timestamp
        str_binary  = 2,  // binary data
        str_string  = 3,  // string data
        str_reserved      // for expansion
    } stream_type_e;

    typedef struct
    {
        uint8_t         stream_id;          // ID of this stream object
        uint8_t         access_permissions; // read, write or both
        char           *name;               // UTF-8 
        stream_type_e  format;             // Format of this stream
        char           *description;
    } cr_stream_s;

    int crcb_stream_discover_next(cr_stream_s *stream_desc);
    int crcb_stream_discover_reset(uint8_t  stream_id);

    // A stream is sent as an array of records.
    int crcb_stream_send_packet(const uint8_t stream_id,
                              void *data,
                              size_t num_packets);
#endif // def INCLUDE_STREAM_SERVICE

#ifdef __cplusplus
}
#endif


#endif  // ndef _CR_STACK_H

