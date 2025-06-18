/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_H

#include "filemgmt_libn.h"
#include "file_filter.h"

#include <dirent.h>

namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
class ListFile {
public:
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Async(napi_env env, napi_callback_info info);
};

class ListFileArgs {
public:
    std::vector<std::string> dirents;
};

struct NameListArg {
    struct dirent** namelist = { nullptr };
    int direntNum = 0;
};

constexpr int DEFAULT_SIZE = -1;
constexpr int DEFAULT_MODIFY_AFTER = -1;
struct OptionArgs {
    FileFilter filter = FileFilterBuilder()
        .SetFileSizeOver(DEFAULT_SIZE)
        .SetLastModifiedAfter(DEFAULT_MODIFY_AFTER)
        .Build();
    int listNum = 0;
    int countNum = 0;
    bool recursion = false;
    std::string path = "";
    void Clear()
    {
        filter.FilterClear();
        filter.SetFileSizeOver(DEFAULT_SIZE);
        filter.SetLastModifiedAfter(DEFAULT_MODIFY_AFTER);
        listNum = 0;
        countNum = 0;
        recursion = false;
        path = "";
    }
};

constexpr int FILTER_MATCH = 1;
constexpr int FILTER_DISMATCH = 0;
const int32_t MAX_SUFFIX_LENGTH = 256;
const std::string LIST_FILE_PRODUCE_NAME = "FileIOListFile";
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_H