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
 * \brief "cr_params.c" defines the parameter specific API to the Cygnus Reach device stack
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_params.c
 * @brief     Contains the private parts of the Cygnus Reach firmware stack 
 *            supporting the parameter repository. Functions
 *            that are not static are prefixed with pvtCrParam_.
 *            The entire contents can be excluded from the build
 *            when INCLUDE_PARAMETER_SERVICE is not defined.
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


#ifdef INCLUDE_PARAMETER_SERVICE

    #include "cr_stack.h"
    #include "cr_private.h"
    #include "i3_log.h"

    #include "pb_decode.h"
    #include "pb_encode.h"

    #include "message_util.h"
    #include "reach_decode.h"
    #include "reach_version.h"

    static uint32_t sCr_num_ex_this_pid = 0;
    static int16_t sCr_requested_param_array[REACH_COUNT_PARAMS_IN_REQUEST];
    static uint8_t sCr_requested_param_info_count = 0;
    static uint8_t sCr_requested_param_index = 0;
    static uint8_t sCr_requested_notify_count = 0;
    static uint8_t sCr_requested_param_read_count = 0;
  #if NUM_SUPPORTED_PARAM_NOTIFY != 0
    /// check these params for notification
    static uint32_t sCr_numNotificationsSent = 0;
    static cr_ParameterNotifyConfig sCr_param_notify_list[NUM_SUPPORTED_PARAM_NOTIFY];
    /// storage of the previous value
    static cr_ParameterValue sCr_last_param_values[NUM_SUPPORTED_PARAM_NOTIFY];
    static uint8_t sCr_requested_notify_index = 0;
  #endif

    /**
    * @brief   pvtCrParam_discover_parameters
    * @details Private function responsible to respond to a discover 
    *          parameter request. This can be called directly in
    *          response to the discovery request or it can be called
    *          on a continuing basis to complete the discovery
    *          transaction.
    * @param   request decoded request.
    * @param   response uncoded response structure is produced here. 
    * @return  cr_ErrorCodes_NO_ERROR if data is produced.  A non-zero
    *          error like cr_ErrorCodes_NO_DATA if there are no more parameters.
    */
    int 
    pvtCrParam_discover_parameters(const cr_ParameterInfoRequest *request,
                                   cr_ParameterInfoResponse *response)
    {
        int rval;

        if (!crcb_challenge_key_is_valid()) {
            sCr_requested_param_info_count = 0;
            response->parameter_infos_count = 0;
            return cr_ErrorCodes_NO_DATA;
        }

        if (request != NULL) {
            // request will be null on repeated calls.
            // Here implies we are responding to the initial request.
            sCr_requested_param_index = 0;
            sCr_requested_param_info_count = request->parameter_ids_count;
            I3_LOG(LOG_MASK_PARAMS, "discover params, count %d.", sCr_requested_param_info_count);

            if (request->parameter_ids_count != 0) {
                sCr_requested_param_index = 0;
                // init them all to -1 meaning invalid.
                memset(sCr_requested_param_array, -1, sizeof(sCr_requested_param_array));
                // copy the requested numbers
                for (int i=0; i < request->parameter_ids_count; i++) {
                    affirm(request->parameter_ids[i] < MAX_NUM_PARAM_ID);
                    sCr_requested_param_array[i] = request->parameter_ids[i];
                }
                sCr_requested_param_index = 0;
                // default on first.
                pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAMETERS;
                pvtCr_num_remaining_objects = request->parameter_ids_count;
            }
            else
            {
                pvtCr_num_remaining_objects = crcb_parameter_get_count();
            }
            if (pvtCr_num_remaining_objects > REACH_COUNT_PARAM_DESC_IN_RESPONSE)
            {
                pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAMETERS;
                I3_LOG(LOG_MASK_PARAMS, "discover params, Too many for one.");
            }
        }

        // here this could be the first response or a continued response.
        if (sCr_requested_param_info_count == 0)
        {
            if (request != NULL)
            {   // first time
                crcb_parameter_discover_reset(0);
                sCr_requested_param_info_count = 0;
                pvtCr_num_remaining_objects = crcb_parameter_get_count();
                // default on first.
                pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAMETERS;
            }

            // GCC 12 produces a warning here, google it to see controversy.
            response->parameter_infos_count = 0;
            for (int i=0; i<REACH_COUNT_PARAM_DESC_IN_RESPONSE; i++) 
            {
                rval = crcb_parameter_discover_next(&response->parameter_infos[i]);
                if (rval != cr_ErrorCodes_NO_ERROR) 
                {   // there are no more params.  clear on last.
                    pvtCr_num_remaining_objects = 0;
                    if (i==0)
                    {
                        I3_LOG(LOG_MASK_PARAMS, "No data on i=0.");
                        pvtCr_num_remaining_objects = 0;
                        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                        return cr_ErrorCodes_NO_DATA; 
                    }
                    I3_LOG(LOG_MASK_PARAMS, "Added %d.", response->parameter_infos_count);
                    return 0;
                }
                I3_LOG(LOG_MASK_PARAMS, "Add param %d.", sCr_requested_param_index);
                sCr_requested_param_index++;
                pvtCr_num_remaining_objects--;
                response->parameter_infos_count++;
            }
            if (response->parameter_infos_count == 0)
            {
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                return cr_ErrorCodes_NO_DATA; 
            }
            I3_LOG(LOG_MASK_PARAMS, "Added %d.", response->parameter_infos_count);
            return 0;
        }

        // we are supplied a list of params.
        I3_LOG(LOG_MASK_PARAMS, "%s: Supplied a list.", __FUNCTION__);
        response->parameter_infos_count = 0;
        for (int i=0; i<REACH_COUNT_PARAM_DESC_IN_RESPONSE; i++)
        {
            affirm(sCr_requested_param_index < REACH_PARAM_BUFFER_COUNT);
            if (sCr_requested_param_index >= sCr_requested_param_info_count) {
                // we've done them all.
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                break;
            }
            if (sCr_requested_param_array[sCr_requested_param_index] < 0) {
                // we've done them all.
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                break;
            }
            I3_LOG(LOG_MASK_PARAMS, "Add param %d from list of %d", 
                   sCr_requested_param_index, sCr_requested_param_info_count);
            crcb_parameter_discover_reset(sCr_requested_param_array[sCr_requested_param_index]);
            rval = crcb_parameter_discover_next(&response->parameter_infos[i]);
            sCr_requested_param_array[sCr_requested_param_index] = -1;
            if (rval != cr_ErrorCodes_NO_ERROR) {
                // we've done them all.
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                sCr_requested_param_info_count = 0;
                break;
            }
            sCr_requested_param_index++;
            pvtCr_num_remaining_objects--;
            response->parameter_infos_count++;
        }

        if (response->parameter_infos_count == 0)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA; 
        }
        return 0;
    }

    /**
    * @brief   pvtCrParam_discover_parameters_ex
    * @details Private function gandles extended parameter data describing enums and
    *          bitfields.
    * @return  cr_ErrorCodes_NO_ERROR or a non-zero error (see cr_ErrorCodes_). 
    */
    int 
    pvtCrParam_discover_parameters_ex(const cr_ParameterInfoRequest *request,
                                      cr_ParamExInfoResponse *response) 
    {
        if (!crcb_challenge_key_is_valid()) {
            sCr_requested_param_info_count = 0;
            pvtCr_num_remaining_objects = response->enumerations_count = 0;
            return cr_ErrorCodes_NO_DATA;
        }

        int rval;
        if (request != NULL) 
        {
            // request will be null on repeated calls.
            // Here implies we are responding to the initial request.
            // set up to call next().
            sCr_requested_param_index = 0;
            sCr_requested_param_info_count = request->parameter_ids_count;
            sCr_num_ex_this_pid = 0;
            I3_LOG(LOG_MASK_PARAMS, "discover params ex, param count %d.", sCr_requested_param_info_count);

            if (request->parameter_ids_count != 0) 
            {
                sCr_num_ex_this_pid = crcb_parameter_ex_get_count(request->parameter_ids[0]);
                sCr_requested_param_index = 0;
                // init them all to -1 meaning invalid.
                memset(sCr_requested_param_array, -1, sizeof(sCr_requested_param_array));
                pvtCr_num_remaining_objects = 0;
                // copy the requested numbers
                for (int i=0; i < request->parameter_ids_count; i++) {
                    affirm(request->parameter_ids[i] < MAX_NUM_PARAM_ID);
                    sCr_requested_param_array[i] = request->parameter_ids[i];
                    pvtCr_num_remaining_objects += crcb_parameter_ex_get_count(
                        request->parameter_ids[i]);
                }
                if (pvtCr_num_remaining_objects == 0)
                {   // there is no ex data
                    response->enumerations_count = 0;
                    I3_LOG(LOG_MASK_PARAMS, "dpx: %d params, no ex.", 
                           request->parameter_ids_count);
                    return 0;
                }
                sCr_requested_param_index = 0;
                // default on first.
                pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAM_EX;
            }
            else
            {
                // count is zero, so setup for all
                crcb_parameter_ex_discover_reset(-1);
                pvtCr_num_remaining_objects = crcb_parameter_ex_get_count(-1);
                if (pvtCr_num_remaining_objects == 0)
                {
                    I3_LOG(LOG_MASK_PARAMS, "discover params ex, object found no ex.");
                    pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                    return 0;
                }
            }
            // one object in each response.
            I3_LOG(LOG_MASK_PARAMS, "discover params ex, object count %d.", pvtCr_num_remaining_objects);

            // here we've found at least one so use it.
            rval = crcb_parameter_ex_discover_next(response);
            pvtCr_num_remaining_objects--;
            if (pvtCr_num_remaining_objects == 0)
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            else
                pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAM_EX;
            return 0;
        }

        // here this could be the first response or a continued response.
        // determine if there is another.
        if (pvtCr_num_remaining_objects == 0)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA;
        }
        rval = crcb_parameter_ex_discover_next(response);
        if (rval == 0)
        {   // there was another for this param.
            pvtCr_num_remaining_objects--;
            pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAM_EX;
            sCr_num_ex_this_pid = 0;
            return 0;
        }
        if (sCr_requested_param_info_count == 0)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA;
        }

        // Need to reset for the next param.
        sCr_requested_param_index++;
        if (sCr_requested_param_index >= sCr_requested_param_info_count)
        {
            pvtCr_num_remaining_objects = 0;
            I3_LOG(LOG_MASK_PARAMS, "No more params.");
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA;
        }

        if (sCr_num_ex_this_pid == 0)
        {
            sCr_num_ex_this_pid = crcb_parameter_ex_get_count(sCr_requested_param_array[sCr_requested_param_index]);
            crcb_parameter_ex_discover_reset(sCr_requested_param_array[sCr_requested_param_index]);
        }
        rval = crcb_parameter_ex_discover_next(response);
        if (rval == 0)
        {   // there was another for this param.
            pvtCr_num_remaining_objects--;
            pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_PARAM_EX;
            sCr_num_ex_this_pid = 0;
            return 0;
        }
        I3_LOG(LOG_MASK_PARAMS, "End of %s?", __FUNCTION__);
        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
        return cr_ErrorCodes_NO_DATA;
    }


    // This can be called directly in response to the read request
    // or it can be called on a continuing basis to complete the 
    // read transaction.  
    int pvtCrParam_read_param(const cr_ParameterRead *request,
                              cr_ParameterReadResponse *response) 
    {
        if (!crcb_challenge_key_is_valid()) {
            pvtCr_num_remaining_objects = 0;
            memset(response, 0, sizeof(cr_ParameterReadResponse));
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA; 
        }

        int rval;
        if (request != NULL) {
            // request will be null on repeated calls.
            // Here implies we are responding to the initial request.
            sCr_requested_param_read_count = request->parameter_ids_count;
            I3_LOG(LOG_MASK_PARAMS, "read params, count %d.", sCr_requested_param_info_count);

            if (request->parameter_ids_count != 0) {
                // init them all to -1 meaning invalid.
                sCr_requested_param_index = 0;
                memset(sCr_requested_param_array, -1, sizeof(sCr_requested_param_array));
                // copy the requested numbers
                for (int i=0; i < request->parameter_ids_count; i++) {
                    affirm(request->parameter_ids[i] < MAX_NUM_PARAM_ID);
                    sCr_requested_param_array[i] = request->parameter_ids[i];
                }
                // default on first.
                pvtCr_continued_message_type = cr_ReachMessageTypes_READ_PARAMETERS;
                pvtCr_num_remaining_objects = request->parameter_ids_count;
            }
            else
            {
                sCr_requested_param_index = 0;
                I3_LOG(LOG_MASK_PARAMS, "READ all PARAMETERS.");
                pvtCr_num_remaining_objects = crcb_parameter_get_count();
            }
            if (pvtCr_num_remaining_objects > REACH_COUNT_PARAM_READ_VALUES)
            {
                pvtCr_continued_message_type = cr_ReachMessageTypes_READ_PARAMETERS;
                I3_LOG(LOG_MASK_PARAMS, "read params, Too many for one.");
            }
        }

        // here this could be the first response or a continued response.
        if (sCr_requested_param_read_count == 0)
        {
            if (request != NULL)
            {   // first time
                crcb_parameter_discover_reset(0);
                pvtCr_num_remaining_objects = crcb_parameter_get_count();
                // default on first.
                pvtCr_continued_message_type = cr_ReachMessageTypes_READ_PARAMETERS;
            }
            response->values_count = 0;
            for (int i=0; i<REACH_COUNT_PARAM_READ_VALUES; i++) 
            {
                // Would use less stack if we got a pointer into flash instead of the actual data.
                // But that makes other calls more complicated. 
                cr_ParameterInfo paramInfo;
                rval = crcb_parameter_discover_next(&paramInfo);
                if (rval != cr_ErrorCodes_NO_ERROR) 
                {   // there are no more params.  clear on last.
                    pvtCr_num_remaining_objects = 0;
                    if (i==0)
                    {
                        I3_LOG(LOG_MASK_PARAMS, "No read data on i=0.");
                        pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                        return cr_ErrorCodes_NO_DATA; 
                    }
                    I3_LOG(LOG_MASK_PARAMS, "Added read %d.", response->values_count);
                    return 0;
                }
                crcb_parameter_read(paramInfo.id, &response->values[i]);
                I3_LOG(LOG_MASK_PARAMS, "Add param read %d.", sCr_requested_param_index);
                sCr_requested_param_index++;
                pvtCr_num_remaining_objects--;
                response->values_count++;
            }
            if (response->values_count == 0)
            {
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                return cr_ErrorCodes_NO_DATA; 
            }
            I3_LOG(LOG_MASK_PARAMS, "Read added %d.", response->values_count);
            return 0;
        }

        // we are supplied a list of params.
        response->values_count = 0;
        for (int i=0; i<REACH_COUNT_PARAM_READ_VALUES; i++)
        {
            affirm(sCr_requested_param_index < REACH_PARAM_BUFFER_COUNT);
            if (sCr_requested_param_index >= sCr_requested_param_read_count) {
                // we've done them all.
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                break;
            }
            if (sCr_requested_param_array[sCr_requested_param_index] < 0) {
                // we've done them all.
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                break;
            }
            I3_LOG(LOG_MASK_PARAMS, "Read param %d from list of %d", 
                   sCr_requested_param_index, sCr_requested_param_read_count);
            cr_ParameterValue paramVal;
            rval = crcb_parameter_read(sCr_requested_param_array[sCr_requested_param_index], &paramVal);
            if (rval == cr_ErrorCodes_INVALID_PARAMETER) {
                I3_LOG(LOG_MASK_ERROR, "crcb_parameter_read(pid %d) returned %d, INVALID_PARAMETER.", 
                          sCr_requested_param_array[sCr_requested_param_index], rval);
                cr_report_error(rval, "pid %d is not valid.", 
                                sCr_requested_param_array[sCr_requested_param_index]);

                memset(&paramVal, 0, sizeof(paramVal));
                paramVal.parameter_id = sCr_requested_param_array[sCr_requested_param_index];
            }
            else if (rval != cr_ErrorCodes_NO_ERROR) {
                cr_report_error(rval, "pid %d is not valid, ret %d.", 
                                sCr_requested_param_array[sCr_requested_param_index], rval);
                I3_LOG(LOG_MASK_ERROR, "crcb_parameter_read(pid %d) returned %d.",
                          sCr_requested_param_array[sCr_requested_param_index], rval);
                memset(&paramVal, 0, sizeof(paramVal));
                paramVal.parameter_id = sCr_requested_param_array[sCr_requested_param_index];
            }
            response->values[i] = paramVal;
            sCr_requested_param_array[sCr_requested_param_index] = -1;
            sCr_requested_param_index++;
            pvtCr_num_remaining_objects--;
            response->values_count++;
        }

        if (response->values_count == 0)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA; 
        }
        return 0;
    }

    int pvtCrParam_write_param(const cr_ParameterWrite *request,
                               cr_ParameterWriteResponse *response) 
    {
        if (!crcb_challenge_key_is_valid()) {
            pvtCr_num_remaining_objects = 0;
            memset(response, 0, sizeof(cr_ParameterWriteResponse));
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA; 
        }

        int rval;
        affirm(request);
        affirm(response);
        response->result = 0;
        switch (request->values_count)
        {
        case 1: // see NUM_PARAM_WRITE_IN_REQUEST
        case 2:
        case 3:
        case 4:
            break;
        default:
            cr_report_error(cr_ErrorCodes_INVALID_PARAMETER, "Invalid values_count param write.");
            return cr_ErrorCodes_INVALID_PARAMETER; 
        }

        // we are supplied a list of params.
        for (int i=0; i<request->values_count; i++)
        {
            I3_LOG(LOG_MASK_PARAMS, "%s(): Write param[%d] id %d", __FUNCTION__, i, request->values[i].parameter_id);
            rval = crcb_parameter_write(request->values[i].parameter_id, &request->values[i]);
            if (rval != cr_ErrorCodes_NO_ERROR) {
                cr_report_error(cr_ErrorCodes_WRITE_FAILED, "Parameter write of ID %d failed.", request->values[i].parameter_id);
                return cr_ErrorCodes_WRITE_FAILED;
            }
        }
        return 0;
    }

  #if NUM_SUPPORTED_PARAM_NOTIFY != 0

    static int sParameter_notification_get_by_pid(uint32_t pid, cr_ParameterNotifyConfig **ppConfig)
    {
        if (pid == 0)
        {
            *ppConfig = NULL;
            return cr_ErrorCodes_INVALID_PARAMETER;
        }
        for (int i=0; i<NUM_SUPPORTED_PARAM_NOTIFY; i++)
        {
            if (sCr_param_notify_list[i].parameter_id == pid)
            {
                *ppConfig = &sCr_param_notify_list[i];
                return cr_ErrorCodes_NO_ERROR;
            }
        }
        *ppConfig = NULL;
        return cr_ErrorCodes_INVALID_PARAMETER;
    }

    //
    void cr_get_notification_statistics(uint32_t *numActive, uint32_t *numSent)
    {
      #if NUM_SUPPORTED_PARAM_NOTIFY == 0
        *numActive = 0; 
        *numSent   = 0;
        return;
      #else
        uint32_t active = 0;
        for (int i=0; i<NUM_SUPPORTED_PARAM_NOTIFY; i++)
        {
            if (sCr_param_notify_list[i].enabled)
                active++;
        }
        *numActive = active; 
        *numSent = sCr_numNotificationsSent;
        sCr_numNotificationsSent = 0;
      #endif  // NUM_SUPPORTED_PARAM_NOTIFY == 0
    }

    ///  Private helper function to discover current parameter
    ///  notifications
    int pvtCrParam_discover_notifications(const cr_DiscoverParameterNotifySetup *request,
                                          cr_DiscoverParameterNotifySetupResponse *response)
    {
        if (!crcb_challenge_key_is_valid()) {
            sCr_requested_notify_count = 0;
            pvtCr_num_remaining_objects = response->configs_count = 0;
            return cr_ErrorCodes_NO_DATA;
        }

        int rval;
        cr_ParameterNotifyConfig *pConfig;
        cr_ParameterInfo paramInfo;

        // init them all to 0 meaning invalid.
        memset(response, 0, sizeof(cr_DiscoverParameterNotifySetupResponse));

        if (request != NULL) 
        {
            // request will be null on repeated calls.
            // Here implies we are responding to the initial request.
            // set up to call next().

            sCr_requested_notify_index = 0;
            if (request->parameter_ids_count != 0) 
            {
                // some specific numbers are requested.  Remember them.
                affirm(request->parameter_ids_count<= REACH_COUNT_PARAM_IDS);
                for (int i=0; i<request->parameter_ids_count; i++)
                    sCr_requested_param_array[i] = request->parameter_ids[i];
                sCr_requested_notify_count = request->parameter_ids_count;
                sCr_requested_notify_index = 0;
                I3_LOG(LOG_MASK_PARAMS, "%s, partial notification count %d.", 
                       __FUNCTION__, sCr_requested_notify_count);

                // copy the requested numbers
                for (int i=0; i < REACH_PARAM_NOTE_SETUP_COUNT; i++) {
                    if (sCr_requested_notify_index >= sCr_requested_notify_count)
                        break;
                    sParameter_notification_get_by_pid(
                        sCr_requested_param_array[sCr_requested_notify_index],
                        &pConfig);
                    if (pConfig)
                    {
                        memcpy(&response->configs[sCr_requested_notify_index],
                               pConfig, sizeof(cr_ParameterNotifyConfig));
                        I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d IS notifying.", __FUNCTION__, 
                                        sCr_requested_param_array[sCr_requested_notify_index]);
                    }
                    else 
                    {
                        response->configs[sCr_requested_notify_index].parameter_id = 
                            sCr_requested_param_array[sCr_requested_notify_index];
                        I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d not notifying.", __FUNCTION__, 
                                        sCr_requested_param_array[sCr_requested_notify_index]);
                    }
                    sCr_requested_notify_index++;
                }
                I3_LOG(LOG_MASK_PARAMS, "Filled first %d of %d requested notifications.", 
                       sCr_requested_notify_index, sCr_requested_notify_count);
                pvtCr_num_remaining_objects = sCr_requested_notify_count - sCr_requested_notify_index;

            }
            else
            {
                // count is zero, so setup for all
                sCr_requested_notify_count = NUM_PARAMS;
                sCr_requested_notify_index = 0;
                I3_LOG(LOG_MASK_PARAMS, "%s, full notification count %d.", 
                       __FUNCTION__, sCr_requested_notify_count);

                crcb_parameter_discover_reset(-1);

                // copy the requested numbers
                for (int i=0; i < REACH_PARAM_NOTE_SETUP_COUNT; i++) {
                    rval = crcb_parameter_discover_next(&paramInfo);
                    if (rval != 0)
                    {
                        // Normal end of the parameter list.
                        break;
                    }
                    sParameter_notification_get_by_pid(paramInfo.id, &pConfig);
                    if (pConfig)
                    {
                        memcpy(&response->configs[sCr_requested_notify_index],
                               pConfig, sizeof(cr_ParameterNotifyConfig));
                        I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d IS notifying.", __FUNCTION__, paramInfo.id);

                    }
                    else 
                    {
                        response->configs[sCr_requested_notify_index].parameter_id = paramInfo.id;
                        I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d not notifying.", __FUNCTION__, paramInfo.id);
                    }
                    sCr_requested_notify_index++;
                }
                I3_LOG(LOG_MASK_PARAMS, "Filled first %d of %d all notifications.", 
                       sCr_requested_notify_index, sCr_requested_notify_count);
                pvtCr_num_remaining_objects = sCr_requested_notify_count - sCr_requested_notify_index;

            }
            if (pvtCr_num_remaining_objects == 0)
            {
                pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
                return 0;
            }
            pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS;
            return 0;
        }

        // here this is a continued response.
        if (sCr_requested_notify_count != NUM_PARAMS)
        {
            // copy the requested numbers
            for (int i=0; i < REACH_PARAM_NOTE_SETUP_COUNT; i++) {
                if (sCr_requested_notify_index >= sCr_requested_notify_count)
                    break;

                sParameter_notification_get_by_pid(
                    sCr_requested_param_array[sCr_requested_notify_index],
                    &pConfig);
                if (pConfig)
                {
                    memcpy(&response->configs[sCr_requested_notify_index],
                           pConfig, sizeof(cr_ParameterNotifyConfig));
                    I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d IS notifying.", __FUNCTION__, 
                                    sCr_requested_param_array[sCr_requested_notify_index]);
                }
                else 
                {
                    response->configs[sCr_requested_notify_index].parameter_id = 
                        sCr_requested_param_array[sCr_requested_notify_index];
                    I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d not notifying.", __FUNCTION__, 
                                    sCr_requested_param_array[sCr_requested_notify_index]);
                }
                sCr_requested_notify_index++;
            }
            I3_LOG(LOG_MASK_PARAMS, "Filled next %d of %d requested notifications.", 
                   sCr_requested_notify_index, sCr_requested_notify_count);
            pvtCr_num_remaining_objects = sCr_requested_notify_count - sCr_requested_notify_index;

        }
        else {
            // Get the PID of the next param
            for (int i=0; i < REACH_PARAM_NOTE_SETUP_COUNT; i++) {
                rval = crcb_parameter_discover_next(&paramInfo);
                if (rval != 0)
                {
                    // Normal end of the parameter list.
                    break;
                }
                sParameter_notification_get_by_pid(paramInfo.id, &pConfig);
                if (pConfig)
                {
                    memcpy(&response->configs[sCr_requested_notify_index],
                           pConfig, sizeof(cr_ParameterNotifyConfig));
                    I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d IS notifying.", __FUNCTION__, paramInfo.id);

                }
                else 
                {
                    response->configs[sCr_requested_notify_index].parameter_id = paramInfo.id;
                    I3_LOG(LOG_MASK_PARAMS, "%s: Param ID %d not notifying.", __FUNCTION__, paramInfo.id);
                }
                sCr_requested_notify_index++;
            }
            I3_LOG(LOG_MASK_PARAMS, "Filled next %d of %d all notifications.", 
                   sCr_requested_notify_index, sCr_requested_notify_count);
            pvtCr_num_remaining_objects = sCr_requested_notify_count - sCr_requested_notify_index;
        }

        if (pvtCr_num_remaining_objects == 0)
        {
            pvtCr_continued_message_type = cr_ReachMessageTypes_INVALID;
            return cr_ErrorCodes_NO_DATA;
        }
        pvtCr_continued_message_type = cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS;
        return 0;
    }

    static cr_ParameterNotifyConfig sCr_param_notify_list[NUM_SUPPORTED_PARAM_NOTIFY];

    int pvtCrParam_config_param_notify(const cr_ParameterNotifyConfig *pnc,
                                       cr_ParameterNotifyConfigResponse *pncr)
    {
        int idx;

        if (!pnc->enabled) 
        {
            bool disabledOne = false;
            // try to disable.
            for (idx=0; idx<NUM_SUPPORTED_PARAM_NOTIFY; idx++ ) {
                if (pnc->parameter_id == sCr_param_notify_list[idx].parameter_id)
                {
                    sCr_param_notify_list[idx].enabled = false;
                    i3_log(LOG_MASK_PARAMS, "Disabled notification %d on PID %d", idx, pnc->parameter_id);
                    pncr->result = cr_ErrorCodes_NO_ERROR;
                    disabledOne = true;
                    // Don't return, check for others
                }
            }
            if (!disabledOne) {
                // No enabled match found
                pncr->result = cr_ErrorCodes_NO_ERROR;
                i3_log(LOG_MASK_WARN, "Requested disable of notify on %d, but not enabled.", 
                       pnc->parameter_id);
            }
            return cr_ErrorCodes_NO_ERROR;
        }

        // reject enable on non-existing PID's.
        int rval = crcb_parameter_discover_reset(pnc->parameter_id);
        if (rval != cr_ErrorCodes_NO_ERROR) {
            cr_report_error(cr_ErrorCodes_INVALID_PARAMETER, "Notificaiton: PID %d not found.", 
                            pnc->parameter_id);
            pncr->result = cr_ErrorCodes_INVALID_PARAMETER;
            return cr_ErrorCodes_INVALID_PARAMETER;
        }

        // see if an active notification already exists
        for (idx=0; idx<NUM_SUPPORTED_PARAM_NOTIFY; idx++ ) {
            if (!sCr_param_notify_list[idx].enabled)
                continue;
            if (pnc->parameter_id == sCr_param_notify_list[idx].parameter_id) {
                sCr_param_notify_list[idx] = *pnc;
                // store the index of the param with this PID.
                i3_log(LOG_MASK_PARAMS, "Updated notification %d on PID %d", idx, pnc->parameter_id);
                pncr->result = cr_ErrorCodes_NO_ERROR;
                return cr_ErrorCodes_NO_ERROR;
            }
        }

        // Find an open entry
        for (idx=0; idx<NUM_SUPPORTED_PARAM_NOTIFY; idx++ ) {
            if (!sCr_param_notify_list[idx].enabled)
                break;
        }
        if (idx >= NUM_SUPPORTED_PARAM_NOTIFY) {
            // All notifications are in use.  
            pncr->result = cr_ErrorCodes_NO_RESOURCE;
            cr_report_error(cr_ErrorCodes_NO_RESOURCE, "No notificaiton slot available for PID %d.", pnc->parameter_id);
            return cr_ErrorCodes_NO_RESOURCE;
        }
        sCr_param_notify_list[idx] = *pnc;
        // store the index of the param with this PID.
        i3_log(LOG_MASK_PARAMS, "Enabled notification %d on PID %d", idx, pnc->parameter_id);
        pncr->result = cr_ErrorCodes_NO_ERROR;
        return cr_ErrorCodes_NO_ERROR;
    }
  #else
    ///  Private helper function to discover current parameter
    ///  notifications
    int pvtCrParam_discover_notifications(const cr_DiscoverParameterNotifySetup *request,
                                          cr_DiscoverParameterNotifySetupResponse *response)
    {
        (void)request;
        sCr_requested_notify_count = 0;
        pvtCr_num_remaining_objects = response->configs_count = 0;
        return cr_ErrorCodes_NO_DATA;
    } 
  #endif // NUM_SUPPORTED_PARAM_NOTIFY != 0

    ///  Private helper function to initialize parameter notificaitons.
    ///  Calls crcb_parameter_notification_init()
    void  cr_init_param_notifications(void)
    {
      #if NUM_SUPPORTED_PARAM_NOTIFY == 0
        cr_clear_param_notifications();
        I3_LOG(LOG_MASK_WARN, "No notifications are supported.");
      #else
        int rval;
        const cr_ParameterNotifyConfig *pNoteArray;
        size_t num;
        crcb_parameter_notification_init(&pNoteArray, &num);

        I3_LOG(LOG_MASK_ALWAYS, "%d Notifications Enabled", num);
        if (num > NUM_SUPPORTED_PARAM_NOTIFY)
        {
            cr_report_error(cr_ErrorCodes_INVALID_PARAMETER, "Not enough notifications slots (%d) for init (%d).\n",
                            NUM_SUPPORTED_PARAM_NOTIFY, num);
            return;
        }

        for (size_t i=0; i<num; i++)
        {
            cr_ParameterInfo paramInfo;
            sCr_param_notify_list[i].parameter_id = pNoteArray[i].parameter_id;
            sCr_param_notify_list[i].enabled = true;
            sCr_param_notify_list[i].minimum_notification_period = pNoteArray[i].minimum_notification_period;
            sCr_param_notify_list[i].maximum_notification_period = pNoteArray[i].maximum_notification_period;
            sCr_param_notify_list[i].minimum_delta = pNoteArray[i].minimum_delta;
            sCr_last_param_values[i].parameter_id = pNoteArray[i].parameter_id;
            sCr_last_param_values[i].timestamp = 0;
            rval = crcb_parameter_discover_reset(pNoteArray[i].parameter_id);
            if (rval != 0)
            {
                cr_report_error(cr_ErrorCodes_INVALID_PARAMETER, "PID %d doesn't exist for notify[%d].\n",
                                pNoteArray[i].parameter_id, i);
                continue;  // try to do the other ones.
            }
            // I don't see how this can fail right after discover.
            crcb_parameter_discover_next(&paramInfo);
            sCr_last_param_values[i].which_value = paramInfo.data_type;
            sCr_last_param_values[i].value.sint32_value = 0;
        }
        return;
      #endif
    }
