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

package ohos.ace.plugin.device_infoplugin;

import android.app.UiModeManager;
import android.view.WindowManager;
import android.content.Context;
import android.content.res.Configuration;
import android.util.Log;
import android.util.DisplayMetrics;
import android.os.Build;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * DeviceInfoPlugin
 *
 * @since 1
 */
public class DeviceInfoPlugin {
    private static final String CLASS_SYSTEM_PROPERTIES = "android.os.SystemProperties";

    private static final String LOG_TAG = "DeviceInfoPlugin";

    private Context context;

    /**
     * DeviceInfoPlugin
     *
     * @param context context of the application
     */
    public DeviceInfoPlugin(Context context) {
        new DeviceInfoPlugin(context, true);
        this.context = context;
    }

    /**
     * DeviceInfoPlugin
     *
     * @param context      context of the application
     * @param isNativeInit call nativeInit or not
     */
    public DeviceInfoPlugin(Context context, boolean isNativeInit) {
        if (isNativeInit) {
            nativeInit();
        }
        this.context = context;
    }

    /**
     * getOSFullName
     *
     * @param defValue default value
     * @return OSFullName
     */
    public String getOSFullName(String defValue) {
        return "Android " + Build.VERSION.RELEASE;
    }

    /**
     * getManufacture
     *
     * @param defValue default value
     * @return manufacture
     */
    public String getManufacture(String defValue) {
        return Build.MANUFACTURER;
    }

    /**
     * getDeviceType
     *
     * @param defValue default value
     * @return device type
     */

    public String getDeviceType(String defValue) {
        if (context.getPackageManager().hasSystemFeature("amazon.hardware.fire_tv")) {
            return "Tv";
        }

        UiModeManager uiManager = (UiModeManager) context.getSystemService(Context.UI_MODE_SERVICE);
        if (uiManager != null && uiManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) {
            return "Tv";
        }

        String deviceType = getDeviceTypeByResourceConfiguration();
        if (deviceType != null && !"Unknown".equals(deviceType)) {
            return deviceType;
        }

        return getDeviceTypeByPhysicalSize();
    }

    /**
     * getBrand
     *
     * @param defValue default value
     * @return brand
     */
    public String getBrand(String defValue) {
        return Build.BRAND;
    }

    /**
     * getMarketName
     *
     * @param defValue default value
     * @return MarketName
     */
    public String getMarketName(String defValue) {
        return Build.PRODUCT; // ro.product.name
    }

    /**
     * getProductSeries
     *
     * @param defValue default value
     * @return ProductSeries
     */
    public String getProductSeries(String defValue) {
        return Build.DEVICE; // ro.product.device
    }

    /**
     * getProductModel
     *
     * @param defValue default value
     * @return ProductModel
     */
    public String getProductModel(String defValue) {
        return Build.MODEL; // ro.product.model
    }

    /**
     * getProductModelAlias
     *
     * @param defValue default value
     * @return ProductModelAlias
     */
    public String getProductModelAlias(String defValue) {
        return Build.MODEL; // ro.product.model
    }

    /**
     * getSoftwareModel
     *
     * @param defValue default value
     * @return SoftwareModel
     */
    public String getSoftwareModel(String defValue) {
        return Build.MODEL; // ro.product.model
    }

    /**
     * getHardwareModel
     *
     * @param defValue default value
     * @return hardware model
     */
    public String getHardwareModel(String defValue) {
        return Build.BOARD; // ro.product.board
    }

    /**
     * getHardwareProfile
     *
     * @param defValue default value
     * @return HardwareProfile
     */
    public String getHardwareProfile(String defValue) {
        return defValue; //
    }

    /**
     * getBootLoaderVersion
     *
     * @param defValue default value
     * @return BootLoaderVersion
     */
    public String getBootLoaderVersion(String defValue) {
        return Build.BOOTLOADER;
    }

