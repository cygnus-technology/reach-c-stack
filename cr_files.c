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
 * \brief "cr_stack.c" defines the API to the Cygnus Reach device stack
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_files.c
 * @brief     Contains the private parts of the Cygnus Reach firmware stack 
 *            supporting the file service.
 * @note      Functions that are not static are prefixed with pvtCrFile_.  The 
 *            entire contents can be excluded from the build when
 *            INCLUDE_FILE_SERVICE is not defined.
 * @author    Chuck Peplinski
 * @date      2024-01-17
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
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

#include "cJSON.h"
#include "pb_decode.h"
#include "pb_encode.h"

#include "message_util.h"
#include "reach_decode.h"


//----------------------------------------------------------------------------
// static (private) "member" functions
//----------------------------------------------------------------------------


#ifdef INCLUDE_FILE_SERVICE
//*************************************************************************
//  File Service
//*************************************************************************

int pvtCrFile_discover(const cr_DiscoverFiles *request,
                                cr_DiscoverFilesResponse *response)
{
    if (!pvtCr_challenge_key_is_valid()) {
        pvtCr_num_continued_objects = pvtCr_num_remaining_objects = 0;
        memset(response, 0, sizeof(cr_DiscoverFilesResponse));
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    int rval;
    if (request != NULL) {
        // request will be null on repeated calls.
        // Here implies we are responding to the initial request.
        crcb_file_discover_reset(0);
        pvtCr_num_continued_objects = pvtCr_num_remaining_objects = crcb_file_get_file_count();
        if (pvtCr_num_remaining_objects > REACH_COUNT_PARAM_READ_VALUES)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_FILES;
            I3_LOG(LOG_MASK_PARAMS, "discover files, Too many for one.");
        }
        // pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_FILES;
        I3_LOG(LOG_MASK_PARAMS, "discover files, count %d.", pvtCr_num_remaining_objects);
    }

    
    response->file_infos_count = 0;
    for (int i=0; i<REACH_COUNT_PARAM_READ_VALUES; i++)
    {
        rval = crcb_file_discover_next(&response->file_infos[i]);
        if (rval != cr_ErrorCodes_NO_ERROR) 
        {   // there are no more params.  clear on last.
            pvtCr_num_remaining_objects = 0;
            if (i==0)
            {
                I3_LOG(LOG_MASK_FILES, "No files with i=0.");
                return cr_ErrorCodes_NO_DATA; 
            }
            return 0;
        }
        I3_LOG(LOG_MASK_PARAMS, "Added file %d.", response->file_infos_count);
        response->file_infos_count++;
    }
    return 0;
}

typedef PB_BYTES_ARRAY_T(REACH_BYTES_IN_A_FILE_PACKET) cr_FileTransferStateMachine_message_data_t;
typedef struct _cr_FileTransferStateMachine {
    cr_FileTransferState    state;
    uint32_t                transfer_id;    // determined on init
    int32_t                 file_id;        // requested at init
    int32_t                 timeout_in_ms;  // requested at init
    uint32_t                request_offset; // requested at init
    uint32_t                transfer_length;    // requested at init
    uint8_t                 read_write;         // 0: read, 1: write.
    uint32_t                message_number;     // rolling counter
    int32_t                 crc32;
    uint32_t                messages_per_ack;   // target, fixed
    uint32_t                messages_until_ack; // current, counts down
    uint32_t                bytes_transfered;   // to date
} cr_FileTransferStateMachine;

cr_FileTransferStateMachine sCr_file_xfer_state;

