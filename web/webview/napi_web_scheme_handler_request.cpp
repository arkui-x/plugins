/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi_web_scheme_handler_request.h"

#include <algorithm>
#include <unordered_set>

#include "business_error.h"
#include "log.h"
#include "napi_parse_utils.h"
#include "web_errors.h"
#include "web_scheme_handler_request.h"

namespace OHOS::Plugin {
static const std::unordered_map<int32_t, std::string> g_netErrorCodeMap = {
    {ARKWEB_NET_OK, "NET_OK"},
    {ARKWEB_ERR_IO_PENDING, "ERR_IO_PENDING"},
    {ARKWEB_ERR_FAILED, "ERR_FAILED"},
    {ARKWEB_ERR_ABORTED, "ERR_ABORTED"},
    {ARKWEB_ERR_INVALID_ARGUMENT, "ERR_INVALID_ARGUMENT"},
    {ARKWEB_ERR_INVALID_HANDLE, "ERR_INVALID_HANDLE"},
    {ARKWEB_ERR_FILE_NOT_FOUND, "ERR_FILE_NOT_FOUND"},
    {ARKWEB_ERR_TIMED_OUT, "ERR_TIMED_OUT"},
    {ARKWEB_ERR_FILE_TOO_LARGE, "ERR_FILE_TOO_LARGE"},
    {ARKWEB_ERR_UNEXPECTED, "ERR_UNEXPECTED"},
    {ARKWEB_ERR_ACCESS_DENIED, "ERR_ACCESS_DENIED"},
    {ARKWEB_ERR_NOT_IMPLEMENTED, "ERR_NOT_IMPLEMENTED"},
    {ARKWEB_ERR_INSUFFICIENT_RESOURCES, "ERR_INSUFFICIENT_RESOURCES"},
    {ARKWEB_ERR_OUT_OF_MEMORY, "ERR_OUT_OF_MEMORY"},
    {ARKWEB_ERR_UPLOAD_FILE_CHANGED, "ERR_UPLOAD_FILE_CHANGED"},
    {ARKWEB_ERR_SOCKET_NOT_CONNECTED, "ERR_SOCKET_NOT_CONNECTED"},
    {ARKWEB_ERR_FILE_EXISTS, "ERR_FILE_EXISTS"},
    {ARKWEB_ERR_FILE_PATH_TOO_LONG, "ERR_FILE_PATH_TOO_LONG"},
    {ARKWEB_ERR_FILE_NO_SPACE, "ERR_FILE_NO_SPACE"},
    {ARKWEB_ERR_FILE_VIRUS_INFECTED, "ERR_FILE_VIRUS_INFECTED"},
    {ARKWEB_ERR_BLOCKED_BY_CLIENT, "ERR_BLOCKED_BY_CLIENT"},
    {ARKWEB_ERR_NETWORK_CHANGED, "ERR_NETWORK_CHANGED"},
    {ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR, "ERR_BLOCKED_BY_ADMINISTRATOR"},
    {ARKWEB_ERR_SOCKET_CONNECTED, "ERR_SOCKET_CONNECTED"},
    {ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED, "ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED"},
    {ARKWEB_ERR_CONTEXT_SHUT_DOWN, "ERR_CONTEXT_SHUT_DOWN"},
    {ARKWEB_ERR_BLOCKED_BY_RESPONSE, "ERR_BLOCKED_BY_RESPONSE"},
    {ARKWEB_ERR_CLEARTEXT_NOT_PERMITTED, "ERR_CLEARTEXT_NOT_PERMITTED"},
    {ARKWEB_ERR_BLOCKED_BY_CSP, "ERR_BLOCKED_BY_CSP"},
    {ARKWEB_ERR_H2_OR_QUIC_REQUIRED, "ERR_H2_OR_QUIC_REQUIRED"},
    {ARKWEB_ERR_BLOCKED_BY_ORB, "ERR_BLOCKED_BY_ORB"},
    {ARKWEB_ERR_CONNECTION_CLOSED, "ERR_CONNECTION_CLOSED"},
    {ARKWEB_ERR_CONNECTION_RESET, "ERR_CONNECTION_RESET"},
    {ARKWEB_ERR_CONNECTION_REFUSED, "ERR_CONNECTION_REFUSED"},
    {ARKWEB_ERR_CONNECTION_ABORTED, "ERR_CONNECTION_ABORTED"},
    {ARKWEB_ERR_CONNECTION_FAILED, "ERR_CONNECTION_FAILED"},
    {ARKWEB_ERR_NAME_NOT_RESOLVED, "ERR_NAME_NOT_RESOLVED"},
    {ARKWEB_ERR_INTERNET_DISCONNECTED, "ERR_INTERNET_DISCONNECTED"},
    {ARKWEB_ERR_SSL_PROTOCOL_ERROR, "ERR_SSL_PROTOCOL_ERROR"},
    {ARKWEB_ERR_ADDRESS_INVALID, "ERR_ADDRESS_INVALID"},
    {ARKWEB_ERR_ADDRESS_UNREACHABLE, "ERR_ADDRESS_UNREACHABLE"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NEEDED, "ERR_SSL_CLIENT_AUTH_CERT_NEEDED"},
    {ARKWEB_ERR_TUNNEL_CONNECTION_FAILED, "ERR_TUNNEL_CONNECTION_FAILED"},
    {ARKWEB_ERR_NO_SSL_VERSIONS_ENABLED, "ERR_NO_SSL_VERSIONS_ENABLED"},
    {ARKWEB_ERR_SSL_VERSION_OR_CIPHER_MISMATCH, "ERR_SSL_VERSION_OR_CIPHER_MISMATCH"},
    {ARKWEB_ERR_SSL_RENEGOTIATION_REQUESTED, "ERR_SSL_RENEGOTIATION_REQUESTED"},
    {ARKWEB_ERR_PROXY_AUTH_UNSUPPORTED, "ERR_PROXY_AUTH_UNSUPPORTED"},
    {ARKWEB_ERR_BAD_SSL_CLIENT_AUTH_CERT, "ERR_BAD_SSL_CLIENT_AUTH_CERT"},
    {ARKWEB_ERR_CONNECTION_TIMED_OUT, "ERR_CONNECTION_TIMED_OUT"},
    {ARKWEB_ERR_HOST_RESOLVER_QUEUE_TOO_LARGE, "ERR_HOST_RESOLVER_QUEUE_TOO_LARGE"},
    {ARKWEB_ERR_SOCKS_CONNECTION_FAILED, "ERR_SOCKS_CONNECTION_FAILED"},
    {ARKWEB_ERR_SOCKS_CONNECTION_HOST_UNREACHABLE, "ERR_SOCKS_CONNECTION_HOST_UNREACHABLE"},
    {ARKWEB_ERR_ALPN_NEGOTIATION_FAILED, "ERR_ALPN_NEGOTIATION_FAILED"},
    {ARKWEB_ERR_SSL_NO_RENEGOTIATION, "ERR_SSL_NO_RENEGOTIATION"},
    {ARKWEB_ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES, "ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES"},
    {ARKWEB_ERR_SSL_DECOMPRESSION_FAILURE_ALERT, "ERR_SSL_DECOMPRESSION_FAILURE_ALERT"},
    {ARKWEB_ERR_SSL_BAD_RECORD_MAC_ALERT, "ERR_SSL_BAD_RECORD_MAC_ALERT"},
    {ARKWEB_ERR_PROXY_AUTH_REQUESTED, "ERR_PROXY_AUTH_REQUESTED"},
    {ARKWEB_ERR_PROXY_CONNECTION_FAILED, "ERR_PROXY_CONNECTION_FAILED"},
    {ARKWEB_ERR_MANDATORY_PROXY_CONFIGURATION_FAILED, "ERR_MANDATORY_PROXY_CONFIGURATION_FAILED"},
    {ARKWEB_ERR_PRECONNECT_MAX_SOCKET_LIMIT, "ERR_PRECONNECT_MAX_SOCKET_LIMIT"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED, "ERR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY, "ERR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY"},
    {ARKWEB_ERR_PROXY_CERTIFICATE_INVALID, "ERR_PROXY_CERTIFICATE_INVALID"},
    {ARKWEB_ERR_NAME_RESOLUTION_FAILED, "ERR_NAME_RESOLUTION_FAILED"},
    {ARKWEB_ERR_NETWORK_ACCESS_DENIED, "ERR_NETWORK_ACCESS_DENIED"},
    {ARKWEB_ERR_TEMPORARILY_THROTTLED, "ERR_TEMPORARILY_THROTTLED"},
    {ARKWEB_ERR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT, "ERR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED, "ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED"},
    {ARKWEB_ERR_MSG_TOO_BIG, "ERR_MSG_TOO_BIG"},
    {ARKWEB_ERR_WS_PROTOCOL_ERROR, "ERR_WS_PROTOCOL_ERROR"},
    {ARKWEB_ERR_ADDRESS_IN_USE, "ERR_ADDRESS_IN_USE"},
    {ARKWEB_ERR_SSL_HANDSHAKE_NOT_COMPLETED, "ERR_SSL_HANDSHAKE_NOT_COMPLETED"},
    {ARKWEB_ERR_SSL_BAD_PEER_PUBLIC_KEY, "ERR_SSL_BAD_PEER_PUBLIC_KEY"},
    {ARKWEB_ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN, "ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN"},
    {ARKWEB_ERR_CLIENT_AUTH_CERT_TYPE_UNSUPPORTED, "ERR_CLIENT_AUTH_CERT_TYPE_UNSUPPORTED"},
    {ARKWEB_ERR_SSL_DECRYPT_ERROR_ALERT, "ERR_SSL_DECRYPT_ERROR_ALERT"},
    {ARKWEB_ERR_WS_THROTTLE_QUEUE_TOO_LARGE, "ERR_WS_THROTTLE_QUEUE_TOO_LARGE"},
    {ARKWEB_ERR_SSL_SERVER_CERT_CHANGED, "ERR_SSL_SERVER_CERT_CHANGED"},
    {ARKWEB_ERR_SSL_UNRECOGNIZED_NAME_ALERT, "ERR_SSL_UNRECOGNIZED_NAME_ALERT"},
    {ARKWEB_ERR_SOCKET_SET_RECEIVE_BUFFER_SIZE_ERROR, "ERR_SOCKET_SET_RECEIVE_BUFFER_SIZE_ERROR"},
    {ARKWEB_ERR_SOCKET_SET_SEND_BUFFER_SIZE_ERROR, "ERR_SOCKET_SET_SEND_BUFFER_SIZE_ERROR"},
    {ARKWEB_ERR_SOCKET_RECEIVE_BUFFER_SIZE_UNCHANGEABLE, "ERR_SOCKET_RECEIVE_BUFFER_SIZE_UNCHANGEABLE"},
    {ARKWEB_ERR_SOCKET_SEND_BUFFER_SIZE_UNCHANGEABLE, "ERR_SOCKET_SEND_BUFFER_SIZE_UNCHANGEABLE"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_CERT_BAD_FORMAT, "ERR_SSL_CLIENT_AUTH_CERT_BAD_FORMAT"},
    {ARKWEB_ERR_ICANN_NAME_COLLISION, "ERR_ICANN_NAME_COLLISION"},
    {ARKWEB_ERR_SSL_SERVER_CERT_BAD_FORMAT, "ERR_SSL_SERVER_CERT_BAD_FORMAT"},
    {ARKWEB_ERR_CT_STH_PARSING_FAILED, "ERR_CT_STH_PARSING_FAILED"},
    {ARKWEB_ERR_CT_STH_INCOMPLETE, "ERR_CT_STH_INCOMPLETE"},
    {ARKWEB_ERR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH, "ERR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH"},
    {ARKWEB_ERR_CT_CONSISTENCY_PROOF_PARSING_FAILED, "ERR_CT_CONSISTENCY_PROOF_PARSING_FAILED"},
    {ARKWEB_ERR_SSL_OBSOLETE_CIPHER, "ERR_SSL_OBSOLETE_CIPHER"},
    {ARKWEB_ERR_WS_UPGRADE, "ERR_WS_UPGRADE"},
    {ARKWEB_ERR_READ_IF_READY_NOT_IMPLEMENTED, "ERR_READ_IF_READY_NOT_IMPLEMENTED"},
    {ARKWEB_ERR_NO_BUFFER_SPACE, "ERR_NO_BUFFER_SPACE"},
    {ARKWEB_ERR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS, "ERR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS"},
    {ARKWEB_ERR_EARLY_DATA_REJECTED, "ERR_EARLY_DATA_REJECTED"},
    {ARKWEB_ERR_WRONG_VERSION_ON_EARLY_DATA, "ERR_WRONG_VERSION_ON_EARLY_DATA"},
    {ARKWEB_ERR_TLS13_DOWNGRADE_DETECTED, "ERR_TLS13_DOWNGRADE_DETECTED"},
    {ARKWEB_ERR_SSL_KEY_USAGE_INCOMPATIBLE, "ERR_SSL_KEY_USAGE_INCOMPATIBLE"},
    {ARKWEB_ERR_INVALID_ECH_CONFIG_LIST, "ERR_INVALID_ECH_CONFIG_LIST"},
    {ARKWEB_ERR_ECH_NOT_NEGOTIATED, "ERR_ECH_NOT_NEGOTIATED"},
    {ARKWEB_ERR_ECH_FALLBACK_CERTIFICATE_INVALID, "ERR_ECH_FALLBACK_CERTIFICATE_INVALID"},
    {ARKWEB_ERR_CERT_COMMON_NAME_INVALID, "ERR_CERT_COMMON_NAME_INVALID"},
    {ARKWEB_ERR_CERT_DATE_INVALID, "ERR_CERT_DATE_INVALID"},
    {ARKWEB_ERR_CERT_AUTHORITY_INVALID, "ERR_CERT_AUTHORITY_INVALID"},
    {ARKWEB_ERR_CERT_CONTAINS_ERRORS, "ERR_CERT_CONTAINS_ERRORS"},
    {ARKWEB_ERR_CERT_NO_REVOCATION_MECHANISM, "ERR_CERT_NO_REVOCATION_MECHANISM"},
    {ARKWEB_ERR_CERT_UNABLE_TO_CHECK_REVOCATION, "ERR_CERT_UNABLE_TO_CHECK_REVOCATION"},
    {ARKWEB_ERR_CERT_REVOKED, "ERR_CERT_REVOKED"},
    {ARKWEB_ERR_CERT_INVALID, "ERR_CERT_INVALID"},
    {ARKWEB_ERR_CERT_WEAK_SIGNATURE_ALGORITHM, "ERR_CERT_WEAK_SIGNATURE_ALGORITHM"},
    {ARKWEB_ERR_CERT_NON_UNIQUE_NAME, "ERR_CERT_NON_UNIQUE_NAME"},
    {ARKWEB_ERR_CERT_WEAK_KEY, "ERR_CERT_WEAK_KEY"},
    {ARKWEB_ERR_CERT_NAME_CONSTRAINT_VIOLATION, "ERR_CERT_NAME_CONSTRAINT_VIOLATION"},
    {ARKWEB_ERR_CERT_VALIDITY_TOO_LONG, "ERR_CERT_VALIDITY_TOO_LONG"},
    {ARKWEB_ERR_CERTIFICATE_TRANSPARENCY_REQUIRED, "ERR_CERTIFICATE_TRANSPARENCY_REQUIRED"},
    {ARKWEB_ERR_CERT_SYMANTEC_LEGACY, "ERR_CERT_SYMANTEC_LEGACY"},
    {ARKWEB_ERR_CERT_KNOWN_INTERCEPTION_BLOCKED, "ERR_CERT_KNOWN_INTERCEPTION_BLOCKED"},
    {ARKWEB_ERR_SSL_OBSOLETE_VERSION_OR_CIPHER, "ERR_SSL_OBSOLETE_VERSION_OR_CIPHER"},
    {ARKWEB_ERR_CERT_END, "ERR_CERT_END"},
    {ARKWEB_ERR_INVALID_URL, "ERR_INVALID_URL"},
    {ARKWEB_ERR_DISALLOWED_URL_SCHEME, "ERR_DISALLOWED_URL_SCHEME"},
    {ARKWEB_ERR_UNKNOWN_URL_SCHEME, "ERR_UNKNOWN_URL_SCHEME"},
    {ARKWEB_ERR_INVALID_REDIRECT, "ERR_INVALID_REDIRECT"},
    {ARKWEB_ERR_TOO_MANY_REDIRECTS, "ERR_TOO_MANY_REDIRECTS"},
    {ARKWEB_ERR_UNSAFE_REDIRECT, "ERR_UNSAFE_REDIRECT"},
    {ARKWEB_ERR_UNSAFE_PORT, "ERR_UNSAFE_PORT"},
    {ARKWEB_ERR_INVALID_RESPONSE, "ERR_INVALID_RESPONSE"},
    {ARKWEB_ERR_INVALID_CHUNKED_ENCODING, "ERR_INVALID_CHUNKED_ENCODING"},
    {ARKWEB_ERR_METHOD_UNSUPPORTED, "ERR_METHOD_UNSUPPORTED"},
    {ARKWEB_ERR_UNEXPECTED_PROXY_AUTH, "ERR_UNEXPECTED_PROXY_AUTH"},
    {ARKWEB_ERR_EMPTY_RESPONSE, "ERR_EMPTY_RESPONSE"},
    {ARKWEB_ERR_RESPONSE_HEADERS_TOO_BIG, "ERR_RESPONSE_HEADERS_TOO_BIG"},
    {ARKWEB_ERR_PAC_SCRIPT_FAILED, "ERR_PAC_SCRIPT_FAILED"},
    {ARKWEB_ERR_REQUEST_RANGE_NOT_SATISFIABLE, "ERR_REQUEST_RANGE_NOT_SATISFIABLE"},
    {ARKWEB_ERR_MALFORMED_IDENTITY, "ERR_MALFORMED_IDENTITY"},
    {ARKWEB_ERR_CONTENT_DECODING_FAILED, "ERR_CONTENT_DECODING_FAILED"},
    {ARKWEB_ERR_NETWORK_IO_SUSPENDED, "ERR_NETWORK_IO_SUSPENDED"},
    {ARKWEB_ERR_SYN_REPLY_NOT_RECEIVED, "ERR_SYN_REPLY_NOT_RECEIVED"},
    {ARKWEB_ERR_ENCODING_CONVERSION_FAILED, "ERR_ENCODING_CONVERSION_FAILED"},
    {ARKWEB_ERR_UNRECOGNIZED_FTP_DIRECTORY_LISTING_FORMAT, "ERR_UNRECOGNIZED_FTP_DIRECTORY_LISTING_FORMAT"},
    {ARKWEB_ERR_NO_SUPPORTED_PROXIES, "ERR_NO_SUPPORTED_PROXIES"},
    {ARKWEB_ERR_HTTP2_PROTOCOL_ERROR, "ERR_HTTP2_PROTOCOL_ERROR"},
    {ARKWEB_ERR_INVALID_AUTH_CREDENTIALS, "ERR_INVALID_AUTH_CREDENTIALS"},
    {ARKWEB_ERR_UNSUPPORTED_AUTH_SCHEME, "ERR_UNSUPPORTED_AUTH_SCHEME"},
    {ARKWEB_ERR_ENCODING_DETECTION_FAILED, "ERR_ENCODING_DETECTION_FAILED"},
    {ARKWEB_ERR_MISSING_AUTH_CREDENTIALS, "ERR_MISSING_AUTH_CREDENTIALS"},
    {ARKWEB_ERR_UNEXPECTED_SECURITY_LIBRARY_STATUS, "ERR_UNEXPECTED_SECURITY_LIBRARY_STATUS"},
    {ARKWEB_ERR_MISCONFIGURED_AUTH_ENVIRONMENT, "ERR_MISCONFIGURED_AUTH_ENVIRONMENT"},
    {ARKWEB_ERR_UNDOCUMENTED_SECURITY_LIBRARY_STATUS, "ERR_UNDOCUMENTED_SECURITY_LIBRARY_STATUS"},
    {ARKWEB_ERR_RESPONSE_BODY_TOO_BIG_TO_DRAIN, "ERR_RESPONSE_BODY_TOO_BIG_TO_DRAIN"},
    {ARKWEB_ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH, "ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH"},
    {ARKWEB_ERR_INCOMPLETE_HTTP2_HEADERS, "ERR_INCOMPLETE_HTTP2_HEADERS"},
    {ARKWEB_ERR_PAC_NOT_IN_DHCP, "ERR_PAC_NOT_IN_DHCP"},
    {ARKWEB_ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION, "ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION"},
    {ARKWEB_ERR_RESPONSE_HEADERS_MULTIPLE_LOCATION, "ERR_RESPONSE_HEADERS_MULTIPLE_LOCATION"},
    {ARKWEB_ERR_HTTP2_SERVER_REFUSED_STREAM, "ERR_HTTP2_SERVER_REFUSED_STREAM"},
    {ARKWEB_ERR_HTTP2_PING_FAILED, "ERR_HTTP2_PING_FAILED"},
    {ARKWEB_ERR_CONTENT_LENGTH_MISMATCH, "ERR_CONTENT_LENGTH_MISMATCH"},
    {ARKWEB_ERR_INCOMPLETE_CHUNKED_ENCODING, "ERR_INCOMPLETE_CHUNKED_ENCODING"},
    {ARKWEB_ERR_QUIC_PROTOCOL_ERROR, "ERR_QUIC_PROTOCOL_ERROR"},
    {ARKWEB_ERR_RESPONSE_HEADERS_TRUNCATED, "ERR_RESPONSE_HEADERS_TRUNCATED"},
    {ARKWEB_ERR_QUIC_HANDSHAKE_FAILED, "ERR_QUIC_HANDSHAKE_FAILED"},
    {ARKWEB_ERR_HTTP2_INADEQUATE_TRANSPORT_SECURITY, "ERR_HTTP2_INADEQUATE_TRANSPORT_SECURITY"},
    {ARKWEB_ERR_HTTP2_FLOW_CONTROL_ERROR, "ERR_HTTP2_FLOW_CONTROL_ERROR"},
    {ARKWEB_ERR_HTTP2_FRAME_SIZE_ERROR, "ERR_HTTP2_FRAME_SIZE_ERROR"},
    {ARKWEB_ERR_HTTP2_COMPRESSION_ERROR, "ERR_HTTP2_COMPRESSION_ERROR"},
    {ARKWEB_ERR_PROXY_AUTH_REQUESTED_WITH_NO_CONNECTION, "ERR_PROXY_AUTH_REQUESTED_WITH_NO_CONNECTION"},
    {ARKWEB_ERR_HTTP_1_1_REQUIRED, "ERR_HTTP_1_1_REQUIRED"},
    {ARKWEB_ERR_PROXY_HTTP_1_1_REQUIRED, "ERR_PROXY_HTTP_1_1_REQUIRED"},
    {ARKWEB_ERR_PAC_SCRIPT_TERMINATED, "ERR_PAC_SCRIPT_TERMINATED"},
    {ARKWEB_ERR_INVALID_HTTP_RESPONSE, "ERR_INVALID_HTTP_RESPONSE"},
    {ARKWEB_ERR_CONTENT_DECODING_INIT_FAILED, "ERR_CONTENT_DECODING_INIT_FAILED"},
    {ARKWEB_ERR_HTTP2_RST_STREAM_NO_ERROR_RECEIVED, "ERR_HTTP2_RST_STREAM_NO_ERROR_RECEIVED"},
    {ARKWEB_ERR_HTTP2_PUSHED_STREAM_NOT_AVAILABLE, "ERR_HTTP2_PUSHED_STREAM_NOT_AVAILABLE"},
    {ARKWEB_ERR_HTTP2_CLAIMED_PUSHED_STREAM_RESET_BY_SERVER, "ERR_HTTP2_CLAIMED_PUSHED_STREAM_RESET_BY_SERVER"},
    {ARKWEB_ERR_TOO_MANY_RETRIES, "ERR_TOO_MANY_RETRIES"},
    {ARKWEB_ERR_HTTP2_STREAM_CLOSED, "ERR_HTTP2_STREAM_CLOSED"},
    {ARKWEB_ERR_HTTP2_CLIENT_REFUSED_STREAM, "ERR_HTTP2_CLIENT_REFUSED_STREAM"},
    {ARKWEB_ERR_HTTP2_PUSHED_RESPONSE_DOES_NOT_MATCH, "ERR_HTTP2_PUSHED_RESPONSE_DOES_NOT_MATCH"},
    {ARKWEB_ERR_HTTP_RESPONSE_CODE_FAILURE, "ERR_HTTP_RESPONSE_CODE_FAILURE"},
    {ARKWEB_ERR_QUIC_UNKNOWN_CERT_ROOT, "ERR_QUIC_UNKNOWN_CERT_ROOT"},
    {ARKWEB_ERR_QUIC_GOAWAY_REQUEST_CAN_BE_RETRIED, "ERR_QUIC_GOAWAY_REQUEST_CAN_BE_RETRIED"},
    {ARKWEB_ERR_TOO_MANY_ACCEPT_CH_RESTARTS, "ERR_TOO_MANY_ACCEPT_CH_RESTARTS"},
    {ARKWEB_ERR_INCONSISTENT_IP_ADDRESS_SPACE, "ERR_INCONSISTENT_IP_ADDRESS_SPACE"},
    {ARKWEB_ERR_CACHED_IP_ADDRESS_SPACE_BLOCKED_BY_LOCAL_NETWORK_ACCESS_POLICY,
        "ERR_CACHED_IP_ADDRESS_SPACE_BLOCKED_BY_LOCAL_NETWORK_ACCESS_POLICY"},
    {ARKWEB_ERR_CACHE_MISS, "ERR_CACHE_MISS"},
    {ARKWEB_ERR_CACHE_READ_FAILURE, "ERR_CACHE_READ_FAILURE"},
    {ARKWEB_ERR_CACHE_WRITE_FAILURE, "ERR_CACHE_WRITE_FAILURE"},
    {ARKWEB_ERR_CACHE_OPERATION_UNSUPPORTED, "ERR_CACHE_OPERATION_UNSUPPORTED"},
    {ARKWEB_ERR_CACHE_OPEN_FAILURE, "ERR_CACHE_OPEN_FAILURE"},
    {ARKWEB_ERR_CACHE_CREATE_FAILURE, "ERR_CACHE_CREATE_FAILURE"},
    {ARKWEB_ERR_CACHE_RACE, "ERR_CACHE_RACE"},
    {ARKWEB_ERR_CACHE_CHECKSUM_READ_FAILURE, "ERR_CACHE_CHECKSUM_READ_FAILURE"},
    {ARKWEB_ERR_CACHE_CHECKSUM_MISMATCH, "ERR_CACHE_CHECKSUM_MISMATCH"},
    {ARKWEB_ERR_CACHE_LOCK_TIMEOUT, "ERR_CACHE_LOCK_TIMEOUT"},
    {ARKWEB_ERR_CACHE_AUTH_FAILURE_AFTER_READ, "ERR_CACHE_AUTH_FAILURE_AFTER_READ"},
    {ARKWEB_ERR_CACHE_ENTRY_NOT_SUITABLE, "ERR_CACHE_ENTRY_NOT_SUITABLE"},
    {ARKWEB_ERR_CACHE_DOOM_FAILURE, "ERR_CACHE_DOOM_FAILURE"},
    {ARKWEB_ERR_CACHE_OPEN_OR_CREATE_FAILURE, "ERR_CACHE_OPEN_OR_CREATE_FAILURE"},
    {ARKWEB_ERR_INSECURE_RESPONSE, "ERR_INSECURE_RESPONSE"},
    {ARKWEB_ERR_NO_PRIVATE_KEY_FOR_CERT, "ERR_NO_PRIVATE_KEY_FOR_CERT"},
    {ARKWEB_ERR_ADD_USER_CERT_FAILED, "ERR_ADD_USER_CERT_FAILED"},
    {ARKWEB_ERR_INVALID_SIGNED_EXCHANGE, "ERR_INVALID_SIGNED_EXCHANGE"},
    {ARKWEB_ERR_INVALID_WEB_BUNDLE, "ERR_INVALID_WEB_BUNDLE"},
    {ARKWEB_ERR_TRUST_TOKEN_OPERATION_FAILED, "ERR_TRUST_TOKEN_OPERATION_FAILED"},
    {ARKWEB_ERR_TRUST_TOKEN_OPERATION_SUCCESS_WITHOUT_SENDING_REQUEST,
        "ERR_TRUST_TOKEN_OPERATION_SUCCESS_WITHOUT_SENDING_REQUEST"},
    {ARKWEB_ERR_FTP_FAILED, "ERR_FTP_FAILED"},
    {ARKWEB_ERR_FTP_SERVICE_UNAVAILABLE, "ERR_FTP_SERVICE_UNAVAILABLE"},
    {ARKWEB_ERR_FTP_TRANSFER_ABORTED, "ERR_FTP_TRANSFER_ABORTED"},
    {ARKWEB_ERR_FTP_FILE_BUSY, "ERR_FTP_FILE_BUSY"},
    {ARKWEB_ERR_FTP_SYNTAX_ERROR, "ERR_FTP_SYNTAX_ERROR"},
    {ARKWEB_ERR_FTP_COMMAND_UNSUPPORTED, "ERR_FTP_COMMAND_UNSUPPORTED"},
    {ARKWEB_ERR_FTP_BAD_COMMAND_SEQUENCE, "ERR_FTP_BAD_COMMAND_SEQUENCE"},
    {ARKWEB_ERR_PKCS12_IMPORT_BAD_PASSWORD, "ERR_PKCS12_IMPORT_BAD_PASSWORD"},
    {ARKWEB_ERR_PKCS12_IMPORT_FAILED, "ERR_PKCS12_IMPORT_FAILED"},
    {ARKWEB_ERR_IMPORT_CA_CERT_NOT_CA, "ERR_IMPORT_CA_CERT_NOT_CA"},
    {ARKWEB_ERR_IMPORT_CERT_ALREADY_EXISTS, "ERR_IMPORT_CERT_ALREADY_EXISTS"},
    {ARKWEB_ERR_IMPORT_CA_CERT_FAILED, "ERR_IMPORT_CA_CERT_FAILED"},
    {ARKWEB_ERR_IMPORT_SERVER_CERT_FAILED, "ERR_IMPORT_SERVER_CERT_FAILED"},
    {ARKWEB_ERR_PKCS12_IMPORT_INVALID_MAC, "ERR_PKCS12_IMPORT_INVALID_MAC"},
    {ARKWEB_ERR_PKCS12_IMPORT_INVALID_FILE, "ERR_PKCS12_IMPORT_INVALID_FILE"},
    {ARKWEB_ERR_PKCS12_IMPORT_UNSUPPORTED, "ERR_PKCS12_IMPORT_UNSUPPORTED"},
    {ARKWEB_ERR_KEY_GENERATION_FAILED, "ERR_KEY_GENERATION_FAILED"},
    {ARKWEB_ERR_PRIVATE_KEY_EXPORT_FAILED, "ERR_PRIVATE_KEY_EXPORT_FAILED"},
    {ARKWEB_ERR_SELF_SIGNED_CERT_GENERATION_FAILED, "ERR_SELF_SIGNED_CERT_GENERATION_FAILED"},
    {ARKWEB_ERR_CERT_DATABASE_CHANGED, "ERR_CERT_DATABASE_CHANGED"},
    {ARKWEB_ERR_CERT_VERIFIER_CHANGED, "ERR_CERT_VERIFIER_CHANGED"},
    {ARKWEB_ERR_DNS_MALFORMED_RESPONSE, "ERR_DNS_MALFORMED_RESPONSE"},
    {ARKWEB_ERR_DNS_SERVER_REQUIRES_TCP, "ERR_DNS_SERVER_REQUIRES_TCP"},
    {ARKWEB_ERR_DNS_SERVER_FAILED, "ERR_DNS_SERVER_FAILED"},
    {ARKWEB_ERR_DNS_TIMED_OUT, "ERR_DNS_TIMED_OUT"},
    {ARKWEB_ERR_DNS_CACHE_MISS, "ERR_DNS_CACHE_MISS"},
    {ARKWEB_ERR_DNS_SEARCH_EMPTY, "ERR_DNS_SEARCH_EMPTY"},
    {ARKWEB_ERR_DNS_SORT_ERROR, "ERR_DNS_SORT_ERROR"},
    {ARKWEB_ERR_DNS_SECURE_RESOLVER_HOSTNAME_RESOLUTION_FAILED, "ERR_DNS_SECURE_RESOLVER_HOSTNAME_RESOLUTION_FAILED"},
    {ARKWEB_ERR_DNS_NAME_HTTPS_ONLY, "ERR_DNS_NAME_HTTPS_ONLY"},
    {ARKWEB_ERR_DNS_REQUEST_CANCELED, "ERR_DNS_REQUEST_CANCELED"},
    {ARKWEB_ERR_DNS_NO_MATCHING_SUPPORTED_ALPN, "ERR_DNS_NO_MATCHING_SUPPORTED_ALPN"}
};

struct NetErrorInfo {
    bool isValid;
    std::string errorName;

    explicit NetErrorInfo(bool valid = false, const std::string& name = "") : isValid(valid), errorName(name) {}
};

NetErrorInfo ValidateAndGetNetErrorName(int32_t code)
{
    auto it = g_netErrorCodeMap.find(code);
    if (it != g_netErrorCodeMap.end()) {
        return NetErrorInfo(true, it->second);
    }
    return NetErrorInfo(false, "UNKNOWN_ERROR_CODE");
}

napi_value NapiWebSchemeHandlerRequest::Init(napi_env env, napi_value exports)
{
    ExportWebSchemeHandlerRequestClass(env, &exports);
    return exports;
}

void NapiWebSchemeHandlerRequest::ExportWebSchemeHandlerRequestClass(napi_env env, napi_value* exportsPointer)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getHeader", JS_GetHeader),
        DECLARE_NAPI_FUNCTION("getRequestUrl", JS_GetRequestUrl),
        DECLARE_NAPI_FUNCTION("getRequestMethod", JS_GetRequestMethod),
        DECLARE_NAPI_FUNCTION("getReferrer", JS_GetReferrer),
        DECLARE_NAPI_FUNCTION("isRedirect", JS_IsRedirect),
        DECLARE_NAPI_FUNCTION("isMainFrame", JS_IsMainFrame),
        DECLARE_NAPI_FUNCTION("hasGesture", JS_HasGesture),
        DECLARE_NAPI_FUNCTION("getHttpBodyStream", JS_HttpBodyStream),
        DECLARE_NAPI_FUNCTION("getRequestResourceType", JS_GetRequestResourceType),
        DECLARE_NAPI_FUNCTION("getFrameUrl", JS_GetFrameUrl),
    };
    napi_value webSchemeHandlerRequest = nullptr;
    napi_define_class(env, WEB_SCHEME_HANDLER_REQUEST.c_str(), WEB_SCHEME_HANDLER_REQUEST.length(), JS_Constructor,
        nullptr, sizeof(properties) / sizeof(properties[0]), properties, &webSchemeHandlerRequest);
    napi_set_named_property(env, *exportsPointer, WEB_SCHEME_HANDLER_REQUEST.c_str(), webSchemeHandlerRequest);
}

napi_status NapiWebSchemeHandlerRequest::DefineProperties(napi_env env, napi_value* object)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getHeader", JS_GetHeader),
        DECLARE_NAPI_FUNCTION("getRequestUrl", JS_GetRequestUrl),
        DECLARE_NAPI_FUNCTION("getRequestMethod", JS_GetRequestMethod),
        DECLARE_NAPI_FUNCTION("getReferrer", JS_GetReferrer),
        DECLARE_NAPI_FUNCTION("isRedirect", JS_IsRedirect),
        DECLARE_NAPI_FUNCTION("isMainFrame", JS_IsMainFrame),
        DECLARE_NAPI_FUNCTION("hasGesture", JS_HasGesture),
        DECLARE_NAPI_FUNCTION("getHttpBodyStream", JS_HttpBodyStream),
        DECLARE_NAPI_FUNCTION("getRequestResourceType", JS_GetRequestResourceType),
        DECLARE_NAPI_FUNCTION("getFrameUrl", JS_GetFrameUrl),
    };
    return napi_define_properties(env, *object, sizeof(properties) / sizeof(properties[0]), properties);
}

