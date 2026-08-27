/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0
 */

package ohos.ace.plugin.fsuriopener;

import android.content.Context;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

public class FsUriOpenerPlugin {
    private static final String TAG = "FsUriOpenerPlugin";

    // POSIX open flags
    private static final int O_RDONLY = 0;
    private static final int O_WRONLY = 1;
    private static final int O_RDWR = 2;
    private static final int O_TRUNC = 01000;
    private static final int O_APPEND = 02000;

    // POSIX errno values (negative, matching kernel convention)
    private static final int ERRNO_EPERM = -1;
    private static final int ERRNO_EIO = -5;
    private static final int ERRNO_EACCES = -13;
    private static final int ERRNO_EINVAL = -22;

    private static final int BUF_SIZE = 64 * 1024;

    private final Context context;

    public FsUriOpenerPlugin(Context context) {
        this.context = context;
        nativeInit();
    }

    protected native void nativeInit();

    /**
     * Open a file descriptor for the given URI via ContentResolver.
     *
     * @param uriString  the content:// or other platform URI string
     * @param posixFlags POSIX open flags (O_RDONLY, O_WRONLY, O_RDWR, O_TRUNC, O_APPEND)
     * @return file descriptor (>= 0) on success, or negative errno on failure
     */
    public int openFd(String uriString, int posixFlags) {
        if (context == null) {
            Log.e(TAG, "openFd: context is null");
            return ERRNO_EPERM;
        }
        Uri uri;
        try {
            uri = Uri.parse(uriString);
        } catch (Exception e) {
            Log.e(TAG, "openFd: invalid uri " + uriString);
            return ERRNO_EINVAL;
        }
        if (uri == null) {
            return ERRNO_EINVAL;
        }
        String modeStr = posixFlagsToString(posixFlags);
        try {
            ParcelFileDescriptor pfd = context.getContentResolver().openFileDescriptor(uri, modeStr);
            if (pfd == null) {
                Log.e(TAG, "openFd: pfd is null for " + uriString);
                return ERRNO_EIO;
            }
            int fdInt = pfd.getFd();
            pfd.detachFd();
            return fdInt;
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "openFd illegal arg: " + e.getMessage());
            return ERRNO_EINVAL;
        } catch (SecurityException e) {
            Log.e(TAG, "openFd security: " + e.getMessage());
            return ERRNO_EACCES;
        } catch (Exception e) {
            Log.e(TAG, "openFd failed: " + e.getMessage());
            return ERRNO_EIO;
        }
    }

    /**
     * Convert POSIX open flags to ContentResolver mode string.
     *
     * ContentResolver.openFileDescriptor mode strings:
     *   "r"   - read only
     *   "w"   - write only (truncates existing file)
     *   "wt"  - write only + truncate (same as "w" for ContentResolver)
     *   "wa"  - write only + append
     *   "rw"  - read + write
     *   "rwt" - read + write + truncate
     *   "rwa" - read + write + append
     *
     * Note: ContentResolver "w" implicitly includes O_CREAT|O_TRUNC.
     *       O_WRONLY without O_TRUNC maps to "wa" to preserve file content.
     *       O_APPEND is honored by mapping to "wa" or "rwa".
     */
    private static String posixFlagsToString(int flags) {
        int accmode = flags & (O_RDONLY | O_WRONLY | O_RDWR);
        boolean trunc = (flags & O_TRUNC) != 0;
        boolean append = (flags & O_APPEND) != 0;

        if (accmode == O_RDONLY) {
            return "r";
        }
        if (accmode == O_WRONLY) {
            // O_WRONLY alone should not truncate; use "wa" to preserve file content.
            // Only truncate when O_TRUNC is explicitly set.
            if (append) {
                return "wa";
            }
            return trunc ? "w" : "wa";
        }
        if (append) {
            return "rwa";
        }
        return trunc ? "rwt" : "rw";
    }
}
