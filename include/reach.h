/**
 * @file      reach.h
 * @brief     This file an annotated copy of reach.pb.h which is generated from reach.proto. 
 *            reach.pb.h defines the protobuf structures in C. 
 *            This version is hand edited to provide Doxygen comments.
 *             See reach.pb.h for the latest.
 * @date      2024-04-02
 * @copyright (c) Copyright 2023-2024 i3 Product Development. 
 * All Rights Reserved. The Cygngus Reach firmware stack is 
 * shared under an MIT license. 
 */

#ifndef PB_CR_REACH_PB_H_INCLUDED
#define PB_CR_REACH_PB_H_INCLUDED
#include <pb.h>
#include "sys/types.h"

#error reach.h should not be included.  Use reach.ph.h.

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
/** The major version generally changes to signal a break in compatibility */
typedef enum _cr_ReachProto_MAJOR_Version {
    /* MAJOR_V0       = 0;   // Must have a zero */
    cr_ReachProto_MAJOR_Version_MAJOR_VERSION = 0
} cr_ReachProto_MAJOR_Version;

/** The minor version changes at a release which is not completely compatible */
typedef enum _cr_ReachProto_MINOR_Version {
    cr_ReachProto_MINOR_Version_MINOR_V0 = 0, /**< Must have a zero */
    cr_ReachProto_MINOR_Version_MINOR_VERSION = 2 /**< Update at a release or a big change */
} cr_ReachProto_MINOR_Version;

/** The patch version changes every time a hex file goes out the door. */
typedef enum _cr_ReachProto_PATCH_Version {
    cr_ReachProto_PATCH_Version_PATCH_V0 = 0, /**< Must have a zero */
    cr_ReachProto_PATCH_Version_PATCH_VERSION = 2 /**< Update when something changes */
} cr_ReachProto_PATCH_Version;

/** These values identify the type of the Reach message. */
typedef enum _cr_ReachMessageTypes {
    cr_ReachMessageTypes_INVALID = 0, /**< No Valid Message */
    cr_ReachMessageTypes_ERROR_REPORT = 1, /**< Inform client of an error condition */
    cr_ReachMessageTypes_PING = 2, /**< Test the link to the server */
    cr_ReachMessageTypes_GET_DEVICE_INFO = 3, /**< Open the communication with a server */
    /** Parameters */
    cr_ReachMessageTypes_DISCOVER_PARAMETERS = 5, /**< Get a list of all of the parameters in the repository */
    cr_ReachMessageTypes_DISCOVER_PARAM_EX = 6, /**< An extension to discover used by verbose parameters */
    cr_ReachMessageTypes_READ_PARAMETERS = 7, /**< Get the values of a set of parameters */
    cr_ReachMessageTypes_WRITE_PARAMETERS = 8, /**< Change the values of a set of parameters */
    cr_ReachMessageTypes_PARAMETER_NOTIFICATION = 10, /**< Sent from the server to the client when a parameter has changed */
    cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS = 11, /**< Find out how notifications are setup */
    cr_ReachMessageTypes_PARAM_ENABLE_NOTIFY = 50, /**< setup parameter notifications */
    cr_ReachMessageTypes_PARAM_DISABLE_NOTIFY = 51, /**< disable parameter notifications */
    /** File Transfers */
    cr_ReachMessageTypes_DISCOVER_FILES = 12, /**< Get a list of supported files */
    cr_ReachMessageTypes_TRANSFER_INIT = 13, /**< Begin a file transfer */
    cr_ReachMessageTypes_TRANSFER_DATA = 14, /**< (bi-directional) sends the requested data */
    cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION = 15, /**< (bi-directional) Clears Sender to Send More Data */
    cr_ReachMessageTypes_ERASE_FILE = 16, /**< Set file size to zero. */
    /** Commands */
    cr_ReachMessageTypes_DISCOVER_COMMANDS = 17, /**< Get a list of supported commands */
    cr_ReachMessageTypes_SEND_COMMAND = 18, /**< Reqeuest excecution of a command */
    /* Command Line Interface */
    cr_ReachMessageTypes_CLI_NOTIFICATION = 20, /**< Inform the other side (bi-directional) of a command line. */
    /* Streams */
    cr_ReachMessageTypes_DISCOVER_STREAMS = 25, /**< Get a list of supported streams */
    cr_ReachMessageTypes_OPEN_STREAM = 26, /**< Open a stream */
    cr_ReachMessageTypes_CLOSE_STREAM = 27, /**< Close a stream */
    cr_ReachMessageTypes_STREAM_DATA_NOTIFICATION = 28, /**< Inform the other side (bi-directional) of data on a stream. */
    /* Time */
    cr_ReachMessageTypes_SET_TIME = 30, /**< Set the real time clock */
    cr_ReachMessageTypes_GET_TIME = 31, /**< Read the real time clock */
    /* WiFi */
    cr_ReachMessageTypes_DISCOVER_WIFI = 40, /**< Get a list of WiFi acces points */
    cr_ReachMessageTypes_WIFI_CONNECT = 41 /**< Connect or disconnect to an access point */
} cr_ReachMessageTypes;

