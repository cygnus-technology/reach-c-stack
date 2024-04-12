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
/* ReachProtoVersion is replaced by the MAJOR, MINOR and PATCH, below. */
typedef enum _cr_ReachProtoVersion {
    cr_ReachProtoVersion_NOT_USED = 0, /* Must have a zero */
    cr_ReachProtoVersion_CURRENT_VERSION = 258 /* update this when you change this file. */
} cr_ReachProtoVersion;

typedef enum _cr_ReachProto_MAJOR_Version {
    /* MAJOR_V0       = 0;   // Must have a zero */
    cr_ReachProto_MAJOR_Version_MAJOR_VERSION = 0
} cr_ReachProto_MAJOR_Version;

typedef enum _cr_ReachProto_MINOR_Version {
    cr_ReachProto_MINOR_Version_MINOR_V0 = 0, /* Must have a zero */
    cr_ReachProto_MINOR_Version_MINOR_VERSION = 1 /* Update at a release */
} cr_ReachProto_MINOR_Version;

typedef enum _cr_ReachProto_PATCH_Version {
    cr_ReachProto_PATCH_Version_PATCH_V0 = 0, /* Must have a zero */
    cr_ReachProto_PATCH_Version_PATCH_VERSION = 4 /* Update when something changes */
} cr_ReachProto_PATCH_Version;

/// These values identify the type of the Reach message.
typedef enum _cr_ReachMessageTypes {
    cr_ReachMessageTypes_INVALID = 0, /** No Valid Message */
    cr_ReachMessageTypes_ERROR_REPORT = 1,
    cr_ReachMessageTypes_PING = 2,
    cr_ReachMessageTypes_GET_DEVICE_INFO = 3,
    /** Parameters */
    cr_ReachMessageTypes_DISCOVER_PARAMETERS = 5,
    cr_ReachMessageTypes_DISCOVER_PARAM_EX = 6,
    cr_ReachMessageTypes_READ_PARAMETERS = 7,
    cr_ReachMessageTypes_WRITE_PARAMETERS = 8,
    cr_ReachMessageTypes_CONFIG_PARAM_NOTIFY = 9, /** setup parameter notification */
    cr_ReachMessageTypes_PARAMETER_NOTIFICATION = 10, /** A parameter has changed */
    cr_ReachMessageTypes_DISCOVER_NOTIFICATIONS = 11, /** find out how notifications are setup */
    /* File Transfers */
    cr_ReachMessageTypes_DISCOVER_FILES = 12,
    cr_ReachMessageTypes_TRANSFER_INIT = 13, /** Begins a Transfer */
    cr_ReachMessageTypes_TRANSFER_DATA = 14, /** Sends Data */
    /** Clears Sender (Client / Service) to Send More Data: */
    cr_ReachMessageTypes_TRANSFER_DATA_NOTIFICATION = 15,
    cr_ReachMessageTypes_DISCOVER_COMMANDS = 17,
    cr_ReachMessageTypes_SEND_COMMAND = 18,
    /** Command Line Interface */
    cr_ReachMessageTypes_CLI_NOTIFICATION = 20,
    /** Streams */
    cr_ReachMessageTypes_DISCOVER_STREAMS = 25,
    cr_ReachMessageTypes_OPEN_STREAM = 26,
    cr_ReachMessageTypes_CLOSE_STREAM = 27,
    cr_ReachMessageTypes_STREAM_DATA_NOTIFICATION = 28,
    /** Time */
    cr_ReachMessageTypes_SET_TIME = 30,
    cr_ReachMessageTypes_GET_TIME = 31,
    /** WiFi */
    cr_ReachMessageTypes_DISCOVER_WIFI = 40,
    cr_ReachMessageTypes_WIFI_CONNECT = 41
} cr_ReachMessageTypes;

/** Bits identifying services.
 *  These binary bit masks or'ed together into the
 *  DeviceInfoResponse.services */
typedef enum _cr_ServiceIds {
    cr_ServiceIds_NO_SVC_ID = 0,
    cr_ServiceIds_PARAMETER_REPO = 1,
    cr_ServiceIds_FILES = 2,
    cr_ServiceIds_STREAMS = 4,
    cr_ServiceIds_COMMANDS = 8,
    cr_ServiceIds_CLI = 16,
    cr_ServiceIds_TIME = 32,
    cr_ServiceIds_WIFI = 64
} cr_ServiceIds;

