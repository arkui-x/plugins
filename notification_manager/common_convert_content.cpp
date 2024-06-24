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

#include "common.h"
#include "ans_inner_errors.h"
#include "log.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi_common_util.h"

namespace OHOS {
namespace NotificationNapi {

napi_value Common::GetNotificationContent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LOGD("enter");

    napi_value result = AppExecFwk::GetPropertyValueByPropertyName(env, value, "content", napi_object);
    if (result == nullptr) {
        LOGE("No content.");
        return nullptr;
    }

    int32_t type = 0;
    if (GetNotificationContentType(env, result, type) == nullptr) {
        return nullptr;
    }
    NotificationContent::Type outType = NotificationContent::Type::NONE;
    if (!AnsEnumUtil::ContentTypeJSToC(ContentType(type), outType)) {
        return nullptr;
    }
    switch (outType) {
        case NotificationContent::Type::BASIC_TEXT:
            if (GetNotificationBasicContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::LONG_TEXT:
            if (GetNotificationLongTextContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::MULTILINE:
            if (GetNotificationMultiLineContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        default:
            return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBasicContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, result, "normal", &hasProperty));
    if (!hasProperty) {
        LOGE("Property normal expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "normal", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    if (normalContent == nullptr) {
        LOGE("normalContent is null");
        return nullptr;
    }

    if (GetNotificationBasicContentDetailed(env, contentResult, normalContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(normalContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBasicContentDetailed(
    const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent)
{
    LOGD("enter");

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // title: string
    auto value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "title", napi_string);
    if (value == nullptr) {
        LOGE("Failed to get title from js.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property title is empty");
        return nullptr;
    }
    basicContent->SetTitle(str);
    LOGD("normal::title = %{public}s", str);

    // text: string
    value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "text", napi_string);
    if (value == nullptr) {
        LOGE("Failed to get text from js.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property text is empty");
        return nullptr;
    }
    basicContent->SetText(str);
    LOGD("normal::text = %{public}s", str);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationLongTextContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "longText", &hasProperty));
    if (!hasProperty) {
        LOGE("Property longText expected.");
        return nullptr;
    }

    napi_get_named_property(env, result, "longText", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationLongTextContent> longContent =
        std::make_shared<OHOS::Notification::NotificationLongTextContent>();
    if (longContent == nullptr) {
        LOGE("longContent is null");
        return nullptr;
    }

    if (GetNotificationLongTextContentDetailed(env, contentResult, longContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(longContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationLongTextContentDetailed(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLongTextContent> &longContent)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value longContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    char long_str[LONG_STR_MAX_SIZE + 1] = {0};
    size_t strLen = 0;

    if (GetNotificationBasicContentDetailed(env, contentResult, longContent) == nullptr) {
        return nullptr;
    }

    // longText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longText", &hasProperty));
    if (!hasProperty) {
        LOGE("Property longText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "longText", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, long_str, LONG_STR_MAX_SIZE, &strLen));
    if (std::strlen(long_str) == 0) {
        LOGE("Property longText is empty");
        return nullptr;
    }
    longContent->SetLongText(long_str);
    LOGD("longText::longText = %{public}s", long_str);

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (!hasProperty) {
        LOGE("Property briefText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "briefText", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property briefText is empty");
        return nullptr;
    }
    longContent->SetBriefText(str);
    LOGD("longText::briefText = %{public}s", str);

    // expandedTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "expandedTitle", &hasProperty));
    if (!hasProperty) {
        LOGE("Property expandedTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "expandedTitle", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property expandedTitle is empty");
        return nullptr;
    }
    longContent->SetExpandedTitle(str);
    LOGD("longText::expandedTitle = %{public}s", str);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationMultiLineContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value multiLineContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "multiLine", &hasProperty));
    if (!hasProperty) {
        LOGE("Property multiLine expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "multiLine", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> multiLineContent =
        std::make_shared<OHOS::Notification::NotificationMultiLineContent>();
    if (multiLineContent == nullptr) {
        LOGE("multiLineContent is null");
        return nullptr;
    }

    if (GetNotificationBasicContentDetailed(env, contentResult, multiLineContent) == nullptr) {
        return nullptr;
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (!hasProperty) {
        LOGE("Property briefText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "briefText", &multiLineContentResult);
    NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
    if (valuetype != napi_string) {
        LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property briefText is empty");
        return nullptr;
    }
    multiLineContent->SetBriefText(str);
    LOGD("multiLine: briefText = %{public}s", str);

    // longTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longTitle", &hasProperty));
    if (!hasProperty) {
        LOGE("Property longTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "longTitle", &multiLineContentResult);
    NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
    if (valuetype != napi_string) {
        LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        LOGE("Property longTitle is empty");
        return nullptr;
    }
    multiLineContent->SetExpandedTitle(str);
    LOGD("multiLine: longTitle = %{public}s", str);

    // lines: Array<String>
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "lines", &hasProperty));
    if (!hasProperty) {
        LOGE("Property lines expected.");
        return nullptr;
    }
    if (GetNotificationMultiLineContentLines(env, contentResult, multiLineContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(multiLineContent));

    LOGD("end");
    return NapiGetNull(env);
}

napi_value Common::GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent)
{
    LOGD("enter");

    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value multilines = nullptr;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    uint32_t length = 0;

    napi_get_named_property(env, result, "lines", &multilines);
    napi_is_array(env, multilines, &isArray);
    if (!isArray) {
        LOGE("Property lines is expected to be an array.");
        return nullptr;
    }

    napi_get_array_length(env, multilines, &length);
    if (length == 0) {
        LOGE("The array is empty.");
        return nullptr;
    }
    for (size_t i = 0; i < length; i++) {
        napi_value line = nullptr;
        napi_get_element(env, multilines, i, &line);
        NAPI_CALL(env, napi_typeof(env, line, &valuetype));
        if (valuetype != napi_string) {
            LOGE("Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, line, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->AddSingleLine(str);
        LOGD("multiLine: lines : addSingleLine = %{public}s", str);
    }

    return NapiGetNull(env);
}
}
}