/** binary bit masks or'ed together into the DeviceInfoResponse.services */
typedef enum _cr_ServiceIds {
    cr_ServiceIds_NO_SVC_ID = 0, /**< No services.  Device info service is required. */
    cr_ServiceIds_PARAMETER_REPO = 1, /**< Set this bit when the device supports the parameter service */
    cr_ServiceIds_FILES = 2, /**< Set this bit when the device supports the file service */
    cr_ServiceIds_STREAMS = 4, /**< Set this bit when the device supports the stream service */
    cr_ServiceIds_COMMANDS = 8, /**< Set this bit when the device supports the command service */
    cr_ServiceIds_CLI = 16, /**< Set this bit when the device supports the command line interface */
    cr_ServiceIds_TIME = 32, /**< Set this bit when the device supports the time service */
    cr_ServiceIds_WIFI = 64 /**< Set this bit when the device supports the WiFi service */
} cr_ServiceIds;

/** binary bit masks or'ed together into the DeviceInfoResponse.endpoints */
typedef enum _cr_EndpointIds {
    cr_EndpointIds_NO_ENDPOINTS = 0, /**< No other endpoints */
    cr_EndpointIds_ONE = 1, /**< This is the first of multiple endpoints. */
    cr_EndpointIds_TWO = 2, /**< This is the second of multiple endpoints. */
    cr_EndpointIds_THREE = 4, /**< This is the third of multiple endpoints. */
    cr_EndpointIds_FOUR = 8 /**< This is the fourth of multiple endpoints. */
} cr_EndpointIds;

/** These are the types of data that can be stored as parameters. */
typedef enum _cr_ParameterDataType {
    cr_ParameterDataType_UINT32 = 0, /**< Unsigned 32 bit integer. */
    cr_ParameterDataType_INT32 = 1, /**< Signed 32 bit integer. */
    cr_ParameterDataType_FLOAT32 = 2, /**< 32 bit floating point. */
    cr_ParameterDataType_UINT64 = 3, /**< Unsigned 64 bit integer. */
    cr_ParameterDataType_INT64 = 4, /**< Signed 64 bit integer. */
    cr_ParameterDataType_FLOAT64 = 5, /**< 64 bit floating point. */
    cr_ParameterDataType_BOOL = 6, /**< boolean (0 or 1) */
    cr_ParameterDataType_STRING = 7, /**< ASCII or UTF-8. Null Terminated. */
    cr_ParameterDataType_ENUMERATION = 8, /**< An extended description gives names to 32 bit integer values. */
    cr_ParameterDataType_BIT_FIELD = 9, /**< An extended description gives names to up to 64 bit positions. */
    cr_ParameterDataType_BYTE_ARRAY = 10 /**< An array of bytes. */
} cr_ParameterDataType;

/** permissions */
typedef enum _cr_AccessLevel {
    cr_AccessLevel_NO_ACCESS = 0, /**< No permission */
    cr_AccessLevel_READ = 1, /**< Read permission */
    cr_AccessLevel_WRITE = 2, /**< Write permission */
    cr_AccessLevel_READ_WRITE = 3 /**< Read and write permission */
} cr_AccessLevel;

/** The types of memory in which parameters are stored.  RAM and NVM (Non Volatile Memory) are most common. */
typedef enum _cr_StorageLocation {
    cr_StorageLocation_STORAGE_LOCATION_INVALID = 0, /**< unknown, not used */
    cr_StorageLocation_RAM = 1, /**< RAM, volatile, not saved on reboot. */
    cr_StorageLocation_NONVOLATILE = 2, /**< Flash or some other method saved over reboot */
    cr_StorageLocation_RAM_EXTENDED = 3, /**< In case a device has two RAM locations */
    cr_StorageLocation_NONVOLATILE_EXTENDED = 4 /**< In case a device has two non volatile locations */
} cr_StorageLocation;

/** WiFi security type */
typedef enum _cr_WiFiSecurity {
    cr_WiFiSecurity_OPEN = 0, /**< No security */
    cr_WiFiSecurity_WEP = 1, /**< WEP */
    cr_WiFiSecurity_WPA = 2, /**< WPA */
    cr_WiFiSecurity_WPA2 = 3, /**< WPA2 */
    cr_WiFiSecurity_WPA3 = 4 /**< WPA3 */
} cr_WiFiSecurity;

/** WiFi Band */
typedef enum _cr_WiFiBand {
    cr_WiFiBand_NO_BAND = 0, /**< Not specified */
    cr_WiFiBand_BAND_2 = 2, /**< 2.4GHz */
    cr_WiFiBand_BAND_5 = 5 /**< 5GHz */
} cr_WiFiBand;