#endif // def INCLUDE_PARAMETER_SERVICE



/// <summary>
/// clears any stale notifications. 
/// To be called on connection to client 
/// Must be available (empty) in all no-param case. 
/// </summary>
void cr_clear_param_notifications(void)
{
#if (defined(INCLUDE_PARAMETER_SERVICE) && (NUM_SUPPORTED_PARAM_NOTIFY != 0) )
    memset(sCr_param_notify_list, 0, sizeof(sCr_param_notify_list));
    memset(sCr_last_param_values, 0, sizeof(sCr_last_param_values));
  #endif
}

/// <summary>
/// A local function called in cr_process() to determine whether
/// any parameter notifications need to be generated. 
/// Must be available (empty) in all no-param case. 
/// </summary>
void pvtCrParam_check_for_notifications()
{
  #if (defined(INCLUDE_PARAMETER_SERVICE) && (NUM_SUPPORTED_PARAM_NOTIFY != 0) )

    for (int idx=0; idx<NUM_SUPPORTED_PARAM_NOTIFY; idx++ )
    {
        if (!sCr_param_notify_list[idx].enabled)
            continue;

        cr_ParameterValue curVal;
        float delta;
        bool needToNotify = false;
        bool checkedDelta = false;
        uint32_t  timeSinceLastNotify = cr_get_current_ticks() - sCr_last_param_values[idx].timestamp;

        // 0 will cause this to be ignored.
        if (timeSinceLastNotify < sCr_param_notify_list[idx].minimum_notification_period)
            continue;

        // 0 will cause this to be ignored.
        if ((sCr_param_notify_list[idx].maximum_notification_period != 0) &&
            (timeSinceLastNotify > sCr_param_notify_list[idx].maximum_notification_period))
            needToNotify = true;

        crcb_parameter_read(sCr_param_notify_list[idx].parameter_id, &curVal);
        switch (curVal.which_value) {
        // To match the apps and protobufs, must use _value_tags!
        case cr_ParameterValue_uint32_value_tag:
        case cr_ParameterValue_enum_value_tag:
        case cr_ParameterValue_bitfield_value_tag:
        {
            int64_t cur  = curVal.value.uint32_value;
            int64_t last = sCr_last_param_values[idx].value.uint32_value;
            if (cur>last)
              delta = cur-last;
            else
              delta = last-cur;
            checkedDelta = true;
            break;
        }
        case cr_ParameterValue_sint32_value_tag:
            delta = abs(curVal.value.sint32_value - sCr_last_param_values[idx].value.sint32_value);
            checkedDelta = true;
            break;
        case cr_ParameterValue_float32_value_tag:
            delta = fabs(curVal.value.float32_value - sCr_last_param_values[idx].value.float32_value);
            checkedDelta = true;
            break;
        case cr_ParameterValue_uint64_value_tag:
          {
          int64_t cur  = curVal.value.uint64_value;
          int64_t last = sCr_last_param_values[idx].value.uint64_value;
          if (cur>last)
            delta = (float)(cur-last);
          else
            delta = (float)(last-cur);
          checkedDelta = true;
          break;
          }
        case cr_ParameterValue_sint64_value_tag:
          {
            int64_t cur  = curVal.value.sint64_value;
            int64_t last = sCr_last_param_values[idx].value.sint64_value;
            if (cur>last)
              delta = (float)(cur-last);
            else
              delta = (float)(last-cur);
            checkedDelta = true;
            break;
          }
        case cr_ParameterValue_float64_value_tag:
            delta = fabs(curVal.value.float64_value - sCr_last_param_values[idx].value.float64_value);
            checkedDelta = true;
            break;
        case cr_ParameterValue_bool_value_tag:
            delta = abs(curVal.value.bool_value - sCr_last_param_values[idx].value.bool_value);
            checkedDelta = true;
            break;
        case cr_ParameterValue_string_value_tag:
        case cr_ParameterValue_bytes_value_tag:
        default:
            checkedDelta = false;
            break;
        }
        if (checkedDelta && (delta >= sCr_param_notify_list[idx].minimum_delta))
        {
            i3_log(LOG_MASK_PARAMS, TEXT_MAGENTA "Notify PID %d on delta %.1f" TEXT_RESET,
                   sCr_param_notify_list[idx].parameter_id, delta);
            needToNotify = true;
        }

        if (curVal.which_value == cr_ParameterValue_string_value_tag) {
            if (strncmp(curVal.value.string_value, sCr_last_param_values[idx].value.string_value, REACH_PVAL_STRING_LEN))
                needToNotify = true;
        }
        if (curVal.which_value == cr_ParameterValue_bytes_value_tag) {
            if (memcmp(curVal.value.bytes_value.bytes, 
                       sCr_last_param_values[idx].value.bytes_value.bytes, 
                       curVal.value.bytes_value.size))
                needToNotify = true;
        }

        if ((sCr_param_notify_list[idx].maximum_notification_period !=0) &&
            (timeSinceLastNotify > sCr_param_notify_list[idx].maximum_notification_period) )
        {
            i3_log(LOG_MASK_PARAMS, TEXT_MAGENTA "Notify PID %d on max period" TEXT_RESET,
                   sCr_param_notify_list[idx].parameter_id);
            needToNotify = true;
        }

        if (needToNotify)
        {
            sCr_numNotificationsSent++;
            crcb_notify_param(&curVal);

            // save it for next time
            sCr_last_param_values[idx] = curVal;
            sCr_last_param_values[idx].timestamp = cr_get_current_ticks();
        }
    }
  #endif  // NUM_SUPPORTED_PARAM_NOTIFY != 0
}


