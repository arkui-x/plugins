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
 * event type
 *
 * @since 2024-05-31
 */
public interface EventType {
    /**
     * event type
     */
    String PROGRESS = "progress";

    /**
     * event type
     */
    String RESPONSE = "response";

    /**
     * event type
     */
    String COMPLETED = "completed";

    /**
     * event type
     */
    String PAUSE = "pause";

    /**
     * event type
     */
    String RESUME = "resume";

    /**
     * event type
     */
    String REMOVE = "remove";

    /**
     * event type
     */
    String FAILED = "failed";
}
