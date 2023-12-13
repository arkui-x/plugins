/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "plugins/util/plugin/ios/util_plugin_impl.h"

#include <string>

#import "util_plugin.h"

namespace OHOS::Plugin {
std::unique_ptr<Util> Util::Create()
{
    return std::make_unique<UtilPluginImpl>();
}

std::string hexTostring(const std::string& str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); i += 2)    // 2: two hex characters combined to one character
    {
        std::string byte = str.substr(i, 2);
        char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
        result.push_back(chr);
    }
    return result;
}

std::string UtilPluginImpl::EncodeIntoChinese(std::string input, std::string encoding)
{
    NSString *inputString = [NSString stringWithCString:input.c_str() encoding:NSUTF8StringEncoding];
    NSString *encodingString = [NSString stringWithCString:encoding.c_str() encoding:[NSString defaultCStringEncoding]];
    NSString *origin = [[UtilPlugin shareinstance] encodeIntoChinese:inputString encoding:encodingString];
    std::string hexString = std::string([origin UTF8String]);
    std::string result = hexTostring(hexString);
    return result;
}

std::string UtilPluginImpl::Decode(std::string input, std::string encoding)
{
    NSString *inputString = [NSString stringWithCString:input.c_str() encoding:NSISOLatin1StringEncoding];
    NSString *encodingString = [NSString stringWithCString:encoding.c_str() encoding:[NSString defaultCStringEncoding]];
    NSString *origin = [[UtilPlugin shareinstance] decode:inputString encoding:encodingString];
    std::string result = std::string([origin UTF8String]);
    return result;
}

} // namespace OHOS::Plugin