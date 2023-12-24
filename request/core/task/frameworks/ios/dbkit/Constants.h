/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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


#ifndef Constants_h
#define Constants_h

typedef NS_ENUM(int, TaskAction) {
    ActionDownload = 0,
    ActionUpload = 1,
};

typedef NS_ENUM(int, TaskVersion) {
    API9 = 0,
    API10 = 1,
    UNKNOWN
};

typedef NS_ENUM(int, TaskMode) {
    ModeBackground = 0,
    ModeForeground = 1,
};

typedef NS_ENUM(int, TaskFaults) {
    FaultsOthers = 0xFF,
    FaultsDisconnected = 0x00,
    FaultsTimeout = 0x10,
    FaultsProtocol = 0x20,
    FaultsFsio = 0x40,
};

typedef NS_ENUM(int, TaskProgressState) {
    StateInitialized = 0x00,
    StateWarning = 0x10,
    StateRuning = 0x20,
    StateRetrying = 0x21,
    StatePaused = 0x30,
    StateStoped = 0x31,
    StateCompleted = 0x40,
    StateFailed = 0x41,
    StateRemoved = 0x50,
};

#endif /* Constants_h */
