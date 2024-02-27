 package ohos.ace.plugin.wifimanager;

 public interface WifiBroadcastInterface {
    // 0-已断开
    int WIFI_STATE_DISCONNECT = 0;

    // 1-已连接
    int WIFI_STATE_LINK = 1;

    // 正在打开WiFi
    int WIFI_STATE_ENABLING = 2;

    // WiFi已打开
    int WIFI_STATE_ENABLED = 3;

    // 正在关闭WiFi
    int WIFI_STATE_DISABLING = 0;

    // WiFi已关闭
    int WIFI_STATE_DISABLED = 1;

    // WiFi状态未知
    int WIFI_STATE_UNKNOWN = 4;

    void wifiConnectState(int state);
    void wifiSwitchState(int state);
    
}