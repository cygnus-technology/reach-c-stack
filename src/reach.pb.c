/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.4.8-dev */

#include "reach.pb.h"
#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

PB_BIND(cr_ReachMessageHeader, cr_ReachMessageHeader, AUTO)


PB_BIND(cr_ReachMessage, cr_ReachMessage, AUTO)


PB_BIND(cr_ErrorReport, cr_ErrorReport, AUTO)


PB_BIND(cr_PingRequest, cr_PingRequest, AUTO)


PB_BIND(cr_PingResponse, cr_PingResponse, AUTO)


PB_BIND(cr_DeviceInfoRequest, cr_DeviceInfoRequest, AUTO)


PB_BIND(cr_DeviceInfoResponse, cr_DeviceInfoResponse, AUTO)


PB_BIND(cr_ParameterInfoRequest, cr_ParameterInfoRequest, AUTO)


PB_BIND(cr_ParameterInfoResponse, cr_ParameterInfoResponse, 2)


PB_BIND(cr_ParameterInfo, cr_ParameterInfo, AUTO)


PB_BIND(cr_ParamExKey, cr_ParamExKey, AUTO)


PB_BIND(cr_ParamExInfoResponse, cr_ParamExInfoResponse, AUTO)


PB_BIND(cr_ParameterRead, cr_ParameterRead, AUTO)


PB_BIND(cr_ParameterReadResult, cr_ParameterReadResult, AUTO)


PB_BIND(cr_ParameterWrite, cr_ParameterWrite, AUTO)


PB_BIND(cr_ParameterWriteResult, cr_ParameterWriteResult, AUTO)


PB_BIND(cr_ParameterNotifyConfig, cr_ParameterNotifyConfig, AUTO)


PB_BIND(cr_ParameterNotifyConfigResponse, cr_ParameterNotifyConfigResponse, AUTO)


PB_BIND(cr_ParameterNotification, cr_ParameterNotification, AUTO)


PB_BIND(cr_ParameterValue, cr_ParameterValue, AUTO)


PB_BIND(cr_DiscoverFiles, cr_DiscoverFiles, AUTO)


PB_BIND(cr_DiscoverFilesResponse, cr_DiscoverFilesResponse, AUTO)


PB_BIND(cr_FileInfo, cr_FileInfo, AUTO)


PB_BIND(cr_FileTransferInit, cr_FileTransferInit, AUTO)


PB_BIND(cr_FileTransferInitResponse, cr_FileTransferInitResponse, AUTO)


PB_BIND(cr_FileTransferData, cr_FileTransferData, AUTO)


PB_BIND(cr_FileTransferDataNotification, cr_FileTransferDataNotification, AUTO)


PB_BIND(cr_FileEraseRequest, cr_FileEraseRequest, AUTO)


PB_BIND(cr_FileEraseResponse, cr_FileEraseResponse, AUTO)


PB_BIND(cr_DiscoverStreams, cr_DiscoverStreams, AUTO)


PB_BIND(cr_DiscoverStreamsResponse, cr_DiscoverStreamsResponse, AUTO)


PB_BIND(cr_StreamInfo, cr_StreamInfo, AUTO)


PB_BIND(cr_StreamOpen, cr_StreamOpen, AUTO)


PB_BIND(cr_StreamOpenResponse, cr_StreamOpenResponse, AUTO)


PB_BIND(cr_StreamClose, cr_StreamClose, AUTO)


PB_BIND(cr_StreamData, cr_StreamData, AUTO)


PB_BIND(cr_DiscoverCommands, cr_DiscoverCommands, AUTO)


PB_BIND(cr_DiscoverCommandsResponse, cr_DiscoverCommandsResponse, AUTO)


PB_BIND(cr_CommandInfo, cr_CommandInfo, AUTO)


PB_BIND(cr_SendCommand, cr_SendCommand, AUTO)


PB_BIND(cr_SendCommandResponse, cr_SendCommandResponse, AUTO)


PB_BIND(cr_CLIData, cr_CLIData, AUTO)


PB_BIND(cr_TimeSetRequest, cr_TimeSetRequest, AUTO)


PB_BIND(cr_TimeSetResponse, cr_TimeSetResponse, AUTO)


PB_BIND(cr_TimeGetRequest, cr_TimeGetRequest, AUTO)


PB_BIND(cr_TimeGetResponse, cr_TimeGetResponse, AUTO)


PB_BIND(cr_ConnectionDescription, cr_ConnectionDescription, AUTO)


PB_BIND(cr_DiscoverWiFiRequest, cr_DiscoverWiFiRequest, AUTO)


PB_BIND(cr_DiscoverWiFiResponse, cr_DiscoverWiFiResponse, AUTO)


PB_BIND(cr_WiFiConnectionRequest, cr_WiFiConnectionRequest, AUTO)


PB_BIND(cr_WiFiConnectionResponse, cr_WiFiConnectionResponse, AUTO)


PB_BIND(cr_BufferSizes, cr_BufferSizes, AUTO)

















#ifndef PB_CONVERT_DOUBLE_FLOAT
/* On some platforms (such as AVR), double is really float.
 * To be able to encode/decode double on these platforms, you need.
 * to define PB_CONVERT_DOUBLE_FLOAT in pb.h or compiler command line.
 */
PB_STATIC_ASSERT(sizeof(double) == 8, DOUBLE_MUST_BE_8_BYTES)
#endif