/** binary bit masks or'ed together into the DeviceInfoResponse.endpoints */
typedef enum _cr_EndpointIds {
    cr_EndpointIds_NO_ENDPOINTS = 0,
    cr_EndpointIds_ONE = 1,
    cr_EndpointIds_TWO = 2,
    cr_EndpointIds_THREE = 4,
    cr_EndpointIds_FOUR = 8
} cr_EndpointIds;

/** The data types supported in the parameter repository   */
typedef enum _cr_ParameterDataType {
    cr_ParameterDataType_UINT32 = 0,
    cr_ParameterDataType_INT32 = 1,
    cr_ParameterDataType_FLOAT32 = 2,
    cr_ParameterDataType_UINT64 = 3,
    cr_ParameterDataType_INT64 = 4,
    cr_ParameterDataType_FLOAT64 = 5,
    cr_ParameterDataType_BOOL = 6,
    cr_ParameterDataType_STRING = 7, /** ASCII or UTF-8. Null Terminated. */
    cr_ParameterDataType_ENUMERATION = 8,
    cr_ParameterDataType_BIT_FIELD = 9,
    cr_ParameterDataType_BYTE_ARRAY = 10
} cr_ParameterDataType;

typedef enum _cr_CLIType {
    cr_CLIType_NO_CLI = 0,
    cr_CLIType_COMMAND = 1,
    cr_CLIType_REPORT = 2
} cr_CLIType;

/** Bits identifying parameter access rules */
typedef enum _cr_AccessLevel {
    cr_AccessLevel_NO_ACCESS = 0,
    cr_AccessLevel_READ = 1,
    cr_AccessLevel_WRITE = 2,
    cr_AccessLevel_READ_WRITE = 3
} cr_AccessLevel;

/** The types of memory in which parameters are stored.  RAM
 *  and NVM (Non Volatile Memory) are most common. */
typedef enum _cr_StorageLocation {
    cr_StorageLocation_STORAGE_LOCATION_INVALID = 0,
    cr_StorageLocation_RAM = 1,
    cr_StorageLocation_NONVOLATILE = 2,
    cr_StorageLocation_RAM_EXTENDED = 3,
    cr_StorageLocation_NONVOLATILE_EXTENDED = 4
} cr_StorageLocation;

typedef enum _cr_WiFiSecurity {
    cr_WiFiSecurity_OPEN = 0, /* No security */
    cr_WiFiSecurity_WEP = 1, /* WEP */
    cr_WiFiSecurity_WPA = 2, /* WPA */
    cr_WiFiSecurity_WPA2 = 3, /* WPA2 */
    cr_WiFiSecurity_WPA3 = 4 /* WPA3 */
} cr_WiFiSecurity;

typedef enum _cr_WiFiBand {
    cr_WiFiBand_NO_BAND = 0, /* Not specified */
    cr_WiFiBand_BAND_2 = 2, /* 2.4GHz */
    cr_WiFiBand_BAND_5 = 5 /* 5GHz */
} cr_WiFiBand;

typedef enum _cr_WiFiState {
    cr_WiFiState_NOT_CONNECTED = 0, /* connected, active */
    cr_WiFiState_CONNECTED = 1 /* disconnected */
} cr_WiFiState;

/** Error codes used by Reach functions   */
typedef enum _cr_ErrorCodes {
    cr_ErrorCodes_NO_ERROR = 0,
    cr_ErrorCodes_NO_DATA = 1, /**  */
    cr_ErrorCodes_READ_FAILED = 2, /**  */
    cr_ErrorCodes_WRITE_FAILED = 3, /**  */
    cr_ErrorCodes_NOT_IMPLEMENTED = 4, /** returned by weak implementations */
    cr_ErrorCodes_MALFORMED_MESSAGE = 5, /** not yet used */
    cr_ErrorCodes_NO_SERVICE = 6, /** not yet used */
    cr_ErrorCodes_PERMISSION_DENIED = 7, /**  */
    cr_ErrorCodes_BUFFER_TOO_SMALL = 8, /**  */
    cr_ErrorCodes_INVALID_PARAMETER = 9, /**  */
    cr_ErrorCodes_CHECKSUM_MISMATCH = 10, /** not yet used */
    cr_ErrorCodes_DECODING_FAILED = 11, /**  */
    cr_ErrorCodes_ENCODING_FAILED = 12, /**  */
    cr_ErrorCodes_INVALID_STATE = 13, /**  */
    cr_ErrorCodes_NO_RESPONSE = 14, /** handler signals no response is necessary */
    cr_ErrorCodes_BAD_FILE = 15, /** bad file ID */
    cr_ErrorCodes_PACKET_COUNT_ERR = 16,
    cr_ErrorCodes_CHALLENGE_FAILED = 17,
    cr_ErrorCodes_PARAMETER_LOCKED = 18,
    cr_ErrorCodes_NO_RESOURCE = 19, /** as in no more param notification slots. */
    cr_ErrorCodes_ABORT = 1000 /** Operation cancellation */
} cr_ErrorCodes;

