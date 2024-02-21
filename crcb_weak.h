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
 *                          Copyright i3 Product Development 2023-2024
 *
 * \brief "crcb_weak.h" defines the weak implementations of the callback functions the user of
 * the Cygnus Reach device stack should override.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      crcb_weak.h
 * @brief     The Reach stack relies on these callback functions being 
 *            implemented by the device. The gcc supported "weak" keyword allows
 *            us to build without having real implementations.
 *            The code supporting the various services are
 *            included or excluded via configuration in
 *            reach-server.h.  The corresponding default
 *            implementations of these "weak" functions are
 *            found in cr_weak.c.
 * @author    Chuck Peplinski
 * @date      2024-02-21
 * @copyright (c) Copyright 2023-2024 i3 Product Development. 
 * All Rights Reserved. The Cygngus Reach firmware stack is 
 * shared under an MIT license. 
 */


#include <stdint.h>
#include <stdbool.h>

#include "cr_stack.h"
#include "i3_log.h"

 
/**
* @brief   crcb_get_coded_prompt
* @details The cr_process function calls this function to get any available prompt in coded 
*          format. An overriding implementation is responsible to copy the data into the
*          provided buffer and set the size. Alternatively, cr_store_coded_prompt() can be used
*          to push data into the stack.  Then this weak implementation can remain, as the data
*          is already where it belongs.
*          crcb_get_coded_prompt() must not block as that would disable any notifications.
* @param   prompt    Pointer to raw data (output)
* @param   len    Pointer to the number of bytes in the supplied prompt (output)
* @return  cr_ErrorCodes_NO_ERROR on success.  cr_ErrorCodes_NO_DATA if no data is available.
*/
int crcb_get_coded_prompt(uint8_t *prompt, size_t *len);

/**
* @brief   crcb_send_coded_response
* @details The cr_process function calls this function to send responses to the client. 
*          Must be overridden to send the data to the client.
* @param   response    Pointer to coded data to be send (input)
* @param   len    Number of bytes to be sent (input)
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
*          cr_ErrorCodes_ enumeration
*/
int crcb_send_coded_response(const uint8_t *response, size_t len);

/**
* @brief   crcb_notify_error
* @details Called by cr_report_error().  Can be called at any point.
*          The device must override this implementation to send 
*          error messages to the client.
* @param   err Pointer to a structure with a code and a string.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
*          the cr_ErrorCodes_ enumeration
*/
int crcb_notify_error(cr_ErrorReport *err);


///*************************************************************************
///  Device Service
///*************************************************************************

/**
* @brief   crcb_device_get_info
* @details Called by the stack in response to a device info request.
*          The device must override the weak implementation to provide a valid
*         device info structure to the stack.  Response message members like
*         hash and services are computed by the reach stack.
* @param   pDi A pointer to memory provided by the stack to be populated with 
*             the basic device information.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
*          the cr_ErrorCodes_ enumeration
*/
int crcb_device_get_info(cr_DeviceInfoResponse *pDi);


///*************************************************************************
///  Link (ping) Service 
///*************************************************************************

/**
* @brief   crcb_ping_get_signal_strength
* @details Called by the stack in response to a ping request.
*          The device can override the weak implementation to provide an  
*         estimate of the signal strength.
* @param   rssi A pointer to be populated with a signal strength typically 
*              negative in dB.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably 
*         from the cr_ErrorCodes_ enumeration
*/
int crcb_ping_get_signal_strength(int8_t *rssi);

#ifdef DOXYGEN_PARSE
  /// #define in reach-server.h to include the command line
  /// interface service.
  #define INCLUDE_CLI_SERVICE
  /// #define in reach-server.h to include the parameter 
  /// repository service.
  #define INCLUDE_PARAMETER_SERVICE
  /// #define in reach-server.h to specify the number of parameter
  /// notifications supported.
  #define NUM_SUPPORTED_PARAM_NOTIFY 8
  /// #define in reach-server.h to include the command service.
  #define INCLUDE_COMMAND_SERVICE
  /// #define in reach-server.h to include the file service.
  #define INCLUDE_FILE_SERVICE
  /// #define in reach-server.h to include the time service.
  #define INCLUDE_TIME_SERVICE
#endif // DOXYGEN_PARSE 

