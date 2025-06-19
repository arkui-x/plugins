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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_ATOMIC_FILE_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_ATOMIC_FILE_ENTITY_H

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
struct AtomicFileEntity {
    std::string baseFileName = "";
    std::string newFileName = "";
    napi_ref writeStreamObj = nullptr;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_ATOMIC_FILE_ENTITY_H