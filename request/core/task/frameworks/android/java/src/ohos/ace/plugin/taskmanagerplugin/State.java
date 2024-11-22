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

package ohos.ace.plugin.taskmanagerplugin;

public interface State {
    int INITIALIZED = 0x00;
    int WAITING = 0x10;
    int RUNNING = 0x20;
    int RETRYING = 0x21;
    int PAUSED = 0x30;
    int STOPPED = 0x31;
    int COMPLETED = 0x40;
    int FAILED = 0x41;
    int REMOVED = 0x50;
    int DEFAULT = 0x60;
}
