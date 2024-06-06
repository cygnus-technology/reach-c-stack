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
 * \brief "cr_streams.c" defines the API to the Cygnus Reach device stack
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_streams.c
 * @brief     Contains the private parts of the Cygnus Reach firmware stack 
 *            supporting the stream service.
 * @note      Functions that are not static are prefixed with pvtCrFile_.  The 
 *            entire contents can be excluded from the build when
 *            INCLUDE_STREAM_SERVICE is not defined.
 * @author    Chuck Peplinski
 * @date      2024-04-23
 * @copyright (c) Copyright 2024 i3 Product Development. All 
 * Rights Reserved. The Cygngus Reach firmware stack is shared 
 * under an MIT license. 
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

#include "pb_decode.h"
#include "pb_encode.h"

#include "message_util.h"
#include "reach_decode.h"


//----------------------------------------------------------------------------
// static (private) "member" functions
//----------------------------------------------------------------------------


#ifdef INCLUDE_STREAM_SERVICE
//*************************************************************************
//  Stream Service
//*************************************************************************
// The stream service implementation is only partially tested.

int pvtCr_discover_streams(const cr_DiscoverStreams *request,
                           cr_DiscoverStreamsResponse *response)
{
    int rval;
    if (request != NULL) {
        // request will be null on repeated calls.
        // Here implies we are responding to the initial request.
        crcb_stream_discover_reset(0);
        pvtCr_num_remaining_objects = crcb_stream_get_count();
        if (pvtCr_num_remaining_objects > REACH_COUNT_PARAM_READ_VALUES)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_STREAMS;
            I3_LOG(LOG_MASK_PARAMS, "discover files, Too many for one.");
        }
        I3_LOG(LOG_MASK_PARAMS, "discover streams, count %d.", pvtCr_num_remaining_objects);
    }
    
    response->streams_count = 0;
    for (int i=0; i<REACH_COUNT_STREAM_DESC_IN_RESPONSE; i++)
    {
        rval = crcb_stream_discover_next(&response->streams[i]);
        if (rval != cr_ErrorCodes_NO_ERROR) 
        {   // there are no more params.  clear on last.
            pvtCr_num_remaining_objects = 0;
            if (i==0)
            {
                I3_LOG(LOG_MASK_FILES, "No streams with i=0.");
                return cr_ErrorCodes_NO_DATA; 
            }
            return rval;
        }
        //I3_LOG(LOG_MASK_PARAMS, "Added stream %d.", response->streams_count);
        response->streams_count++;
    }
    return 0;
}

int pvtCr_open_stream(const cr_StreamOpen *req, cr_StreamResponse *resp)
{
    resp->result = crcb_stream_open(req->stream_id);
    resp->stream_id = req->stream_id;
    resp->has_result_message = false;
    return 0;
}

int pvtCr_close_stream(const cr_StreamClose *req, cr_StreamResponse *resp)
{
    resp->result = crcb_stream_close(req->stream_id);
    resp->stream_id = req->stream_id;
    resp->has_result_message = false;
    return 0;
}

// Write: The stream flows to the device.
int pvtCr_stream_receive_notification(cr_StreamData *data)
{
    crcb_stream_write(data->stream_id, data);
    // there is no response.
    return 0;
}

// read:  Stream flows from the device
int pvtCr_stream_send_notification(cr_StreamData *data)
{
    if (!cr_get_comm_link_connected())
        return 0;

    uint8_t *pCoded;
    size_t size;
    // I can encode directly, no copy to raw necessary.
    pvtCr_encode_message(cr_ReachMessageTypes_CLI_NOTIFICATION, data, NULL);
    pvtCr_get_coded_notification_buffers(&pCoded, &size);

    LOG_DUMP_WIRE("Stream", pCoded, size);
    crcb_send_coded_response(pCoded, size);
    return 0;
}



#endif  // def INCLUDE_STREAM_SERVICE


