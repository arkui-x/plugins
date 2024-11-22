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

#ifndef DATASHARE_ERRNO_H
#define DATASHARE_ERRNO_H

namespace OHOS {
namespace DataShare {

/**
* @brief The error code in the correct case.
*/
constexpr int E_OK = 0;

/**
* @brief The base code of the exception error code.
*/
constexpr int E_BASE = 1000;

/**
* @brief The error code for common exceptions.
*/
constexpr int E_ERROR = (E_BASE + 1);

/**
* @brief The error code for register exceptions.
*/
constexpr int E_REGISTERED_REPEATED = (E_BASE + 2);


/**
* @brief The error code for register exceptions.
*/
constexpr int E_UNREGISTERED_EMPTY = (E_BASE + 3);

/**
* @brief The error code for invalid statement.
*/
constexpr int E_INVALID_STATEMENT = (E_BASE + 7);

/**
* @brief The error code for invalid column index.
*/
constexpr int E_INVALID_COLUMN_INDEX = (E_BASE + 8);

/**
* @brief The error code for invalid object type.
*/
constexpr int E_INVALID_OBJECT_TYPE = (E_BASE + 20);

/**
* @brief The error code for invalid parcel.
*/
constexpr int E_INVALID_PARCEL = (E_BASE + 42);

/**
* @brief The version is smaller than exist.
*/
constexpr int E_VERSION_NOT_NEWER = (E_BASE + 45);

/**
* @brief Cannot find the template
*/
constexpr int E_TEMPLATE_NOT_EXIST = (E_BASE + 46);

/**
* @brief Cannot find the subscriber
*/
constexpr int E_SUBSCRIBER_NOT_EXIST = (E_BASE + 47);

/**
* @brief Cannot find the uri
*/
constexpr int E_URI_NOT_EXIST = (E_BASE + 48);

/**
* @brief Cannot find the bundleName
*/
constexpr int E_BUNDLE_NAME_NOT_EXIST = (E_BASE + 49);

/**
* @brief BMS not ready
*/
constexpr int E_BMS_NOT_READY = (E_BASE + 50);

/**
* @brief metaData not exists
*/
constexpr int E_METADATA_NOT_EXISTS = (E_BASE + 51);
/**
* @brief silent proxy is disable
*/
constexpr int E_SILENT_PROXY_DISABLE = (E_BASE + 52);
} // namespace DataShare
} // namespace OHOS

#endif