napi_value NapiWebSchemeHandlerRequest::JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebSchemeHandlerRequest* request = new (std::nothrow) WebSchemeHandlerRequest(env);
    if (request == nullptr) {
        return nullptr;
    }

    napi_wrap(
        env, thisVar, request,
        [](napi_env /* env */, void* data, void* /* hint */) {
            WebSchemeHandlerRequest* request = reinterpret_cast<WebSchemeHandlerRequest*>(data);
            delete request;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetHeader(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetHeader request is nullptr");
        return nullptr;
    }

    napi_handle_scope scope;
    napi_status status_scope = napi_open_handle_scope(env, &scope);
    if (status_scope != napi_ok) {
        LOGE("scheme handler RequestStart scope is nullptr");
        return nullptr;
    }
    WebHeaderList list = request->GetHeaderList();
    napi_value result = nullptr;
    napi_create_array(env, &result);
    size_t headerSize = list.size();
    for (size_t index = 0; index < headerSize; index++) {
        napi_value webHeaderObj = nullptr;
        napi_value headerKey = nullptr;
        napi_value headerValue = nullptr;
        NAPI_CALL(env, napi_create_object(env, &webHeaderObj));
        napi_create_string_utf8(env, list[index].first.c_str(), NAPI_AUTO_LENGTH, &headerKey);
        napi_create_string_utf8(env, list[index].second.c_str(), NAPI_AUTO_LENGTH, &headerValue);
        napi_set_named_property(env, webHeaderObj, "headerKey", headerKey);
        napi_set_named_property(env, webHeaderObj, "headerValue", headerValue);
        napi_set_element(env, result, index, webHeaderObj);
    }
    napi_close_handle_scope(env, scope);
    return result;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetRequestUrl(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetRequestUrl request is nullptr");
        return nullptr;
    }

    napi_value value;
    napi_status status = napi_create_string_utf8(env, request->GetUrl().c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetRequestUrl response get url failed");
        return nullptr;
    }
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetRequestMethod(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetRequestMethod request is nullptr");
        return nullptr;
    }

    napi_value value;
    napi_status status = napi_create_string_utf8(env, request->GetMethod().c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetRequestMethod response get url failed");
        return nullptr;
    }
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetReferrer(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetReferrer request is nullptr");
        return nullptr;
    }

    napi_value value;
    napi_status status = napi_create_string_utf8(env, request->GetReferrer().c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetReferrer response get url failed");
        return nullptr;
    }
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_IsRedirect(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_IsRedirect request is nullptr");
        return nullptr;
    }

    napi_value value;
    NAPI_CALL(env, napi_get_boolean(env, request->GetIsRedirect(), &value));
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_IsMainFrame(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_IsMainFrame request is nullptr");
        return nullptr;
    }

    napi_value value;
    NAPI_CALL(env, napi_get_boolean(env, request->GetIsMainFrame(), &value));
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_HasGesture(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_HasGesture request is nullptr");
        return nullptr;
    }

    napi_value value;
    NAPI_CALL(env, napi_get_boolean(env, request->GetHasGesture(), &value));
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_HttpBodyStream(napi_env env, napi_callback_info cbinfo)
{
    napi_value value = nullptr;
    napi_get_undefined(env, &value);
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetRequestResourceType(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetRequestResourceType request is nullptr");
        return nullptr;
    }

    napi_value value;
    napi_create_int32(env, request->GetRequestResourceType(), &value);
    return value;
}

napi_value NapiWebSchemeHandlerRequest::JS_GetFrameUrl(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerRequest* request = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&request);
    if (!request) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetFrameUrl request is nullptr");
        return nullptr;
    }

    napi_value value;
    napi_status status = napi_create_string_utf8(env, request->GetFrameUrl().c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerRequest::JS_GetFrameUrl response get frame url failed");
        return nullptr;
    }
    return value;
}

