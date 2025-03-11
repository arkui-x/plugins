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

/**
 * Task State
 *
 * @since 2024-05-31
 */
public interface State {
    /**
     * Task State
     */
    int INITIALIZED = 0x00;

    /**
     * Task State
     */
    int WAITING = 0x10;

    /**
     * Task State
     */
    int RUNNING = 0x20;

    /**
     * Task State
     */
    int RETRYING = 0x21;

    /**
     * Task State
     */
    int PAUSED = 0x30;

    /**
     * Task State
     */
    int STOPPED = 0x31;

    /**
     * Task State
     */
    int COMPLETED = 0x40;

    /**
     * Task State
     */
    int FAILED = 0x41;

    /**
     * Task State
     */
    int REMOVED = 0x50;

    /**
     * Task State
     */
    int DEFAULT = 0x60;
}
