/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2026. All rights reserved.
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

#ifndef PASTEBOARD_PROGRESS_SIGNAL_H
#define PASTEBOARD_PROGRESS_SIGNAL_H

#include <atomic>

#include "api/visibility.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT ProgressSignalClient {
public:
    static ProgressSignalClient& GetInstance();
    void Cancel();
    bool IsCanceled();
    bool CheckCancelIfNeed();
    void Init();
    void SetRemoteTaskCancel();

private:
    ProgressSignalClient() = default;
    ~ProgressSignalClient() = default;
    std::atomic_bool needCancel_ { false };
    std::atomic_bool remoteTask_ { false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_PROGRESS_SIGNAL_H