napi_status NapiWebSchemeHandlerRequest::ExportEnumWebResourceType(napi_env env, napi_value* value)
{
    napi_value enumValue = nullptr;
    const std::string napiWebResourceTypeEnumName = "WebResourceType";
    return napi_set_named_property(env, *value, napiWebResourceTypeEnumName.c_str(), enumValue);
}

napi_value NapiWebSchemeHandlerResponse::Init(napi_env env, napi_value exports)
{
    ExportWebSchemeHandlerResponseClass(env, &exports);
    return exports;
}

void NapiWebSchemeHandlerResponse::ExportWebSchemeHandlerResponseClass(napi_env env, napi_value* exportsPointer)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getUrl", JS_GetUrl),
        DECLARE_NAPI_FUNCTION("setUrl", JS_SetUrl),
        DECLARE_NAPI_FUNCTION("getStatus", JS_GetStatus),
        DECLARE_NAPI_FUNCTION("setStatus", JS_SetStatus),
        DECLARE_NAPI_FUNCTION("getStatusText", JS_GetStatusText),
        DECLARE_NAPI_FUNCTION("setStatusText", JS_SetStatusText),
        DECLARE_NAPI_FUNCTION("getMimeType", JS_GetMimeType),
        DECLARE_NAPI_FUNCTION("setMimeType", JS_SetMimeType),
        DECLARE_NAPI_FUNCTION("getEncoding", JS_GetEncoding),
        DECLARE_NAPI_FUNCTION("setEncoding", JS_SetEncoding),
        DECLARE_NAPI_FUNCTION("getHeaderByName", JS_GetHeaderByName),
        DECLARE_NAPI_FUNCTION("setHeaderByName", JS_SetHeaderByName),
        DECLARE_NAPI_FUNCTION("getNetErrorCode", JS_GetNetErrorCode),
        DECLARE_NAPI_FUNCTION("setNetErrorCode", JS_SetNetErrorCode),
    };
    napi_value webSchemeHandlerResponse = nullptr;
    napi_define_class(env, WEB_SCHEME_HANDLER_RESPONSE.c_str(), WEB_SCHEME_HANDLER_RESPONSE.length(), JS_Constructor,
        nullptr, sizeof(properties) / sizeof(properties[0]), properties, &webSchemeHandlerResponse);
    napi_set_named_property(env, *exportsPointer, WEB_SCHEME_HANDLER_RESPONSE.c_str(), webSchemeHandlerResponse);
}

