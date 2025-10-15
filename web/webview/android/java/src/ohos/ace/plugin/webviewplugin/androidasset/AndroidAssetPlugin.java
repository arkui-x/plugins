/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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

package ohos.ace.plugin.webviewplugin.androidasset;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.IOException;
import java.io.InputStream;

/**
 * AndroidAssetPlugin is used to manage AndroidAsset
 *
 * @since 22
 */
public class AndroidAssetPlugin {
    private static final String LOG_TAG = "AndroidAssetPlugin";
    private static final String ANDROID_VIRTUAL_ASSET_PREFIX = "file:///android_asset/";

    private final Context context;

    public AndroidAssetPlugin(Context context) {
        this.context = context;
        nativeInit();
    }

    /**
     * Checks whether a virtual asset exists at the specified URL.
     *
     * <p>This method verifies if the given URL starts with the expected virtual asset prefix.
     * If so, it attempts to open the asset using the {@link AssetManager}. If the asset can be
     * opened without throwing an {@link IOException}, it is considered to exist.</p>
     *
     * @param url The URL of the virtual asset to check.
     * @return {@code true} if the virtual asset exists; {@code false} otherwise.
     */
    public boolean existsVirtualAsset(String url) {
        if (!url.startsWith(ANDROID_VIRTUAL_ASSET_PREFIX)) {
            return false;
        }

        AssetManager assetManager = this.context.getAssets();
        String assetPath = url.substring(ANDROID_VIRTUAL_ASSET_PREFIX.length());
        try (InputStream inputStream = assetManager.open(assetPath)) {
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * Initialize the native environment.
     */
    protected final native void nativeInit();
}