/** Reach uses these error codes. */
typedef enum _cr_ErrorCodes {
    cr_ErrorCodes_NO_ERROR = 0, /**< Normal conclusion, no problem. */
    cr_ErrorCodes_NO_DATA = 1, /**< No data was produced */
    cr_ErrorCodes_READ_FAILED = 2, /**< The read failed. */
    cr_ErrorCodes_WRITE_FAILED = 3, /**< The write failed. */
    cr_ErrorCodes_NOT_IMPLEMENTED = 4, /**< returned by weak implementations */
    cr_ErrorCodes_RESERVED_1 = 5, /**< not yet used */
    cr_ErrorCodes_RESERVED_2 = 6, /**< not yet used */
    cr_ErrorCodes_PERMISSION_DENIED = 7, /**< access not allowed */
    cr_ErrorCodes_BUFFER_TOO_SMALL = 8, /**< Requested a size larger than the buffer. */
    cr_ErrorCodes_INVALID_PARAMETER = 9, /**< Some function parameter is out of range */
    cr_ErrorCodes_CHECKSUM_MISMATCH = 10, /**< Received data does not match checksum */
    cr_ErrorCodes_DECODING_FAILED = 11, /**< Protobuf decoding failed */
    cr_ErrorCodes_ENCODING_FAILED = 12, /**< Protobuf encoding failed */
    cr_ErrorCodes_INVALID_STATE = 13, /**< The current state machine does not handle this case. */
    cr_ErrorCodes_NO_RESPONSE = 14, /**< handler signals no response is necessary */
    cr_ErrorCodes_BAD_FILE = 15, /**< bad file ID */
    cr_ErrorCodes_PACKET_COUNT_ERR = 16, /**< A received packet has an unexpected serial number */
    cr_ErrorCodes_CHALLENGE_FAILED = 17, /**< Access denied due to challenge key */
    cr_ErrorCodes_RESERVED_3 = 18, /**< not yet used */
    cr_ErrorCodes_NO_RESOURCE = 19, /**< Some required resource is not available. */
    cr_ErrorCodes_INVALID_ID = 20, /**< The ID provided is not valid */
    cr_ErrorCodes_ABORT = 1000 /**< Operation cancellation */
} cr_ErrorCodes;

/** Describes file transfer state machine */
typedef enum _cr_FileTransferState {
    cr_FileTransferState_FILE_TRANSFER_INVALID = 0, /**< FILE_TRANSFER_INVALID */
    cr_FileTransferState_IDLE = 1, /**< IDLE */
    cr_FileTransferState_INIT = 2, /**< INIT */
    cr_FileTransferState_DATA = 3, /**< DATA */
    cr_FileTransferState_COMPLETE = 4 /**< COMPLETE */
} cr_FileTransferState;

/** This describes the offset of each member of the BufferSizes message when packed for transmission */
typedef enum _cr_SizesOffsets {
    cr_SizesOffsets_MAX_MESSAGE_SIZE_OFFSET = 0, /**< uint16_t, little endian */
    cr_SizesOffsets_BIG_DATA_BUFFER_SIZE_OFFSET = 2, /**< uint16_t, little endian */
    cr_SizesOffsets_PARAMETER_BUFFER_COUNT_OFFSET = 4, /**< uint8_t */
    cr_SizesOffsets_NUM_PARAMS_IN_RESPONSE_OFFSET = 5, /**< uint8_t */
    cr_SizesOffsets_DESCRIPTION_LEN_OFFSET = 6, /**< uint8_t */
    cr_SizesOffsets_MAX_PARAM_BYTES_OFFSET = 7, /**< uint8_t */
    cr_SizesOffsets_PARAM_INFO_DESCRIPTION_LEN_OFFSET = 8, /**< uint8_t */
    cr_SizesOffsets_MEDIUM_STRING_LEN_OFFSET = 9, /**< uint8_t */
    cr_SizesOffsets_SHORT_STRING_LEN_OFFSET = 10, /**< uint8_t */
    cr_SizesOffsets_PARAM_NOTIFY_CONFIG_COUNT_OFFSET = 11, /**< uint8_t */
    cr_SizesOffsets_NUM_DESCRIPTORS_IN_RESPONSE_OFFSET = 12, /**< uint8_t */
    cr_SizesOffsets_NUM_PARAM_NOTIFICATIONS_OFFSET = 13, /**< uint8_t */
    cr_SizesOffsets_NUM_COMMANDS_IN_RESPONSE_OFFSET = 14, /**< uint8_t */
    cr_SizesOffsets_COUNT_PARAM_DESC_IN_RESPONSE_OFFSET = 15, /**< uint8_t */
    cr_SizesOffsets_STRUCTURE_SIZE = 16 /**< just the size */
} cr_SizesOffsets;

/* Struct definitions */
/** This is the "classic" Reach service routing message header.  
/ It is now deprecated in favor of the AhsokaMessageHeader. */
typedef struct _cr_ReachMessageHeader {
    uint32_t message_type; /**< This ID defines the Type of Message being carried in the Envelope / Header */
    uint32_t endpoint_id; /**< Routing for endpoints other than zero. */
    uint32_t client_id; /**< To support multiple clients */
    uint32_t remaining_objects; /**< Zero when transaction is complete */
    uint32_t transaction_id; /**< An ID for a series of messages */
} cr_ReachMessageHeader;

typedef PB_BYTES_ARRAY_T(208) cr_ReachMessage_payload_t;
/** A reach packet includes a header and a payload. */
typedef struct _cr_ReachMessage {
    bool has_header;  ///< Controls corresponding optional member
    cr_ReachMessageHeader header; /**< A routing header */
    cr_ReachMessage_payload_t payload; /**< The encoded payload */
} cr_ReachMessage;

