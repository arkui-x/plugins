/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.testplugin;

import android.content.Context;
import android.util.Log;

/**
 * TestPlugin class is used to test plugin
 *
 * @since 2024-08-06
 */
public class TestPlugin {
    private static final String LOG_TAG = "TestPlugin";
    public TestPlugin(Context context) {
        nativeInit();
    }

    /**
     * hello is used to test plugin
     */
    public void hello() {
        Log.i(LOG_TAG, "TestPlugin: hello from java");
    }

    /**
     * nativeInit is used to init native
     */
    protected native void nativeInit();
}
