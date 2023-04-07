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

import android.content.Context;
import android.util.Log;
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

    /**
     * DeviceInfoPlugin
     *
     * @param context context of the application
     */
    public DeviceInfoPlugin(Context context) {
        new DeviceInfoPlugin(context, true);
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
        return getProperty("ro.build.characteristics", defValue);
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
        return String.join(",", Build.SUPPORTED_ABIS);
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
            return getProperty("persist.product.firstapiversion", def);
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

    /**
     * nativeInit
     * Register the initialization method of the plugin for the plugin construction to call.
     * @return void
     */
    protected native void nativeInit();
}