typedef PB_BYTES_ARRAY_T(4) cr_AhsokaMessageHeader_client_id_t;
/** This Service Routing Message Header is used in the OpenPV system.
/ Reach can speak it.
/ This object represents the Layer 2 Message Format for OpenPV Service Messages.
/ The ordinals are presereved but the names are changed to match */
typedef struct _cr_AhsokaMessageHeader {
    /** This ID defines the Type of Message being carried in the Envelope / Header
        Called transport_id in OpenPV terminology.
        Called message_type in Reach terms. */
    int32_t message_type;
    /** This ID defines a unique Message / Response used when out of order messages are needed
        Called transaction_id in the OpenPV system.
        Called transaction_id in Reach terms. */
    int32_t transaction_id;
    /** Unique ID for a Client used in Services that support Multiple Clients 
       OpenPV would use a GUID but Reach uses a 4 byte integer */
    cr_AhsokaMessageHeader_client_id_t client_id;
    /** Called message_size in the OpenPV system.
        Called remaining_objects in Reach terms
        In Reach it defines the number of objects that remain to be 
        transmitted in a continuued transaction.
/ The size of the message payload (in packets) that follows this header */
    int32_t remaining_objects;
    /** Routing for Non-Endpoint Style Transports. 
       Note: Endpoint 0 is Reserved for Service Discovery for Non-Endpoint Transports */
    uint32_t endpoint_id;
    /** Not used or supported in Reach.  
        In OpenPV, indicates that the message has used deflate compression in addition to pbuff encoding */
    bool is_message_compressed;
} cr_AhsokaMessageHeader;

/** ERROR_REPORT: Could be sent asynchronously to indicate an error. */
typedef struct _cr_ErrorReport {
    int32_t result; /**< The integer error code being reported, preferrably from the ErrorCodes enum. */
    char result_message[194]; /**< A human readable string describing the error. */
} cr_ErrorReport;

typedef PB_BYTES_ARRAY_T(194) cr_PingRequest_echo_data_t;
/** Request Object used to Echo Data for testing the Device Communication */
typedef struct _cr_PingRequest {
    cr_PingRequest_echo_data_t echo_data; /**< Data which should be echoed in the response */
} cr_PingRequest;

typedef PB_BYTES_ARRAY_T(194) cr_PingResponse_echo_data_t;
/** Response Object used to Echo Data for testing the Device Communication */
typedef struct _cr_PingResponse {
    cr_PingResponse_echo_data_t echo_data; /**< The same data sent in the request */
    int32_t signal_strength; /**< rssi : The server's received signal strength expressed in dB. */
} cr_PingResponse;

/** Required Device Information Service
    Reach sessions are opened with the device info request and response.  */
typedef struct _cr_DeviceInfoRequest {
    bool has_challenge_key; ///< Controls corresponding optional member
    char challenge_key[32]; /**< The request can include a challenge key to grant or deny access to parts of the system. */
    char client_protocol_version[16]; /**< The client shares its version to enable backward compatibility. */
} cr_DeviceInfoRequest;

typedef PB_BYTES_ARRAY_T(16) cr_DeviceInfoResponse_application_identifier_t;
typedef PB_BYTES_ARRAY_T(16) cr_DeviceInfoResponse_sizes_struct_t;
/** This message is returned in answer to the first "who are you" query from the client. */
typedef struct _cr_DeviceInfoResponse {
    char device_name[24]; /**< Human readable name of the device */
    char manufacturer[24]; /**< Human readable name of the manufacturer */
    char device_description[48]; /**< A longer human readable description. */
    /** Each endpoint advertises a "main" FW version.
 If there are other FW versions, put them in the parameter repo. */
    char firmware_version[16];
    char protocol_version_string[16]; /**< The protocol version as a string against which this device is built. */
    uint32_t services; /**< A bit mask, allowing support for up to 32 services */
    uint32_t parameter_metadata_hash; /**< Used to avoid reloading the parameter descriptions */
    bool has_application_identifier;    ///< Controls corresponding optional member
    cr_DeviceInfoResponse_application_identifier_t application_identifier; /**< A UUID to find a custom user interface */
    uint32_t endpoints; /**< A bit mask, non-zero if the device supports more than one endpoint. */
    cr_DeviceInfoResponse_sizes_struct_t sizes_struct; /**< A packed structure informing the client of the size limitations of the server. See SizesOffsets for descriptions. */
} cr_DeviceInfoResponse;

/**
  Parameter Service
  Parameters provide a simple key:value database. The key is an ID number.  
  The value can be of various common types up to (typically) 32 bytes.  
  Parameters support a robust description which can be const, stored in flash.  
  Parameters can be configured to support notifying the client.
  The ParameterInfoRequest is used by the client to request detailed information
  about a set of parameters. */
typedef struct _cr_ParameterInfoRequest {
    pb_size_t parameter_ids_count;  ///< Controls corresponding repeated member
    uint32_t parameter_ids[32]; /**< ID's to Fetch (Empty to Get All) */
} cr_ParameterInfoRequest;

