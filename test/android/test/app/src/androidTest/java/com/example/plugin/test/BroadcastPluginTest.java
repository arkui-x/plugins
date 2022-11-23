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

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import ohos.ace.plugin.broadcastplugin.BroadcastPlugin;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * BroadcastPluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class BroadcastPluginTest {
    private Context context;
    private BroadcastPlugin broadcastPlugin;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        broadcastPlugin = new BroadcastPlugin(this.context, false);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
    }

    /**
     * sendBroadcast
     */
    @Test
    public void sendBroadcast_0100() {
        BroadcastPlugin broadcastPluginTest = new BroadcastPlugin(null, false);
        boolean result = broadcastPluginTest.sendBroadcast("sendBroadcast", "");
        assertFalse(result);
    }

    /**
     * sendBroadcast
     */
    @Test
    public void sendBroadcast_0200() {
        boolean result = broadcastPlugin.sendBroadcast("", "");
        assertFalse(result);
    }

    /**
     * sendBroadcast
     */
    @Test
    public void sendBroadcast_0300() {
        boolean result = broadcastPlugin.sendBroadcast("sendBroadcast", "XXX");
        assertFalse(result);
    }

    /**
     * sendBroadcast
     */
    @Test
    public void sendBroadcast_0400() {
        boolean result = broadcastPlugin.sendBroadcast("sendBroadcast", "");
        assertTrue(result);
    }

    /**
     * sendBroadcast
     */
    @Test
    public void sendBroadcast_0500() {
        boolean result = broadcastPlugin.sendBroadcast("sendBroadcast", "{\"key\":\"value\", \"key2\":\"value2\"}");
        assertTrue(result);
    }

    /**
     * registerBroadcast
     */
    @Test
    public void registerBroadcast_0100() {
        BroadcastPlugin broadcastPluginTest = new BroadcastPlugin(null, false);
        boolean result = broadcastPluginTest.registerBroadcast("registerBroadcast", null);
        assertFalse(result);
    }

    /**
     * registerBroadcast
     */
    @Test
    public void registerBroadcast_0200() {
        boolean result = broadcastPlugin.registerBroadcast("", null);
        assertFalse(result);
    }

    /**
     * registerBroadcast
     */
    @Test
    public void registerBroadcast_0300() {
        boolean result = broadcastPlugin.registerBroadcast("key", null);
        assertFalse(result);
    }

    /**
     * registerBroadcast
     */
    @Test
    public void registerBroadcast_0400() {
        String[] actions = {};
        boolean result = broadcastPlugin.registerBroadcast("key", actions);
        assertFalse(result);
    }

    /**
     * registerBroadcast
     */
    @Test
    public void registerBroadcast_0500() {
        String[] actions = {"action", "actions2"};
        boolean result = broadcastPlugin.registerBroadcast("key", actions);
        assertTrue(result);
    }

    /**
     * unRegisterBroadcast
     */
    @Test
    public void unRegisterBroadcast_0100() {
        BroadcastPlugin broadcastPluginTest = new BroadcastPlugin(null, false);
        boolean result = broadcastPluginTest.unRegisterBroadcast("key");
        assertFalse(result);
    }

    /**
     * unRegisterBroadcast
     */
    @Test
    public void unRegisterBroadcast_0200() {
        boolean result = broadcastPlugin.unRegisterBroadcast("");
        assertFalse(result);
    }

    /**
     * unRegisterBroadcast
     */
    @Test
    public void unRegisterBroadcast_0300() {
        boolean result = broadcastPlugin.unRegisterBroadcast("key");
        assertTrue(result);
    }
}