/** Values of a state variable used in file transfer   */
typedef enum _cr_FileTransferState {
    cr_FileTransferState_FILE_TRANSFER_INVALID = 0,
    cr_FileTransferState_IDLE = 1,
    cr_FileTransferState_INIT = 2,
    cr_FileTransferState_DATA = 3,
    cr_FileTransferState_COMPLETE = 4
} cr_FileTransferState;

/** Service Routing Message Header.  This header is found on
 *  each Reach packet.
 */
typedef struct _cr_ReachMessageHeader {
    /** This ID defines the Type of Message being carried in the Envelope / Header */
    uint32_t message_type;
    /** Routing for endpoints other than zero. */
    uint32_t endpoint_id;
    /** To support multiple clients */
    uint32_t client_id;
    /** Zero when transaction is complete */
    uint32_t remaining_objects;
    /** An ID for a series of messages */
    uint32_t transaction_id;
} cr_ReachMessageHeader;

/** A reach packet includes a header and a payload  */
typedef PB_BYTES_ARRAY_T(208) cr_ReachMessage_payload_t;
typedef struct _cr_ReachMessage {
    bool has_header;
    cr_ReachMessageHeader header;
    cr_ReachMessage_payload_t payload;
} cr_ReachMessage;

typedef PB_BYTES_ARRAY_T(4) cr_AhsokaMessageHeader_client_id_t;
/** ----------------------------
 This Service Routing Message Header is used in the OpenPV system.
 Reach can speak it.
 This object represents the Layer 2 Message Format for OpenPV Service Messages.
---------------------------- */
typedef struct _cr_AhsokaMessageHeader {
    /** This ID defines the Type of Message being carried in the Envelope / Header */
    int32_t transport_id;
    /** This ID defines a unique Message / Response used when out of order messages are needed */
    int32_t client_message_id;
    /** Unique ID for a Client used in Services that support Multiple Clients 
       OpenPV would use a GUID but Reach uses a 4 byte integer */
    cr_AhsokaMessageHeader_client_id_t client_id;
    /** The size of the message payload (in packets) that follows this header */
    int32_t message_size;
    /** Routing for Non-Endpoint Style Transports. 
       Note: Endpoint 0 is Reserved for Service Discovery for Non-Endpoint Transports */
    uint32_t endpoint_id;
    /** Not supported) Indicates that the message has used deflate compression in addition to pbuff encoding */
    bool is_message_compressed;
} cr_AhsokaMessageHeader;

/** ERROR_REPORT: Could be sent asynchronously to indicate an error. */
typedef struct _cr_ErrorReport {
    int32_t result; /** Error Result */
    char result_message[194]; /** Error String */
} cr_ErrorReport;

typedef PB_BYTES_ARRAY_T(194) cr_PingRequest_echo_data_t;
/** Request Object used to Echo Data for testing the Device Communication */
typedef struct _cr_PingRequest {
    cr_PingRequest_echo_data_t echo_data; /** d : Data */
} cr_PingRequest;

typedef PB_BYTES_ARRAY_T(194) cr_PingResponse_echo_data_t;
/** Response Object used to Echo Data for testing the Device Communication */
typedef struct _cr_PingResponse {
    cr_PingResponse_echo_data_t echo_data; /** d : Data */
    int32_t signal_strength; /** rssi : Rssi express in strength so clients don't have to interpret */
} cr_PingResponse;


/**
* @brief   Required Device Information Service
* @details Reach sessions are opened with the device info 
*          request and response.  The request can include a
*          challenge key to grant or deny access to parts of the
*          system.
*/
typedef struct _cr_DeviceInfoRequest {
    /** A device can be configured to require a challenge_key before
     *  access is granted to the various services.  How the
     *  challenge key is handled is up the designer of the device.
     */
    bool has_challenge_key;
    char challenge_key[32];
    /** The client shares its version to enable backward compatibility. */
    char client_protocol_version[16];
} cr_DeviceInfoRequest;

