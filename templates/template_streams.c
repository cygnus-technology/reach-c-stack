/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2024
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
 * @file      definitions.c
 * @brief     A minimal implementation of Reach data access.  Auto-generated by a Python script.
 * @copyright 2023-2024 i3 Product Development. All Rights Reserved.
 *
 * Original Author: Chuck Peplinski
 * Script Author: Joseph Peplinski
 *
 *
 ********************************************************************************************/

#include "definitions.h"

#ifdef INCLUDE_STREAM_SERVICE

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "i3_log.h"
#include "app_version.h"
#include "cr_stack.h"

// Extra includes and forward declarations here.
// User code start [S1]
// User code end [S1]

/**
* @brief   crcb_stream_get_count
* @return  The overriding implementation must returns the number 
*          of streams implemented by the device.
*/    
int crcb_stream_get_count()
{
    // User code start [crcb_stream_get_count]
    // User code end [crcb_stream_get_count]
    return 0;
}

/**
* @brief   crcb_stream_discover_reset
* @details The overriding implementation must reset a pointer into the stream 
*          table such that the next call to crcb_stream_discover_next() will
*          return the description of this stream.
* @param   sid The ID to which the stream table pointer 
*              should be reset.  use 0 for the first command.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
*          cr_ErrorCodes_INVALID_PARAMETER.
*/
int crcb_stream_discover_reset(const uint8_t sid)
{
    // User code start [crcb_stream_discover_reset]
    // User code end [crcb_stream_discover_reset]
    return 0;
}

/**
* @brief   crcb_stream_discover_next
* @details Gets the  description for the next stream.
*          The overriding implementation must post-increment its pointer into 
*          the stream table.
* @param   stream_desc Pointer to stack provided memory into which the 
*               stream description is to be copied.
* @return  cr_ErrorCodes_NO_ERROR on success or cr_ErrorCodes_INVALID_PARAMETER 
*          if the last stream has already been returned.
*/
int crcb_stream_discover_next(cr_StreamInfo *stream_desc)
{
    // User code start [crcb_stream_discover_next]
    // User code end [crcb_stream_discover_next]
    return 0;
}

/**
* @brief   crcb_stream_get_description
* @details Get the description matching the stream ID.
* @param   sid The ID of the desired stream.
* @param   stream_desc Pointer to stack provided memory into which the 
*               stream description is to be copied
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
*          cr_ErrorCodes_INVALID_PARAMETER.
*/
int crcb_stream_get_description(uint32_t sid, cr_StreamInfo *stream_desc)
{
    // User code start [crcb_stream_get_description]
    // User code end [crcb_stream_get_description]
    return 0;
}

/**
* @brief   crcb_stream_read
* @details The stream flows from the device.
*           Prepare a StreamData packet to be sent to the
*           client.  This is called in the main Reach loop.
* @param   sid The ID of the desired stream.
* @param   data Pointer to stack provided memory into which the 
*               data is to be copied
* @return  cr_ErrorCodes_NO_ERROR when the data is ready to be 
*          sent.  cr_ErrorCodes_NO_DATA if there is no data to
*          be sent.
*/
int crcb_stream_read(uint32_t sid, cr_StreamData *data)
{
    // User code start [crcb_stream_read]
    // User code end [crcb_stream_read]
    return 0;
}

/**
* @brief   crcb_stream_write
* @details The stream flows to the device.
*           Record or consume this data provided by the client.
*           Increment and populate the roll count.
* @param   sid The ID of the desired stream.
* @param   data Pointer to stack provided memory containing the 
*               stream data to be populated.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
*          cr_ErrorCodes_INVALID_PARAMETER.
*/
int crcb_stream_write(uint32_t sid, cr_StreamData *data)
{
    // User code start [crcb_stream_write]
    // User code end [crcb_stream_write]
    return 0;
}

/**
* @brief   crcb_stream_open
* @details Open the stream matching the stream ID. Zero the roll 
*          count for this stream.
* @param   sid The ID of the desired stream.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
*          cr_ErrorCodes_INVALID_PARAMETER.
*/
int crcb_stream_open(uint32_t sid)
{
    // User code start [crcb_stream_open]
    // User code end [crcb_stream_open]
    return 0;
}

/**
* @brief   crcb_stream_close
* @details Close the stream matching the stream ID.
* @param   sid The ID of the desired stream.
* @return  cr_ErrorCodes_NO_ERROR on success or a non-zero error like 
*          cr_ErrorCodes_INVALID_PARAMETER.
*/
int crcb_stream_close(uint32_t sid)
{
    // User code start [crcb_stream_close]
    // User code end [crcb_stream_close]
    return 0;
}

// local variables and functions here.
// User code start [S2]
// User code end [S2]


#endif  // def INCLUDE_STREAM_SERVICE
