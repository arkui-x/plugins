package ohos.ace.plugin.wifimanager;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.text.TextUtils;
import android.os.Build;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

public class WifiDeviceUtils {
    private static final String LOG_TAG = "WifiDeviceUtils";

    private Context mContext;
 
    private WifiManager mWifiManager;

    public WifiDeviceUtils(Context mContext) {
        this.mContext = mContext;
        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
    }

    /**
     * 获取链接WIFI的信息
     */
    public String getWifiInfo() {
        WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
        if (wifiInfo == null) {
            return "";
        }
        String ssid = wifiInfo.getSSID();
        if (!TextUtils.isEmpty(ssid) && ssid.contains("\"")) {
            ssid = ssid.replace("\"", "");
        }
        String bssid = wifiInfo.getBSSID();
        int networkId = wifiInfo.getNetworkId();
        int rssi = wifiInfo.getRssi();
        int linkSpeed = wifiInfo.getLinkSpeed();
        int frequency = wifiInfo.getFrequency();
        boolean isHidden = wifiInfo.getHiddenSSID();
        String macAddress = wifiInfo.getMacAddress();
        int ipAddress = wifiInfo.getIpAddress();
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("ssid", ssid);
            jsonObject.put("bssid", bssid);
            jsonObject.put("networkId", networkId);
            jsonObject.put("rssi", rssi);
            jsonObject.put("linkSpeed", linkSpeed);
            jsonObject.put("frequency", frequency);
            jsonObject.put("isHidden", isHidden);
            return jsonObject.toString();
        } catch (JSONException exception) {
            Log.e(LOG_TAG, "WifiDeviceUtils JSONException");
        }
        return "";
    }

    /**
     * 判断WiFi开关是否打开状态
     */
    public boolean getWifiActive() {
        if (mWifiManager == null) {
            Log.e(LOG_TAG, "WifiDeviceUtils mWifiManager is null");
            return false;
        }
        if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) { // 3
            return true;
        }
        return false;
    }
    
    /**
     * 获取WIFI链接状态
     */
    public boolean getIsConnected() {
        boolean isConnected = false;
        // 获取网络连接管理器
        ConnectivityManager connectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

        // 获取当前网络状态信息
        NetworkInfo info = connectivityManager.getActiveNetworkInfo();
        if (info != null && info.isAvailable()) {
            // 当NetworkInfo不为空且是可用的情况下，获取当前网络的Type状态
            // 根据NetworkInfo.getTypeName()判断当前网络
            String name = info.getTypeName();
            if (name.equals("WIFI")) {
                isConnected = true;
            } else {
                isConnected = false;
            }
        } else {
            isConnected = false;
        }
        return isConnected;
    }
}
