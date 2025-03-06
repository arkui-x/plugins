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
package ohos.ace.plugin.photoaccesshelper;

import android.content.Context;
import android.util.Log;
import android.content.Intent;
import android.content.ClipData;
import android.content.ActivityNotFoundException;
import android.provider.MediaStore;
import android.app.Activity;
import android.net.Uri;
import android.database.Cursor;
import android.os.Build;
import android.os.Bundle;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.pm.PackageManager;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

import ohos.ace.plugin.photoaccesshelper.AlbumValues;

/**
 * PhotoPlugin class is used to implement the photo picker function
 *
 * @since 2024-06-24
 */
public class PhotoPlugin {
    private static String TAG = "photoPlugin";

    private static String IMAGE_PREFIX = "file://";

    private static String EMPTY_STR = "";

    private static String SELECTION_KEY = "android:query-arg-sql-selection";

    private Context mContext;

    private Object mProxyInstance = null;

    private ContentResolver mResolver;

    public PhotoPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    /**
     * startPhotoPicker
     *
     * @param type type
     */
    public void startPhotoPicker(String type) {
        if (mContext == null) {
            Log.i(TAG, "mContext is null");
            return;
        }
        Intent intent = new Intent(Intent.ACTION_PICK);
        if ("*/*".equals(type)) {
            intent.setType("image/*;video/*");
        } else {
            intent.setType(type);
        }
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        Activity activity = getActivity();
        if (activity == null) {
            return;
        }
        setCallback(activity);
        try {
            activity.startActivityForResult(intent, 1001);
        } catch (ActivityNotFoundException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * checkPermission
     *
     * @return boolean
     */
    public boolean checkPermission() {
        if (mContext == null) {
            Log.i(TAG, "mContext is null");
            return false;
        }
        PackageManager pm = mContext.getPackageManager();
        if (Build.VERSION.SDK_INT >= 33) {
            if (pm.checkPermission("android.permission.READ_MEDIA_IMAGES",
                mContext.getPackageName()) == PackageManager.PERMISSION_GRANTED ||
                pm.checkPermission("android.permission.READ_MEDIA_VIDEO",
                mContext.getPackageName()) == PackageManager.PERMISSION_GRANTED) {
                return true;
            } else {
                Log.e(TAG, "not has permission");
                return false;
            }
        } else {
            if (pm.checkPermission("android.permission.READ_EXTERNAL_STORAGE",
                mContext.getPackageName()) == PackageManager.PERMISSION_GRANTED) {
                return true;
            } else {
                Log.e(TAG, "not has permission");
                return false;
            }
        }
    }

    /**
     * queryPhoto
     *
     * @param selection selection
     * @param selectionArgs selectionArgs
     * @param projection projection
     * @return Cursor
     */
    public Cursor queryPhoto(Bundle selection, String[] selectionArgs, String[] projection) {
        if (mContext == null) {
            Log.i(TAG, "mContext is null");
            return null;
        }
        Activity activity = getActivity();
        if (activity == null) {
            return null;
        }
        if (mResolver == null) {
            mResolver = activity.getContentResolver();
        }
        Uri uri = MediaStore.Files.getContentUri("external");
        String selectionStr = selection.getString(SELECTION_KEY);
        if (Build.VERSION.SDK_INT >= 30) {
            String str = "is_pending = ? AND " + selectionStr;
            selection.putString(SELECTION_KEY, str);
            String[] selectionArgsWithPending = new String[selectionArgs.length + 1];
            selectionArgsWithPending[0] = "0";
            for (int index = 0; index < selectionArgs.length; index++) {
                selectionArgsWithPending[index + 1] = selectionArgs[index];
            }
            selection.putStringArray("android:query-arg-sql-selection-args", selectionArgsWithPending);
            return mResolver.query(uri, projection, selection, null);
        } else if (Build.VERSION.SDK_INT >= 26) {
            StringBuilder filterArg = new StringBuilder();
            appendSelectionArg(filterArg, selection.getString("android:query-arg-sql-group-by"), ") GROUP BY(");
            selection.putString(SELECTION_KEY, selectionStr + filterArg.toString());
            return mResolver.query(uri, removeWhichNotSupport(projection), selection, null);
        } else {
            StringBuilder filterArg = new StringBuilder();
            filterArg.append(selection.getString(SELECTION_KEY));
            appendSelectionArg(filterArg, selection.getString("android:query-arg-sql-group-by"), ") GROUP BY(");
            return mResolver.query(uri, removeWhichNotSupport(projection), filterArg.toString(), selectionArgs,
                selection.getString("android:query-arg-sql-sort-order"));
        }
    }

    /**
     * query album
     *
     * @param selection selection
     * @param selectionArgs selectionArgs
     * @param projection projection
     * @return AlbumValues
     */
    public AlbumValues queryAlbum(Bundle selection, String[] selectionArgs, String[] projection) {
        if (mContext == null) {
            Log.i(TAG, "mContext is null");
            return null;
        }
        Cursor cursor = queryPhoto(selection, selectionArgs, projection);
        ContentQueryMap queryMap = new ContentQueryMap(cursor, MediaStore.Files.FileColumns._ID, false, null);
        AlbumValues albumValues = new AlbumValues(queryMap);
        return albumValues;
    }

    private void appendSelectionArg(StringBuilder filterArg, String clause, String name) {
        if (clause == null || clause.isEmpty()) {
            return;
        }
        filterArg.append(name);
        filterArg.append(clause);
    }

    private String[] removeWhichNotSupport(String[] projection) {
        if (projection == null || projection.length <= 0) {
            return projection;
        }
        List<String> list = Arrays.asList(projection);
        List<String> arrList = new ArrayList<String>(list);
        if (arrList.contains("is_favorite")) {
            arrList.remove("is_favorite");
            String[] supportColumns = new String[arrList.size()];
            for (int index = 0; index < arrList.size(); index++) {
                supportColumns[index] = arrList.get(index);
            }
            return supportColumns;
        } else {
            return projection;
        }
    }

    private void setCallback(final Activity activity) {
        String packageName = activity.getPackageName();
        try {
            Class<?> delegate = Class.forName("ohos.stage.ability.adapter.StageActivityDelegate");
            Class<?> callbackInterface =
                Class.forName("ohos.stage.ability.adapter.StageActivityDelegate$INTENTCALLBACK");
            Method addIntentMethod = delegate.getMethod("addIntentCallback", callbackInterface);
            if (mProxyInstance == null) {
                mProxyInstance = Proxy.newProxyInstance(
                    callbackInterface.getClassLoader(),
                    new Class<?>[] {callbackInterface},
                    (proxy, method, args) -> {
                        if (!"onResult".equals(method.getName())) {
                            return null;
                        }
                        int requestCode = (int) args[0];
                        int resultCode = (int) args[1];
                        Intent data = (Intent) args[2];
                        onResult(requestCode, resultCode, data, activity);
                        return null;
                    });
                addIntentMethod.invoke(null, mProxyInstance);
            }
        } catch (NoSuchMethodException | ClassNotFoundException | InvocationTargetException
                | IllegalAccessException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * onResult method is called when the activity result is returned
     *
     * @param requestCode requestCode
     * @param resultCode resultCode
     * @param data data
     * @param activity activity
     */
    public void onResult(int requestCode, int resultCode, Intent data, Activity activity) {
        Log.i(TAG, "onResult enter requestCode is " + requestCode + ", " + resultCode);

        if (!checkResult(requestCode, resultCode, data)) {
            return;
        }

        List<String> rst = new ArrayList<>();
        int RESULT_OK = 0;
        mResolver = activity.getContentResolver();
        if (mResolver == null) {
            onPickerResult(rst, RESULT_OK);
            return;
        }
        ClipData clipData = data.getClipData();
        if (clipData == null || clipData.getItemCount() <= 0) {
            rst = getIntentData(data);
            onPickerResult(rst, RESULT_OK);
            return;
        }
        int count = clipData.getItemCount();
        for (int i = 0; i < count; i++) {
            Uri uri = clipData.getItemAt(i).getUri();
            String imagePath = getImagePath(uri);
            if (imagePath.equals(EMPTY_STR)) {
                continue;
            }
            rst.add(imagePath);
        }
        onPickerResult(rst, RESULT_OK);
    }

    private List<String> getIntentData(Intent data) {
        List<String> rst = new ArrayList<>();
        Uri uri = data.getData();
        if (uri != null) {
            String imagePath = getImagePath(uri);
            if (!imagePath.equals(EMPTY_STR)) {
                rst.add(imagePath);
            }
        }
        return rst;
    }

    private String getImagePath(Uri uri) {
        if (mResolver == null) {
            return EMPTY_STR;
        }
        String[] columns = {MediaStore.MediaColumns.DATA};
        Cursor cursor = mResolver.query(uri, columns, null, null, null);
        if (cursor == null) {
            return EMPTY_STR;
        }
        cursor.moveToFirst();
        int index = cursor.getColumnIndex(MediaStore.Images.Media.DATA);
        String relativePath = cursor.getString(index);
        if (relativePath == null || relativePath.isEmpty()) {
            return EMPTY_STR;
        }
        String imagePath = IMAGE_PREFIX + relativePath;
        return imagePath;
    }

    private boolean checkResult(int requestCode, int resultCode, Intent data) {
        String errorMsg = "requestCode:" + requestCode + ".   resultCode:" + resultCode;

        if (requestCode != 1001) {
            Log.d(TAG, errorMsg);
            return false;
        }

        if (resultCode != Activity.RESULT_OK) {
            Log.i(TAG, errorMsg);
            onPickerResult(new ArrayList<>(), 0);
            return false;
        }

        if (data == null) {
            errorMsg += ".  data is null";
            Log.i(TAG, errorMsg);
            onPickerResult(new ArrayList<>(), 0);
            return false;
        }
        return true;
    }

    private Activity getActivity() {
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
            Field mActivities = activityThreadClass.getDeclaredField("mActivities");
            mActivities.setAccessible(true);
            Map activitiesMap = (Map) mActivities.get(activityThread);
            for (Object activityClientRecord : activitiesMap.values()) {
                Class activityClientRecordClass = activityClientRecord.getClass();
                Field paused = activityClientRecordClass.getDeclaredField("paused");
                paused.setAccessible(true);
                if (!paused.getBoolean(activityClientRecord)) {
                    Field activityField = activityClientRecordClass.getDeclaredField("activity");
                    activityField.setAccessible(true);
                    Activity activity = (Activity) activityField.get(activityClientRecord);
                    return activity;
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * nativeInit
     */
    protected native void nativeInit();

    /**
     * onPickerResult
     *
     * @param rst result
     * @param errCode error code
     */
    protected native void onPickerResult(List<String> rst, int errCode);
}