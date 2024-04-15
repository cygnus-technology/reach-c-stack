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

/**
 * @file      cr_weak.c
 * @brief     The Reach stack relies on these callback functions being 
 *            implemented by the device. The gcc supported "weak" keyword allows
 *            us to build without having real implementations..
 * @author    Chuck Peplinski
 * @date      2024-01-17
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
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
* @note    crcb_get_coded_prompt() must not block as that would disable any notifications.
* @param   prompt    Pointer to raw data (output)
* @param   len    Pointer to the number of bytes in the supplied prompt (output)
* @return  cr_ErrorCodes_NO_ERROR on success.  cr_ErrorCodes_NO_DATA if no data is available.
*/
int __attribute__((weak)) crcb_get_coded_prompt(uint8_t *prompt, size_t *len)
{
    (void)prompt;
    affirm(*len <= CR_CODED_BUFFER_SIZE);
    if (*len != 0)
        return cr_ErrorCodes_NO_ERROR;
    return cr_ErrorCodes_NO_DATA;
}

/**
* @brief   crcb_send_coded_response
* @details The cr_process function calls this function to send 
*          responses to the client. A complete implementation
*          must overridde this to send the data to the client.
* @param   response    Pointer to coded data to be send (input)
* @param   len    Number of bytes to be sent (input)
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
*          cr_ErrorCodes_ enumeration
*/
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

/**
* @brief   crcb_notify_error
* @details Called by cr_report_error().  Can be called at any 
*          point. A complete implementation
*          must overridde this implementation to send 
*          error messages to the client.
* @param   err Pointer to a structure with a code and a string.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
*          the cr_ErrorCodes_ enumeration
*/
int __attribute__((weak)) crcb_notify_error(cr_ErrorReport *err)
{
    (void)err;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}



///*************************************************************************
///  Device Service
///*************************************************************************

/**
* @brief   crcb_device_get_info
* @details Called by the stack in response to a device info 
*         request. The device must override the weak
*         implementation to provide a valid device info
*         structure to the stack.  Response message members like
*         hash and services are computed by the reach stack.
* @param   request Includes challenge key for access control.
* @param   pDi A pointer to memory provided by the stack to be populated with 
*             the basic device information.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from
*          the cr_ErrorCodes_ enumeration
*/
int __attribute__((weak)) crcb_device_get_info(const cr_DeviceInfoRequest *request,
                                               cr_DeviceInfoResponse *pDi)
{
    (void)request;
    (void)pDi;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

/**
* @brief   crcb_challenge_key_is_valid
* @details Called by the stack in various places to check 
*          whether access is granted by the challenge key.
*           The access grant here is binary.  Finer control
*           based on the specific key is determined by the
*           overriding application specific implementation.
* @return  true if access is granted.
*/
bool __attribute__((weak)) crcb_challenge_key_is_valid(void)
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return true;
}

/**
* @brief   crcb_invalidate_challenge_key
* @details Called by the stack on disconnect or other condition 
*          that warrants invalidating access.
*/
void __attribute__((weak)) crcb_invalidate_challenge_key(void)
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
}

/**
* @brief   crcb_enable_remote_cli
* @details As the logging utility is technically part of Reach, 
*          this callback lets the app block the remote CLI.
* @return  true if access is granted.
*/
bool __attribute__((weak)) crcb_enable_remote_cli(void)
{
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return true;
}



///*************************************************************************
///  Link (ping) Service 
///*************************************************************************

