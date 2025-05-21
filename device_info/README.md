# DeviceInfo 接口实现说明

|   接口名   |  Android |  IOS   |
| ---------- | ----- |  -------  |
|  |  实现  |  实现    |
|DeviceTypes|<br/>TYPE_DEFAULT = 'default'<br/>TYPE_PHONE = 'phone'<br/>TYPE_TABLET = 'tablet'<br/>TYPE_2IN1 = '2in1'<br/>TYPE_TV = 'tv'<br/>TYPE_WEARABLE = 'wearable'<br/>TYPE_CAR = 'car'|<br/>TYPE_DEFAULT = 'default'<br/>TYPE_PHONE = 'phone'<br/>TYPE_TABLET = 'tablet'<br/>TYPE_2IN1 = '2in1'<br/>TYPE_TV = 'tv'<br/>TYPE_WEARABLE = 'wearable'<br/>TYPE_CAR = 'car'|
|GetDeviceType|根据应用类型或屏幕尺寸获取|device.userInterfaceIdiom|
|GetManufacture|Build.MANUFACTURER|"Apple"|
|GetBrand|Build.BRAND|"Apple"|
|GetMarketName|Build.PRODUCT / ro.product.name|device.name|
|GetProductSeries|Build.DEVICE / ro.product.device|systemInfo.version|
|GetProductModel|Build.MODEL / ro.product.model|device.model|
|GetProductModelAlias|Build.MODEL / ro.product.model|device.model|
|GetSoftwareModel|Build.MODEL / ro.product.model|device.model|
|GetHardwareModel|Build.BOARD / ro.product.board|device.model|
|GetHardwareProfile|default|default|
|GetBootLoaderVersion|Build.BOOTLOADER|default 无，暂时显示device.systemName|
|GetAbiList|Build.SUPPORTED_ABIS|NXGetLocalArchInfo|
|GetSecurityPatchTag|自定义 const.ohos.version.security_patch|自定义 const.ohos.version.security_patch|
|GetDisplayVersion|Build.VERSION.RELEASE|device.systemVersion|
|GetIncrementalVersion|Build.VERSION.INCREMENTAL|default  无，暂时显示device.systemName|
|GetOsReleaseType|const.ohos.releasetype||
|GetOSFullName|const.ohos.fullname||
|GetMajorVersion|majorVersion||
|GetSeniorVersion|seniorVersion||
|GetFeatureVersion|featureVersion||
|GetBuildVersion|persist.product.firstapiversion|default|
|GetSdkApiVersion|const.ohos.apiversion||
|GetFirstApiVersion|const.product.firstapiversion|default|
|GetVersionId|        String output = String.format("%s/%s/%s/%s/%s/%s/%s/%s/%s/%s", <br> GetDeviceType(), <br> GetManufacture(), <br> GetBrand(), <br> GetProductSeries(), <br> GetOSFullName(), <br> GetProductModel(), <br> GetSoftwareModel(), <br> GetSdkApiVersion(), <br> GetIncrementalVersion(), <br> GetBuildType());||
|GetBuildType|编译生成||
|GetBuildUser|编译生成||
|GetBuildHost|编译生成||
|GetBuildTime|编译生成||
|GetBuildRootHash|const.ohos.buildroothash||