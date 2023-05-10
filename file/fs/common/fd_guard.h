/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FD_GUARD_H
#define FD_GUARD_H

namespace OHOS {
namespace DistributedFS {
class FDGuard final {
public:
    FDGuard() = default;
    explicit FDGuard(int fd);
    FDGuard(int fd, bool autoClose);

    FDGuard(const FDGuard &fdg) = delete;
    FDGuard &operator=(const FDGuard &fdg) = delete;

    FDGuard(FDGuard &&fdg);
    FDGuard &operator=(FDGuard &&fdg);

    operator bool() const;

    ~FDGuard();

    int GetFD() const;
    void SetFD(int fd, bool autoClose = true);
    void ClearFD();

private:
    int fd_ = -1;
    bool autoClose_ = true;
};
} // namespace DistributedFS
} // namespace OHOS
#endif