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
#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H

#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "notification_helper.h"
#include "ans_convert_enum.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

constexpr int32_t STR_MAX_SIZE = 200;
constexpr int32_t LONG_STR_MAX_SIZE = 1024;
constexpr int8_t ERROR = -1;
constexpr uint8_t PARAM0 = 0;
constexpr uint8_t PARAM1 = 1;
constexpr uint8_t PARAM2 = 2;
constexpr uint8_t PARAM3 = 3;
constexpr uint8_t PARAM4 = 4;

struct NotificationSubscribeInfo {
    std::vector<std::string> bundleNames;
    int32_t userId = 0;
    bool hasSubscribeInfo = false;
};

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int32_t errorCode = 0;
};

class Common {
    Common();

    ~Common();

public:

    /**
     * @brief Gets the napi value that is used to represent the null object
     *
     * @param env Indicates the environment that the API is invoked under
     * @return Returns the napi value that is used to represent the null object
     */
    static napi_value NapiGetNull(napi_env env);

    /**
     * @brief Gets the napi value that is used to represent the undefined object
     *
     * @param env Indicates the environment that the API is invoked under
     * @return Returns the napi value that is used to represent the undefined object
     */
    static napi_value NapiGetUndefined(napi_env env);

    /**
     * @brief Pads the CallbackPromiseInfo struct
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callback Indicates a napi_ref for callback
     * @param info Indicates the CallbackPromiseInfo struct to be padded
     * @param promise Indicates the promise to be created when the callback is null
     */
    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);

    /**
     * @brief Calls the callback with the result and error code
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callbackIn Indicates the callback to be called
     * @param errCode Indicates the error code returned by the callback
     * @param result Indicates the result returned by the callback
     */
    static void SetCallback(const napi_env &env,
        const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result, bool newType);

    /**
     * @brief Calls the callback with the result
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callbackIn Indicates the callback to be called
     * @param result Indicates the result returned by the callback
     */
    static void SetCallback(
        const napi_env &env, const napi_ref &callbackIn, const napi_value &result);

    /**
     * @brief Processes the promise with the result and error code
     *
     * @param env Indicates the environment that the API is invoked under
     * @param deferred Indicates the deferred object whose associated promise to resolve
     * @param errorCode Indicates the error code returned by the callback
     * @param result Indicates the result returned by the callback
     */
    static void SetPromise(const napi_env &env,
        const napi_deferred &deferred, const int32_t &errorCode, const napi_value &result, bool newType);

    /**
     * @brief Gets the returned result by the callback when an error occurs
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callback Indicates a napi_ref for callback
     * @return Returns the null object
     */
    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);

    /**
     * @brief Parses a single parameter for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param info Indicates the callback info passed into the callback function
     * @param callback Indicates the napi_ref for the callback parameter
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value ParseParaOnlyCallback(const napi_env &env, const napi_callback_info &info, napi_ref &callback);

    /**
     * @brief Gets a NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param result Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequest(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by number type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByNumber(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by string type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByString(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by bool type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByBool(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by custom type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByCustom(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the id of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationId(const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isOngoing flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsOngoing(
        const napi_env& env, const napi_value& value, NotificationRequest& request);

    /**
     * @brief Gets the delivery time of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationDeliveryTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the tapDismissed flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationtapDismissed(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the group name of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationGroupName(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the auto deleted time of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationAutoDeletedTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isAlertOnce flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsAlertOnce(
    
        const napi_env &env, const napi_value &value, NotificationRequest &request);
		
	/**
     * @brief Gets the isStopwatch flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsStopwatch(
        const napi_env& env, const napi_value& value, NotificationRequest& request);

    /**
     * @brief Gets the isCountDown flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsCountDown(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the showDeliveryTime flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationShowDeliveryTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationContent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a content type of notification from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param type Indicates a the content type of notification from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type);

    /**
     * @brief Gets a basic content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBasicContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationBasicContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param basicContent Indicates a NotificationBasicContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBasicContentDetailed(
        const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent);

    /**
     * @brief Gets a long-text content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLongTextContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationLongTextContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param longContent Indicates a NotificationLongTextContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLongTextContentDetailed(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLongTextContent> &longContent);

    /**
     * @brief Gets the multi-line content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationMultiLineContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets the lines of NotificationMultiLineContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param multiLineContent Indicates a NotificationMultiLineContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
        std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent);

    /**
     * @brief Gets the liveView content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLiveViewContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets the number of badge of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBadgeNumber(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Create a napi value with specified error object for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param errCode Indicates specified err code
     * @return Returns a napi value with specified error object for callback
     */
    static napi_value CreateErrorValue(napi_env env, int32_t errCode, bool newType);

    /**
     * @brief Create a napi value with specified error object for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param errCode Indicates specified err code
     * @param msg Indicates specified msg
     * @return Returns a napi value with specified error object for callback
     */
    static napi_value CreateErrorValue(napi_env env, int32_t errCode, std::string &msg);

    static void NapiThrow(napi_env env, int32_t errCode);
    static void NapiThrow(napi_env env, int32_t errCode, std::string &msg);
    static int32_t ErrorToExternal(uint32_t errCode);
    static void CreateReturnValue(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);
private:
    static const int32_t ARGS_ONE = 1;
    static const int32_t ARGS_TWO = 2;
    static const int32_t ONLY_CALLBACK_MAX_PARA = 1;
    static const int32_t ONLY_CALLBACK_MIN_PARA = 0;
    static std::mutex mutex_;
};
}  // namespace NotificationNapi
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H