#ifdef INCLUDE_CLI_SERVICE
/** \addtogroup cli_group
 *  These functions support the command line service. @{
 */

    /**
    * @brief   crcb_cli_enter
    * @details When the CLI service is active, the stack can provide the device with 
    *          remotely entered CLI input by calling crcb_cli_enter().
    *          The device must override the weak implementation to support remote 
     *         access to the command line.
    * @param   cli A string being commanded to the CLI.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
    *          cr_ErrorCodes_ enumeration
    */
    int crcb_cli_enter(const char *cli);

    /**
    * @brief   crcb_cli_respond
    * @details When the device supports a CLI it is expected to share anything 
    *          printed to the CLI back to the stack for remote display using
    *          crcb_cli_respond()
    *          The device must override the weak implementation to support remote 
    *          access to the command line.  The implementation can call this at any
    *          time to print to the remote CLI
    * @param   cli A string being sent back to the remote CLI.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
    *          cr_ErrorCodes_ enumeration.
    */
    int crcb_cli_respond(char *cli);

    /**
    * @brief   crcb_set_command_line
    * @details store the command line to be parsed elsewhere.
    *          When there are both the remote and local command lines, this set and 
    *          get pair allows one set of functions to handle
    *          commands from either source.
    * @param   ins A string pointer to be retrieved by crcb_get_command_line 
    * @return  none
    */
    void crcb_set_command_line(const char *ins);

    /**
    * @brief   crcb_get_command_line
    * @details Retrieve the command line stored with crcb_set_command_line().
    *          When there are both the remote and local command lines, this set and 
    *          get pair allow one set of functions to handle commands from either
    *          source.
    * @return  A string pointer stored by crcb_set_command_line().
    */
    const char *crcb_get_command_line();
/** @}  */

#endif  // def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_PARAMETER_SERVICE

    /**
    * @brief   crcb_parameter_get_count
    * @details returns the number of parameters exposed by this device.
    * @return  Total number of parameter descriptions
    */
    int crcb_parameter_get_count();

    /**
    * @brief   crcb_parameter_discover_reset
    * @details The overriding implementation must reset a pointer into the parameter 
    *          table such that the next call to crcb_parameter_discover_next() will
    *          return the description of this parameter.
    * @param   pid The parameter ID to which the parameter table pointer should be 
    *              reset. Use 0 for the first entry.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int crcb_parameter_discover_reset(const uint32_t pid);

    /**
    * @brief   crcb_parameter_discover_next
    * @details Gets the parameter description for the next parameter. Allows the 
    *              stack to iterate through the parameter list.  Implies an order in
    *              the parameter list that is known by the application, but not
    *              directly by the stack.
    *          The overriding implementation must post-increment its pointer into 
    *          the parameter table.  Parameter ID's need not be continuous or in
    *          order.
    * @param   pDesc Stack provided memory into which the description must be 
    *                copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last parameter has already been returned.
    */
    int crcb_parameter_discover_next(cr_ParameterInfo *pDesc);

    /**
    * @brief   crcb_parameter_ex_get_count
    * @details returns the number of parameter extension exposed by this device.
    * @return  Total number of parameter extensions
    */
    int crcb_parameter_ex_get_count(const int32_t pid);

    /**
    * @brief   crcb_parameter_ex_discover_reset
    * @details The overriding implementation must reset a pointer into the parameter
    *          extension table such that the next call to
    *          crcb_parameter_ex_discover_next() will return the description of this
    *          parameter extension. 
    * @param   pid The parameter ID to which the parameter extension table pointer 
    *              should be reset. Use 0 for the first entry.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int crcb_parameter_ex_discover_reset(const int32_t pid);

    /**
    * @brief   crcb_parameter_ex_discover_next
    * @details Gets the parameter extension for the next parameter. Parameter 
    *          extensions allow more data to be provided about enumerations and
    *          bitfields.
    *          The overriding implementation must post-increment its pointer into 
    *          the parameter extension table.
    * @param   pDesc Pointer to stack provided memory into which the extension 
    *               is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last parameter extension has already been returned.
    */
    int crcb_parameter_ex_discover_next(cr_ParamExInfoResponse *pDesc);

    /**
    * @brief   crcb_parameter_read
    * @details The overriding implementation allows the stack to access the
    *          parameter repository of the device.  The parameter descripion of this
    *          pid specifying the size and type of the data is known both by the app
    *          and the stack. 
    * @param   pid (input) parameter ID
    * @param   data Pointer to stack provided memory into which the parameter data 
    *               must be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error like  
    *          cr_ErrorCodes_INVALID_PARAMETER if the parameter ID is not valid.
    *          Also can return cr_ErrorCodes_READ_FAILED or
    *          cr_ErrorCodes_PERMISSION_DENIED
    */
    int crcb_parameter_read(const uint32_t pid, cr_ParameterValue *data);

    /**
    * @brief   crcb_parameter_write
    * @details The overriding implementation allows the stack to access the
    *          parameter repository of the device.  The parameter descripion of this
    *          pid specifying the size and type of the data is known both by the app
    *          and the stack. 
    * @param   pid (input) parameter ID
    * @param   data Pointer to stack provided memory containing data to be written 
    *               into the devices parameter repository.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error like  
    *          cr_ErrorCodes_INVALID_PARAMETER if the parameter ID is not valid.
    *          Also can return cr_ErrorCodes_WRITE_FAILED or
    *          cr_ErrorCodes_PERMISSION_DENIED
    */
    int crcb_parameter_write(const uint32_t pid, const cr_ParameterValue *data);

    /**
    * @brief   crcb_compute_parameter_hash
    * @details The overriding implementation is to compute a number that will change
    *          if the table of parmeter descriptions is changed.  This allows the
    *          client to cache a large table of parameter descriptions. 
    *               into the devices parameter repository.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration.
    */
    uint32_t crcb_compute_parameter_hash(void);

  #if NUM_SUPPORTED_PARAM_NOTIFY >= 0
    /**
    * @brief   crcb_notify_param
    * @details parameter notifications are handled by the Reach stack. The stack 
    * will use the read parameters to be notified on an appropriate timescale and 
    *          send notifications if enough changes. The overriding implementation
    *          must signal the client that this parameter may have changed.
    * @param   param (input) pointer to the parameter data that has 
    *                changed.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration if the notification fails.
    */
    int crcb_notify_param(cr_ParameterValue *param);

  #endif /// NUM_SUPPORTED_PARAM_NOTIFY >= 0