/** A member of a union (oneof) that describes a uint32 */
typedef struct _cr_Uint32ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member 
    uint32_t range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    uint32_t range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    uint32_t default_value; /**< The (optional) default value for this parameter. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Uint32ParameterInfo;

/** A member of a union (oneof) that describes an sint32 */
typedef struct _cr_Int32ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    int32_t range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    int32_t range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    int32_t default_value; /**< The (optional) default value for this parameter. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Int32ParameterInfo;

/** A member of a union (oneof) that describes a float32 */
typedef struct _cr_Float32ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    float range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    float range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    float default_value; /**< The (optional) default value for this parameter. */
    bool has_precision;  ///< Controls corresponding optional member
    uint32_t precision; /**< How many digits to be displayed to the right of the decimal point. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Float32ParameterInfo;

/** A member of a union (oneof) that describes a uint64 */
typedef struct _cr_Uint64ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    uint64_t range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    uint64_t range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    uint64_t default_value; /**< The (optional) default value for this parameter. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Uint64ParameterInfo;

/** A member of a union (oneof) that describes an sint64 */
typedef struct _cr_Int64ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    int64_t range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    int64_t range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    int64_t default_value; /**< The (optional) default value for this parameter. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Int64ParameterInfo;

/** A member of a union (oneof) that describes a float64 */
typedef struct _cr_Float64ParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    double range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    double range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    double default_value; /**< The (optional) default value for this parameter. */
    bool has_precision;  ///< Controls corresponding optional member
    uint32_t precision; /**< How many digits to be displayed to the right of the decimal point. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_Float64ParameterInfo;

/** A member of a union (oneof) that describes a boolean */
typedef struct _cr_BoolParameterInfo {
    bool has_default_value;  ///< Controls corresponding optional member
    bool default_value; /**< The (optional) default value for this parameter. */
    bool has_pei_id;    ///< Controls corresponding optional member
    uint32_t pei_id; /**< The (optional) ID of the extended description that names the two states. */
} cr_BoolParameterInfo;

/** A member of a union (oneof) that describes a string */
typedef struct _cr_StringParameterInfo {
    bool has_default_value;  ///< Controls corresponding optional member
    char default_value[32]; /**< The (optional) default value for this parameter.. */
    uint32_t max_size; /**< The length of the longest string to be stored here. */
} cr_StringParameterInfo;

/** A member of a union (oneof) that describes an enumerated value */
typedef struct _cr_EnumParameterInfo {
    bool has_range_min;  ///< Controls corresponding optional member
    uint32_t range_min; /**< The (optional) minimum value for this parameter. */
    bool has_range_max;  ///< Controls corresponding optional member
    uint32_t range_max; /**< The (optional) minimum value for this parameter. */
    bool has_default_value;  ///< Controls corresponding optional member
    uint32_t default_value; /**< The (optional) default value for this parameter. */
    bool has_pei_id;    ///< Controls corresponding optional member
    uint32_t pei_id; /**< The (optional) ID of the extended description that names the values. */
    bool has_units;  ///< Controls corresponding optional member
    char units[16]; /**< A string (UTF-8) to be displayed as the units of the parameter. */
} cr_EnumParameterInfo;

/** A member of a union (oneof) that describes a bitfield */
typedef struct _cr_BitfieldParameterInfo {
    bool has_default_value;  ///< Controls corresponding optional member
    uint64_t default_value; /**< The (optional) default value for this parameter. */
    uint32_t bits_available; /**< How many bits of this bifield are valid */
    bool has_pei_id;    ///< Controls corresponding optional member
    uint32_t pei_id; /**< The (optional) ID of the extended description that names the bits. */
} cr_BitfieldParameterInfo;

typedef PB_BYTES_ARRAY_T(32) cr_ByteArrayParameterInfo_default_value_t;
/** A member of a union (oneof) that describes a byte array */
typedef struct _cr_ByteArrayParameterInfo {
    bool has_default_value;  ///< Controls corresponding optional member
    cr_ByteArrayParameterInfo_default_value_t default_value; /**< The (optional) default value for this parameter. */
    uint32_t max_size; /**< The size in bytes of the largest array to be stored. */
} cr_ByteArrayParameterInfo;

/** A ParameterInfo structure describes a parameter so that it can be easily viewed an edited 
/ by humans using a generic parameter editor. */
typedef struct _cr_ParameterInfo {
    uint32_t id; /**< The integer ID used to reference this parameter. */
    char name[24]; /**< A human readable name for this parameter. */
    bool has_description;  ///< Controls corresponding optional member
    char description[32]; /**< A longer human readable description of this parameter. */
    cr_AccessLevel access; /**< Read or write access.  Can be used for access control. */
    cr_StorageLocation storage_location; /**< RAM or NVM or ? */
    pb_size_t which_desc;   ///< Specifies which description in the union
    union {
        cr_Uint32ParameterInfo uint32_desc; /**< If uint32 */
        cr_Int32ParameterInfo int32_desc; /**< If sint32 */
        cr_Float32ParameterInfo float32_desc; /**< If float32 */
        cr_Uint64ParameterInfo uint64_desc; /**< If uint64 */
        cr_Int64ParameterInfo int64_desc; /**< If sint64 */
        cr_Float64ParameterInfo float64_desc; /**< If float64 */
        cr_BoolParameterInfo bool_desc; /**< If a boolean */
        cr_StringParameterInfo string_desc; /**< If a string */
        cr_EnumParameterInfo enum_desc; /**< If an enumeated type */
        cr_BitfieldParameterInfo bitfield_desc; /**< If a bitfield */
        cr_ByteArrayParameterInfo bytearray_desc; /**< If a byte array */
    } desc;
} cr_ParameterInfo;

/** The response to ParameterInfoRequest */
typedef struct _cr_ParameterInfoResponse {
    pb_size_t parameter_infos_count;    ///< Controls corresponding repeated member
    cr_ParameterInfo parameter_infos[2]; /**< An array of Param Info structures */
} cr_ParameterInfoResponse;

/** The ParamExKey is used to give names to enums, bitfields, and booleans.
/ It is typically supplied in an array.
/ More than one parameter can point to the same ParamExKey. */
typedef struct _cr_ParamExKey {
    uint32_t id; /**< the value of the enum or the bit position of the bitfield */
    char name[16]; /**< the name of the enum/bit/state */
} cr_ParamExKey;

