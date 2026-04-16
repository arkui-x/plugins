/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef MOCK_PLUGIN_UTILS_INNER_H
#define MOCK_PLUGIN_UTILS_INNER_H
#define CHECK_NULL_VOID(ptr) do { if (!(ptr)) return; } while (0)
#define CHECK_NULL_RETURN(ptr, ret) do { if (!(ptr)) return ret; } while (0)
#define CHECK_NULL_VOID_DELREF(ptr, env, cls) do { if (!(ptr)) { (env)->DeleteLocalRef((cls)); return; } } while (0)
#endif
