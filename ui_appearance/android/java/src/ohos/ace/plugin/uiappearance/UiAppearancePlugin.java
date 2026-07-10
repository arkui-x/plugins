/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos.ace.plugin.uiappearance;

import android.content.Context;
import android.content.res.Configuration;

/**
 * UiAppearancePlugin
 *
 * @since 26.0.0
 */
public class UiAppearancePlugin {
    private static final int ALWAYS_DARK = 0;
    private static final int ALWAYS_LIGHT = 1;

    private final Context context;

    /**
     * UiAppearancePlugin
     *
     * @param context context of the application
     */
    public UiAppearancePlugin(Context context) {
        this.context = context;
        nativeInit();
    }

    /**
     * UiAppearance getDarkMode
     *
     * @return The dark mode setting.
     */
    public int getDarkMode() {
        if (context == null) {
            return ALWAYS_LIGHT;
        }
        int uiMode = context.getResources().getConfiguration().uiMode;
        int nightMode = uiMode & Configuration.UI_MODE_NIGHT_MASK;
        return nightMode == Configuration.UI_MODE_NIGHT_YES ? ALWAYS_DARK : ALWAYS_LIGHT;
    }

    /**
     * UiAppearance getFontScale
     *
     * @return The font scale setting.
     */
    public double getFontScale() {
        if (context == null) {
            return 1.0;
        }
        return context.getResources().getConfiguration().fontScale;
    }

    /**
     * UiAppearance isNativeInit
     */
    protected native void nativeInit();
}
