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

package com.example.plugin.test;

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import ohos.ace.plugin.displayplugin.DisplayInfo;
import ohos.ace.plugin.displayplugin.DisplayPlugin;

import static org.junit.Assert.assertEquals;

/**
 * DisplayPluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class DisplayPluginTest {
    private Context context;
    private DisplayPlugin displayPlugin;
    private DisplayInfo displayInfo;
    private WindowManager windowManager;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.displayPlugin = new DisplayPlugin(this.context, false);
        this.displayInfo = (DisplayInfo) this.displayPlugin.getDisplayInfo();
        this.windowManager = (WindowManager) this.context.getSystemService(Context.WINDOW_SERVICE);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.displayPlugin = null;
        this.displayInfo = null;
        this.windowManager = null;
    }

    /**
     * get displayInfo id
     */
    @Test
    public void getDisplayInfo_id_0100() {
        assertEquals(windowManager.getDefaultDisplay().getDisplayId(), displayInfo.id);
    }

    /**
     * get displayInfo name
     */
    @Test
    public void getDisplayInfo_name_0100() {
        assertEquals(windowManager.getDefaultDisplay().getName(), displayInfo.name);
    }

    /**
     * get displayInfo alive
     */
    @Test
    public void getDisplayInfo_alive_0100() {
        assertEquals(windowManager.getDefaultDisplay().isValid(), displayInfo.alive);
    }

    /**
     * get displayInfo state
     */
    @Test
    public void getDisplayInfo_state_0100() {
        assertEquals(windowManager.getDefaultDisplay().getState(), displayInfo.state);
    }

    /**
     * get displayInfo refreshRate
     */
    @Test
    public void getDisplayInfo_refreshRate_0100() {
        assertEquals(windowManager.getDefaultDisplay().getRefreshRate(), displayInfo.refreshRate, 1);
    }

    /**
     * get displayInfo rotation
     */
    @Test
    public void getDisplayInfo_rotation_0100() {
        assertEquals(windowManager.getDefaultDisplay().getRotation(), displayInfo.rotation);
    }

    /**
     * get displayInfo width
     */
    @Test
    public void getDisplayInfo_width_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.widthPixels, displayInfo.width);
    }

    /**
     * get displayInfo height
     */
    @Test
    public void getDisplayInfo_height_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.heightPixels, displayInfo.height);
    }

    /**
     * get displayInfo densityDPI
     */
    @Test
    public void getDisplayInfo_densityDPI_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.densityDpi, displayInfo.densityDPI);
    }

    /**
     * get displayInfo densityPixels
     */
    @Test
    public void getDisplayInfo_densityPixels_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.density, displayInfo.densityPixels, 1);
    }

    /**
     * get displayInfo scaledDensity
     */
    @Test
    public void getDisplayInfo_scaledDensity_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.scaledDensity, displayInfo.scaledDensity, 1);
    }

    /**
     * get displayInfo xDPI
     */
    @Test
    public void getDisplayInfo_xDPI_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.xdpi, displayInfo.xDPI, 1);
    }

    /**
     * get displayInfo yDPI
     */
    @Test
    public void getDisplayInfo_yDPI_0100() {
        DisplayMetrics metric = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(metric);
        assertEquals(metric.ydpi, displayInfo.yDPI, 1);
    }
}