typedef PB_BYTES_ARRAY_T(16) cr_DeviceInfoResponse_application_identifier_t;
typedef PB_BYTES_ARRAY_T(16) cr_DeviceInfoResponse_sizes_struct_t;
typedef struct _cr_DeviceInfoResponse {
    int32_t protocol_version; /** Supported Protocol Version (deprecated) */
    char device_name[24]; /** Name, Typically Model Name */
    char manufacturer[24];
    char device_description[48]; /** Description */
    /** Each endpoint advertises a "main" FW version.
 If there are other FW versions, put them in the parameter repo. */
    char firmware_version[16];
    /** protocol version as a string */
    char protocol_version_string[16];
    /** A bit mask, allowing support for up to 32 services */
    uint32_t services;
    /** Used to avoid reloading the parameter descriptions */
    uint32_t parameter_metadata_hash;
    bool has_application_identifier;
    cr_DeviceInfoResponse_application_identifier_t application_identifier; /** A UUID to find a Custom firmware_version */
    uint32_t endpoints; /** bit mask, non-zero if other endpoints. */
    cr_DeviceInfoResponse_sizes_struct_t sizes_struct; /** packed. See SizesOffsets */
} cr_DeviceInfoResponse;


/**
* @brief   Parameter Service
* @details Parameters provide a simple key:value database. The 
*          key is an ID number.  The value can be of various
*          common types up to (typically) 32 bytes.  Parameters
*          support a robust description which can be const,
*          stored in flash.  Parameters can be configured to
*          support notifying the client.
*/
typedef struct _cr_ParameterInfoRequest {
    pb_size_t parameter_ids_count;
    uint32_t parameter_ids[32]; /** ID's to Fetch (Empty to Get All) */
} cr_ParameterInfoRequest;

typedef struct _cr_ParameterInfo {
    uint32_t id; /** Id */
    cr_ParameterDataType data_type; /** DataType */
    uint32_t size_in_bytes; /** used by some devices */
    char name[24]; /** Name */
    cr_AccessLevel access; /** Access */
    bool has_description;
    char description[32]; /** Description */
    char units[16]; /** Units */
    bool has_range_min;
    double range_min; /** Range Max */
    bool has_range_max;
    double range_max; /** Range Max */
    bool has_default_value;
    double default_value; /** Show instead of value if no value. */
    cr_StorageLocation storage_location; /** RAM or NVM or ? */
} cr_ParameterInfo;

typedef struct _cr_ParameterInfoResponse {
    pb_size_t parameter_infos_count;
    cr_ParameterInfo parameter_infos[2]; /** Array of Param Infos */
} cr_ParameterInfoResponse;

/** Give names to enums and bitfields */
typedef struct _cr_ParamExKey {
    uint32_t id; /** the valud of the enum */
    char name[16]; /** the name of the enum */
} cr_ParamExKey;

/** also used for bitfields */
typedef struct _cr_ParamExInfoResponse {
    uint32_t associated_pid;
    cr_ParameterDataType data_type;
    pb_size_t enumerations_count;
    cr_ParamExKey enumerations[8];
} cr_ParamExInfoResponse;

typedef struct _cr_ParameterRead {
    pb_size_t parameter_ids_count;
    uint32_t parameter_ids[32]; /** i: ID -  Leave Empty to Retrieve All */
    uint32_t read_after_timestamp; /** Allows for retrieval of only new / changed values. */
} cr_ParameterRead;

typedef struct _cr_ParameterWriteResponse {
    int32_t result; /** 0 if OK */
    bool has_result_message;
    char result_message[194]; /** Error String */
} cr_ParameterWriteResponse;

/** Parameter Notification configuration Notification can be
 *  enabled or disabled */
typedef struct _cr_ParameterNotifyConfig {
    uint32_t parameter_id; /** Which param */
    bool enabled; /** Enabled or Disabled */
    uint32_t minimum_notification_period; /** min_ms: Minimum Notification Interval (ms) */
    uint32_t maximum_notification_period; /** max_ms: Minimum Notification Interval (ms) */
    float minimum_delta; /** notify only if change by this much */
} cr_ParameterNotifyConfig;

typedef struct _cr_ParameterNotifyConfigResponse {
    int32_t result; /** zero if all OK */
    bool has_result_message;
    char result_message[194]; /** Error String */
} cr_ParameterNotifyConfigResponse;

/** ------------------------------------------------------
 The client can discover how notifications are setup.
 ------------------------------------------------------ */
typedef struct _cr_ParameterNotifySetupRequest {
    pb_size_t parameter_ids_count;
    uint32_t parameter_ids[32]; /** i: ID -  Leave Empty to Retrieve All */
} cr_ParameterNotifySetupRequest;