/** Describes enum, bitfield, and boolean labels */
typedef struct _cr_ParamExInfoResponse {
    cr_ParameterDataType data_type; /**< The type of data being described. */
    pb_size_t keys_count;   ///< Controls corresponding repeated member
    cr_ParamExKey keys[8]; /**< Each key associates a string with a number */
    uint32_t pei_id; /**< An ID by which this Parameter Extended Information is retrieved. */
} cr_ParamExInfoResponse;

/** A structure used to read one or more parameters */
typedef struct _cr_ParameterRead {
    pb_size_t parameter_ids_count;  ///< Controls corresponding repeated member
    uint32_t parameter_ids[32]; /**< An array of parameters to be read, or empty to Retrieve All */
} cr_ParameterRead;

/** The response to a parameter write */
typedef struct _cr_ParameterWriteResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_ParameterWriteResponse;

/** Parameter Notification configuration
/ Notification can be enabled with conditions for each parameter. */
typedef struct _cr_ParameterNotifyConfig {
    uint32_t parameter_id; /**< Which param to configure */
    uint32_t minimum_notification_period; /**< min_ms: Minimum Notification Interval (ms) */
    uint32_t maximum_notification_period; /**< max_ms: Minimum Notification Interval (ms) */
    float minimum_delta; /**< notify only if change by this much */
} cr_ParameterNotifyConfig;

/** A message used to enable notifications */
typedef struct _cr_ParameterEnableNotifications {
    pb_size_t configs_count;    ///< Controls corresponding repeated member
    cr_ParameterNotifyConfig configs[8]; /**< An array of descriptions of notifications */
    bool disable_all_first; /**< If true, all notifications are disabled before applying the new request. */
} cr_ParameterEnableNotifications;

/** A message used to disable notifications */
typedef struct _cr_ParameterDisableNotifications {
    pb_size_t parameter_ids_count;  ///< Controls corresponding repeated member
    uint32_t parameter_ids[32]; /**< An array of parameter ID's to be disabled. */
} cr_ParameterDisableNotifications;

/** The response to enable and disable notifications */
typedef struct _cr_ParameterNotifyConfigResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_ParameterNotifyConfigResponse;

/** The client can discover how notifications are setup. */
typedef struct _cr_DiscoverParameterNotifications {
    pb_size_t parameter_ids_count;  ///< Controls corresponding repeated member
    uint32_t parameter_ids[32]; /**< An array of parameter ID's to be queried for notification.  Leave Empty to Retrieve All */
} cr_DiscoverParameterNotifications;

/** Reports which notifications are enabled. */
typedef struct _cr_DiscoverParameterNotificationsResponse {
    pb_size_t configs_count;    ///< Controls corresponding repeated member
    cr_ParameterNotifyConfig configs[8]; /**< An array of parameter notification configurations containing information only on parameters that have notifications enabled. */
} cr_DiscoverParameterNotificationsResponse;

typedef PB_BYTES_ARRAY_T(32) cr_ParameterValue_bytes_value_t;
/** A message used to send or receive a single parameter value. */
typedef struct _cr_ParameterValue {
    uint32_t parameter_id; /**< The integer ID of this parameter. */
    uint32_t timestamp; /**< The time at which this was last read or written.  Used for notifications. */
    pb_size_t which_value;  ///< Specifies which value in the union
    union {
        uint32_t uint32_value; /**< Unsigned 32 bit integer. */
        int32_t int32_value; /**< Signed 32 bit integer. */
        float float32_value; /**< 32 bit floating point. */
        uint64_t uint64_value; /**< Unsigned 64 bit integer. */
        int64_t int64_value; /**< Signed 64 bit integer. */
        double float64_value; /**< 64 bit floating point. */
        bool bool_value; /**< boolean (0 or 1) */
        char string_value[32]; /**< ASCII or UTF-8. Null Terminated. */
        uint32_t enum_value; /**< An extended description gives names to 32 bit integer values. */
        uint64_t bitfield_value; /**< An extended description gives names to up to 64 bit positions. */
        cr_ParameterValue_bytes_value_t bytes_value; /**< An array of bytes. */
    } value;
} cr_ParameterValue;

/** A structure used to return the values read from one or more parameters */
typedef struct _cr_ParameterReadResponse {
    pb_size_t values_count;     ///< Controls corresponding repeated member
    cr_ParameterValue values[4]; /**< An array of Result Values */
} cr_ParameterReadResponse;

/** A structure used to write one or more Parameters */
typedef struct _cr_ParameterWrite {
    pb_size_t values_count;     ///< Controls corresponding repeated member
    cr_ParameterValue values[4]; /**< An array of Write Values */
} cr_ParameterWrite;

/** A message of this type is sent by the server when parameters change. */
typedef struct _cr_ParameterNotification {
    pb_size_t values_count;     ///< Controls corresponding repeated member
    cr_ParameterValue values[4]; /**< An array of parameter values containing those that changed. */
} cr_ParameterNotification;

/** The optional file service provides a method of efficiently transfering large blocks of data. */
typedef struct _cr_DiscoverFiles {
    char dummy_field;
} cr_DiscoverFiles;

