/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugins/test_plugin/android/java/jni/test_plugin_impl.h"

#include <memory>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/test_plugin/android/java/jni/test_plugin_jni.h"

namespace OHOS::Plugin {

std::unique_ptr<TestPlugin> TestPlugin::Create()
{
    return std::make_unique<TestPluginImpl>();
}

void TestPluginImpl::Hello()
{
    LOGI("TestPluginImpl Hello called");
    PluginUtilsInner::RunTaskOnPlatform([]() { TestPluginJni::Hello(); });
}

} // namespace OHOS::Plugin