napi_value NapiWebSchemeHandlerResponse::JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    WebSchemeHandlerResponse* response = new (std::nothrow) WebSchemeHandlerResponse(env);
    if (response == nullptr) {
        LOGE("NapiWebSchemeHandlerResponse::JS_Constructor response is nullptr");
        NAPI_CALL(env, napi_get_undefined(env, &thisVar));
        return thisVar;
    }

    napi_wrap(
        env, thisVar, response,
        [](napi_env /* env */, void* data, void* /* hint */) {
            WebSchemeHandlerResponse* response = (WebSchemeHandlerResponse*)data;
            delete response;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetUrl(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetUrl response is nullptr");
        return nullptr;
    }

    napi_value urlValue;
    napi_status status = napi_create_string_utf8(env, response->GetUrl().c_str(), NAPI_AUTO_LENGTH, &urlValue);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetUrl response get url failed");
        return nullptr;
    }
    return urlValue;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetUrl(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    std::string url = "";
    if (!NapiParseUtils::ParseString(env, argv[0], url)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "url", "string"));
        return nullptr;
    }
    response->SetUrl(url);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetStatus(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetStatus response is nullptr");
        return nullptr;
    }

    napi_value status;
    napi_create_int32(env, response->GetStatus(), &status);
    return status;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetStatus(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    int32_t status = 0;
    if (!NapiParseUtils::ParseInt32(env, argv[0], status)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "code", "int"));
        LOGE("NapiWebSchemeHandlerResponse::JS_SetStatus parse failed");
        return nullptr;
    }
    response->SetStatus(status);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetStatusText(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetStatusText response is nullptr");
        return nullptr;
    }

    napi_value statusText;
    napi_status status = napi_create_string_utf8(env, response->GetStatusText().c_str(), NAPI_AUTO_LENGTH, &statusText);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetStatusText response get failed");
        return nullptr;
    }
    return statusText;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetStatusText(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    std::string statusText = "";
    if (!NapiParseUtils::ParseString(env, argv[0], statusText)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "text", "string"));
        return nullptr;
    }
    response->SetStatusText(statusText);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetMimeType(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetMimeType response is nullptr");
        return nullptr;
    }

    napi_value mimeType;
    napi_status status = napi_create_string_utf8(env, response->GetMimeType().c_str(), NAPI_AUTO_LENGTH, &mimeType);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetMimeType response get failed");
        return nullptr;
    }
    return mimeType;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetMimeType(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);
    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    std::string mimeType = "";
    if (!NapiParseUtils::ParseString(env, argv[0], mimeType)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "type", "string"));
        return nullptr;
    }
    response->SetMimeType(mimeType);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetEncoding(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetEncoding response is nullptr");
        return nullptr;
    }

    napi_value encoding;
    napi_status status = napi_create_string_utf8(env, response->GetEncoding().c_str(), NAPI_AUTO_LENGTH, &encoding);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetEncoding response get failed");
        return nullptr;
    }
    return encoding;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetEncoding(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    std::string encoding = "";
    if (!NapiParseUtils::ParseString(env, argv[0], encoding)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "encoding", "string"));
        return nullptr;
    }
    response->SetEncoding(encoding);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetHeaderByName(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetHeaderByName response is nullptr");
        return nullptr;
    }

    std::string name;
    if (!NapiParseUtils::ParseString(env, argv[0], name)) {
        return nullptr;
    }

    napi_value headerValue;
    std::string value = response->GetHeaderByName(name);
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &headerValue);
    if (status != napi_ok) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetHeaderByName response get failed");
        return nullptr;
    }
    return headerValue;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetHeaderByName(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = INTEGER_THREE;
    napi_value argv[INTEGER_THREE] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    WebSchemeHandlerResponse* response = nullptr;
    napi_unwrap(env, thisVar, (void**)&response);

    if (!response) {
        LOGE("unwrap WebSchemeHandlerResponse failed");
        return nullptr;
    }
    std::string name;
    std::string value;
    bool overwrite = false;
    if (argc != INTEGER_THREE) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "three"));
        LOGE("NapiWebSchemeHandlerResponse::JS_SetHeaderByName parse failed");
        return nullptr;
    }
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ZERO], name)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "name", "string"));
        LOGE("NapiWebSchemeHandlerResponse::JS_SetHeaderByName parse failed");
        return nullptr;
    }
    if (!NapiParseUtils::ParseString(env, argv[INTEGER_ONE], value)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "value", "string"));
        LOGE("NapiWebSchemeHandlerResponse::JS_SetHeaderByName parse failed");
        return nullptr;
    }
    if (!NapiParseUtils::ParseBoolean(env, argv[INTEGER_TWO], overwrite)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "overwrite", "booleane"));
        LOGE("NapiWebSchemeHandlerResponse::JS_SetHeaderByName parse failed");
        return nullptr;
    }
    response->SetHeaderByName(name, value, overwrite);
    return nullptr;
}

