/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef http_ios_constant_h
#define http_ios_constant_h
#import <Foundation/Foundation.h>
typedef enum {
    IOS_OK = 0,
    IOS_UNSUPPORTED_PROTOCOL,    /* 1 */
    IOS_FAILED_INIT,             /* 2 */
    IOS_URL_MALFORMAT,           /* 3 */
    IOS_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                      7.17.0, reused in April 2011 for 7.21.5] */
    IOS_COULDNT_RESOLVE_PROXY,   /* 5 */
    IOS_COULDNT_RESOLVE_HOST,    /* 6 */
    IOS_COULDNT_CONNECT,         /* 7 */
    IOS_WEIRD_SERVER_REPLY,      /* 8 */
    IOS_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                      due to lack of access - when login fails
                                      this is not returned. */
    IOS_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                      7.15.4, reused in Dec 2011 for 7.24.0] */
    IOS_FTP_WEIRD_PASS_REPLY,    /* 11 */
    IOS_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                      [was obsoleted in August 2007 for 7.17.0,
                                      reused in Dec 2011 for 7.24.0] */
    IOS_FTP_WEIRD_PASV_REPLY,    /* 13 */
    IOS_FTP_WEIRD_227_FORMAT,    /* 14 */
    IOS_FTP_CANT_GET_HOST,       /* 15 */
    IOS_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                      [was obsoleted in August 2007 for 7.17.0,
                                      reused in July 2014 for 7.38.0] */
    IOS_FTP_COULDNT_SET_TYPE,    /* 17 */
    IOS_PARTIAL_FILE,            /* 18 */
    IOS_FTP_COULDNT_RETR_FILE,   /* 19 */
    IOS_OBSOLETE20,              /* 20 - NOT USED */
    IOS_QUOTE_ERROR,             /* 21 - quote command failure */
    IOS_HTTP_RETURNED_ERROR,     /* 22 */
    IOS_WRITE_ERROR,             /* 23 */
    IOS_OBSOLETE24,              /* 24 - NOT USED */
    IOS_UPLOAD_FAILED,           /* 25 - failed upload "command" */
    IOS_READ_ERROR,              /* 26 - couldn't open/read from file */
    IOS_OUT_OF_MEMORY,           /* 27 */
    IOS_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
    IOS_OBSOLETE29,              /* 29 - NOT USED */
    IOS_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
    IOS_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
    IOS_OBSOLETE32,              /* 32 - NOT USED */
    IOS_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
    IOS_HTTP_POST_ERROR,         /* 34 */
    IOS_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
    IOS_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
    IOS_FILE_COULDNT_READ_FILE,  /* 37 */
    IOS_LDAP_CANNOT_BIND,        /* 38 */
    IOS_LDAP_SEARCH_FAILED,      /* 39 */
    IOS_OBSOLETE40,              /* 40 - NOT USED */
    IOS_FUNCTION_NOT_FOUND,      /* 41 - NOT USED starting with 7.53.0 */
    IOS_ABORTED_BY_CALLBACK,     /* 42 */
    IOS_BAD_FUNCTION_ARGUMENT,   /* 43 */
    IOS_OBSOLETE44,              /* 44 - NOT USED */
    IOS_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
    IOS_OBSOLETE46,              /* 46 - NOT USED */
    IOS_TOO_MANY_REDIRECTS,      /* 47 - catch endless re-direct loops */
    IOS_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
    IOS_SETOPT_OPTION_SYNTAX,    /* 49 - Malformed setopt option */
    IOS_OBSOLETE50,              /* 50 - NOT USED */
    IOS_OBSOLETE51,              /* 51 - NOT USED */
    IOS_GOT_NOTHING,             /* 52 - when this is a specific error */
    IOS_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
    IOS_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as default */
    IOS_SEND_ERROR,              /* 55 - failed sending network data */
    IOS_RECV_ERROR,              /* 56 - failure in receiving network data */
    IOS_OBSOLETE57,              /* 57 - NOT IN USE */
    IOS_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
    IOS_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
    IOS_PEER_FAILED_VERIFICATION, /* 60 - peer's certificate or fingerprint wasn't verified fine */
    IOS_SSL_CACERT = IOS_PEER_FAILED_VERIFICATION, /* DEPRECATED: 60 - problem with the CA cert (path?) */
    IOS_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
    IOS_OBSOLETE62,              /* 62 - NOT IN USE since 7.82.0 */
    IOS_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
    IOS_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
    IOS_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind that failed */
    IOS_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
    IOS_LOGIN_DENIED,            /* 67 - user, password or similar was not accepted and we failed to login */
    IOS_TFTP_NOTFOUND,           /* 68 - file not found on server */
    IOS_TFTP_PERM,               /* 69 - permission problem on server */
    IOS_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
    IOS_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
    IOS_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
    IOS_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
    IOS_TFTP_NOSUCHUSER,         /* 74 - No such user */
    IOS_OBSOLETE75,              /* 75 - NOT IN USE since 7.82.0 */
    IOS_OBSOLETE76,              /* 76 - NOT IN USE since 7.82.0 */
    IOS_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing or wrong format */
    IOS_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
    IOS_SSH,                     /* 79 - error from the SSH layer, somewhat
                                      generic so the error message will be of
                                      interest when this has happened */

    IOS_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL connection */
    IOS_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                            wait till it's ready and try again (Added in 7.18.2) */
    IOS_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or wrong format (Added in 7.19.0) */
    IOS_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in 7.19.0) */
    IOS_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
    IOS_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
    IOS_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
    IOS_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
    IOS_CHUNK_FAILED,            /* 88 - chunk callback reported error */
    IOS_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the session will be queued */
    IOS_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not match */
    IOS_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
    IOS_HTTP2_STREAM,            /* 92 - stream error in HTTP/2 framing layer */
    IOS_RECURSIVE_API_CALL,      /* 93 - an api function was called from inside a callback */
    IOS_AUTH_ERROR,              /* 94 - an authentication function returned an error */
    IOS_HTTP3,                   /* 95 - An HTTP/3 layer problem */
    IOS_QUIC_CONNECT_ERROR,      /* 96 - QUIC connection error */
    IOS_PROXY,                   /* 97 - proxy handshake error */
    IOS_SSL_CLIENTCERT,          /* 98 - client-side certificate required */
    IOS_UNRECOVERABLE_POLL,      /* 99 - poll/select returned fatal error */
    IOS_LAST /* never use! */
} IOSErrorCode;