/** A structure describing a file */
typedef struct _cr_FileInfo {
    uint32_t file_id; /**< ID by which this file is referenced */
    char file_name[24]; /**< Human readable file name */
    cr_AccessLevel access; /**< Access Level (Read / Write) */
    int32_t current_size_bytes; /**< size in bytes */
    cr_StorageLocation storage_location; /**< NVM or RAM */
    bool require_checksum; /**< set true to request checksum generation and validation. */
    bool has_maximum_size_bytes;    ///< Controls corresponding optional member
    uint32_t maximum_size_bytes; /**< Determined by storage space */
} cr_FileInfo;

/** The response to discover files */
typedef struct _cr_DiscoverFilesResponse {
    pb_size_t file_infos_count; ///< Controls corresponding repeated member
    cr_FileInfo file_infos[4]; /**< An array of File Info structures */
} cr_DiscoverFilesResponse;

/** Begins a File Transfer (Upload / Download) */
typedef struct _cr_FileTransferRequest {
    uint32_t file_id; /**< ID by which this file is referenced */
    uint32_t read_write; /**< 0 for read, 1 for write. */
    uint32_t request_offset; /**< where to access in the file, in bytes */
    uint32_t transfer_length; /**< number of bytes to read or write */
    uint32_t transfer_id; /**< Copied from the header, the same for the continuued transfer. */
    uint32_t timeout_in_ms; /**< ms before abandonment */
    bool has_requested_ack_rate;    ///< Controls corresponding optional member
    uint32_t requested_ack_rate; /**< number of messages before ACK. */
    bool require_checksum; /**< set true to enable checksum generation and validation. */
} cr_FileTransferRequest;

/** The response to a file transfer request */
typedef struct _cr_FileTransferResponse {
    int32_t result; /**< 0 if OK */
    uint32_t transfer_id; /**< Echos the request. */
    uint32_t ack_rate; /**< confirms or overrides request */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< In case of error, a human readable explanation. */
    uint32_t transfer_length; /**< If the file is smaller than the requested offset + length, this will reflect how much data can be transferred */
} cr_FileTransferResponse;

typedef PB_BYTES_ARRAY_T(194) cr_FileTransferData_message_data_t;
/** A bidirectional message describing a packet of file data */
typedef struct _cr_FileTransferData {
    int32_t result; /**< non-zero for error */
    uint32_t transfer_id; /**< Unchanged during the continuued transfer. */
    uint32_t message_number; /**< counts up from 1 in the first transfer */
    cr_FileTransferData_message_data_t message_data; /**< Data */
    bool has_checksum;  ///< Controls corresponding optional member
    int32_t checksum; /**< Optional RFC 1071 checksum for integrity checking */
} cr_FileTransferData;

/** The response to a file transfer */
typedef struct _cr_FileTransferDataNotification {
    int32_t result; /**< 0 for success */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Provides more information if an error occurs. */
    bool is_complete; /**< Set to true when all data has been trasnferred. */
    uint32_t transfer_id; /**< Unchanged during the continuued transfer. */
    uint32_t retry_offset; /**< If there is an error, this gives the offset at which a new transfer should start with good data. */
} cr_FileTransferDataNotification;

/** A request to erase (set size to zero) a file. */
typedef struct _cr_FileEraseRequest {
    uint32_t file_id; /**< File ID to be erased */
} cr_FileEraseRequest;

/** The response to a file erase request */
typedef struct _cr_FileEraseResponse {
    uint32_t file_id; /**< File ID that has been erased */
    int32_t result; /**< 0 on success */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Provides more information if an error occurs. */
} cr_FileEraseResponse;

/** A request to list the streams supported by the device */
typedef struct _cr_DiscoverStreams {
    char dummy_field;
} cr_DiscoverStreams;

/** A structure describing a stream */
typedef struct _cr_StreamInfo {
    int32_t stream_id; /**< The ID by which this stream is addressed. */
    cr_AccessLevel access; /**< Access Level for Stream  (Read / Write). */
    char name[24]; /**< A human readable name for this stream. */
} cr_StreamInfo;

/** The response to DiscoverStreams */
typedef struct _cr_DiscoverStreamsResponse {
    pb_size_t streams_count;    ///< Controls corresponding repeated member
    cr_StreamInfo streams[4]; /**< An array containing descriptions of the supported streams */
} cr_DiscoverStreamsResponse;

/** A structure requesting to open a stream */
typedef struct _cr_StreamOpen {
    int32_t stream_id; /**< The ID by which this stream is addressed. */
    cr_AccessLevel access; /**< Read or write access */
} cr_StreamOpen;