napi_value NapiWebSchemeHandlerResponse::JS_GetNetErrorCode(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetEncoding response is nullptr");
        return nullptr;
    }

    napi_value code;
    NAPI_CALL(env, napi_create_int32(env, response->GetErrorCode(), &code));
    return code;
}

napi_value NapiWebSchemeHandlerResponse::JS_SetNetErrorCode(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = INTEGER_ONE;
    napi_value argv[INTEGER_ONE] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WebSchemeHandlerResponse* response = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    napi_unwrap(env, thisVar, (void**)&response);
    if (!response) {
        LOGE("NapiWebSchemeHandlerResponse::JS_GetEncoding response is nullptr");
        return nullptr;
    }
    int32_t code = 0;
    if (argc != INTEGER_ONE) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return nullptr;
    }
    if (!NapiParseUtils::ParseInt32(env, argv[0], code)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "code", "int"));
        return nullptr;
    }
    NetErrorInfo errorInfo = ValidateAndGetNetErrorName(code);
    response->SetErrorCode(code, errorInfo.errorName.c_str());
    return nullptr;
}

napi_value NapiWebResourceHandler::Init(napi_env env, napi_value exports)
{
    ExportWebResourceHandlerClass(env, &exports);
    return exports;
}

void NapiWebResourceHandler::ExportWebResourceHandlerClass(napi_env env, napi_value* exportsPointer)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("didReceiveResponse", JS_DidReceiveResponse),
        DECLARE_NAPI_FUNCTION("didReceiveResponseBody", JS_DidReceiveResponseBody),
        DECLARE_NAPI_FUNCTION("didFinish", JS_DidFinish),
        DECLARE_NAPI_FUNCTION("didFail", JS_DidFailWithError),
    };
    napi_value webResourceHandler = nullptr;
    napi_define_class(env, WEB_RESOURCE_HANDLER.c_str(), WEB_RESOURCE_HANDLER.length(), JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &webResourceHandler);
    napi_set_named_property(env, *exportsPointer, WEB_RESOURCE_HANDLER.c_str(), webResourceHandler);
}

