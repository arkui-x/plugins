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

package com.example.plugin.test;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.util.Log;
import ohos.ace.plugin.netconnclientplugin.NetConnClientPlugin;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

/**
 * ConnectionPluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class ConnectionPluginTest {
    private Context context;
    private NetConnClientPlugin netConnClientPlugin;
    private NetConnClientPlugin.NetworkCallbackImpl networkCallbackImpl;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        Log.i("ConnectionPluginTest", "netConnCLient test doBefore");
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        netConnClientPlugin = new NetConnClientPlugin(this.context, false);
        networkCallbackImpl = netConnClientPlugin.new NetworkCallbackImpl();
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
    }

    /**
     * Checks if the default network is active when the context is valid.
     */
    @Test
    public void isDefaultNetworkActive_0100() {
        boolean result = netConnClientPlugin.isDefaultNetworkActive();
        assertTrue(result);
    }

    /**
     * Registers a default network callback
     */
    @Test
    public void registerDefaultNetConnCallback_0100() {
        netConnClientPlugin.registerDefaultNetConnCallback(1);
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertNotEquals(callbackKey, 1);

        netConnClientPlugin.unregisterNetConnCallback(1);
    }

    /**
     * Registers a default network callback which is already exists
     */
    @Test
    public void registerDefaultNetConnCallback_0200() {
        netConnClientPlugin.registerDefaultNetConnCallback(2);
        netConnClientPlugin.registerDefaultNetConnCallback(2);
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertNotEquals(callbackKey, 2);

        netConnClientPlugin.unregisterNetConnCallback(2);
    }

    /**
     * Registers a network callback
     */
    @Test
    public void registerNetConnCallback_0100() {
        int[] types = {1};
        netConnClientPlugin.registerNetConnCallback(1, types);
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertNotEquals(callbackKey, 1);

        netConnClientPlugin.unregisterNetConnCallback(1);
    }

    /**
     * Registers a network callback which is already exists
     */
    @Test
    public void registerNetConnCallback_0200() {
        int[] types = {1};
        netConnClientPlugin.registerNetConnCallback(2, types);
        netConnClientPlugin.registerNetConnCallback(2, types);
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertNotEquals(callbackKey, 2);

        netConnClientPlugin.unregisterNetConnCallback(2);
    }

    /**
     * Unregisters a network callback
     */
    @Test
    public void unregisterNetConnCallback_0100() {
        int[] types = {1, 2};
        netConnClientPlugin.registerNetConnCallback(2, types);
        netConnClientPlugin.unregisterNetConnCallback(2);
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertNotEquals(callbackKey, 2);
    }

    /**
     * Gets callback key when there is no callback
     */
    @Test
    public void getCallbackKey_0100() {
        long callbackKey = netConnClientPlugin.getCallbackKey(networkCallbackImpl);
        assertEquals(callbackKey, 0);
    }
}