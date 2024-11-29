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

#include "disk_handler.h"

#include <fstream>
#include <sstream>

#include "netstack_log.h"

namespace OHOS::NetStack::Http {
DiskHandler::DiskHandler(std::string fileName) : fileName_(std::move(fileName)) {}

void DiskHandler::Write(const std::string &str)
{
    std::lock_guard<std::mutex> guard(mutex_);
    std::ofstream w(fileName_);
    if (!w.is_open()) {
        return;
    }
    w << str;
    w.close();
}

std::string DiskHandler::Read()
{
    std::lock_guard<std::mutex> guard(mutex_);
    std::ifstream r(fileName_);
    if (!r.is_open()) {
        return {};
    }
    std::stringstream b;
    b << r.rdbuf();
    r.close();
    return b.str();
}

void DiskHandler::Delete()
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (remove(fileName_.c_str()) < 0) {
        NETSTACK_LOGI("remove file error %{public}d", errno);
    }
}
} // namespace OHOS::NetStack::Http