napi_status NapiWebResourceHandler::DefineProperties(napi_env env, napi_value* object)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("didReceiveResponse", JS_DidReceiveResponse),
        DECLARE_NAPI_FUNCTION("didReceiveResponseBody", JS_DidReceiveResponseBody),
        DECLARE_NAPI_FUNCTION("didFinish", JS_DidFinish),
        DECLARE_NAPI_FUNCTION("didFail", JS_DidFailWithError),
    };
    return napi_define_properties(env, *object, sizeof(properties) / sizeof(properties[0]), properties);
}

napi_value NapiWebResourceHandler::JS_Constructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    WebResourceHandler* handler = new WebResourceHandler(env);

    napi_wrap(
        env, thisVar, handler,
        [](napi_env /* env */, void* data, void* /* hint */) {
            WebResourceHandler* handler = (WebResourceHandler*)data;
            delete handler;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value NapiWebResourceHandler::JS_DidReceiveResponse(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    if (argc != 1) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return nullptr;
    }

    WebResourceHandler* resourceHandler = nullptr;
    napi_unwrap(env, thisVar, (void**)&resourceHandler);

    if (!resourceHandler) {
        LOGE("JS_DidReceiveResponse unwrap resource handler failed");
        return nullptr;
    }

    WebSchemeHandlerResponse* response = nullptr;
    napi_value obj = argv[0];
    napi_unwrap(env, obj, (void**)&response);
    if (!response) {
        LOGE("JS_DidReceiveResponse unwrap response failed");
        return nullptr;
    }
    int32_t ret = resourceHandler->DidReceiveResponse(response->GetArkWebResponse());
    if (ret != 0) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::RESOURCE_HANDLER_INVALID);
    }
    return nullptr;
}

