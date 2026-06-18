/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "common_func.h"

#include "application_context.h"
#include "log.h"
#include "sandbox_helper.h"

namespace OHOS {
namespace AppFileService {
namespace {
const std::string FILE_SCHEME_PREFIX = "file://";
const char BACKSLASH = '/';
} // namespace

std::string CommonFunc::GetSelfBundleName()
{
    auto appContext = AbilityRuntime::Platform::ApplicationContext::GetInstance();
    if (appContext == nullptr) {
        LOGE("ApplicationContext instance is nullptr");
        return "";
    }
    return appContext->GetBundleName();
}

bool CommonFunc::EndsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.length() > str.length()) {
        return false;
    }
    return (str.rfind(suffix) == (str.length() - suffix.length()));
}

std::string CommonFunc::GetUriFromPath(const std::string& path)
{
    if (!SandboxHelper::IsValidPath(path)) {
        LOGE("path is invalid path, The path contains '../' characters");
        return "";
    }

    if (path.find(FILE_SCHEME_PREFIX) == 0) {
        return path;
    }

    std::string bundleName = GetSelfBundleName();

    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.front() != BACKSLASH) {
        normalizedPath.insert(normalizedPath.begin(), BACKSLASH);
    }

    return FILE_SCHEME_PREFIX + bundleName + SandboxHelper::Encode(normalizedPath);
}
} // namespace AppFileService
} // namespace OHOS
