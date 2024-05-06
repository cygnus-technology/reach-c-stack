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
 * @file      files.c
 * @brief     An example of functions to handle reading and writing files with Reach
 * @copyright (c) Copyright 2023-2024 i3 Product Development. All Rights Reserved.
 *
 * Original Author: Joseph Peplinski
 * This file is generated from a template with added user code.
 *
 ********************************************************************************************/

#include "definitions.h"

#ifdef INCLUDE_FILE_SERVICE

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "i3_log.h"
#include "app_version.h"
#include "cr_stack.h"


// Extra includes and forward declarations here.
// User code start [F10]
// User code end [F10]

static uint8_t sFid_index = 0;
#if NUM_FILES > 255
  #error "Can't have more than 255 files"
#endif
static int sFindIndexFromFid(uint32_t fid, uint8_t *index)
{
    uint8_t idx;
    for (idx = 0; idx < NUM_FILES; idx++)
    {
        if (file_descriptions[idx].file_id == fid)
        {
            *index = idx;
            return 0;
        }
    }
    return cr_ErrorCodes_INVALID_ID;
}
int crcb_file_get_description(uint32_t fid, cr_FileInfo *file_desc)
{
    int rval = 0;
    affirm(file_desc != NULL);
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (rval != 0) return rval;
    *file_desc = file_descriptions[idx];

    // User code start [F0]
    // User code end [F0]

    return 0;
}

int crcb_file_get_file_count()
{
    int i;
    int numAvailable = 0;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (crcb_access_granted(cr_ServiceIds_FILES, file_descriptions[i].file_id)) numAvailable++;
    }
    return numAvailable;
}

int crcb_file_discover_reset(const uint8_t fid)
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID, using NUM_FILES.", __FUNCTION__, fid);
        sFid_index = NUM_FILES;
        return cr_ErrorCodes_INVALID_ID;
    }
    if (!crcb_access_granted(cr_ServiceIds_FILES, file_descriptions[sFid_index].file_id))
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): Access not granted, using NUM_FILES.", __FUNCTION__, fid);
        sFid_index = NUM_FILES;
        return cr_ErrorCodes_BAD_FILE;
    }
    sFid_index = idx;
    return 0;
}

int crcb_file_discover_next(cr_FileInfo *file_desc)
{
    if (sFid_index >= NUM_FILES) // end of search
        return cr_ErrorCodes_NO_DATA;

    while (!crcb_access_granted(cr_ServiceIds_FILES, file_desc[sFid_index].file_id))
    {
        I3_LOG(LOG_MASK_FILES, "%s: sFid_index (%d) skip, access not granted",
               __FUNCTION__, sFid_index);
        sFid_index++;
        if (sFid_index >= NUM_FILES)
        {
            I3_LOG(LOG_MASK_PARAMS, "%s: skipped to sFid_index (%d) >= NUM_FILES (%d)",
                   __FUNCTION__, sFid_index, NUM_FILES);
            return cr_ErrorCodes_NO_DATA;
        }
    }
    *file_desc = file_descriptions[sFid_index++];
    return 0;
}

// Place helper functions here:
// User code start [F1]
// User code end [F1]


int crcb_read_file(const uint32_t fid,           // which file
                   const int offset,             // offset, negative value specifies current location.
                   const size_t bytes_requested, // how many bytes to read
                   uint8_t *pData,               // where the data goes
                   int *bytes_read)              // bytes actually read, negative for errors.
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID.", __FUNCTION__, fid);
        return cr_ErrorCodes_INVALID_ID;
    }
    if (bytes_requested > REACH_BYTES_IN_A_FILE_PACKET)
    {
        I3_LOG(LOG_MASK_ERROR, "%s: %d is more than the buffer for a file read (%d).", __FUNCTION__, fid, REACH_BYTES_IN_A_FILE_PACKET);
        return cr_ErrorCodes_BUFFER_TOO_SMALL;
    }

    // User code start [F2]
    // User code end [F2]

    return 0;
}

int crcb_file_prepare_to_write(const uint32_t fid, const size_t offset, const size_t bytes)
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID.", __FUNCTION__, fid);
        return cr_ErrorCodes_INVALID_ID;
    }
    // User code start [F3]
    // User code end [F3]
    return 0;
}

int crcb_write_file(const uint32_t fid, // which file
                    const int offset,      // offset, negative value specifies current location.
                    const size_t bytes,    // how many bytes to write
                    const uint8_t *pData)  // where to get the data from
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID.", __FUNCTION__, fid);
        return cr_ErrorCodes_INVALID_ID;
    }
    // User code start [F4]
    // User code end [F4]
    return 0;
}

int crcb_file_transfer_complete(const uint32_t fid)
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID.", __FUNCTION__, fid);
        return cr_ErrorCodes_INVALID_ID;
    }
    // User code start [F5]
    // User code end [F5]
    return 0;
}

// returns zero or an error code
int crcb_erase_file(const uint32_t fid)
{
    int rval = 0;
    uint8_t idx;
    rval = sFindIndexFromFid(fid, &idx);
    if (0 != rval)
    {
        I3_LOG(LOG_MASK_ERROR, "%s(%d): invalid FID.", __FUNCTION__, fid);
        return cr_ErrorCodes_INVALID_ID;
    }
    // User code start [F6]
    // User code end [F6]
    return 0;
}

#endif  // def INCLUDE_FILE_SERVICE