int pvtCrFile_transfer_init(const cr_FileTransferInit *request,
                               cr_FileTransferInitResponse *response)
{
    if (!pvtCr_challenge_key_is_valid()) {
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE; 
        response->result = cr_ErrorCodes_CHALLENGE_FAILED;
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    cr_FileInfo file_desc;
    memset(response, 0, sizeof(cr_FileTransferInitResponse));
    response->transfer_id = request->transfer_id;
    memset(&sCr_file_xfer_state, 0, sizeof(cr_FileTransferStateMachine));
    sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
    int rval = crcb_file_get_description(request->file_id, &file_desc);
    if (rval != 0)
    {
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE; 
        cr_report_error(cr_ErrorCodes_BAD_FILE, 
                        "%s No file description for fid %d.", 
                        __FUNCTION__, request->file_id);
        response->result = rval;
        return rval;
    }
    switch (file_desc.access)
    {
    default:
    case cr_AccessLevel_NO_ACCESS:
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
        cr_report_error(cr_ErrorCodes_PERMISSION_DENIED, 
                        "%s File ID %d access permission denied.", 
                        __FUNCTION__, request->file_id);
        response->result = cr_ErrorCodes_PERMISSION_DENIED;
        return rval;
    case cr_AccessLevel_READ:
        if (request->read_write)    // 1 for write
        {
            sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
            cr_report_error(cr_ErrorCodes_PERMISSION_DENIED, 
                            "%s File ID %d write permission denied.", 
                            __FUNCTION__, request->file_id);
            response->result = cr_ErrorCodes_PERMISSION_DENIED;
            return rval;
        }
        break;
    case cr_AccessLevel_WRITE:
        if (!request->read_write)   // 0 for read
        {
            sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
            cr_report_error(cr_ErrorCodes_PERMISSION_DENIED, 
                            "%s File ID %d read permission denied.", 
                            __FUNCTION__, request->file_id);
            response->result = cr_ErrorCodes_PERMISSION_DENIED;
        }
        break;
    case cr_AccessLevel_READ_WRITE:
        break;
    }

    // If server prefers a rate we use it.
    // If server says zero, then use request.
    // If both are zero, default to 10.
    int preferred_ack_rate = crcb_file_get_preferred_ack_rate(request->read_write);
    if (preferred_ack_rate == 0)
        preferred_ack_rate = request->messages_per_ack;
    if (preferred_ack_rate == 0)
        preferred_ack_rate = 10;  // default

    response->result = 0;

    // sCr_file_xfer_state was zero'ed above.
    sCr_file_xfer_state.state                   = cr_FileTransferState_INIT;
    sCr_file_xfer_state.transfer_id             = request->transfer_id;
    sCr_file_xfer_state.file_id                 = request->file_id;
    sCr_file_xfer_state.timeout_in_ms           = request->timeout_in_ms;  
    sCr_file_xfer_state.request_offset          = request->request_offset; 
    sCr_file_xfer_state.transfer_length         = request->transfer_length;
    sCr_file_xfer_state.read_write              = request->read_write;
    sCr_file_xfer_state.message_number          = 0; 
    sCr_file_xfer_state.crc32                   = 0;
    sCr_file_xfer_state.messages_per_ack        = preferred_ack_rate;
    sCr_file_xfer_state.messages_until_ack      = preferred_ack_rate;
    sCr_file_xfer_state.bytes_transfered        = 0;

    if (request->read_write)
    {
        // give the app a chance to erase flash:
        int rval = 
        crcb_file_prepare_to_write(request->file_id, 
                                   request->request_offset, 
                                   request->transfer_length);
        if (rval == 0) {
            I3_LOG(LOG_MASK_ALWAYS, "Start file write, timeout %d ms:", 
                   sCr_file_xfer_state.timeout_in_ms);
        }
        else
        {
            LOG_ERROR("crcb_file_prepare_to_write failed");
            cr_report_error(cr_ErrorCodes_WRITE_FAILED, 
                            "crcb_file_prepare_to_write() failed with %d.", rval);
        }
    }
    else
    {
        I3_LOG(LOG_MASK_ALWAYS, "Start file read, timeout %d ms:", 
                   sCr_file_xfer_state.timeout_in_ms);
    }

    I3_LOG(LOG_MASK_ALWAYS, "  File ID: %d. offset %d. size %d. msgs per ACK: %d",
           request->file_id, request->request_offset, request->transfer_length,
           request->messages_per_ack);

    pvtCr_watchdog_start_timeout(sCr_file_xfer_state.timeout_in_ms, 
                                 cr_get_current_ticks());

    return 0;
}

// pvtCrFile_transfer_data(() is used with file write.
// We should respond either with cr_FileTransferDataNotification or nothing.
// Notify if message counter is zero or file is complete.
// Return cr_ErrorCodes_NO_RESPONSE if no ack is expected, or 0 to ack.
int pvtCrFile_transfer_data(const cr_FileTransferData *dataTransfer,
                         cr_FileTransferDataNotification *response)
{
    if (!pvtCr_challenge_key_is_valid()) {
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE; 
        response->result = cr_ErrorCodes_CHALLENGE_FAILED;
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    // we receive this on write.
    memset(response, 0, sizeof(cr_FileTransferDataNotification));
    switch (sCr_file_xfer_state.state)
    {
    default:
    case cr_FileTransferState_FILE_TRANSFER_INVALID:
        LOG_ERROR("State invalid");
        cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                        "%s should not be called in state invalid.", __FUNCTION__);
        response->result = cr_ErrorCodes_INVALID_STATE;
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_INVALID_STATE;
    case cr_FileTransferState_IDLE:
        LOG_ERROR("In idle state is not right");
        cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                        "%s should not be called in state idle.", __FUNCTION__);
        response->result = cr_ErrorCodes_INVALID_STATE;
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_INVALID_STATE;
    case cr_FileTransferState_COMPLETE:
        LOG_ERROR("In complete state is not right");
        cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                        "%s should not be called in state complete.", __FUNCTION__);
        response->result = cr_ErrorCodes_INVALID_STATE;
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_INVALID_STATE;
    case cr_FileTransferState_INIT:
    case cr_FileTransferState_DATA:
        break;
    }
    if (dataTransfer->transfer_id != sCr_file_xfer_state.transfer_id)
    {
        // the transfer_id is not rigorously enforced (yet)
        I3_LOG(LOG_MASK_WARN, "Unmatched transfer_id (%d not %d)", 
                  dataTransfer->transfer_id, sCr_file_xfer_state.transfer_id);
        // response->result = cr_ErrorCodes_INVALID_PARAMETER;
        // return 0;
    }
    response->transfer_id = dataTransfer->transfer_id;
    int bytes_to_write = dataTransfer->message_data.size;
    if (bytes_to_write > REACH_BYTES_IN_A_FILE_PACKET)
    {
        LOG_ERROR("Requested write of %d bytes > REACH_BYTES_IN_A_FILE_PACKET (%d).",
                  bytes_to_write, REACH_BYTES_IN_A_FILE_PACKET);
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
        response->result = cr_ErrorCodes_INVALID_PARAMETER;
        cr_report_error(cr_ErrorCodes_INVALID_PARAMETER, 
                        "%s: Requested xfer of %d bytes > REACH_BYTES_IN_A_FILE_PACKET (%d).",
                        __FUNCTION__, bytes_to_write, REACH_BYTES_IN_A_FILE_PACKET);
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_INVALID_PARAMETER;
    }

    sCr_file_xfer_state.bytes_transfered += bytes_to_write;
    int bytes_remaining_to_write = 
        sCr_file_xfer_state.transfer_length - sCr_file_xfer_state.bytes_transfered; 
    // I3_LOG(LOG_MASK_FILES, "fwtd %d bytes, %d remaining of %d.", bytes_to_write,
    //        bytes_remaining_to_write, sCr_file_xfer_state.transfer_length);

    int rval = crcb_write_file(sCr_file_xfer_state.file_id,
                             sCr_file_xfer_state.request_offset,
                             bytes_to_write,
                             dataTransfer->message_data.bytes);
    if (rval != 0)
    {
        LOG_ERROR("File write of %d bytes to fid %d failed with error %d", 
                  bytes_to_write, sCr_file_xfer_state.file_id, rval);
        response->result = cr_ErrorCodes_WRITE_FAILED;
        cr_report_error(cr_ErrorCodes_WRITE_FAILED, 
                        "%s: Requested write of %d bytes for fid %d failed.",
                        __FUNCTION__, bytes_to_write, sCr_file_xfer_state.transfer_id);
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_WRITE_FAILED;
    }

    // update these before checking for message mismatch
    if (sCr_file_xfer_state.messages_until_ack != 0)
        sCr_file_xfer_state.messages_until_ack--;
    sCr_file_xfer_state.message_number++;
    sCr_file_xfer_state.request_offset += bytes_to_write;

    if (dataTransfer->message_number != sCr_file_xfer_state.message_number)
    {
        sCr_file_xfer_state.request_offset -= bytes_to_write;
        LOG_ERROR("At %d, message number mismatch. Got %d, not %d", 
                  sCr_file_xfer_state.bytes_transfered,
                  dataTransfer->message_number, 
                  sCr_file_xfer_state.message_number);
        response->result = cr_ErrorCodes_PACKET_COUNT_ERR;
        // tell the client the offset at which to retry.
        response->retry_offset = sCr_file_xfer_state.request_offset + sCr_file_xfer_state.bytes_transfered;
        sprintf(response->error_message,  
                "At %d, message number mismatch. Got %d, not %d", 
                (int)sCr_file_xfer_state.bytes_transfered,
                (int)dataTransfer->message_number,
                (int)sCr_file_xfer_state.message_number);
        /*
        cr_report_error(cr_ErrorCodes_WRITE_FAILED, 
                        "%s: At %d, message number mismatch. Got %d, not %d", 
                        __FUNCTION__, sCr_file_xfer_state.bytes_transfered,
                        dataTransfer->message_number, sCr_file_xfer_state.message_number);
        */
        // if we don't stop this lets me see on error per mismatch
        sCr_file_xfer_state.message_number = dataTransfer->message_number;
        pvtCr_watchdog_stroke_timeout(cr_get_current_ticks());
        return 0; // cr_ErrorCodes_WRITE_FAILED;
    }



    /*I3_LOG(LOG_MASK_FILES, "fwtd, rem %d. until ack: %d.  num %d.", 
               bytes_remaining_to_write, sCr_file_xfer_state.messages_until_ack,
               sCr_file_xfer_state.message_number);*/

    I3_LOG(LOG_MASK_FILES, "fwtd, msg %d. until ack: %d.  num %d.", 
           dataTransfer->message_number, 
           sCr_file_xfer_state.messages_until_ack,
           sCr_file_xfer_state.message_number);

    if (sCr_file_xfer_state.bytes_transfered >= sCr_file_xfer_state.transfer_length)
    {
        I3_LOG(LOG_MASK_ALWAYS, "file write complete.");
        if (bytes_remaining_to_write != 0)
        {
            I3_LOG(LOG_MASK_WARN, "On file write, remaining bytes is below zero.");
        }
        response->is_complete = true;
        crcb_file_transfer_complete(sCr_file_xfer_state.file_id);
        pvtCr_watchdog_end_timeout();
        return 0;
    }

    if (sCr_file_xfer_state.messages_until_ack != 0)
    {
        /*
        I3_LOG(LOG_MASK_FILES, "file write, no ACK. per ack: %d.  until ack: %d.  num %d.", 
               sCr_file_xfer_state.messages_per_ack, sCr_file_xfer_state.messages_until_ack,
               sCr_file_xfer_state.message_number);
         */
        pvtCr_watchdog_stroke_timeout(cr_get_current_ticks());
        return cr_ErrorCodes_NO_RESPONSE;
    }
    // here we want to ack, also reset the counters.
    I3_LOG(LOG_MASK_FILES, "ACK file write.  per ack: %d.  num %d.", 
               sCr_file_xfer_state.messages_per_ack, sCr_file_xfer_state.message_number);

    sCr_file_xfer_state.messages_until_ack = sCr_file_xfer_state.messages_per_ack;
    sCr_file_xfer_state.message_number = 0;
    response->is_complete = false;
    pvtCr_watchdog_stroke_timeout(cr_get_current_ticks());
    return 0;
}