NSDictionary* ErrorCodeMap = @ {
    @(NSURLErrorCancelled): @(IOS_WEIRD_SERVER_REPLY),
    @(NSURLErrorBadURL): @(IOS_WEIRD_SERVER_REPLY),
    @(NSURLErrorTimedOut): @(IOS_OPERATION_TIMEDOUT),
    @(NSURLErrorUnsupportedURL): @(IOS_URL_MALFORMAT),
    @(NSURLErrorCannotFindHost): @(IOS_COULDNT_RESOLVE_HOST),
    @(NSURLErrorCannotConnectToHost): @(IOS_COULDNT_CONNECT),
    @(NSURLErrorDataLengthExceedsMaximum): @(IOS_FILESIZE_EXCEEDED),
    @(NSURLErrorNetworkConnectionLost): @(IOS_COULDNT_CONNECT),
    @(NSURLErrorDNSLookupFailed): @(IOS_COULDNT_RESOLVE_HOST),
    @(NSURLErrorHTTPTooManyRedirects): @(IOS_TOO_MANY_REDIRECTS),
    @(NSURLErrorResourceUnavailable): @(IOS_GOT_NOTHING),
    @(NSURLErrorNotConnectedToInternet): @(IOS_COULDNT_CONNECT),
    @(NSURLErrorRedirectToNonExistentLocation): @(IOS_COULDNT_RESOLVE_HOST),
    @(NSURLErrorBadServerResponse): @(IOS_WEIRD_SERVER_REPLY),
    @(NSURLErrorUserCancelledAuthentication): @(IOS_AUTH_ERROR),
    @(NSURLErrorUserAuthenticationRequired): @(IOS_REMOTE_ACCESS_DENIED),
    @(NSURLErrorZeroByteResource): @(IOS_URL_MALFORMAT),
    @(NSURLErrorCannotDecodeRawData): @(IOS_BAD_CONTENT_ENCODING),
    @(NSURLErrorCannotDecodeContentData): @(IOS_BAD_CONTENT_ENCODING),
    @(NSURLErrorCannotParseResponse): @(IOS_BAD_CONTENT_ENCODING),
    @(NSURLErrorInternationalRoamingOff): @(IOS_UNRECOVERABLE_POLL),
    @(NSURLErrorCallIsActive): @(IOS_COULDNT_CONNECT),
    @(NSURLErrorDataNotAllowed): @(IOS_WEIRD_SERVER_REPLY),
    @(NSURLErrorAppTransportSecurityRequiresSecureConnection): @(IOS_AUTH_ERROR),
    @(NSURLErrorRequestBodyStreamExhausted): @(IOS_URL_MALFORMAT),
    @(NSURLErrorFileDoesNotExist): @(IOS_REMOTE_FILE_NOT_FOUND),
    @(NSURLErrorFileIsDirectory): @(IOS_REMOTE_FILE_NOT_FOUND),
    @(NSURLErrorNoPermissionsToReadFile): @(IOS_READ_ERROR),
    @(NSURLErrorSecureConnectionFailed): @(IOS_AUTH_ERROR),
    @(NSURLErrorServerCertificateHasBadDate): @(IOS_AUTH_ERROR),
    @(NSURLErrorServerCertificateUntrusted): @(IOS_PEER_FAILED_VERIFICATION),
    @(NSURLErrorServerCertificateHasUnknownRoot): @(IOS_PEER_FAILED_VERIFICATION),
    @(NSURLErrorServerCertificateNotYetValid): @(IOS_PEER_FAILED_VERIFICATION),
    @(NSURLErrorClientCertificateRejected): @(IOS_PEER_FAILED_VERIFICATION),
    @(NSURLErrorClientCertificateRequired): @(IOS_AUTH_ERROR),
    @(NSURLErrorCannotLoadFromNetwork): @(IOS_WEIRD_SERVER_REPLY),
    @(NSURLErrorCannotCreateFile): @(IOS_WRITE_ERROR),
    @(NSURLErrorCannotOpenFile): @(IOS_READ_ERROR),
    @(NSURLErrorCannotCloseFile): @(IOS_WRITE_ERROR),
    @(NSURLErrorCannotWriteToFile): @(IOS_WRITE_ERROR),
    @(NSURLErrorCannotMoveFile): @(IOS_WRITE_ERROR)
};
#endif /* http_ios_constant_h */