napi_value NapiWebResourceHandler::JS_DidReceiveResponseBody(napi_env env, napi_callback_info info)
{
    LOGI("NapiWebResourceHandler::JS_DidReceiveResponseBody enter");
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    if (argc != 1) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
            NWebError::FormatString(ParamCheckErrorMsgTemplate::PARAM_NUMBERS_ERROR_ONE, "one"));
        return nullptr;
    }

    WebResourceHandler* resourceHandler = nullptr;
    napi_unwrap(env, thisVar, (void**)&resourceHandler);

    if (!resourceHandler) {
        LOGE("unwrap resource handler failed");
        return nullptr;
    }
    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[0], &isArrayBuffer));
    if (!isArrayBuffer) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR);
        return nullptr;
    }

    uint8_t* arrBuf = nullptr;
    size_t byteLength = 0;
    napi_get_arraybuffer_info(env, argv[0], (void**)&arrBuf, &byteLength);
    int32_t ret = resourceHandler->DidReceiveResponseBody(arrBuf, static_cast<int64_t>(byteLength));
    if (ret != 0) {
        LOGE("JS_DidReceiveResponseBody ret=%{public}d", ret);
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::RESOURCE_HANDLER_INVALID);
    }
    return nullptr;
}

napi_value NapiWebResourceHandler::JS_DidFinish(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    WebResourceHandler* resourceHandler = nullptr;
    napi_unwrap(env, thisVar, (void**)&resourceHandler);

    if (!resourceHandler) {
        LOGE("unwrap resource handler failed");
        return nullptr;
    }

    int32_t ret = resourceHandler->DidFinish();
    if (ret != 0) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::RESOURCE_HANDLER_INVALID);
        LOGE("JS_DidFinish ret=%{public}d", ret);
    }
    return nullptr;
}