int pvtCrFile_transfer_data_notification(const cr_FileTransferDataNotification *request,
                                      cr_FileTransferData *dataTransfer)
{
    if (!pvtCr_challenge_key_is_valid()) {
        sCr_file_xfer_state.state = cr_FileTransferState_IDLE; 
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA; 
    }

    // We receive this in the case of read file.
    // And it can generate repeated responses.
    if (request)
    {   // responding to a prompt.
        switch (sCr_file_xfer_state.state)
        {
        default:
        case cr_FileTransferState_FILE_TRANSFER_INVALID:
            LOG_ERROR("State invalid");
            cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                            "%s should not be called in state invalid.", __FUNCTION__);
            dataTransfer->result = cr_ErrorCodes_INVALID_STATE;
            pvtCr_watchdog_end_timeout();
            return cr_ErrorCodes_INVALID_STATE;
        case cr_FileTransferState_IDLE:
            LOG_ERROR("In idle state is not right");
            cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                            "%s should not be called in state idle.", __FUNCTION__);
            dataTransfer->result = cr_ErrorCodes_INVALID_STATE;
            pvtCr_watchdog_end_timeout();
            return cr_ErrorCodes_INVALID_STATE;
        case cr_FileTransferState_COMPLETE:
            if (request->is_complete)
            {
                // echo the notification back
                memcpy(dataTransfer, request, sizeof(cr_FileTransferDataNotification));
                sCr_file_xfer_state.state = cr_FileTransferState_IDLE;
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                pvtCr_num_remaining_objects = 0;
                pvtCr_num_continued_objects = 0;
                I3_LOG(LOG_MASK_FILES, "Completing the file read.");
                pvtCr_watchdog_end_timeout();
                return 0;
            }

            LOG_ERROR("In complete state is not right");
            cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                            "%s should not be called in state complete.", __FUNCTION__);
            dataTransfer->result = cr_ErrorCodes_INVALID_STATE;
            pvtCr_watchdog_end_timeout();
            return cr_ErrorCodes_INVALID_STATE;
        case cr_FileTransferState_INIT:
        case cr_FileTransferState_DATA:
            break;
        }

        if (sCr_file_xfer_state.read_write)
        {   // expecting read
            LOG_ERROR("Expecting read, not write");
            cr_report_error(cr_ErrorCodes_INVALID_STATE, 
                            "%s Expecting read, not write.", __FUNCTION__);
            dataTransfer->result = cr_ErrorCodes_WRITE_FAILED;
            pvtCr_watchdog_end_timeout();
            return cr_ErrorCodes_WRITE_FAILED;
        }
        if (request->is_complete)
        {
            I3_LOG(LOG_MASK_ALWAYS, "file read of fid %d is complete.", 
                   sCr_file_xfer_state.file_id);
            sCr_file_xfer_state.state = cr_FileTransferState_COMPLETE;
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            pvtCr_num_remaining_objects = 0;
            pvtCr_num_continued_objects = 0;
            dataTransfer->result = 0;
            pvtCr_watchdog_end_timeout();
            return 0;
        }

        pvtCr_continued_message_type = cr_ReachMessageTypes_TRANSFER_DATA;
        pvtCr_num_remaining_objects = sCr_file_xfer_state.messages_until_ack;
        pvtCr_num_continued_objects = sCr_file_xfer_state.messages_per_ack;
        sCr_file_xfer_state.messages_until_ack = sCr_file_xfer_state.messages_per_ack;
        sCr_file_xfer_state.message_number = 0;
    }
    memset(dataTransfer, 0, sizeof(cr_FileTransferData));

    dataTransfer->transfer_id = sCr_file_xfer_state.transfer_id;
    size_t bytes_remaining_to_read = 
        sCr_file_xfer_state.transfer_length - sCr_file_xfer_state.bytes_transfered;

    size_t 
        bytes_requested = 
            (bytes_remaining_to_read >= REACH_BYTES_IN_A_FILE_PACKET)
                ? REACH_BYTES_IN_A_FILE_PACKET : bytes_remaining_to_read;

    I3_LOG(LOG_MASK_FILES, "file read %d, %d remaining of %d.", bytes_requested,
           bytes_remaining_to_read, sCr_file_xfer_state.transfer_length);
    I3_LOG(LOG_MASK_FILES, " per ack: %d.  until ack: %d.  num %d.", 
           sCr_file_xfer_state.messages_per_ack, sCr_file_xfer_state.messages_until_ack,
           sCr_file_xfer_state.message_number);

    int bytes_read = 0;
    int rval = crcb_read_file(sCr_file_xfer_state.file_id,
                            sCr_file_xfer_state.request_offset,
                            bytes_requested,
                            dataTransfer->message_data.bytes,
                            &bytes_read);
    if (rval != 0)
    {
        dataTransfer->result = cr_ErrorCodes_READ_FAILED;
        cr_report_error(cr_ErrorCodes_READ_FAILED, 
                        "%s: File read of %d bytes from fid %d failed with error %d", 
                        __FUNCTION__, bytes_requested, sCr_file_xfer_state.file_id, rval);
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        pvtCr_num_remaining_objects = 0;
        pvtCr_num_continued_objects = 0;
        pvtCr_watchdog_end_timeout();
        return cr_ErrorCodes_READ_FAILED;
    }
    dataTransfer->message_data.size = bytes_read;
    sCr_file_xfer_state.bytes_transfered += bytes_read;
    sCr_file_xfer_state.request_offset += bytes_read;

    // To Do:  Calculate CRC.

    if (sCr_file_xfer_state.messages_until_ack != 0)
        sCr_file_xfer_state.messages_until_ack--;
    
    if (sCr_file_xfer_state.messages_until_ack == 0)
    {
        I3_LOG(LOG_MASK_FILES, "file read wait for ACK now.");
    }
    
    pvtCr_num_remaining_objects = sCr_file_xfer_state.messages_until_ack;
    pvtCr_num_continued_objects = sCr_file_xfer_state.messages_per_ack;
    pvtCr_continued_message_type = pvtCr_num_remaining_objects == 0  ? 
            cr_ReachMessageTypes_INVALID : cr_ReachMessageTypes_TRANSFER_DATA;

    sCr_file_xfer_state.message_number++;
    dataTransfer->message_number = sCr_file_xfer_state.message_number;

    bytes_remaining_to_read -= bytes_read;
    if (bytes_remaining_to_read == 0)
    {
        I3_LOG(LOG_MASK_ALWAYS, "File read complete.");
        pvtCr_num_remaining_objects = 0;
        sCr_file_xfer_state.state = cr_FileTransferState_COMPLETE;
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        pvtCr_watchdog_end_timeout();
        return 0;
    }
    sCr_file_xfer_state.state = cr_FileTransferState_DATA;
    pvtCr_watchdog_stroke_timeout(cr_get_current_ticks());
    return 0;
}

