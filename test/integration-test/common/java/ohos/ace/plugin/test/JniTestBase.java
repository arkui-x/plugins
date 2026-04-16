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

package ohos.ace.plugin.test;

/**
 * L3 JNI test base class (no Activity dependency).
 * Encapsulates Mock reset logic per test-rules.md section 4.3.
 *
 * Note: This class lives in the common main source set (shared with ALog etc.)
 * so it cannot use JUnit annotations. Subclasses must call {@code doResetMock()}
 * in their own {@code @Before} method.
 *
 * @since 1
 */
public abstract class JniTestBase {
    /**
     * Subclass implements: call module-specific Mock reset method.
     */
    protected abstract void doResetMock();
}
