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

import ohos.ace.plugin.runninglockplugin.RunningLockPlugin;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * RunningLockPluginTest
 *
 * @since 1
 */
@RunWith(AndroidJUnit4.class)
public class RunningLockPluginTest {
    private Context context;
    private RunningLockPlugin runningLockPlugin;

    /**
     * Do something before test method start
     */
    @Before
    public void doBefore() {
        this.context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        this.runningLockPlugin = new RunningLockPlugin(this.context, false);
    }

    /**
     * Do something after test method end
     */
    @After
    public void doAfter() {
        this.context = null;
        this.runningLockPlugin = null;
    }

    /**
     * init
     */
    @Test
    public void init_0100() {
        RunningLockPlugin runningLockPluginTest = new RunningLockPlugin(null, false);
        boolean result = runningLockPluginTest.init("RunningLockTest", 1);
        assertFalse(result);
    }

    /**
     * init
     */
    @Test
    public void init_0200() {
        boolean result = runningLockPlugin.init("RunningLockTest", 0);
        assertFalse(result);
    }

    /**
     * init
     */
    @Test
    public void init_0300() {
        boolean result = runningLockPlugin.init("RunningLockTest", 1);
        assertTrue(result);
    }

    /**
     * init
     */
    @Test
    public void init_0400() {
        boolean result = runningLockPlugin.init("RunningLockTest", 2);
        assertTrue(result);
    }

    /**
     * isUsed
     */
    @Test
    public void isUsed_0100() {
        runningLockPlugin.init("RunningLockTest", 0);
        boolean result = runningLockPlugin.isUsed();
        assertFalse(result);
    }

    /**
     * isUsed
     */
    @Test
    public void isUsed_0200() {
        runningLockPlugin.init("RunningLockTest", 1);
        boolean result = runningLockPlugin.isUsed();
        assertFalse(result);
    }

    /**
     * isUsed
     */
    @Test
    public void isUsed_0300() {
        runningLockPlugin.init("RunningLockTest", 1);
        runningLockPlugin.lock(2000);
        runningLockPlugin.unLock();
        boolean result = runningLockPlugin.isUsed();
        assertFalse(result);
    }

    /**
     * isUsed
     */
    @Test
    public void isUsed_0400() {
        runningLockPlugin.init("RunningLockTest", 1);
        runningLockPlugin.lock(2000);
        boolean result = runningLockPlugin.isUsed();
        assertTrue(result);
    }

    /**
     * lock
     */
    @Test
    public void lock_0100() {
        runningLockPlugin.init("RunningLockTest", 0);
        boolean result = runningLockPlugin.lock(2000);
        assertFalse(result);
    }

    /**
     * lock
     */
    @Test
    public void lock_0200() {
        runningLockPlugin.init("RunningLockTest", 1);
        boolean result = runningLockPlugin.lock(0);
        assertTrue(result);
    }

    /**
     * lock
     */
    @Test
    public void lock_0300() {
        runningLockPlugin.init("RunningLockTest", 1);
        boolean result = runningLockPlugin.lock(2000);
        assertTrue(result);
    }

    /**
     * unLock
     */
    @Test
    public void unLock_0100() {
        runningLockPlugin.init("RunningLockTest", 0);
        runningLockPlugin.lock(2000);
        boolean result = runningLockPlugin.unLock();
        assertFalse(result);
    }

    /**
     * unLock
     */
    @Test
    public void unLock_0200() {
        runningLockPlugin.init("RunningLockTest", 1);
        runningLockPlugin.lock(2000);
        boolean result = runningLockPlugin.unLock();
        assertTrue(result);
    }

}