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
 * "cr_private.h" defines things internal to the Reach stack.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

/**
 * @file      cr_provate.h
 * @brief     defines things internal to the Reach stack.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
 */

#ifndef _CR_PRIVATE_H
#define _CR_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "reach-server.h"
#include "cr_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// <summary>
    /// Private variables controlling the sort of continuing 
    /// transactions used by file transfers, etc. 
    /// </summary>
    extern cr_ReachMessageTypes pvtCr_continued_message_type;
    extern uint32_t             pvtCr_num_continued_objects;
    extern uint32_t             pvtCr_num_remaining_objects;

    /// <summary>
    /// Returns the state of the challenge key which may block 
    /// access to the Reach interface 
    /// </summary>
    bool pvtCr_challenge_key_is_valid(void);

    ///  
    /// pvtCrFile_ functions support the (optional) files service. 
    ///  
    int pvtCrFile_discover(const cr_DiscoverFiles *request,
                              cr_DiscoverFilesReply *response);
    int pvtCrFile_transfer_init(const cr_FileTransferInit *request,
                             cr_FileTransferInitReply *response);
    int pvtCrFile_transfer_data(const cr_FileTransferData *dataTransfer,
                             cr_FileTransferDataNotification *response);
    int pvtCrFile_transfer_data_notification(const cr_FileTransferDataNotification *request,
                                             cr_FileTransferData *dataTransfer);

    /// <summary>
    /// The file service includes a timeout Watchdog. 
    /// 0 ms disables watchdog. 
    /// </summary> 
    void pvtCr_watchdog_start_timeout(uint32_t msec, uint32_t ticks);

    // resets the timeout period to original
    void pvtCr_watchdog_stroke_timeout(uint32_t ticks);

    // disables the watchdog
    void pvtCr_watchdog_end_timeout();

    // if active, compares ticks to expected timeout.
    // return 1 if timeout occurred
    int pvtCr_watchdog_check_timeout(uint32_t ticks);

    ///  
    /// pvtCrParam_ functions support the (optional) parameters 
    /// service. 
    ///  
    void pvtCrParam_clear_notifications(void);

    int pvtCrParam_discover_parameters(const cr_ParameterInfoRequest *,
                                       cr_ParameterInfoResponse *);
    int pvtCrParam_discover_parameters_ex(const cr_ParameterInfoRequest *,
                                          cr_ParamExInfoResponse *);
    int pvtCrParam_read_param(const cr_ParameterRead *, 
                              cr_ParameterReadResult *);
    int pvtCrParam_write_param(const cr_ParameterWrite *, 
                               cr_ParameterWriteResult *);
  #if NUM_SUPPORTED_PARAM_NOTIFY != 0
    int pvtCrParam_config_param_notify(const cr_ParameterNotifyConfig *,
                                       cr_ParameterNotifyConfigResult *);
  #endif // NUM_SUPPORTED_PARAM_NOTIFY != 0
    
    void pvtCrParam_check_for_notifications(void);


#ifdef __cplusplus
}
#endif

#endif  // ndef _CR_PRIVATE_H

