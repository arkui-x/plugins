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

package ohos.ace.plugin.netconnclientplugin;

import android.content.Context;
import android.util.Log;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.Network;
import android.net.NetworkRequest;
import java.util.Map;
import java.util.HashMap;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * NetConnClientPlugin
 *
 * @since 1
 */
public class NetConnClientPlugin {
    private static final String LOG_TAG = "NetConnClientPlugin";

    private ConnectivityManager connectivity;

    private Map<Long, NetworkCallbackImpl> observer = new HashMap<>();

    /**
     * NetConnClientPlugin
     *
     * @param context context of the application
     */
    public NetConnClientPlugin(Context context) {
        new NetConnClientPlugin(context, true);
    }

    /**
     * NetConnClientPlugin
     *
     * @param context context of the application
     * @param isNativeInit Initialize NativeInit.
     */
    public NetConnClientPlugin(Context context, boolean isNativeInit) {
        if (context != null) {
            Object service = context.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (service instanceof ConnectivityManager) {
                connectivity = (ConnectivityManager) service;
                Log.i(LOG_TAG, "enable to get ConnectivityManager service");
            } else {
                Log.e(LOG_TAG, "unable to get ConnectivityManager service");
                connectivity = null;
            }
        } else {
            Log.e(LOG_TAG, "context is null");
            connectivity = null;
        }
        if (isNativeInit) {
            nativeInit();
        }
    }

    /**
     * NetworkCallbackImpl The class for NetworkCallback callbacks. Used for notifications about network changes.
     *
     * @since 1
     */
    public class NetworkCallbackImpl extends ConnectivityManager.NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            long callbackKey = getCallbackKey(this);
            nativeOnAvailable(callbackKey, getNetworkId(network));
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            long callbackKey = getCallbackKey(this);
            nativeOnUnavailable(callbackKey);
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            long callbackKey = getCallbackKey(this);
            nativeOnLost(callbackKey, getNetworkId(network));
        }

        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
            super.onCapabilitiesChanged(network, networkCapabilities);
            long callbackKey = getCallbackKey(this);
            nativeOnCapabilitiesChanged(networkCapabilities, callbackKey, getNetworkId(network));
        }

        private int getNetworkId(Network network) {
            int networkId = 0;
            try {
                final Method method = Network.class.getMethod("getNetId");
                if (method == null) {
                    Log.e(LOG_TAG, "getNetId method is null");
                    return networkId;
                }
                Object object = method.invoke(network);
                if (object instanceof Integer) {
                    networkId = (Integer) object;
                }
            } catch (NoSuchMethodError | NoSuchMethodException | InvocationTargetException |
                     IllegalAccessException error) {
                Log.e(LOG_TAG, "NoSuchMethodError");
            }
            return networkId;
        }
    };

    /**
     * Registers a default Callback to listen to network status.
     *
     * @param callbackKey the key of the callback.
     */
    public void registerDefaultNetConnCallback(long callbackKey) {
        NetworkCallbackImpl callback = observer.get(callbackKey);
        if (callback != null) {
            return;
        }
        callback = new NetworkCallbackImpl();
        connectivity.registerDefaultNetworkCallback(callback);
        observer.put(callbackKey, callback);
        Network network = connectivity.getActiveNetwork();
        if (network == null) {
            callback.onUnavailable();
        }
    }

    /**
     * Registers a specified Callback to listen to network status.
     *
     * @param callbackKey The key of the callback.
     * @param types The types of the network.
     */
    public void registerNetConnCallback(long callbackKey, int[] types) {
        NetworkCallbackImpl callback = observer.get(callbackKey);
        if (callback != null) {
            return;
        }
        callback = new NetworkCallbackImpl();
        NetworkRequest.Builder builder = new NetworkRequest.Builder();
        for (int type : types) {
            builder = builder.addTransportType(type);
        }

        NetworkRequest request = builder.build();
        connectivity.registerNetworkCallback(request, callback);
        observer.put(callbackKey, callback);
        Network network = connectivity.getActiveNetwork();
        if (network == null) {
            callback.onUnavailable();
        } else {
            NetworkCapabilities cap = connectivity.getNetworkCapabilities(network);
            boolean isUnavailable = true;
            for (int type : types) {
                if (cap.hasTransport(type)) {
                    isUnavailable = false;
                }
            }
            if (isUnavailable) {
                callback.onUnavailable();
            }
        }
    }

    /**
     * unregister the callback.
     *
     * @param callbackKey the key of the callback
     */
    public void unregisterNetConnCallback(long callbackKey) {
        NetworkCallbackImpl callback = observer.get(callbackKey);
        if (callback != null) {
            observer.remove(callbackKey);
        }
        connectivity.unregisterNetworkCallback(callback);
    }

    /**
     * Return whether the data network is currently active.
     *
     * @return boolean True if it has a active defualt network, or false.
     */
    public boolean isDefaultNetworkActive() {
        boolean isActive = true;
        Network network = connectivity.getActiveNetwork();
        if (network != null) {
            isActive = true;
        } else {
            isActive = false;
        }
        return isActive;
    }

    /**
     * Return the callback key of the callback.
     *
     * @param callback the object of the callback
     * @return long The callback key of the callback.
     */
    public long getCallbackKey(NetworkCallbackImpl callback) {
        for (long key : observer.keySet()) {
            NetworkCallbackImpl value = observer.get(key);
            if (value == callback) {
                Log.i("LOG_TAG", "getCallbackKey successfull!");
                return key;
            }
        }
        return 0;
    }

    /**
     * Init NetConnClientPlugin jni.
     */
    protected native void nativeInit();

    /**
     * Notify network is available
     *
     * @param callbackKey The key of the callback
     * @param networkId The id of the network
     */
    protected native void nativeOnAvailable(long callbackKey, int networkId);

    /**
     * Notify network capability changes.
     *
     * @param networkCapabilities The capailities of the network.
     * @param callbackKey The key of the callback
     * @param networkId The id of the network
     */
    protected native void nativeOnCapabilitiesChanged(NetworkCapabilities networkCapabilities,
        long callbackKey, int networkId);

    /**
     * Notify the network that it can be lost
     *
     * @param callbackKey The key of the callback
     * @param networkId The id of the network
     */
    protected native void nativeOnLost(long callbackKey, int networkId);

    /**
     * Notify network is unavailable
     *
     * @param network Identifies a Network
     * @param callbackKey The key of the callback
     */
    protected native void nativeOnUnavailable(long callbackKey);
}
