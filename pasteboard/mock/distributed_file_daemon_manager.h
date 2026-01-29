/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef PLUGINS_PASTEBOARD_MOCK_DISTRIBUTED_FILE_DAEMON_MANAGER_H
#define PLUGINS_PASTEBOARD_MOCK_DISTRIBUTED_FILE_DAEMON_MANAGER_H

#include <cstdint>
#include <string>

namespace OHOS {
template<typename T>
class sptr;

namespace Storage {
namespace DistributedFile {

class FileDfsListenerStub;
class FileTransListenerStub;
struct HmdfsInfo;

class DistributedFileDaemonManager final {
public:
    static DistributedFileDaemonManager& GetInstance()
    {
        static DistributedFileDaemonManager instance;
        return instance;
    }

    int32_t ConnectDfs(const std::string& networkId)
    {
        return 0;
    }

    int32_t DisconnectDfs(const std::string& networkId)
    {
        return 0;
    }

    int32_t OpenP2PConnectionEx(const std::string& networkId, const sptr<FileDfsListenerStub>& listener)
    {
        return 0;
    }

    int32_t CloseP2PConnectionEx(const std::string& networkId)
    {
        return 0;
    }

    int32_t PrepareSession(const std::string& srcUri, const std::string& destUri, const std::string& networkId,
        FileTransListenerStub* listener, HmdfsInfo& info)
    {
        return 0;
    }

private:
    DistributedFileDaemonManager() = default;
    ~DistributedFileDaemonManager() = default;
    DistributedFileDaemonManager(const DistributedFileDaemonManager&) = delete;
    DistributedFileDaemonManager& operator=(const DistributedFileDaemonManager&) = delete;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // PLUGINS_PASTEBOARD_MOCK_DISTRIBUTED_FILE_DAEMON_MANAGER_H