// 
// Timeout Watchdog interface
// This is used in the file write sequences.
// 
static bool  sTimeoutWatchdog_is_active = false;
static uint32_t sTimeoutWatchdog_period = 0;
static uint32_t sTimeoutWatchdog_target = 0;

// 0 ms disables watchdog.
void pvtCr_watchdog_start_timeout(uint32_t msec, uint32_t ticks)
{
    if (msec > 0) {
        sTimeoutWatchdog_is_active = true;
        sTimeoutWatchdog_period    = msec;
        sTimeoutWatchdog_target = ticks + msec;
        I3_LOG(LOG_MASK_DEBUG, "%s: set timeout to %d ms at %d ticks.", __FUNCTION__, msec, ticks);
        return;
    }
    I3_LOG(LOG_MASK_DEBUG, "%s: Disable timeout with %d ms at %d ticks.", __FUNCTION__, msec, ticks);
    sTimeoutWatchdog_is_active = false;
}

// resets the timeout period to original
void pvtCr_watchdog_stroke_timeout(uint32_t ticks)
{
    if (sTimeoutWatchdog_is_active) {
        sTimeoutWatchdog_target = ticks + sTimeoutWatchdog_period;
        I3_LOG(LOG_MASK_DEBUG, "%s: Stroke timeout with %d ms at %d ticks.", 
               __FUNCTION__, sTimeoutWatchdog_period, ticks);
        return;
    }
    I3_LOG(LOG_MASK_DEBUG, "%s: Stroke timeout inactive.", __FUNCTION__);
}

// disables the watchdog
void pvtCr_watchdog_end_timeout()
{
    sTimeoutWatchdog_is_active = false;
    I3_LOG(LOG_MASK_DEBUG, "%s: End timeout.", __FUNCTION__);
}

// if active, compares ticks to expected timeout.
// return 1 if timeout occurred
int pvtCr_watchdog_check_timeout(uint32_t ticks)
{
    if (!sTimeoutWatchdog_is_active)
        return 0;

    if (ticks > sTimeoutWatchdog_target)
    {
        I3_LOG(LOG_MASK_DEBUG, TEXT_RED "%s: timeout Expired.", __FUNCTION__);
        return 1;
    }
    return 0;
}


#endif  // def INCLUDE_FILE_SERVICE


