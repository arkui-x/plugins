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
#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_JSON_CONVERT_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_JSON_CONVERT_H

#include <string>
#include "log.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace Notification {
class NotificationJsonConvertionBase {
public:
    virtual ~NotificationJsonConvertionBase() = default;

    /**
     * @brief Converts NotificationJsonConvertionBase object to json object.
     *
     * @param jsonObject Indicates the json object.
     */
    virtual bool ToJson(nlohmann::json &jsonObject) const = 0;
};

class NotificationJsonConverter {
public:
    /**
     * @brief Converts NotificationJsonConvertionBase object to json object.
     *
     * @param convertionBase Indicates the NotificationJsonConvertionBase object.
     * @param jsonObject Indicates the json object.
     * @return Returns true if the conversion is successful; returns false otherwise.
     */
    static bool ConvertToJson(const NotificationJsonConvertionBase *convertionBase, nlohmann::json &jsonObject)
    {
        if (convertionBase == nullptr) {
            LOGE("Converter : Invalid base object");
            return false;
        }

        return convertionBase->ToJson(jsonObject);
    }

    /**
     * @brief Converts NotificationJsonConvertionBase object to json string.
     *
     * @param convertionBase Indicates the NotificationJsonConvertionBase object.
     * @param jsonString Indicates the json string.
     * @return Returns true if the conversion is successful; returns false otherwise.
     */
    static bool ConvertToJsonString(const NotificationJsonConvertionBase *convertionBase, std::string &jsonString)
    {
        if (convertionBase == nullptr) {
            LOGE("Converter : Invalid base object");
            return false;
        }

        nlohmann::json jsonObject;
        if (!convertionBase->ToJson(jsonObject)) {
            LOGE("Converter : Cannot convert to JSON object");
            return false;
        }
        jsonString = jsonObject.dump();

        return true;
    }

    /**
     * @brief Converts json object to a subclass object whose base class is NotificationJsonConvertionBase.
     *
     * @param jsonObject Indicates the json object.
     * @return Returns the subclass object.
     */
    template <typename T>
    static T *ConvertFromJson(const nlohmann::json &jsonObject)
    {
        if (jsonObject.is_null() or !jsonObject.is_object()) {
            LOGE("Converter : Invalid JSON object");
            return nullptr;
        }

        return T::FromJson(jsonObject);
    }

    /**
     * @brief Converts json string to a subclass object whose base class is NotificationJsonConvertionBase.
     *
     * @param jsonString Indicates the json string.
     * @return Returns the subclass object.
     */
    template <typename T>
    static T *ConvertFromJsonString(const std::string &jsonString)
    {
        if (jsonString.empty()) {
            LOGE("Converter : Invalid JSON string");
            return nullptr;
        }

        auto jsonObject = nlohmann::json::parse(jsonString);
        if (jsonObject.is_null() or !jsonObject.is_object()) {
            LOGE("Converter : Invalid JSON object");
            return nullptr;
        }

        return T::FromJson(jsonObject);
    }
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_JSON_CONVERT_H