napi_value NapiWebResourceHandler::JS_DidFailWithError(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    WebResourceHandler* resourceHandler = nullptr;
    napi_unwrap(env, thisVar, (void**)&resourceHandler);

    if (!resourceHandler) {
        LOGE("unwrap resource handler failed");
        return nullptr;
    }

    int32_t errorCode;
    static constexpr int LEVEL_COUNT = 2;
    if (!NapiParseUtils::ParseInt32(env, argv[0], errorCode)) {
        LOGE("JS_DidFailWithError unwrap error code failed");
        if (argc < LEVEL_COUNT) {
            NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::PARAM_CHECK_ERROR,
                NWebError::FormatString(ParamCheckErrorMsgTemplate::TYPE_ERROR, "code", "int"));
        } else {
            NWebError::BusinessError::ThrowErrorByErrcode(
                env, NWebError::INVALID_NET_ERROR, NWebError::GetErrMsgByErrCode(NWebError::INVALID_NET_ERROR));
        }

        return nullptr;
    }
 
    NetErrorInfo errorInfo = ValidateAndGetNetErrorName(errorCode);
    if (argc >= LEVEL_COUNT && (!errorInfo.isValid || errorCode == ARKWEB_NET_OK)) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::INVALID_NET_ERROR,
            NWebError::GetErrMsgByErrCode(NWebError::INVALID_NET_ERROR));
        return nullptr;
    }

    bool completeIfNoResponse = false;
    if (!NapiParseUtils::ParseBoolean(env, argv[1], completeIfNoResponse)) {
        LOGE("JS_DidFailWithError unwrap error completeIfNoResponse failed");
    }
    int32_t ret = resourceHandler->DidFailWithError(
        static_cast<ArkWeb_NetError>(errorCode), errorInfo.errorName.c_str(), completeIfNoResponse);
    if (ret != 0) {
        NWebError::BusinessError::ThrowErrorByErrcode(env, NWebError::RESOURCE_HANDLER_INVALID);
        LOGE("JS_DidFailWithError ret=%{public}d", ret);
    }
    return nullptr;
}
} // namespace OHOS::Plugin