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

#ifndef COMMUNICATIONNETSTACK_DISK_IOS_HANDLER_H
#define COMMUNICATIONNETSTACK_DISK_IOS_HANDLER_H

#include <mutex>
#include <string>

namespace OHOS::NetStack::Http {
class DiskHandler final {
public:
    DiskHandler() = delete;

    explicit DiskHandler(std::string fileName);

    void Write(const std::string &str);

    void Delete();

    [[nodiscard]] std::string Read();

private:
    std::mutex mutex_;

    std::string fileName_;
};
} // namespace OHOS::NetStack::Http
#endif /* COMMUNICATIONNETSTACK_DISK_IOS_HANDLER_H */
