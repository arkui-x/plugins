/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.plugin.storagestatistics;

import android.app.usage.StorageStats;
import android.app.usage.StorageStatsManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Process;
import android.os.storage.StorageManager;

import java.io.File;
import java.io.IOException;
import java.util.ArrayDeque;

import ohos.ace.adapter.ALog;

/**
 * Storage statistics plugin.
 *
 * @since 26
 */
public class StorageStatisticsPlugin {
    private static final String LOG_TAG = "StorageStatisticsPlugin";
    private static final int ANDROID_O_API_LEVEL = 26;
    private static final long INVALID_INODES = -1L;
    private static final int BUNDLE_STATS_LENGTH = 3;
    private static final int APP_BYTES_INDEX = 0;
    private static final int CACHE_BYTES_INDEX = 1;
    private static final int DATA_BYTES_INDEX = 2;
    private static final int MAX_TRAVERSAL_DEPTH = 64;

    private final Context mContext;

    /**
     * StorageStatisticsPlugin
     *
     * @param context application context
     */
    public StorageStatisticsPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    /**
     * Queries current application storage statistics.
     *
     * @return a long array containing app size, cache size and data size; returns an empty array if query fails
     */
    public long[] getCurrentBundleStats() {
        if (mContext == null) {
            ALog.e(LOG_TAG, "context is null");
            return new long[0];
        }
        if (Build.VERSION.SDK_INT < ANDROID_O_API_LEVEL) {
            ALog.e(LOG_TAG, "StorageStatsManager requires Android 26+");
            return new long[0];
        }
        Object service = mContext.getSystemService(Context.STORAGE_STATS_SERVICE);
        if (!(service instanceof StorageStatsManager)) {
            ALog.e(LOG_TAG, "storage stats service is unavailable");
            return new long[0];
        }
        try {
            StorageStats stats = ((StorageStatsManager) service).queryStatsForPackage(
                StorageManager.UUID_DEFAULT, mContext.getPackageName(), Process.myUserHandle());
            return buildBundleStats(stats);
        } catch (PackageManager.NameNotFoundException | IOException | SecurityException exception) {
            ALog.e(LOG_TAG, "getCurrentBundleStats failed, exception="
                + exception.getClass().getSimpleName());
            return new long[0];
        }
    }

    /**
     * Counts inode-like file entries for the current application's internal storage directory tree.
     *
     * @return the counted entry size, or {@link #INVALID_INODES} if the query fails
     */
    public long getCurrentBundleInodes() {
        if (mContext == null) {
            ALog.e(LOG_TAG, "context is null");
            return INVALID_INODES;
        }
        long inodes = 0L;
        try {
            File filesDir = mContext.getFilesDir();
            if (filesDir == null) {
                ALog.e(LOG_TAG, "filesDir is null");
                return INVALID_INODES;
            }
            File internalDir = filesDir.getParentFile();
            if (internalDir != null && internalDir.exists()) {
                inodes += countFilesInDirectory(internalDir);
                return inodes;
            }
            ALog.e(LOG_TAG, "internalDir is invalid");
            return INVALID_INODES;
        } catch (SecurityException e) {
            ALog.e(LOG_TAG, "getCurrentBundleInodes failed, exception="
                + e.getClass().getSimpleName());
            return INVALID_INODES;
        }
    }

    private static long[] buildBundleStats(StorageStats stats) {
        if (stats == null) {
            return new long[0];
        }
        long[] bundleStats = new long[BUNDLE_STATS_LENGTH];
        bundleStats[APP_BYTES_INDEX] = stats.getAppBytes();
        bundleStats[CACHE_BYTES_INDEX] = stats.getCacheBytes();
        bundleStats[DATA_BYTES_INDEX] = stats.getDataBytes();
        return bundleStats;
    }

    private static long countFilesInDirectory(File dir) {
        if (dir == null || !dir.exists()) {
            return 0L;
        }
        long count = 0L;
        ArrayDeque<File> pendingDirs = new ArrayDeque<>();
        ArrayDeque<Integer> pendingDepths = new ArrayDeque<>();
        pendingDirs.push(dir);
        pendingDepths.push(0);
        while (!pendingDirs.isEmpty()) {
            File currentDir = pendingDirs.pop();
            int currentDepth = pendingDepths.pop();
            if (currentDepth >= MAX_TRAVERSAL_DEPTH) {
                ALog.w(LOG_TAG, "countFilesInDirectory reached max traversal depth");
                continue;
            }
            File[] files = currentDir.listFiles();
            if (files == null) {
                continue;
            }
            for (File file : files) {
                count++;
                if (file.isDirectory()) {
                    pendingDirs.push(file);
                    pendingDepths.push(currentDepth + 1);
                }
            }
        }
        return count;
    }

    /**
     * nativeInit
     */
    protected native void nativeInit();
}