/**
* @brief   crcb_ping_get_signal_strength
* @details Called by the stack in response to a ping request. 
*         The device can override the weak implementation to
*         provide an estimate of the signal strength.
* @param   rssi A pointer to be populated with a signal strength typically 
*              negative in dB.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably 
*         from the cr_ErrorCodes_ enumeration
*/
int __attribute__((weak)) crcb_ping_get_signal_strength(int8_t *rssi)
{
    (void)rssi;
    I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

#ifdef INCLUDE_CLI_SERVICE
    /**
    * @brief   crcb_cli_enter
    * @details When the CLI service is active, the stack can provide the device with 
     *         remotely entered CLI input by calling
     *         crcb_cli_enter(). The device must override the weak
     *         implementation to support remote access to the
     *         command line.
    * @param   cli A string being commanded to the CLI.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
    *          cr_ErrorCodes_ enumeration
    */
    int __attribute__((weak)) crcb_cli_enter(const char *cli)
    {
        (void)cli;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_cli_respond
    * @details When the device supports a CLI it is expected to share anything 
    *          printed to the CLI back to the stack for remote
    *          display using crcb_cli_respond().  The device must
    *          override the weak implementation to support remote
    *          access to the command line.  The implementation can
    *          call this at any time to print to the remote CLI
    * @param   cli A string being sent back to the remote CLI.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error preferably from the 
    *          cr_ErrorCodes_ enumeration.
    */
    int __attribute__((weak)) crcb_cli_respond(char *cli)
    {
        (void)cli;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_set_command_line
    * @details store the command line to be parsed elsewhere.  When 
    *          there are both the remote and local command lines,
    *          this set and get pair allow one set of functions to
    *          handle commands from either source.
    * @param   ins A string pointer to be retrieved by crcb_get_command_line 
    * @return  none
    */
    const char *sSaveIns = NULL;
    void __attribute__((weak)) crcb_set_command_line(const char *ins)
    {
        sSaveIns = ins;
    }

    /**
    * @brief   crcb_get_command_line
    * @details Retrieve the command line stored with 
    *          crcb_set_command_line(). When there are both the
    *          remote and local command lines, this set and get pair
    *          allow one set of functions to handle commands from
    *          either source.
    * @return  A string pointer stored by crcb_set_command_line().
    */
    const char *__attribute__((weak)) crcb_get_command_line()
    {
        return sSaveIns;
    }
#endif  /// def INCLUDE_CLI_SERVICE

#ifdef INCLUDE_PARAMETER_SERVICE

    /**
    * @brief   crcb_parameter_get_count
    * @details returns the number of parameters exposed by this device.
    * @return  Total number of parameter descriptions
    */
    int __attribute__((weak)) crcb_parameter_get_count()
    {
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) crcb_parameter_discover_reset(const uint32_t pid)
    {
        (void)pid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }


    /**
    * @brief   crcb_parameter_discover_next
    * @details Gets the parameter description for the next parameter. Allows the 
    *              stack to iterate through the parameter list.  Implies an order in
    *              the parameter list that is known by the application, but not
    *              directly by the stack. The overriding
    *              implementation must post-increment its pointer
    *              into the parameter table.  Parameter ID's need
    *              not be continuous or in order.
    * @param   pDesc Stack provided memory into which the description must be 
    *                copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last parameter has already been returned.
    */
    int __attribute__((weak)) crcb_parameter_discover_next(cr_ParameterInfo *pDesc)
    {
        (void)pDesc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_parameter_ex_get_count
    * @details returns the number of parameter extension exposed by this device.
    * @return  Total number of parameter extensions
    */
    int __attribute__((weak)) crcb_parameter_ex_get_count(const int32_t pid)
    {
        (void)pid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) crcb_parameter_ex_discover_reset(const int32_t pid)
    {
        (void)pid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_parameter_ex_discover_next
    * @details Gets the parameter extension for the next parameter. Parameter 
    *          extensions allow more data to be provided about
    *          enumerations and bitfields. The overriding
    *          implementation must post-increment its pointer into
    *          the parameter extension table.
    * @param   pDesc Pointer to stack provided memory into which the extension 
    *               is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last parameter extension has already been returned.
    */
    int __attribute__((weak)) crcb_parameter_ex_discover_next(cr_ParamExInfoResponse *pDesc)
    {
        (void)pDesc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }


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
    int __attribute__((weak)) crcb_parameter_read(const uint32_t pid, cr_ParameterValue *data)
    {
        (void)pid;
        (void)data;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

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
    int __attribute__((weak)) crcb_parameter_write(const uint32_t pid, const cr_ParameterValue *data)
    {
        (void)pid;
        (void)data;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_compute_parameter_hash
    * @details The overriding implementation is to compute a number that will change
    *          if the table of parmeter descriptions is changed.  This allows the
    *          client to cache a large table of parameter descriptions. 
    *               into the devices parameter repository.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration.
    */
    uint32_t __attribute__((weak)) crcb_compute_parameter_hash(void)
    {
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) crcb_notify_param(cr_ParameterValue *param)
    {
        (void)param;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }
  #endif /// NUM_SUPPORTED_PARAM_NOTIFY >= 0
#endif /// INCLUDE_PARAMETER_SERVICE

#ifdef INCLUDE_COMMAND_SERVICE

    /**
    * @brief   crcb_get_command_count
    * @details The overriding implementation must returns the number of commands 
    *          implemented by the device.
    * @return  the number of commands implemented by the device.
    */    
    int __attribute__((weak)) crcb_get_command_count()
    {
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) crcb_command_discover_reset(const uint32_t cid)
    {
        (void)cid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_command_discover_next
    * @details Gets the command description for the next command. 
    *          The overriding implementation must post-increment its
    *          pointer into the command table.
    * @param   cmd_desc Pointer to stack provided memory into which the 
    *               command description is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last command has already been returned.
    */
    int __attribute__((weak)) crcb_command_discover_next(cr_CommandInfo *cmd_desc)
    {
        (void)cmd_desc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_command_execute
    * @details Execute the command associated with this command ID.
    * @param   cid command ID.
    * @return  cr_ErrorCodes_NO_ERROR on success or an error from the cr_ErrorCodes_
    *          enumeration if the command fails.
    */
    int __attribute__((weak)) crcb_command_execute(const uint8_t cid)
    {
        (void)cid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }
#endif  /// def INCLUDE_COMMAND_SERVICE

#ifdef INCLUDE_FILE_SERVICE

    /**
    * @brief   crcb_file_get_file_count
    * @details The overriding implementation must returns the number of files 
    *          implemented by the device.
    * @return  the number of files implemented by the device.
    */    
    int __attribute__((weak)) crcb_file_get_file_count()
    {
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) crcb_file_discover_reset(const uint8_t fid)
    {
        (void)fid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_file_discover_next
    * @details Gets the  description for the next file. The 
    *          overriding implementation must post-increment its
    *          pointer into the file table.
    * @param   file_desc Pointer to stack provided memory into which the 
    *               file description is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last file has already been returned.
    */
    int __attribute__((weak)) crcb_file_discover_next(cr_FileInfo *file_desc)
    {
        (void)file_desc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_file_get_description
    * @details Get the description matching the file ID.
    * @param   fid The ID of the desired file.
    * @param   file_desc Pointer to stack provided memory into which the 
    *               file description is to be copied
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int __attribute__((weak)) crcb_file_get_description(uint32_t fid, 
                                                      cr_FileInfo *file_desc)
    {
        (void)fid;
        (void)file_desc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_file_get_preferred_ack_rate
    * @details If the device has a preferred acknowledge rate it can implement this 
    *          function to advise the file transfer code of the rate.
    *          Higher ack rates mean less acknowlegements and faster file trasnfer.
    * @param   fid : File ID, in case this affects the decision 
    * @param   requested_rate: might factor into the decison. 
    * @param   is_write true if enquiring about write.
    * @return  A return value of zero means that there is no preferred rate and the 
    *          client can specify it.
    */
    int __attribute__((weak)) crcb_file_get_preferred_ack_rate(uint32_t fid,
                                                               uint32_t requested_rate,
                                                               bool is_write)
    {
        (void)fid;
        (void)requested_rate;
        (void)is_write;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return 0;
    }

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
    int __attribute__((weak)) 
    crcb_read_file(const uint32_t fid,            // which file
                   const int offset,              // offset, negative value specifies current location.
                   const size_t bytes_requested,  // how many bytes to read
                   uint8_t *pData,                // where the data goes
                   int *pBytes_read)              // bytes actually read, negative for errors.
    {
        (void)fid;
        (void)offset;
        (void)bytes_requested;
        (void)pData,
        (void)pBytes_read;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }


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
    int __attribute__((weak)) 
    crcb_write_file(const uint32_t fid,    // which file
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

    /**
    * @brief   crcb_erase_file
    * @details The device overrides this method to accept a command to set the 
    *          length of a file to zero, erasing it.
    * @param   fid (input) which file
    * @param   offset (input) offset, negative value specifies current location.
    * @return  returns zero or an error code
    */
    int __attribute__((weak)) crcb_erase_file(const uint32_t fid)
    {
        (void)fid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_file_transfer_complete
    * @details Called when the last bytes have been received and a file write 
    *          (upload) is complete.
    * @param   fid (input) which file
    * @return  returns zero or an error code
    */
    int __attribute__((weak)) crcb_file_transfer_complete(const uint32_t fid)
    {
        (void)fid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

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
    int __attribute__((weak)) crcb_file_prepare_to_write(const uint32_t fid,
                                                         const size_t offset,
                                                         const size_t bytes_to_write)
    {
        (void)fid;
        (void)offset;
        (void)bytes_to_write;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NO_ERROR;
    }

#endif /// def INCLUDE_FILE_SERVICE

#ifdef INCLUDE_TIME_SERVICE
    /**
    * @brief   crcb_time_get
    * @details Retrieve the device's idea of the current time. Time 
    *          is specified in UTC Epoch format, seconds since 1970.
    *          More than 32 bits are required to remain valid past
    *          2030.
    * @param   response (output) with utc_seconds current time and zone 
    * @return  returns zero or an error code
    */
    int __attribute__((weak)) crcb_time_get(cr_TimeGetResponse *response)
    {
        (void)response;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    /**
    * @brief   crcb_time_set
    * @details Inform the device of the current time to support setting an internal 
    *          time clock. Time is specified in UTC Epoch format,
    *          seconds since 1970. More than 32 bits are required to
    *          remain valid past 2030.
    * @param   request : structure with seconds and timeszone
    * @return  returns zero or an error code
    */
    int __attribute__((weak)) crcb_time_set(const cr_TimeSetRequest *request)
    {
        (void)request;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }
#endif  /// def INCLUDE_TIME_SERVICE

#ifdef INCLUDE_WIFI_SERVICE

    /**
    * @brief   crcb_discover_wifi
    * @details Retrieve the requested information about the WiFi 
    *          system.
    * @param   request (input) What info to get
    * @param   response (output) The requested info
    * @return  returns zero or an error code
    */
    int crcb_discover_wifi(const cr_DiscoverWiFiRequest *request, 
                                cr_DiscoverWiFiResponse *response)
    {
        (void)request;
        (void)response;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

   /**
    * @brief   crcb_get_wifi_count
    * @return  The number of wifi access points available to the
    *          the device.
    */    
    int __attribute__((weak)) crcb_get_wifi_count()
    {
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }


    /**
    * @brief   crcb_wifi_discover_reset
    * @details The overriding implementation must reset a pointer 
    *          into a table of the available wifi access points
    *          such that the next call to
    *          crcb_wifi_discover_next() will return the description
    *          of this access point.
    * @param   cid The ID to which the wifi table pointer 
    *              should be reset.  0 for the first AP.
    * @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
    *          cr_ErrorCodes_INVALID_PARAMETER.
    */
    int __attribute__((weak)) crcb_wifi_discover_reset(const uint32_t cid)
    {
        (void)cid;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }


    /**
    * @brief   crcb_wifi_discover_next
    * @details Gets the wifi description for the next wifi. 
    *          The overriding implementation must post-increment its pointer into 
    *          the wifi table.
    * @param   cmd_desc Pointer to stack provided memory into which the 
    *               wifi description is to be copied.
    * @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
    *          if the last wifi has already been returned.
    */
    int __attribute__((weak)) crcb_wifi_discover_next(cr_ConnectionDescription *AP_desc)
    {
        (void)AP_desc;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }

    int __attribute__((weak)) crcb_wifi_connection(const cr_WiFiConnectionRequest *request, 
                                                   cr_WiFiConnectionResponse *response)
    {
        (void)request;
        (void)response;
        I3_LOG(LOG_MASK_WEAK, "%s: weak default.\n", __FUNCTION__);
        return cr_ErrorCodes_NOT_IMPLEMENTED;
    }
#endif  // def INCLUDE_WIFI_SERVICE

#ifdef INCLUDE_OTA_SERVICE
    ///*************************************************************************
    ///  OTA service not yet supported
    ///*************************************************************************
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
#endif /// def INCLUDE_OTA_SERVICE

#ifdef INCLUDE_STREAM_SERVICE
    ///*************************************************************************
    ///  Stream Service not yet supported
    ///*************************************************************************
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

    /// A stream is sent as an array of records.
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
#endif /// INCLUDE_STREAM_SERVICE


