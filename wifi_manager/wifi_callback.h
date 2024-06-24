/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_WIFI_MANAGER_WIFI_CALLBACK_H
#define PLUGINS_WIFI_MANAGER_WIFI_CALLBACK_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

#include "inner_api/plugin_utils_napi.h"
#include "wifi_msg.h"
#include "wifi_napi_utils.h"

namespace OHOS::Plugin {
class WifiCallback {
public:
    WifiCallback(const WifiCallback& commonEvent) = delete;
    WifiCallback& operator=(const WifiCallback& commonEvent) = delete;
    ~WifiCallback() = default;
    static WifiCallback& GetInstance()
    {
        static WifiCallback instance;
        return instance;
    }
    void SendCallback(const std::string& key, int code);
    void RegisterCallback(const napi_env &env, napi_value callback, const std::string& name);
    void UnRegisterCallback(const napi_env &env, napi_value callback, const std::string& name);
    bool HasWifiRegister(const std::string& name);

private:
    WifiCallback();
    bool HashExitCallback(const napi_env& env, napi_value callback, const std::vector<RegObj>& regObjs);
    void DeleteRegisterObj(const napi_env& env, std::vector<RegObj>& vecRegObjs, napi_value& handler);
    void DeleteAllRegisterObj(const napi_env& env, std::vector<RegObj>& vecRegObjs);

private:
    std::map<std::string, std::vector<RegObj>> maps_;
    std::mutex mutex_;
};
} // namespace OHOS::Plugin

#endif
