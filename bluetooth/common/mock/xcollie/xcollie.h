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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_XCOLLIE_XCOLLIE_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_XCOLLIE_XCOLLIE_H

#include <functional>
#include <string>
#include <mutex>

using XCollieCallback = std::function<void (void *)>;

namespace OHOS {
namespace HiviewDFX {
class XCollie {
public:
    static XCollie& GetInstance()
    {
        std::call_once(initInstanceFlag_, &XCollie::InitializeSingleton);
        return *instance_;
    }

    XCollie(const XCollie&) = delete;

    XCollie& operator=(const XCollie&) = delete;

    int SetTimer(const std::string& name, unsigned int timeout,
        XCollieCallback func, void *arg, unsigned int flag) { return 0; }

    void CancelTimer(int id) {}

    int SetTimerCount(const std::string& name, unsigned int timeLimit, int countLimit) { return 0; }

    void TriggerTimerCount(const std::string& name, bool bTrigger, const std::string& message) {}

private:
    XCollie() {}

    ~XCollie() {}

    static void InitializeSingleton()
    {
        instance_ = new XCollie();
    }

    static XCollie* instance_;

    static std::once_flag initInstanceFlag_;
};

XCollie* XCollie::instance_ = nullptr;
std::once_flag XCollie::initInstanceFlag_;
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_XCOLLIE_XCOLLIE_H