/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.displayplugin;

import android.content.Context;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;

/**
 * DisplayPlugin
 *
 * @since 1
 */
public class DisplayPlugin {
    private static final String LOG_TAG = "DisplayPlugin";

    private WindowManager windowManager;

    /**
     * DisplayPlugin
     *
     * @param context context of the application
     */
    public DisplayPlugin(Context context) {
        new DisplayPlugin(context, true);
    }

    /**
     * DisplayPlugin
     *
     * @param context      context of the application
     * @param isNativeInit call nativeInit or not
     */
    public DisplayPlugin(Context context, boolean isNativeInit) {
        if (context != null) {
            Object service = context.getSystemService(Context.WINDOW_SERVICE);
            if (service instanceof WindowManager) {
                this.windowManager = (WindowManager) service;
            } else {
                Log.e(LOG_TAG, "unable to get display service");
                this.windowManager = null;
            }
        } else {
            Log.e(LOG_TAG, "context is null");
            this.windowManager = null;
        }
        if (isNativeInit) {
            nativeInit();
        }
    }

    /**
     * Get Display Info
     *
     * @return Display Info
     */
    public Object getDisplayInfo() {
        DisplayInfo displayInfo = new DisplayInfo();
        displayInfo.id = getId();
        displayInfo.name = getName();
        displayInfo.alive = isAlive();
        displayInfo.state = getState();
        displayInfo.refreshRate = getRefreshRate();
        displayInfo.rotation = getRotation();
        displayInfo.width = getWidth();
        displayInfo.height = getHeight();
        displayInfo.densityDPI = getDensityDPI();
        displayInfo.densityPixels = getDensityPixels();
        displayInfo.scaledDensity = getScaledDensity();
        displayInfo.xDPI = getXDPI();
        displayInfo.yDPI = getYDPI();
        return displayInfo;
    }

    /**
     * Get Display Id
     *
     * @return Display Id
     */
    private int getId() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().getDisplayId();
        }
        return 0;
    }

    /**
     * Get Display Name
     *
     * @return Display Name
     */
    private String getName() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().getName();
        }
        return null;
    }

    /**
     * Get Display Alive
     *
     * @return The display is alive.
     */
    private boolean isAlive() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().isValid();
        }
        return false;
    }

    /**
     * Get Display State
     *
     * @return The state of display
     */
    private int getState() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().getState();
        }
        return 0;
    }

    /**
     * Get Display Refresh Rate
     *
     * @return Refresh rate
     */
    private float getRefreshRate() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().getRefreshRate();
        }
        return 0;
    }

    /**
     * Get Display Rotation
     *
     * @return Rotation degrees of the display
     */
    private int getRotation() {
        if (windowManager != null) {
            return windowManager.getDefaultDisplay().getRotation();
        }
        return 0;
    }

    /**
     * Get Display Width
     *
     * @return Display width, in pixels.
     */
    private int getWidth() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.widthPixels;
        }
        return 0;
    }

    /**
     * Get Display Height
     *
     * @return Display height, in pixels.
     */
    private int getHeight() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.heightPixels;
        }
        return 0;
    }

    /**
     * Get Display DensityDPI
     *
     * @return Display resolution
     */
    private int getDensityDPI() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.densityDpi;
        }
        return 0;
    }

    /**
     * Get Display DensityPixels
     *
     * @return Display density, in pixels. The value for a low-resolution display is 1.0.
     */
    private float getDensityPixels() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.density;
        }
        return 0;
    }

    /**
     * Get Display ScaledDensity
     *
     * @return Text scale density of the display.
     */
    private float getScaledDensity() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.scaledDensity;
        }
        return 0;
    }

    /**
     * Get Display xDPI
     *
     * @return DPI on the x-axis.
     */
    private float getXDPI() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.xdpi;
        }
        return 0;
    }

    /**
     * Get Display yDPI
     *
     * @return DPI on the y-axis.
     */
    private float getYDPI() {
        if (windowManager != null) {
            DisplayMetrics metric = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(metric);
            return metric.ydpi;
        }
        return 0;
    }

    protected native void nativeInit();
}