typedef struct _cr_ParameterNotifySetupResponse {
    pb_size_t configs_count;
    cr_ParameterNotifyConfig configs[8];
} cr_ParameterNotifySetupResponse;

typedef PB_BYTES_ARRAY_T(32) cr_ParameterValue_bytes_value_t;
/** Message for Sending / Receiving a Single Parameter Value
 Uses OnOf (Union) for Values */
typedef struct _cr_ParameterValue {
    uint32_t parameter_id; /** i: ID */
    uint32_t timestamp; /** ts: Time Stamp for Notification */
    pb_size_t which_value;
    union {
        uint32_t uint32_value; /** Uint */
        int32_t sint32_value; /** Signed Int */
        float float32_value; /** Float */
        uint64_t uint64_value; /** Uint64 */
        int64_t sint64_value; /** Signed Int 64 */
        double float64_value; /** double */
        bool bool_value; /** Bool */
        char string_value[32]; /** String values (UTF8) */
        uint32_t enum_value;
        uint32_t bitfield_value;
        cr_ParameterValue_bytes_value_t bytes_value; /** byte array values */
    } value;
} cr_ParameterValue;

typedef struct _cr_ParameterReadResponse {
    /** The read_timestamp indicates when this param was last read.  
 Reading resets this value to now. */
    uint32_t read_timestamp; /** Returns timestamp of last read...useful for */
    /** polling large variable lists. */
    pb_size_t values_count;
    cr_ParameterValue values[4]; /** Array of Result Values */
} cr_ParameterReadResponse;

/** Parameter Writes */
typedef struct _cr_ParameterWrite {
    pb_size_t values_count;
    cr_ParameterValue values[4]; /** Array of Write Values */
} cr_ParameterWrite;

/** when parameters change */
typedef struct _cr_ParameterNotification {
    pb_size_t values_count;
    cr_ParameterValue values[4]; /** Array of Result Values */
} cr_ParameterNotification;

/**
* @brief   (optional) File Service
* @details The file service provides a method of efficiently 
*          transfering large blocks of data over BLE.
*/
typedef struct _cr_DiscoverFiles {
    char dummy_field;
} cr_DiscoverFiles;

typedef struct _cr_FileInfo {
    uint32_t file_id; /** ID */
    char file_name[24]; /** Name */
    cr_AccessLevel access; /** Access Level (Read / Write) */
    int32_t current_size_bytes; /** size in bytes */
    cr_StorageLocation storage_location;
    bool require_checksum; /** set true to request checksum generation and validation. */
} cr_FileInfo;

typedef struct _cr_DiscoverFilesResponse {
    pb_size_t file_infos_count;
    cr_FileInfo file_infos[4]; /** Array of File Infos */
} cr_DiscoverFilesResponse;

/** Begins a File Transfer (Upload / Download) */
typedef struct _cr_FileTransferRequest {
    uint32_t file_id; /** File ID */
    uint32_t read_write; /** 0 for read, 1 for write. */
    uint32_t request_offset; /** where to access in the file */
    uint32_t transfer_length; /** bytes to read or write */
    uint32_t transfer_id; /** In case of multiple transfers */
    uint32_t messages_per_ack; /** obsolete.  Use requested_ack_rate. */
    uint32_t timeout_in_ms; /** ms before abandonment */
    bool has_requested_ack_rate;
    uint32_t requested_ack_rate; /** number of messages before ACK. */
    bool require_checksum; /** set true to enable checksum generation and validation. */
} cr_FileTransferRequest;

typedef struct _cr_FileTransferResponse {
    int32_t result; /** 0 if OK */
    uint32_t transfer_id; /** Transfer ID */
    uint32_t ack_rate; /** confirms or overrides request */
    bool has_result_message;
    char result_message[194];
} cr_FileTransferResponse;

typedef PB_BYTES_ARRAY_T(194) cr_FileTransferData_message_data_t;
/** Bi-Directional Message */
typedef struct _cr_FileTransferData {
    int32_t result; /** non-zero for error */
    uint32_t transfer_id; /** Transfer ID */
    uint32_t message_number; /** counts up */
    cr_FileTransferData_message_data_t message_data; /** Data */
    bool has_checksum;
    int32_t checksum; /** Optional RFC 1071 checksum for integrity checking */
} cr_FileTransferData;

