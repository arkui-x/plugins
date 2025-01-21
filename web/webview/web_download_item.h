/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGIN_WEB_DOWNLOAD_ITEM_H
#define PLUGIN_WEB_DOWNLOAD_ITEM_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace download {
enum DownloadInterruptReason {
    DOWNLOAD_INTERRUPT_REASON_NONE = 0,

#define INTERRUPT_REASON(name, value) DOWNLOAD_INTERRUPT_REASON_##name = value,

    // Generic file operation failure.
    // "File Error".
    INTERRUPT_REASON(FILE_FAILED, 1)

    // The file cannot be accessed due to security restrictions.
    // "Access Denied".
    INTERRUPT_REASON(FILE_ACCESS_DENIED, 2)

    // There is not enough room on the drive.
    // "Disk Full".
    INTERRUPT_REASON(FILE_NO_SPACE, 3)

    // The directory or file name is too long.
    // "Path Too Long".
    INTERRUPT_REASON(FILE_NAME_TOO_LONG, 5)

    // The file is too large for the file system to handle.
    // "File Too Large".
    INTERRUPT_REASON(FILE_TOO_LARGE, 6)

    // The file contains a virus.
    // "Virus".
    INTERRUPT_REASON(FILE_VIRUS_INFECTED, 7)

    // The file was in use.
    // Too many files are opened at once.
    // We have run out of memory.
    // "Temporary Problem".
    INTERRUPT_REASON(FILE_TRANSIENT_ERROR, 10)

    // The file was blocked due to local policy.
    // "Blocked"
    INTERRUPT_REASON(FILE_BLOCKED, 11)

    // An attempt to check the safety of the download failed due to unexpected
    // reasons. See http://crbug.com/153212.
    INTERRUPT_REASON(FILE_SECURITY_CHECK_FAILED, 12)

    // An attempt was made to seek past the end of a file in opening
    // a file (as part of resuming a previously interrupted download).
    INTERRUPT_REASON(FILE_TOO_SHORT, 13)

    // The partial file didn't match the expected hash.
    INTERRUPT_REASON(FILE_HASH_MISMATCH, 14)

    // The source and the target of the download were the same.
    INTERRUPT_REASON(FILE_SAME_AS_SOURCE, 15)

    // Network errors.

    // Generic network failure.
    // "Network Error".
    INTERRUPT_REASON(NETWORK_FAILED, 20)

    // The network operation timed out.
    // "Operation Timed Out".
    INTERRUPT_REASON(NETWORK_TIMEOUT, 21)

    // The network connection has been lost.
    // "Connection Lost".
    INTERRUPT_REASON(NETWORK_DISCONNECTED, 22)

    // The server has gone down.
    // "Server Down".
    INTERRUPT_REASON(NETWORK_SERVER_DOWN, 23)

    // The network request was invalid. This may be due to the original URL or a
    // redirected URL:
    // - Having an unsupported scheme.
    // - Being an invalid URL.
    // - Being disallowed by policy.
    INTERRUPT_REASON(NETWORK_INVALID_REQUEST, 24)

    // Server responses.

    // The server indicates that the operation has failed (generic).
    // "Server Error".
    INTERRUPT_REASON(SERVER_FAILED, 30)

    // The server does not support range requests.
    // Internal use only:  must restart from the beginning.
    INTERRUPT_REASON(SERVER_NO_RANGE, 31)

    // Precondition failed. This type of interruption could legitimately occur if a
    // partial download resumption was attempted using a If-Match header. However,
    // the downloads logic no longer uses If-Match headers and instead uses If-Range
    // headers where a precondition failure is not expected.
    //
    // Obsolete: INTERRUPT_REASON(SERVER_PRECONDITION, 32)

    // The server does not have the requested data.
    // "Unable to get file".
    INTERRUPT_REASON(SERVER_BAD_CONTENT, 33)

    // Server didn't authorize access to resource.
    INTERRUPT_REASON(SERVER_UNAUTHORIZED, 34)

    // Server certificate problem.
    INTERRUPT_REASON(SERVER_CERT_PROBLEM, 35)

    // Server access forbidden.
    INTERRUPT_REASON(SERVER_FORBIDDEN, 36)

    // Unexpected server response. This might indicate that the responding server
    // may not be the intended server.
    INTERRUPT_REASON(SERVER_UNREACHABLE, 37)

    // The server sent fewer bytes than the content-length header. It may indicate
    // that the connection was closed prematurely, or the Content-Length header was
    // invalid. The download is only interrupted if strong validators are present.
    // Otherwise, it is treated as finished.
    INTERRUPT_REASON(SERVER_CONTENT_LENGTH_MISMATCH, 38)

    // An unexpected cross-origin redirect happened.
    INTERRUPT_REASON(SERVER_CROSS_ORIGIN_REDIRECT, 39)

    // User input.

    // The user canceled the download.
    // "Canceled".
    INTERRUPT_REASON(USER_CANCELED, 40)

    // The user shut down the browser.
    // Internal use only:  resume pending downloads if possible.
    INTERRUPT_REASON(USER_SHUTDOWN, 41)

    // Crash.

    // The browser crashed.
    // Internal use only:  resume pending downloads if possible.
    INTERRUPT_REASON(CRASH, 50)

#undef INTERRUPT_REASON
};
} // namespace download

namespace OHOS {
namespace Plugin {
enum PluginWebDownloadItemState {
    IN_PROGRESS = 0,
    COMPLETE,
    CANCELED,
    INTERRUPTED,
    PENDING,
    PAUSED,
    MAX_DOWNLOAD_STATE,
};

class WebDownloadItem {
public:
    explicit WebDownloadItem(napi_env env);
    ~WebDownloadItem();

    long webDownloadId;
    int32_t currentSpeed;
    int32_t percentComplete;
    int64_t totalBytes;
    int64_t receivedBytes;
    int32_t lastErrorCode;

    std::string guid;
    std::string fullPath;
    std::string url;
    std::string etag;
    std::string originalUrl;
    std::string suggestedFileName;
    std::string contentDisposition;
    std::string mimeType;
    std::string lastModified;

    PluginWebDownloadItemState state = PluginWebDownloadItemState::MAX_DOWNLOAD_STATE;
    std::string method;
    std::string receivedSlices;

    std::string downloadPath;
    std::function<void(std::string)> beforeCallback;
    std::function<void()> downloadCancelCallback;
    std::function<int()> downloadPauseCallback;
    std::function<int()> downloadResumeCallback;

    int32_t nwebId;

private:
    napi_env env_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGIN_WEB_DOWNLOAD_ITEM_H
