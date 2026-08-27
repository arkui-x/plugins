/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0
 */

#ifndef PLUGINS_FILE_FS_SRC_URI_OPENER_H
#define PLUGINS_FILE_FS_SRC_URI_OPENER_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

struct PlatformFdResult {
    int fd = -1;
    bool opened = false;
};

PlatformFdResult OpenByPlatformNative(const std::string &uri, unsigned int mode);

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // PLUGINS_FILE_FS_SRC_URI_OPENER_H