/** The response to a StreamOpen request */
typedef struct _cr_StreamOpenResponse {
    int32_t stream_id; /**< The ID by which this stream is addressed. */
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_StreamOpenResponse;

/** A structure requesting to close a stream */
typedef struct _cr_StreamClose {
    int32_t stream_id; /**< The ID by which this stream is addressed. */
} cr_StreamClose;

typedef PB_BYTES_ARRAY_T(194) cr_StreamData_message_data_t;
/** Bi-Directional message used to asynchronously send stream data to the other side. */
typedef struct _cr_StreamData {
    int32_t stream_id; /**< The ID by which this stream is addressed. */
    uint32_t roll_count; /**< Message Number.  Increases with each send.  As stream transmission may be less relaible, allows for continuity checking. */
    cr_StreamData_message_data_t message_data; /**< An array of bytes representing the streami data. */
    bool has_checksum;  ///< Controls corresponding optional member
    int32_t checksum; /**< Optional RFC 1071 checksum for integrity checking */
} cr_StreamData;

/** The (optional) Command Service allows actions to be triggered from the Reach UI. */
typedef struct _cr_DiscoverCommands {
    char dummy_field;
} cr_DiscoverCommands;

/** The description of a command. */
typedef struct _cr_CommandInfo {
    uint32_t id; /**< The id by which the command is dispatched */
    char name[24]; /**< Human readable descriptive name */
    bool has_description;   ///< Controls corresponding optional member
    char description[48]; /**< Optional longer description of the command */
    bool has_timeout;   ///< Controls corresponding optional member
    uint32_t timeout; /**< Optional command timeout, in milliseconds, to account for slow commands. */
} cr_CommandInfo;

/** Response to a DiscoverCommands request. */
typedef struct _cr_DiscoverCommandsResponse {
    pb_size_t available_commands_count;   ///< Controls corresponding repeated member
    cr_CommandInfo available_commands[2]; /**< An array of command descriptions. */
} cr_DiscoverCommandsResponse;

/** Prompts the device to execute this command. */
typedef struct _cr_SendCommand {
    uint32_t command_id; /**< The ID of the command to execute. */
} cr_SendCommand;

/** A response acknowledging the completion of a command. */
typedef struct _cr_SendCommandResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_SendCommandResponse;

/** The optional Command Line Interface (CLI) service allows command line messages to be transfered
/ between the client and the server.  Messages can travel in both directions.  Messages are asynchronous.
/ The client can send a command line and the server can respond.
/ The server can also asynchrously send strings representing the output of the device. 
/ The CLIData message is used in both directions. */
typedef struct _cr_CLIData {
    char message_data[194]; /**< The command line as a null terminated string. */
} cr_CLIData;

/** The optional Time Service is designed to allow the client to 
/ set and adjust the real time clock in a server device.
/ The time is best specified as UTC plus timezone offset.
/ Although the timezone is optional, it's best to use it.
/ TimeSetRequest requests setting the server device time to this value. */
typedef struct _cr_TimeSetRequest {
    int64_t seconds_utc; /**< linux epoch, since 1970 */
    bool has_timezone;  ///< Controls corresponding optional member
    int32_t timezone; /**< An adjustmeent in seconds to UTC time respresenting the local timezone. */
} cr_TimeSetRequest;

/** The response to the TimeSetRequest */
typedef struct _cr_TimeSetResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_TimeSetResponse;

/** A request to read the time from the server. */
typedef struct _cr_TimeGetRequest {
    char dummy_field;
} cr_TimeGetRequest;

/** The response to a TimeGetRequest */
typedef struct _cr_TimeGetResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
    int64_t seconds_utc; /**< linux epoch, since 1970 */
    bool has_timezone;  ///< Controls corresponding optional member
    int32_t timezone; /**< An adjustmeent in seconds to UTC time respresenting the local timezone. */
} cr_TimeGetResponse;

/** A structure describing a WiFi connection or access point
/ The optional WiFi service is intended to simplify the 
/ repetitive task of communicating WiFi credentials to the device. */
typedef struct _cr_ConnectionDescription {
    char ssid[32]; /**< The SSID of this connection */
    bool is_connected; /**< true if connected */
    bool has_signal_strength;   ///< Controls corresponding optional member
    int32_t signal_strength; /**< The strength of this connection in dB (RSSI) */
    bool has_sec;   ///< Controls corresponding optional member
    cr_WiFiSecurity sec; /**< The type of security used by this connection */
    bool has_band;  ///< Controls corresponding optional member
    cr_WiFiBand band; /**< The RF band used by this connection */
} cr_ConnectionDescription;

/** Commands the server to initiate a scan for WiFi access points.
/ As this may take some time, issue it and check the response later. */
typedef struct _cr_ScanWiFi {
    char dummy_field;
} cr_ScanWiFi;

/** A request to provide a list access points */
typedef struct _cr_DiscoverWiFi {
    char dummy_field;
} cr_DiscoverWiFi;

/** response to DiscoverWiFi */
typedef struct _cr_DiscoverWiFiResponse {
    bool scan_is_valid; /**< true if a recent scan is complete. */
    pb_size_t cd_count; ///< Controls corresponding repeated member
    cr_ConnectionDescription cd[4]; /**< An array of available access points */
} cr_DiscoverWiFiResponse;

/** A structure describing a WiFi connection request */
typedef struct _cr_WiFiConnectionRequest {
    pb_callback_t ssid; /**< The SSID to be addressed */
    bool connect; /**< connect and disconnect false to get info on this SSID */
    bool disconnect; /**< connect and disconnect false to get info on this SSID */
    bool has_password;  ///< Controls corresponding optional member
    char password[32]; /**< required to connect */
    bool has_autoconnect;   ///< Controls corresponding optional member
    bool autoconnect; /**< true to remember and autoconnect to this AP. */
} cr_WiFiConnectionRequest;

/** Describes the response to a WiFi connection request */
typedef struct _cr_WiFiConnectionResponse {
    int32_t result; /**< A result of zero indicates OK */
    bool has_signal_strength;   ///< Controls corresponding optional member
    int32_t signal_strength; /**< RSSI */
    bool has_result_message;    ///< Controls corresponding optional member
    char result_message[194]; /**< Allows to provide a human readable explanation in case of an error. */
} cr_WiFiConnectionResponse;

#endif
