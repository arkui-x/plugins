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

#ifndef PLUGINS_TEST_PLUGIN_TEST_PLUGIN_H
#define PLUGINS_TEST_PLUGIN_TEST_PLUGIN_H

#include <memory>

namespace OHOS::Plugin {

class TestPlugin {
public:
    TestPlugin() = default;
    virtual ~TestPlugin() = default;

    static std::unique_ptr<TestPlugin> Create();

    virtual void Hello() = 0;

#ifdef IOS_PLATFORM
    virtual std::string GetFilesDir() = 0;
#endif
};

} // namespace OHOS::Plugin

#endif // PLUGINS_TEST_PLUGIN_TEST_PLUGIN_H
