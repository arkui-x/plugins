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
    private final Context context;

    public FsUriOpenerPlugin(Context context) {
        this.context = context;
        nativeInit();
    }

    protected native void nativeInit();

    public int openFd(String uriString, int posixFlags) {
        if (context == null) {
            Log.e(TAG, "openFd: context is null");
            return -1;
        }
        Uri uri;
        try {
            uri = Uri.parse(uriString);
        } catch (Exception e) {
            Log.e(TAG, "openFd: invalid uri " + uriString);
            return -22;
        }
        if (uri == null) {
            return -22;
        }
        String modeStr = posixFlagsToString(posixFlags);
        try {
            ParcelFileDescriptor pfd = context.getContentResolver().openFileDescriptor(uri, modeStr);
            if (pfd == null) {
                Log.e(TAG, "openFd: pfd is null for " + uriString);
                return -5;
            }
            int fdInt = pfd.getFd();
            pfd.detachFd();
            return fdInt;
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "openFd illegal arg: " + e.getMessage());
            return -22;
        } catch (SecurityException e) {
            Log.e(TAG, "openFd security: " + e.getMessage());
            return -13;
        } catch (Exception e) {
            Log.e(TAG, "openFd failed: " + e.getMessage());
            return -5;
        }
    }

    private static String posixFlagsToString(int flags) {
        int accmode = flags & 03;
        boolean trunc = (flags & 01000) != 0;
        if (accmode == 0) {
            return "r";
        }
        if (accmode == 1) {
            return "w";
        }
        return trunc ? "rwt" : "rw";
    }
}
