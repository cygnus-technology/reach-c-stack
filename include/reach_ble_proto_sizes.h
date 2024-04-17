// (c) 2023 i3 Product Design, All Rights Reserved
//          i3pd.com, a Helios company.
// 
// reach_ble_proto_sizes.h

/**
 * @file      reach_ble_proto_sizes.h
 * @brief     This file contains structure sizes defining reach.options for BLE 
 *            usage These sizes are based on the 240 byte BLE packetand are
 *            tuned to fit in all cases. Hence there is no need for a separate
 *            packetizing layer. These values are used to populate reach.options
 *            for nanopb. cr_stack.h uses these to define more specific buffer
 *            sizes. If you have a reason to change these, consider the effect
 *            in cr_stack.h, where you could use a different common size and
 *            reach.options.prototype and the DeviceInfoResponse message in
 *            reach.proto.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 */

#ifndef _REACH_BLE_PROTO_SIZES_H
#define _REACH_BLE_PROTO_SIZES_H

// size with headers added, due to BLE.
#define REACH_MAX_RESPONSE_SIZE               244

// payload before header is added
#define REACH_MESSAGE_PAYLOAD_MAX             208
#define REACH_BIG_DATA_BUFFER_LEN             194
#define REACH_DESCRIPTION_LEN                  48
#define REACH_LONG_STRING_LEN                  32 
#define REACH_COUNT_PARAM_IDS                  32
#define REACH_MEDIUM_STRING_LEN                24 
#define REACH_SHORT_STRING_LEN                 16
#define REACH_PARAM_INFO_ENUM_COUNT            12
#define REACH_SERVICES_COUNT                    8
#define REACH_NUM_COMMANDS_IN_RESPONSE          2
#define REACH_NUM_SMALL_STRUCTS_IN_MESSAGE      8
#define REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE     4
#define REACH_COUNT_PARAM_DESC_IN_RESPONSE      2

// These specific sizes and counts are defined in terms of a lesser number
// of generic macros which are used in the reach.options file to set 
// static buffer sizes via nanopb.
// The lesser number of general sizes are also exposed in the device info.
#define REACH_ERROR_BUFFER_LEN                  REACH_BIG_DATA_BUFFER_LEN
#define REACH_PING_ECHO_LEN                     REACH_BIG_DATA_BUFFER_LEN
#define REACH_BYTES_IN_A_FILE_PACKET            REACH_BIG_DATA_BUFFER_LEN
#define REACH_BYTES_IN_AN_ERROR_MSG             REACH_BIG_DATA_BUFFER_LEN
#define REACH_STREAM_DATA_LEN                   REACH_BIG_DATA_BUFFER_LEN
#define REACH_COMMAND_RES_LEN                   REACH_BIG_DATA_BUFFER_LEN
#define REACH_CLI_MSG_LEN                       REACH_BIG_DATA_BUFFER_LEN
#define REACH_COUNT_PARAMS_IN_REQUEST           REACH_COUNT_PARAM_IDS
#define REACH_COUNT_FAILED_PARAM_IDS            REACH_COUNT_PARAM_IDS
#define REACH_PVAL_STRING_LEN                   REACH_LONG_STRING_LEN  
#define REACH_PVAL_BYTES_LEN                    REACH_LONG_STRING_LEN  
#define REACH_PARAM_INFO_DESCRIPTION_LEN        REACH_LONG_STRING_LEN 
#define REACH_DEVICE_NAME_LEN                   REACH_MEDIUM_STRING_LEN
#define REACH_MANUFACTURER_NAME_LEN             REACH_MEDIUM_STRING_LEN
#define REACH_PARAM_INFO_NAME_LEN               REACH_MEDIUM_STRING_LEN 
#define REACH_FILE_NAME_LEN                     REACH_MEDIUM_STRING_LEN 
#define REACH_STREAM_NAME_LEN                   REACH_MEDIUM_STRING_LEN 
#define REACH_COMMAND_NAME_LEN                  REACH_MEDIUM_STRING_LEN 
#define REACH_PARAM_INFO_UNITS_LEN              REACH_SHORT_STRING_LEN 
#define REACH_FW_VERSION_LEN                    REACH_SHORT_STRING_LEN
#define REACH_UUID_BYTE_LEN                     16 // this is constant.
#define REACH_PI_ENUM_NAME_LEN                  REACH_SHORT_STRING_LEN
#define REACH_PI_EX_ENUM_COUNT                  REACH_NUM_SMALL_STRUCTS_IN_MESSAGE
#define REACH_PARAM_NOTE_SETUP_COUNT            REACH_NUM_SMALL_STRUCTS_IN_MESSAGE
#define REACH_COUNT_PARAM_READ_VALUES           REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE
#define REACH_DISCOVER_STREAM_COUNT             REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE
#define REACH_COUNT_PARAM_WRITE_IN_REQUEST      REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE
#define REACH_COUNT_PARAM_NOTIF_VALUES          REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE
#define REACH_DISCOVER_FILES_COUNT              REACH_NUM_MEDIUM_STRUCTS_IN_MESSAGE
#define REACH_NUM_PARAM_BYTES                   32
    
// REACH_SIZE_STRUCT_SIZE must match the size of the reach_sizes_t defined in cr_stack.h
#define REACH_SIZE_STRUCT_SIZE      16    

#endif  // ndef _REACH_BLE_PROTO_SIZES_H
