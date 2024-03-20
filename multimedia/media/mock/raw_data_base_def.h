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

#ifndef HISYSEVENT_INTERFACE_ENCODE_INCLUDE_RAW_BASE_DEF_H
#define HISYSEVENT_INTERFACE_ENCODE_INCLUDE_RAW_BASE_DEF_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string>

#include "def.h"

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
constexpr char BASE_INFO_KEY_DOMAIN[] = "domain_";
constexpr char BASE_INFO_KEY_NAME[] = "name_";
constexpr char BASE_INFO_KEY_TYPE[] = "type_";
constexpr char BASE_INFO_KEY_TIME_STAMP[] = "time_";
constexpr char BASE_INFO_KEY_TIME_ZONE[] = "tz_";
constexpr char BASE_INFO_KEY_ID[] = "id_";
constexpr char BASE_INFO_KEY_PID[] = "pid_";
constexpr char BASE_INFO_KEY_TID[] = "tid_";
constexpr char BASE_INFO_KEY_UID[] = "uid_";
constexpr char BASE_INFO_KEY_TRACE_ID[] = "traceid_";
constexpr char BASE_INFO_KEY_SPAN_ID[] = "spanid_";
constexpr char BASE_INFO_KEY_PARENT_SPAN_ID[] = "pspanid_";
constexpr char BASE_INFO_KEY_TRACE_FLAG[] = "trace_flag_";

#pragma pack(1)
struct HiSysEventHeader {
    /* Event domain */
    char domain[MAX_DOMAIN_LENGTH + 1];

    /* Event name */
    char name[MAX_EVENT_NAME_LENGTH + 1];

    /* Event timestamp */
    uint64_t timestamp;

    /* Time zone */
    uint8_t timeZone;

    /* User id */
    uint32_t uid;

    /* Process id */
    uint32_t pid;

    /* Thread id */
    uint32_t tid;

    /* Event hash code*/
    uint64_t id;

    /* Event type */
    uint8_t type : 2;

    /* Trace info flag */
    uint8_t isTraceOpened : 1;
};

struct TraceInfo {
    /* Hitrace flag */
    uint8_t traceFlag;

    /* Hitrace id */
    uint64_t traceId;

    /* Hitrace span id */
    uint32_t spanId;

    /* Hitrace parent span id */
    uint32_t pSpanId;
};

struct ParamValueType {
    /* Array flag */
    uint8_t isArray : 1;

    /* Type of parameter value */
    uint8_t valueType : 4;

    /* Byte count of parameter value */
    uint8_t valueByteCnt : 3;
};
#pragma pack()

enum ValueType: uint8_t {
    // Unknown value
    UNKNOWN = 0,

    // Bool value
    BOOL,

    // Int8_t value
    INT8,

    // Uint8_t value
    UINT8,

    // Int16_t value
    INT16,

    // Uint16_t value
    UINT16,

    // Int32_t value
    INT32,

    // Uint32_t value
    UINT32,

    // Int64_t value
    INT64,

    // Uint64_t value
    UINT64,

    // Float value
    FLOAT,

    // Double value
    DOUBLE,

    // String value
    STRING,
};

enum EncodeType: int8_t {
    // Varint encoding
    VARINT = 0,

    // Length delimited encoding
    LENGTH_DELIMITED = 1,

    // Reserved
    INVALID = 4,
};

int ParseTimeZone(long tzVal);
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS
#endif // HISYSEVENT_INTERFACE_ENCODE_RAW_INCLUDE_RAW_BASE_DEF_H
