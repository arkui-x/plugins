/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_LOG_H
#define PLUGINS_REQUEST_LOG_H

#include "plugins/interfaces/native/log.h"

#define REQUEST_HILOGF(fmt, ...) LOGF(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGE(fmt, ...) LOGE(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGW(fmt, ...) LOGW(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGD(fmt, ...) LOGD(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGI(fmt, ...) LOGI(fmt, ##__VA_ARGS__)

#endif // PLUGINS_REQUEST_LOG_H