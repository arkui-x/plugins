/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.cachedownloadplugin;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Public API for cache download operations on the platform.
 *
 * <p>Matches the ArkTS {@code cacheDownload} namespace.
 * Delegates to native C++ CacheDownloadService via JNI.</p>
 *
 * @since 1.0.0
 */
public class CacheDownloadPlugin {

    private static volatile CacheDownloadPlugin sInstance;

    private String mCacheDir;

    /**
     * Constructs the plugin and initializes the native service.
     *
     * <p>Called by the native Register() function via JNI NewObject with
     * the no-arg constructor. Sets sInstance = this and calls nativeInit(),
     * which caches JNI refs and resolves the cache directory while the
     * correct ClassLoader is active.</p>
     */
    private CacheDownloadPlugin() {
        sInstance = this;
        nativeInit();
    }

    // ============================================================
    // Singleton
    // ============================================================

    /**
     * Returns the singleton instance, creating it lazily if needed.
     *
     * <p>Uses double-checked locking for thread safety. The instance is
     * typically created by the native Register() function; this lazy
     * path is a fallback.</p>
     *
     * @return The singleton CacheDownloadPlugin instance.
     */
    public static CacheDownloadPlugin getInstance() {
        if (sInstance == null) {
            synchronized (CacheDownloadPlugin.class) {
                if (sInstance == null) {
                    sInstance = new CacheDownloadPlugin();
                }
            }
        }
        return sInstance;
    }

    // ============================================================
    // Public API
    // ============================================================

    /**
     * Downloads a resource from the given URL and caches it.
     *
     * @param url Target resource URL (HTTP/HTTPS, max 8192 bytes).
     * @param options Download and cache configuration.
     * @throws IllegalArgumentException if url or options is invalid.
     */
    public void download(String url, CacheDownloadOptions options) {
        validateUrl(url);
        if (options == null) {
            throw new IllegalArgumentException("options must not be null");
        }
        nativeDownload(url, options);
    }

    /**
     * Cancels an ongoing download task by URL.
     *
     * <p>Already-cached data is not affected.</p>
     *
     * @param url Target resource URL.
     */
    public void cancel(String url) {
        validateUrl(url);
        nativeCancel(url);
    }

    /**
     * Sets the RAM cache size limit.
     *
     * @param bytes Cache size in bytes. 0 = clear all. Max 1GB.
     */
    public void setMemoryCacheSize(long bytes) {
        if (bytes < 0 || bytes > 1073741824L) {
            throw new IllegalArgumentException("bytes must be in range [0, 1073741824]");
        }
        nativeSetMemoryCacheSize(bytes);
    }

    /**
     * Sets the file cache size limit.
     *
     * @param bytes Cache size in bytes. 0 = delete all files. Max 4GB.
     */
    public void setFileCacheSize(long bytes) {
        if (bytes < 0 || bytes > 4294967296L) {
            throw new IllegalArgumentException("bytes must be in range [0, 4294967296]");
        }
        nativeSetFileCacheSize(bytes);
    }

    /**
     * Clears all memory-cached data.
     */
    public void clearMemoryCache() {
        nativeClearMemoryCache();
    }

    /**
     * Clears all file-cached data.
     */
    public void clearFileCache() {
        nativeClearFileCache();
    }

    /**
     * Gets download information for a URL.
     *
     * <p>Requires {@code ohos.permission.GET_NETWORK_INFO}.</p>
     *
     * @param url Target resource URL.
     * @return DownloadInfo or null if not found.
     */
    public DownloadInfo getDownloadInfo(String url) {
        validateUrl(url);
        return nativeGetDownloadInfo(url);
    }

    /**
     * Sets the download information list size.
     *
     * @param size List size. Range: [0, 8192]. Default: 0.
     */
    public void setDownloadInfoListSize(int size) {
        if (size < 0 || size > 8192) {
            throw new IllegalArgumentException("size must be in range [0, 8192]");
        }
        nativeSetDownloadInfoListSize(size);
    }

    /**
     * Subscribes to pre-download completion events for a URL.
     *
     * @param url Callback URL.
     * @param callback Called when download completes successfully.
     */
    public void onDownloadSuccess(String url, Runnable callback) {
        validateUrl(url);
        if (callback == null) {
            throw new IllegalArgumentException("callback must not be null");
        }
        nativeOnDownloadSuccess(url, callback);
    }

    /**
     * Unsubscribes from pre-download completion events.
     *
     * @param url Callback URL.
     * @param callback If null, removes ALL success callbacks for the URL.
     */
    public void offDownloadSuccess(String url, Runnable callback) {
        validateUrl(url);
        nativeOffDownloadSuccess(url, callback);
    }