    /**
     * getAbiList
     *
     * @param defValue default value
     * @return AbiList
     */
    public String getAbiList(String defValue) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return String.join(",", Build.SUPPORTED_ABIS);
        }
        return Build.CPU_ABI;
    }

    /**
     * getDisplayVersion
     *
     * @param defValue default value
     * @return display version
     */
    public String getDisplayVersion(String defValue) {
        return Build.VERSION.RELEASE;
    }

    /**
     * getIncrementalVersion
     *
     * @param defValue default value
     * @return version
     */
    public String getIncrementalVersion(String defValue) {
        return Build.VERSION.INCREMENTAL;
    }

    /**
     * getFirstApiVersion
     *
     * @param defValue default value
     * @return first api version
     */
    public int getFirstApiVersion(int def) {
        int value = getProperty("persist.product.firstapiversion", -1);
        if (value == -1) {
            setProperty("persist.product.firstapiversion", Integer.toString(def));
            return def;
        }
        return value;
    }

    private String getProperty(final String name, String defValue) {
        return getProp(name, defValue);
    }

    private int getProperty(final String name, int defValue) {
        try {
            String value = getProp(name, Integer.toString(defValue));
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            Log.e(LOG_TAG, "Failed to get property name: " + name + " exception: " + e.toString());
            return defValue;
        }
    }

    private static int setProperty(String key, String value) {
        Object result = invoke(CLASS_SYSTEM_PROPERTIES, "set", new Class<?>[] {String.class, String.class}, key, value);
        if (result instanceof Integer) {
            return (int) result;
        }
        return -1;
    }

    private static String getProp(String cfgName, String defValue) {
        Object result = invoke(CLASS_SYSTEM_PROPERTIES, "get", new Class<?>[] {String.class, String.class},
                cfgName, defValue);
        if (result instanceof String) {
            return (String) result;
        }
        return defValue;
    }

    private static Object invoke(String className, String methodName, Class<?>[] parameterTypes, Object... args) {
        Object value = null;

        try {
            Class<?> clz = Class.forName(className);
            Method method = clz.getDeclaredMethod(methodName, parameterTypes);
            value = method.invoke(clz, args);
        } catch (NoSuchMethodException | IllegalAccessException | IllegalArgumentException | InvocationTargetException
                | SecurityException | ClassNotFoundException e) {
            Log.e(LOG_TAG, "invoke " + e.getClass().getSimpleName());
        } catch (Exception e) {
            Log.e(LOG_TAG, "unknown Exception in invoke");
            e.printStackTrace();
        }
        return value;
    }

    private String getDeviceTypeByResourceConfiguration() {
        int minScreenWidth = context.getResources().getConfiguration().smallestScreenWidthDp;

        if (minScreenWidth == Configuration.SMALLEST_SCREEN_WIDTH_DP_UNDEFINED) {
            return "Unknown";
        }

        return minScreenWidth >= 600 ? "tablet" : "phone";
    }

    private String getDeviceTypeByPhysicalSize() {
        String deviceType = "Unknown";
        // Find the current window manager, if none is found we can't measure the device physical size.
        WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        if (windowManager == null) {
            return deviceType;
        }

        DisplayMetrics metrics = new DisplayMetrics();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            windowManager.getDefaultDisplay().getRealMetrics(metrics);
        } else {
            windowManager.getDefaultDisplay().getMetrics(metrics);
        }

        double width = metrics.widthPixels / (double) metrics.xdpi;
        double height = metrics.heightPixels / (double) metrics.ydpi;
        double diagonalSize = Math.sqrt(Math.pow(width, 2) + Math.pow(height, 2));

        if (diagonalSize >= 3.0 && diagonalSize <= 6.9) {
            deviceType = "phone";
        } else if (diagonalSize > 6.9 && diagonalSize <= 18.0) {
            deviceType = "tablet";
        } else {
            deviceType = "Unknown";
        }

        return deviceType;
    }

    /**
     * Get the device type.
     */
    protected native void nativeInit();
}
