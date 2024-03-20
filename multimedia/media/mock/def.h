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

#ifndef HISYSEVENT_DEF_H
#define HISYSEVENT_DEF_H

namespace OHOS {
namespace HiviewDFX {
static constexpr int SUCCESS = 0;

static constexpr int ERR_DOMAIN_NAME_INVALID = -1;
static constexpr int ERR_EVENT_NAME_INVALID = -2;
static constexpr int ERR_DOES_NOT_INIT = -3;
static constexpr int ERR_OVER_SIZE = -4;
static constexpr int ERR_SEND_FAIL = -5;
static constexpr int ERR_WRITE_IN_HIGH_FREQ = -6;
static constexpr int ERR_DOMAIN_MASKED = -7;
static constexpr int ERR_EMPTY_EVENT = -8;
static constexpr int ERR_RAW_DATA_WROTE_EXCEPTION = -9;
static constexpr char ERR_MSG_LEVEL0[][32] = {
    "invalid domain name",
    "invalid event name",
    "socket init error",
    "over size",
    "send data failed",
    "write too frequently",
    "domain has been masked",
    "empty event",
    "raw data wrote failed"
};

static constexpr int ERR_KEY_NAME_INVALID = 1;
static constexpr int ERR_VALUE_LENGTH_TOO_LONG = 2;
static constexpr int ERR_KEY_NUMBER_TOO_MUCH = 3;
static constexpr int ERR_ARRAY_TOO_MUCH = 4;
static constexpr int ERR_VALUE_INVALID = 5;
static constexpr char ERR_MSG_LEVEL1[][32] = {
    "invalid key name",
    "string value too long",
    "too many keys (>128)",
    "too many array items (>100)",
    "invalid value"
};

static constexpr unsigned int MAX_DOMAIN_LENGTH = 16;
static constexpr unsigned int MAX_EVENT_NAME_LENGTH = 32;
static constexpr unsigned int MAX_PARAM_NAME_LENGTH = 48;
static constexpr unsigned int MAX_ARRAY_SIZE = 100;
static constexpr unsigned int MAX_PARAM_NUMBER = 128;
static constexpr unsigned int MAX_STRING_LENGTH = 256 * 1024;
static constexpr unsigned int MAX_DATA_SIZE = 384 * 1024;
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_DEF_H