#endif /// INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_COMMAND_SERVICE

    /**
    * @brief   crcb_get_command_count
    * @details The overriding implementation must returns the number of commands 
    *          implemented by the device.
    * @return  the number of commands implemented by the device.
    */    
    int crcb_get_command_count();

    /**
    * @brief   crcb_command_discover_reset
    * @details The overriding implementation must reset a pointer into the command 
    *          table such that the next call to crcb_command_discover_next() will
    *          return the description of this command.
    * @param   cid The ID to which the command table pointer 
    *              should be reset.  0 for the first command.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int crcb_command_discover_reset(const uint32_t cid);

    /**
    * @brief   crcb_command_discover_next
    * @details Gets the command description for the next command. 
    *          The overriding implementation must post-increment its pointer into 
    *          the command table.
    * @param   cmd_desc Pointer to stack provided memory into which the 
    *               command description is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last command has already been returned.
    */
    int crcb_command_discover_next(cr_CommandInfo *cmd_desc);

    /**
    * @brief   crcb_command_execute
    * @details Execute the command associated with this command ID.
    * @param   cid command ID.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration if the command fails.
    */
    int crcb_command_execute(const uint8_t cid);

#endif  // def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_FILE_SERVICE

    /**
    * @brief   crcb_file_get_file_count
    * @details The overriding implementation must returns the number of files 
    *          implemented by the device.
    * @return  the number of files implemented by the device.
    */    
    int crcb_file_get_file_count();

    /**
    * @brief   crcb_file_discover_reset
    * @details The overriding implementation must reset a pointer into the file 
    *          table such that the next call to crcb_file_discover_next() will
    *          return the description of this file.
    * @param   fid The ID to which the file table pointer 
    *              should be reset.  use 0 for the first command.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int crcb_file_discover_reset(const uint8_t fid);

    /**
    * @brief   crcb_file_discover_next
    * @details Gets the  description for the next file.
    *          The overriding implementation must post-increment its pointer into 
    *          the file table.
    * @param   file_desc Pointer to stack provided memory into which the 
    *               file description is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last file has already been returned.
    */
    int crcb_file_discover_next(cr_FileInfo *file_desc);

    /**
    * @brief   crcb_file_get_description
    * @details Get the description matching the file ID.
    * @param   fid The ID of the desired file.
    * @param   file_desc Pointer to stack provided memory into which the 
    *               file description is to be copied
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int crcb_file_get_description(uint32_t fid, cr_FileInfo *file_desc);

    /**
    * @brief   crcb_file_get_preferred_ack_rate
    * @details If the device has a preferred acknowledge rate it can implement this 
    *          function to advise the file transfer code of the rate.
    *          Higher ack rates mean less acknowlegements and faster file trasnfer.
    * @param   is_write true if enquiring about write.
    * @return  A return value of zero means that there is no preferred rate and the 
    *          client can specify it.
    */
    int crcb_file_get_preferred_ack_rate(bool is_write);

    /**
    * @brief   crcb_read_file
    * @details The device overrides this method to provide data read from the 
    *          specified file.
    * @param   fid (input) which file
    * @param   offset (input) offset, negative value specifies current location.
    * @param   bytes_requested (input) how many bytes to read
    * @param   pData where the (output) data goes
    * @param   pBytes_read (output) bytes actually read, negative for errors.
    * @return  returns zero or an error code
    */
    int 
    crcb_read_file(const uint32_t fid,            // which file
                   const int offset,              // offset, negative value specifies current location.
                   const size_t bytes_requested,  // how many bytes to read
                   uint8_t *pData,                // where the data goes
                   int *pBytes_read);             // bytes actually read, negative for errors.

    /**
    * @brief   crcb_write_file
    * @details The device overrides this method to accept data for the specified 
    *          file.
    * @param   fid (input) which file
    * @param   offset (input) offset, negative value specifies current location.
    * @param   bytes (input) how many bytes to write
    * @param   pData data from the stack. 
    * @return  returns zero or an error code
    */
    int 
    crcb_write_file(const uint32_t fid,    // which file
                    const int offset,      // offset, negative value specifies current location.
                    const size_t bytes,    // how many bytes to write
                    const uint8_t *pData); // where to get the data from

    /**
    * @brief   crcb_erase_file
    * @details The device overrides this method to accept a command to set the 
    *          length of a file to zero, erasing it.
    * @param   fid (input) which file
    * @param   offset (input) offset, negative value specifies current location.
    * @return  returns zero or an error code
    */
    int crcb_erase_file(const uint32_t fid);

    /**
    * @brief   crcb_file_transfer_complete
    * @details Called when the last bytes have been received and a file write is 
    *          complete.
    * @param   fid (input) which file
    * @return  returns zero or an error code
    */
    int crcb_file_transfer_complete(const uint32_t fid);

    /**
    * @brief   crcb_file_prepare_to_write
    * @details Called when a file write has been requested.  Designed to give the 
    *          app the chance to erase flash memory and setup before receiving the
    *          first bytes.
    * @param   fid (input) which file 
    * @param   offset start address of write 
    * @param   bytes_to_write 
    * @return  returns zero or an error code.  The stack reacts to an error code.
    */
    int crcb_file_prepare_to_write(const uint32_t fid,
                                   const size_t offset,
                                   const size_t bytes_to_write);