typedef struct _cr_FileTransferDataNotification {
    int32_t result; /** 0 for success */
    bool has_result_message;
    char result_message[194];
    bool is_complete;
    uint32_t transfer_id; /** Transfer ID */
    uint32_t retry_offset; /** file offset where error occurred */
} cr_FileTransferDataNotification;

typedef struct _cr_FileEraseRequest {
    uint32_t file_id; /** File ID */
} cr_FileEraseRequest;

typedef struct _cr_FileEraseResponse {
    uint32_t file_id; /** File ID */
    int32_t result; /** err~ */
    bool has_result_message;
    char result_message[194];
} cr_FileEraseResponse;


/**
* @brief   (optional) Command Service
* @details Allows actions to be triggered from the Reach UI.
*/
typedef struct _cr_DiscoverCommands {
    char dummy_field;
} cr_DiscoverCommands;

typedef struct _cr_CommandInfo {
    uint32_t id;
    char name[24]; /** Descriptive name */
    bool has_description;
    char description[48]; /** Optional description of the command */
    bool has_timeout;
    uint32_t timeout; /** Optional command timeout */
} cr_CommandInfo;

typedef struct _cr_DiscoverCommandsResponse {
    pb_size_t available_commands_count;
    cr_CommandInfo available_commands[2]; /** Array of v:n    Values : Names */
} cr_DiscoverCommandsResponse;

typedef struct _cr_SendCommand {
    uint32_t command_id;
} cr_SendCommand;

typedef struct _cr_SendCommandResponse {
    int32_t result; /** Carries Success / Result */
    bool has_result_message;
    char result_message[194];
} cr_SendCommandResponse;

/** Optional Command Line Interface (CLI) service
  Bi-Directional Message supporting the CLI service */
typedef struct _cr_CLIData {
    char message_data[194]; /** Data */
} cr_CLIData;

/** Optional Time Service
 The time service is designed to allow the client to 
 set and adjust the real time clock in a server device.
 The time is best specified as UTC plus timezone offset.
 Although the timezone is optional, it's best to use it.*/
typedef struct _cr_TimeSetRequest {
    int64_t seconds_utc; /** linux epoch */
    bool has_timezone;
    int32_t timezone; /** seconds adjustment */
} cr_TimeSetRequest;

typedef struct _cr_TimeSetResponse {
    int32_t result; /** Carries Success / Result */
    bool has_result_message;
    char result_message[194];
} cr_TimeSetResponse;

typedef struct _cr_TimeGetRequest {
    char dummy_field;
} cr_TimeGetRequest;

typedef struct _cr_TimeGetResponse {
    int32_t result; /** Carries Success / Result */
    bool has_result_message;
    char result_message[194];
    int64_t seconds_utc; /** linux epoch */
    bool has_timezone;
    int32_t timezone; /** seconds adjustment */
} cr_TimeGetResponse;

/** ------------------------------------------------------
 Optional WiFi Service
 The WiFi service is intended to simplify the 
 repetitive task of communicating WiFi credentials to the device.
 ------------------------------------------------------ */
typedef struct _cr_ConnectionDescription {
    char ssid[32];
    bool has_signal_strength;
    int32_t signal_strength; /** dB, RSSI */
    bool has_sec;
    cr_WiFiSecurity sec;
    bool has_band;
    cr_WiFiBand band;
} cr_ConnectionDescription;

typedef struct _cr_DiscoverWiFiRequest {
    cr_WiFiState state; /** get the current connection, scanned SSID's */
} cr_DiscoverWiFiRequest;

typedef struct _cr_DiscoverWiFiResponse {
    int32_t result; /** 0 for success */
    bool has_result_message;
    char result_message[48];
    int32_t available_AP;
    cr_WiFiState state;
    uint32_t connectionId; /** connected is 0, others increment. */
    bool has_cd;
    cr_ConnectionDescription cd;
} cr_DiscoverWiFiResponse;

typedef struct _cr_WiFiConnectionRequest {
    cr_WiFiState action; /** connect or disconnect */
    bool has_password;
    char password[32]; /** required to connect */
    bool has_cd;
    cr_ConnectionDescription cd; /** required to connect */
    bool has_autoconnect;
    bool autoconnect; /** required to connect */
} cr_WiFiConnectionRequest;

typedef struct _cr_WiFiConnectionResponse {
    int32_t result; /** 0 for success */
    bool has_signal_strength;
    int32_t signal_strength; /** RSSI */
    bool has_result_message;
    char result_message[194]; /** describes any error */
} cr_WiFiConnectionResponse;

#endif