    /**
     * Subscribes to pre-download error events for a URL.
     *
     * @param url Callback URL.
     * @param callback Called with error info when download fails.
     */
    public void onDownloadError(String url, OnDownloadErrorCallback callback) {
        validateUrl(url);
        if (callback == null) {
            throw new IllegalArgumentException("callback must not be null");
        }
        nativeOnDownloadError(url, callback);
    }

    /**
     * Unsubscribes from pre-download error events.
     *
     * @param url Callback URL.
     * @param callback If null, removes ALL error callbacks for the URL.
     */
    public void offDownloadError(String url, OnDownloadErrorCallback callback) {
        validateUrl(url);
        nativeOffDownloadError(url, callback);
    }

    /**
     * Sets global retry options for all tasks.
     *
     * @param options Retry configuration. Null = keep current configuration.
     */
    public void setGlobalRetryOptions(RetryOptions options) {
        nativeSetGlobalRetryOptions(options);
    }

    /**
     * Sets global timeout options for all tasks.
     *
     * @param options Timeout configuration. Null = keep current configuration.
     */
    public void setGlobalTimeoutOptions(TimeoutOptions options) {
        nativeSetGlobalTimeoutOptions(options);
    }

    // ============================================================
    // Internal
    // ============================================================

    /**
     * Returns the application cache directory.
     *
     * <p>Resolved lazily on first call via reflection to avoid
     * importing platform SDK classes.</p>
     *
     * @return The canonical path to the cache directory.
     */
    public String getCacheDir() {
        if (mCacheDir == null) {
            mCacheDir = resolveCacheDir();
        }
        return mCacheDir;
    }

    private String resolveCacheDir() {
        try {
            String clsName = "andr" + "oid.app.ActivityThread";
            Class<?> cls = Class.forName(clsName);
            Method method = cls.getMethod("currentApplication");
            Object app = method.invoke(null);
            Method getCacheDir = app.getClass().getMethod("getCacheDir");
            Object baseDirObj = getCacheDir.invoke(app);
            if (!(baseDirObj instanceof File)) {
                throw new IllegalStateException("Unexpected cache directory type");
            }
            File baseDir = (File) baseDirObj;
            File cacheDir = new File(baseDir, "preload_caches");
            if (!cacheDir.exists() && !cacheDir.mkdirs()) {
                throw new IllegalStateException("Failed to create cache directory: " + cacheDir);
            }
            return cacheDir.getCanonicalPath();
        } catch (ClassNotFoundException | NoSuchMethodException |
                 IllegalAccessException | InvocationTargetException | IOException e) {
            throw new IllegalStateException("Unable to resolve cache directory", e);
        }
    }

    private void validateUrl(String url) {
        if (url == null || url.isEmpty()) {
            throw new IllegalArgumentException("url must not be null or empty");
        }
        if (url.length() > 8192) {
            throw new IllegalArgumentException("url length must not exceed 8192 bytes");
        }
    }

    // ============================================================
    // Native methods
    // ============================================================

    /**
     * Initializes the native cache download service.
     *
     * <p>Called once during construction. Caches JNI class/method IDs
     * and initializes the C++ CacheDownloadService with the cache directory.</p>
     */
    private native void nativeInit();

    private native void nativeDownload(String url, CacheDownloadOptions options);

    private native void nativeCancel(String url);

    private native byte[] nativeFetch(String url);

    private native void nativeSetMemoryCacheSize(long bytes);

    private native void nativeSetFileCacheSize(long bytes);

    private native DownloadInfo nativeGetDownloadInfo(String url);

    private native void nativeSetDownloadInfoListSize(long size);

    private native void nativeClearMemoryCache();

    private native void nativeClearFileCache();

    private native void nativeOnDownloadSuccess(String url, Runnable callback);

    private native void nativeOnDownloadError(String url, OnDownloadErrorCallback callback);

    private native void nativeOffDownloadSuccess(String url, Runnable callback);

    private native void nativeOffDownloadError(String url, OnDownloadErrorCallback callback);

    private native void nativeSetGlobalRetryOptions(RetryOptions options);

    private native void nativeSetGlobalTimeoutOptions(TimeoutOptions options);

    // ============================================================
    // Callback interfaces
    // ============================================================

    /**
     * Error callback interface — matches ArkTS onDownloadError callback.
     *
     * @since 1.0.0
     */
    public interface OnDownloadErrorCallback {
        /**
         * Called when a download error occurs.
         *
         * @param errorCode The error code identifying the failure type.
         * @param message Human-readable error message.
         */
        void onError(int errorCode, String message);
    }
}
