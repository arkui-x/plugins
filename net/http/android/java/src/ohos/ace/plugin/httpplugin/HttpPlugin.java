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

package ohos.ace.plugin.httpplugin;

import android.content.Context;
import java.io.File;
import java.io.IOException;

/**
 * HttpPlugin
 *
 * @since 1
 */
public class HttpPlugin {
    private static final String LOG_TAG = "Context is null";

    private static final String FIRE_NAME = "/cache.json";

    private final Context context_;

    /**
     * HttpPlugin
     *
     * @param context context of the application
     */
    public HttpPlugin(Context context) {
        this.context_ = context;
        nativeInit();
    }

    /**
     * Get cache file path
     *
     * @return Return cache file path
     * @throws IOException IOException
     */
    public String getCacheDir() throws IOException {
        if (this.context_ == null) {
            return LOG_TAG;
        }
        String cachePath = null;
        cachePath = context_.getCacheDir().getPath();
        String firePath = cachePath + FIRE_NAME;
        File file = new File(firePath);
        if (file.exists()) {
            return firePath;
        } else {
            file.createNewFile();
            return firePath;
        }
    }

    /**
     * Init HttpPlugin jni.
     *
     */
    protected native void nativeInit();
}
