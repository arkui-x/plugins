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
 * Faults
 *
 * @since 2025-05-31
 */
public interface Faults {
    /**
     * Faults Code
     */
    int OTHERS = 0xFF;

    /**
     * Faults Code
     */
    int DISCONNECTED = 0x00;

    /**
     * Faults Code
     */
    int TIMEOUT = 0x10;

    /**
     * Faults Code
     */
    int PROTOCOL = 0x20;

    /**
     * Faults Code
     */
    int FSIO = 0x40;
}
