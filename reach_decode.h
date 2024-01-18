/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023
 *
 *
 * @author  i3 Product Development
 *          Wei Zhou wei.zhou@i3pd.com
 *
 * @version 0.0.1
 * @date    2023-08-22
 *
 ********************************************************************************************/

/**
 * @file      reach_decode.h
 * @brief     Support for protobuf decoding.
 * 
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 */


#ifndef __REACH_DECODE_H__
#define __REACH_DECODE_H__

#include "reach.pb.h"
#include <stdint.h>

bool decode_reach_payload(cr_ReachMessageTypes message_type,  // in:  from the header
                          void *dec_data,                     // out: decode to here.
                          const uint8_t *in_buffer,           // in:  encoded from the header
                          size_t in_size);                    // in:  encoded size

bool decode_reach_message(cr_ReachMessage *message,           // out: decoded
                          const uint8_t *in_buffer,           // in:  encoded
                          size_t in_size);                    // in:  encoded size

uint32_t cr_get_transaction_id();

#endif /* __REACH_DECODE_H__ */