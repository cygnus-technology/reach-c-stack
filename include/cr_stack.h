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

/**
 * @file      cr_stack.h
 * @brief     This file exposes the core of the public Reach stack API. It 
 *            depends on reach-server.h which defines what parts of the stack
 *            are being used by an applicaiton.  crcb_weak.h is incluced. That
 *            is the part of the API that must be implemented by a server
 *            application.
 * @author    Chuck Peplinski
 * @date      2024-01-24
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
 */

#ifndef _CR_STACK_H
#define _CR_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// reach-server.h defines the build configuration of the Reach stack.
// This includes which services, and the basic size of structures appropriate for the 
// current transport means.  For example, BLE.
#include "reach-server.h"

#include "crcb_weak.h"
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

#define MAX_NUM_PARAM_ID                32767 // 16 bits

//----------------------------------------------------------------------------
// API
//----------------------------------------------------------------------------

// Error codes are defined in the proto file so that the client can see them.
    
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

/**
* @brief   cr_init
* @details To be called before starting the stack.
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error like cr_ErrorCodes_. 
*/
int cr_init();

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
int cr_set_advertised_name(char *name, int length);

/**
* @brief   cr_get_advertised_name
* @details Retrieves the name stored by cr_set_advertised_name().
* @return  pointer to a string of length REACH_SHORT_STRING_LEN.
*/
const char *cr_get_advertised_name();

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
int cr_process(uint32_t ticks);

/**
* @brief   cr_store_coded_prompt
* @details Allows the application to store the prompt where the 
*          Reach stack can see it.  The byte data and length are
*          copied into private storage. This data is retrieved
*          using crcb_get_coded_prompt().
* @param   data: The coded prompt to be stored. 
* @param   len : number of bytes to be stored. 
* @return  cr_ErrorCodes_NO_ERROR or a non-zero error code.
*/
int cr_store_coded_prompt(uint8_t *data, size_t len);


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
int cr_get_coded_response_buffer(uint8_t **pResponse, size_t *len);

/**
* @brief   cr_report_error
* @details Report an error condition to the client.  This can be called at any 
*          point as the report to the client is asynchronous and immediate.  The
*          stack can be configured to use only the error code, but the
*          printf-like string describing the error condition is encouraged.
*          This is intended to make it easier to find and eliminate errors
*          during development.
* @param error_code : Use of the cr_ErrorCodes_ enum is encouraged but not 
*                   required.
* @param fmt : A printf-like string with variables. 
*/
void cr_report_error(int error_code, const char *fmt, ...);


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
void cr_set_comm_link_connected(bool connected);


/**
* @brief   cr_get_comm_link_connected
* @details Returns what was set using cr_set_comm_link_connected().
* @return  true if the communication link is connected.
*/
bool cr_get_comm_link_connected(void);


/**
* @brief   cr_clear_param_notifications
* @details Disable all parameter notifications.
*/
void cr_clear_param_notifications(void);

/**
* @brief   cr_init_param_notifications
* @details Enable all locally specified parameter notifications 
*          via crcb_parameter_notification_init().
*/
void  cr_init_param_notifications(void);

/**
* @brief   cr_get_current_ticks
* @details The tick count is passed in to cr_process(). This function gives 
*          other Reach functions access to that value. 
* @return  The same tick count passed into cr_process().
*/
uint32_t cr_get_current_ticks();

/// <summary>
/// Verify that buffer structures fit into limited size memory
/// </summary>
void cr_test_sizes();


/** The reach_sizes_t is used to communicate the sizes of device structures to
 *  clients.  These sizes can vary from one server to another and the client
 *   is required to respect these sizes.  Many of them are determined by the
 *   chosen communication link, such as BLE. reach_sizes_t uses
 *   smaller member sizes that the uint32 favored by protobufs.
 *   Hence it communicates these sizes without blowing up the
 *   size of the device info packet.
 *  Refer also to enum SizesOffsets reach.proto and
 *  sCr_populate_device_info_sizes() which populates the
 *  structure.
 *   */
typedef struct {
    uint16_t  max_message_size;             /**< The largest message that can be communicated */
    uint16_t  big_data_buffer_size;         /**< The size of the buffer most of the packet is one string. */
    /// The number of parameter buffers kept by the device.
    /// This determines the number of parameters that can be
    /// handled in a single message.
    uint8_t   parameter_buffer_count;
    /// The number of parameter values that fit in one message. 
    uint8_t   num_params_in_response;
    /// The length of the device and command description fields. 
    uint8_t   description_len;
    /// The number of bytes in the largest parameter types
    /// eg, strings and byte array.
    uint8_t   max_param_bytes;
    /// Length of the text string for parameter info description. 
    uint8_t   param_info_description_len;
    /// The length of longer strings.
    uint8_t   medium_string_len;
    /// The number of bytes in short strings like the units label.
    uint8_t   short_string_len;
    /// The max number of enumeration descriptions that a server 
    /// will provide in a parameter extended description. 
    uint8_t   param_info_enum_count;
    /// number of descriptors (stream, file) that fit in one message. 
    uint8_t   num_descriptors_in_response;
    /// Number of parameter notifications supported
    uint8_t   num_param_notifications;
    /// number of commands that can be in one info packet
    uint8_t   num_commands_in_response;
    /// number of parameter descriptions that 
    ///  can be in one info packet.
    uint8_t   num_param_desc_in_response;
} reach_sizes_t;

// The size of this structure (REACH_SIZE_STRUCT_SIZE) must be defined in 
// reach_ble_proto_sizes.h, or the analogous file used to set structure sizes.
// #define REACH_SIZE_STRUCT_SIZE      16

/**
* @brief   cr_get_reach_version 
* @details The version is in semantic version format: 
*          MAJOR.MINOR.PATCH
*          with an optional string appended. 
* @return  Returns a pointer to a null terminated string 
*          containing the C stack version.
*/
const char *cr_get_reach_version();
#define CR_STACK_VERSION_LEN    16

/**
* @brief   cr_get_proto_version 
* @details The version is in semantic version format: 
*          MAJOR.MINOR.PATCH
*          with an optional string appended. 
* @return  Returns a pointer to a null terminated string 
*          containing the C protobuf version.
*/
const char *cr_get_proto_version();

#ifdef __cplusplus
}
#endif


#endif  // ndef _CR_STACK_H

