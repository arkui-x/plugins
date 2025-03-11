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

package ohos.ace.plugin.runninglockplugin;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;

/**
 * RunningLockPlugin
 *
 * @since 1
 */
public class RunningLockPlugin {
    private static final String LOG_TAG = "RunningLockPlugin";
    private PowerManager powerManager;
    private WakeLock wakeLock;

    /**
     * RunningLockPlugin
     *
     * @param context context of the application
     */
    public RunningLockPlugin(Context context) {
        new RunningLockPlugin(context, true);
    }

    /**
     * RunningLockPlugin
     *
     * @param context context of the application
     * @param isNativeInit is native init
     */
    public RunningLockPlugin(Context context, boolean isNativeInit) {
        if (context != null) {
            Object service = context.getSystemService(Context.POWER_SERVICE);
            if (service instanceof PowerManager) {
                powerManager = (PowerManager) service;
            } else {
                Log.e(LOG_TAG, "unable to get power service");
                powerManager = null;
            }
        } else {
            Log.e(LOG_TAG, "context is null");
            powerManager = null;
        }
        if (isNativeInit) {
            nativeInit();
        }
    }

    /**
     * init WakeLock
     *
     * @param name name of the wakeLock
     * @param type type of the wakeLock
     */
    public boolean init(String name, int type) {
        int wakeLockLevel;
        if (type == 1) {
            wakeLockLevel = PowerManager.PARTIAL_WAKE_LOCK;
        } else if (type == 2) {
            wakeLockLevel = PowerManager.PROXIMITY_SCREEN_OFF_WAKE_LOCK;
        } else {
            return false;
        }
        if (powerManager != null) {
            wakeLock = powerManager.newWakeLock(wakeLockLevel, name);
            if (wakeLock == null) {
                Log.e(LOG_TAG, "init wakeLock is null");
                return false;
            }
        } else {
            Log.e(LOG_TAG, "init powerManager is null");
            return false;
        }
        return true;
    }

    /**
     * RunningLock isUsed
     *
     * @return The RunningLock is isUsed.
     */
    public boolean isUsed() {
        if (wakeLock != null) {
            return wakeLock.isHeld();
        } else {
            Log.e(LOG_TAG, "isUsed wakeLock is null");
        }
        return false;
    }

    /**
     * RunningLock Lock
     *
     * @param timeOutMs timeOutMs of the wakeLock
     * @return The RunningLock is locked.
     */
    public boolean lock(long timeOutMs) {
        if (wakeLock != null) {
            if (timeOutMs == 0) {
                wakeLock.acquire();
            } else {
                wakeLock.acquire(timeOutMs);
            }
        } else {
            Log.e(LOG_TAG, "lock wakeLock is null");
            return false;
        }
        return true;
    }

    /**
     * RunningLock unLock
     *
     * @return The RunningLock is unlocked.
     */
    public boolean unLock() {
        if (wakeLock != null) {
            wakeLock.release();
        } else {
            Log.e(LOG_TAG, "unLock wakeLock is null");
            return false;
        }
        return true;
    }

    /**
     * RunningLock isNativeInit
     */
    protected native void nativeInit();
}
