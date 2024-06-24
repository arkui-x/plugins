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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H
#define BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H

#include <map>
#include <sstream>
#include "errors.h"

namespace OHOS {
namespace Notification {
/**
 * ErrCode layout
 *
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Bit |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |Field|Reserved|        Subsystem      |  Module      |                              Code             |
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */

constexpr uint32_t EVENT_SUB_MODULE_OFFSET = 8;  // sub-module offset value

// ANS's module const defined.
enum AnsModule : uint32_t {
    ANS_MODULE_COMMON = 0x00,
};

// Offset of common event sub-system's errcode base.
constexpr ErrCode ANS_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_NOTIFICATION, ANS_MODULE_COMMON);

// Error code defined.
enum ErrorCode : uint32_t {
    ERR_ANS_OK = 0,
    ERR_ANS_SERVICE_NOT_READY = ANS_COMMON_ERR_OFFSET + 1,
    ERR_ANS_SERVICE_NOT_CONNECTED,
    ERR_ANS_INVALID_PARAM,
    ERR_ANS_INVALID_UID,
    ERR_ANS_NOT_SYSTEM_SERVICE,
    ERR_ANS_INVALID_PID,
    ERR_ANS_INVALID_BUNDLE,
    ERR_ANS_NOT_ALLOWED,
    ERR_ANS_PARCELABLE_FAILED,
    ERR_ANS_TRANSACT_FAILED,
    ERR_ANS_REMOTE_DEAD,
    ERR_ANS_NO_MEMORY,
    ERR_ANS_TASK_ERR,
    ERR_ANS_NON_SYSTEM_APP,
    ERR_ANS_PERMISSION_DENIED,
    ERR_ANS_NOTIFICATION_NOT_EXISTS,
    ERR_ANS_NOTIFICATION_IS_UNREMOVABLE,
    ERR_ANS_OVER_MAX_ACTIVE_PERSECOND,
    ERR_ANS_ICON_OVER_SIZE,
    ERR_ANS_PICTURE_OVER_SIZE,
    ERR_ANS_PREFERENCES_NOTIFICATION_DB_OPERATION_FAILED,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_EXCEED_MAX_NUM,
    ERR_ANS_PREFERENCES_NOTIFICATION_READ_TEMPLATE_CONFIG_FAILED,
    ERR_ANS_DISTRIBUTED_OPERATION_FAILED,
    ERR_ANS_DISTRIBUTED_GET_INFO_FAILED,
    ERR_ANS_NOTIFICATION_IS_UNALLOWED_REMOVEALLOWED,
    ERR_ANS_GET_ACTIVE_USER_FAILED,
    ERR_ANS_SUBSCRIBER_IS_DELETING,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_ENABLED,
    ERR_ANS_DLP_HAP,
    ERR_ANS_PUSH_CHECK_FAILED,
    ERR_ANS_DIALOG_POP_SUCCEEDED,
    ERR_ANS_DIALOG_IS_POPPING,
    ERR_ANS_PUSH_CHECK_UNREGISTERED,
    ERR_ANS_REPEAT_CREATE,
    ERR_ANS_END_NOTIFICATION,
    ERR_ANS_EXPIRED_NOTIFICATION,
    ERR_ANS_PUSH_CHECK_NETWORK_UNREACHABLE,
    ERR_ANS_PUSH_CHECK_EXTRAINFO_INVALID,
    ERR_ANS_OVER_MAX_UPDATE_PERSECOND,
    ERR_ANS_DUPLICATE_MSG,
    ERR_ANS_NO_AGENT_SETTING
};

enum ReminderErrorCode : uint32_t {
    ERR_REMINDER_PERMISSION_DENIED = 201,
    ERR_REMINDER_INVALID_PARAM = 401,
    ERR_REMINDER_NOTIFICATION_NOT_ENABLE = 1700001,
    ERR_REMINDER_NUMBER_OVERLOAD,
    ERR_REMINDER_NOT_EXIST,
    ERR_REMINDER_PACKAGE_NOT_EXIST,
};

static std::map<uint32_t, std::string> reminderErrCodeMsgMap = {
    { ERR_REMINDER_PERMISSION_DENIED, "BussinessError 201: Permission denied." },
    { ERR_REMINDER_INVALID_PARAM, "BussinessError 401: Parameter error." },
    { ERR_REMINDER_NOTIFICATION_NOT_ENABLE, "BussinessError 1700001: Notification not enable." },
    { ERR_REMINDER_NUMBER_OVERLOAD, "BussinessError 1700002: The number of reminders exceeds the limit." },
    { ERR_REMINDER_NOT_EXIST, "BussinessError 1700003: The reminder not exist." },
    { ERR_REMINDER_PACKAGE_NOT_EXIST, "BussinessError 1700004: The package name not exist." }
};

// Notification error msg
static inline std::string MANDATORY_PARAMETER_ARE_LEFT_UNSPECIFIED = "Mandatory parameters are left unspecified.";
static inline std::string INCORRECT_PARAMETER_TYPES                = "Incorrect parameter types.";
static inline std::string PARAMETER_VERIFICATION_FAILED            = "Parameter verification failed.";

// Common error code
const uint32_t ERROR_PERMISSION_DENIED = 201;          // No permission to call the interface.
const uint32_t ERROR_NOT_SYSTEM_APP    = 202;          // Not system application to call the interface.
const uint32_t ERROR_PARAM_INVALID     = 401;          // Invalid input parameter.
const uint32_t ERROR_SYSTEM_CAP_ERROR  = 801;          // The specified SystemCapability names was not found.

// Notification error code
const int32_t ERROR_INTERNAL_ERROR               = 1600001;    // Internal error.
const int32_t ERROR_IPC_ERROR                    = 1600002;    // marshalling or unmarshalling error.
const int32_t ERROR_SERVICE_CONNECT_ERROR        = 1600003;    // Failed to connect service.
const int32_t ERROR_NOTIFICATION_CLOSED          = 1600004;    // Notification is not enabled.
const int32_t ERROR_SLOT_CLOSED                  = 1600005;    // Notification slot is not enabled.
const int32_t ERROR_NOTIFICATION_UNREMOVABLE     = 1600006;    // Notification is not allowed to remove.
const int32_t ERROR_NOTIFICATION_NOT_EXIST       = 1600007;    // The notification is not exist.
const int32_t ERROR_USER_NOT_EXIST               = 1600008;    // The user is not exist.
const int32_t ERROR_OVER_MAX_NUM_PER_SECOND      = 1600009;    // Over max number notifications per second.
const int32_t ERROR_DISTRIBUTED_OPERATION_FAILED = 1600010;    // Distributed operation failed.
const int32_t ERROR_READ_TEMPLATE_CONFIG_FAILED  = 1600011;    // Read template config failed.
const int32_t ERROR_NO_MEMORY                    = 1600012;    // No memory space.
const int32_t ERROR_DIALOG_IS_POPPING            = 1600013;    // Enable notification dialog is popping.
const int32_t ERROR_NO_RIGHT                     = 1600014;    // No right.
const int32_t ERROR_REPEAT_SET                   = 1600015;    // Repeat create or end.
const int32_t ERROR_EXPIRED_NOTIFICATION         = 1600016;    // Low update version.
const int32_t ERROR_NO_AGENT_SETTING             = 1600017;    // No corresponding agent relationship configuration.
const int32_t ERROR_NETWORK_UNREACHABLE          = 2300007;    // Network unreachable.
const int32_t ERROR_BUNDLE_NOT_FOUND             = 17700001;   // The specified bundle name was not found.
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H
