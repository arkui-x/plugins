/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_XCOLLIE_XCOLLIE_DEFINE_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_XCOLLIE_XCOLLIE_DEFINE_H
namespace OHOS {
namespace HiviewDFX {
/* define watchdog type */
static constexpr unsigned int XCOLLIE_LOCK = (1 << 0);
static constexpr unsigned int XCOLLIE_THREAD = (1 << 1);

/* define timeout flag */
static constexpr unsigned int XCOLLIE_FLAG_DEFAULT = (~0); // do all callback function
static constexpr unsigned int XCOLLIE_FLAG_NOOP = (0); // do nothing but the caller defined function
static constexpr unsigned int XCOLLIE_FLAG_LOG = (1 << 0); // generate log file
static constexpr unsigned int XCOLLIE_FLAG_RECOVERY = (1 << 1); // die when timeout

/* define xcollie id */
static constexpr int INVALID_ID = -1;
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif