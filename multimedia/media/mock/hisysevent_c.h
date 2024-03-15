/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H
#define HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LENGTH_OF_PARAM_NAME 49

/**
 * @brief Define the type of the param.
 */
enum HiSysEventParamType {
    HISYSEVENT_INVALID = 0,
    HISYSEVENT_BOOL = 1,
    HISYSEVENT_INT8 = 2,
    HISYSEVENT_UINT8 = 3,
    HISYSEVENT_INT16 = 4,
    HISYSEVENT_UINT16 = 5,
    HISYSEVENT_INT32 = 6,
    HISYSEVENT_UINT32 = 7,
    HISYSEVENT_INT64 = 8,
    HISYSEVENT_UINT64 = 9,
    HISYSEVENT_FLOAT = 10,
    HISYSEVENT_DOUBLE = 11,
    HISYSEVENT_STRING = 12,
    HISYSEVENT_BOOL_ARRAY = 13,
    HISYSEVENT_INT8_ARRAY = 14,
    HISYSEVENT_UINT8_ARRAY = 15,
    HISYSEVENT_INT16_ARRAY = 16,
    HISYSEVENT_UINT16_ARRAY = 17,
    HISYSEVENT_INT32_ARRAY = 18,
    HISYSEVENT_UINT32_ARRAY = 19,
    HISYSEVENT_INT64_ARRAY = 20,
    HISYSEVENT_UINT64_ARRAY = 21,
    HISYSEVENT_FLOAT_ARRAY = 22,
    HISYSEVENT_DOUBLE_ARRAY = 23,
    HISYSEVENT_STRING_ARRAY = 24
};
typedef enum HiSysEventParamType HiSysEventParamType;

/**
 * @brief Define the value of the param.
 */
union HiSysEventParamValue {
    bool b;
    int8_t i8;
    uint8_t ui8;
    int16_t i16;
    uint16_t ui16;
    int32_t i32;
    uint32_t ui32;
    int64_t i64;
    uint64_t ui64;
    float f;
    double d;
    char *s;
    void *array;
};
typedef union HiSysEventParamValue HiSysEventParamValue;

/**
 * @brief Define param struct.
 */
struct HiSysEventParam {
    char name[MAX_LENGTH_OF_PARAM_NAME];
    HiSysEventParamType t;
    HiSysEventParamValue v;
    size_t arraySize;
};
typedef struct HiSysEventParam HiSysEventParam;

/**
 * @brief Event type.
 */
enum HiSysEventEventType {
    HISYSEVENT_FAULT = 1,
    HISYSEVENT_STATISTIC = 2,
    HISYSEVENT_SECURITY = 3,
    HISYSEVENT_BEHAVIOR = 4
};
typedef enum HiSysEventEventType HiSysEventEventType;

/**
 * @brief Write system event.
 * @param domain event domain.
 * @param name   event name.
 * @param type   event type.
 * @param params event params.
 * @param size   the size of param list.
 * @return 0 means success, less than 0 means failure, greater than 0 means invalid params.
 */
#define OH_HiSysEvent_Write(domain, name, type, params, size) \
    HiSysEvent_Write(__FUNCTION__, __LINE__, domain, name, type, params, size)

int HiSysEvent_Write(const char* func, int64_t line, const char* domain, const char* name,
    HiSysEventEventType type, const HiSysEventParam params[], size_t size);

#ifdef __cplusplus
}
#endif
#endif // HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H