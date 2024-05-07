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
import android.content.ContentResolver;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public class PhotoPlugin {
    private static String TAG = "photoPlugin";

    private static String IMAGE_PREFIX = "file://";

    private Context mContext;

    private Object mProxyInstance = null;

    public PhotoPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    public void startPhotoPicker(String type) {
        if (mContext == null) {
            Log.i(TAG, "mContext is null");
            return;
        }
        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType(type);
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

    private void setCallback(final Activity activity) {
        String packageName = activity.getPackageName();
        try {
            Class<?> delegate = Class.forName("ohos.stage.ability.adapter.StageActivityDelegate");
            Class<?> callbackInterface = Class.forName("ohos.stage.ability.adapter.StageActivityDelegate$Callback");
            Method addIntentMethod = delegate.getMethod("addIntentCallback", callbackInterface);
            Method removeIntentMethod = delegate.getMethod("removeIntentCallback", callbackInterface);
            if (mProxyInstance == null) {
                mProxyInstance = Proxy.newProxyInstance(
                    callbackInterface.getClassLoader(),
                    new Class<?>[] {callbackInterface},
                    (proxy, method, args) -> {
                        if (!method.getName().equals("onResult")) {
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
        } catch (NoSuchMethodException | ClassNotFoundException | InvocationTargetException | IllegalAccessException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private void removeIntentCallback(Class<?> delegate, Class<?> callbackInterface) {
        try {
            Method removeIntentMethod = delegate.getMethod("removeIntentCallback", callbackInterface);
            if (mProxyInstance != null) {
                removeIntentMethod.invoke(null, mProxyInstance);
            }
        } catch (NoSuchMethodException |  InvocationTargetException | IllegalAccessException e) {
            Log.e(TAG, "removeIntentCallback NoSuchMethodException");
        }
    }

    public void onResult(int requestCode, int resultCode, Intent data, Activity activity) {
        Log.i(TAG, "onResult enter requestCode is " + requestCode + ", " + resultCode);

        if (!checkResult(requestCode, resultCode, data)) return;

        List<String> rst = new ArrayList<>();
        int RESULT_OK = 0;
        ClipData clipData = data.getClipData();
        if (clipData == null) {
            onPickerResult(rst, RESULT_OK);
            return;
        }
        ContentResolver resolver = activity.getContentResolver();
        int count = clipData.getItemCount();
        if (resolver == null || count <= 0) {
            onPickerResult(rst, RESULT_OK);
            return;
        }
        for (int i = 0; i < count; i++) {
            Uri uri = clipData.getItemAt(i).getUri();
            String[] columns = {MediaStore.MediaColumns.DATA};
            Cursor cursor = resolver.query(uri, columns, null, null, null);
            if (cursor == null) {
                continue;
            }
            cursor.moveToFirst();
            int index = cursor.getColumnIndex(MediaStore.Images.Media.DATA);
            String relativePath = cursor.getString(index);
            if (relativePath == null || relativePath.isEmpty()) {
                continue;
            }
            String imagePath = IMAGE_PREFIX + relativePath;
            rst.add(imagePath);
        }
        onPickerResult(rst, RESULT_OK);
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

    protected native void nativeInit();

    protected native void onPickerResult(List<String> rst, int errCode);
}
