/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_ERROR_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_ERROR_H

#include <cstdint>
#include <string>

namespace OHOS::Plugin::CacheDownload {

/**
 * Enumerates the specific types of returned error code.
 * Values match the ArkTS ErrorCode enum.
 */
enum class ErrorCode : uint8_t {
    DNS    = 0x00,
    TCP    = 0x10,
    SSL    = 0x20,
    HTTP   = 0x30,
    OTHERS = 0xFF,
};

/**
 * Describes the error message returned when a pre-download error occurs.
 * Matches the ArkTS DownloadError interface.
 */
struct CacheDownloadError {
    /** Specific error type returned by the pre-download error callback. */
    ErrorCode errorCode = ErrorCode::OTHERS;
    /** Raw error code from the underlying HTTP layer. */
    int32_t code = 0;
    /** Human-readable error message. */
    std::string message;

    CacheDownloadError() = default;
    CacheDownloadError(ErrorCode ec, int32_t c, std::string msg)
        : errorCode(ec), code(c), message(std::move(msg)) {}
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_ERROR_H
