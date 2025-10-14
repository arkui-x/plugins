/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef IOS_JSON_PARSER_H
#define IOS_JSON_PARSER_H

#include <string>
#include "raw_file_descriptor.h"

namespace OHOS {
namespace Sensors {

class IosJsonParser {
public:
    IosJsonParser(const std::string &jsonStr);
    IosJsonParser(const RawFileDescriptor &rawFd);
    ~IosJsonParser();

    void* GetObjectItem(const std::string &key) const;
    void* GetObjectItem(const void* parent, const std::string &key) const;
    bool IsArray(void* item) const;
    int32_t GetArraySize(void* array) const;
    void* GetArrayItem(void* array, int32_t index) const;
    bool HasObjectItem(const std::string &key) const;
    int32_t GetIntValue(void* item, int32_t defaultValue = -1) const;
    std::string GetStringValue(void* item, const std::string &defaultValue = "") const;
    bool IsNumber(void* item) const;
    bool IsString(void* item) const;
    double GetDoubleValue(void* item, double defaultValue = 0.0) const;

private:
    void ParseJsonString(const std::string &jsonStr);
    void* root_ = nullptr;
};

} // namespace Sensors
} // namespace OHOS

#endif //IOS_JSON_PARSER_H