#endif // def INCLUDE_FILE_SERVICE

#ifdef INCLUDE_TIME_SERVICE
    /**
    * @brief   crcb_time_get
    * @details Retrieve the device's idea of the current time.
    *          Time is specified in UTC Epoch format, seconds since 1970. More than 
    *          32 bits are required to remain valid past 2030.
    * @param   response (output) with utc_seconds current time and zone 
    * @return  returns zero or an error code
    */
    int crcb_time_get(cr_TimeGetResponse *response);

    /**
    * @brief   crcb_time_set
    * @details Inform the device of the current time to support setting an internal 
    *          time clock.
    *          Time is specified in UTC Epoch format, seconds since 1970. More than 
    *          32 bits are required to remain valid past 2030.
    * @param   request (input) structure with utc_seconds current time and zone
    * @return  returns zero or an error code
    */
    int crcb_time_set(const cr_TimeSetRequest *request);

#endif  // def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_OTA_SERVICE
    ///*************************************************************************
    ///  OTA service not yet supported
    ///*************************************************************************
    int crcb_OTA_discover_next(uint8_t *pOTA_id;             // ID of this OTA object
                               uint8_t *pOTA_file_id;        // Which file stores the OTA data
                               uint8_t *pOTA_command_id);    // Which command triggers the OTA sequence

    int crcb_OTA_discover_reset(uint8_t OTA_id);
#endif // def INCLUDE_OTA_SERVICE

#ifdef INCLUDE_STREAM_SERVICE
    ///*************************************************************************
    ///  Stream Service not yet supported
    ///*************************************************************************

    typedef enum {
        str_number  = 0,  // Array of numbers of the same type
        str_record  = 1,  // Array of objects, may include a timestamp
        str_binary  = 2,  // binary data
        str_string  = 3,  // string data
        str_reserved      // for expansion
    } stream_type_e;

    int crcb_stream_discover_next(cr_stream_s *stream_desc);

    int crcb_stream_discover_reset(uint8_t  stream_id);

    /// A stream is sent as an array of records.
    int crcb_stream_send_packet(const uint8_t stream_id,
                                void *data,
                                size_t num_packets);
#endif /// INCLUDE_STREAM_SERVICE


