# ArkUI-X Plug-ins

## Welcome
Thank you for following the ArkUI-X project. Your contribution to this project will be highly appreciated. If you want to submit plug-ins, go to the [Plugins](https://gitee.com/arkui-x/plugins) repository. Thank you again for your support.

### Introduction
The Plugin repository provides a series of plug-ins to help you build ArkUI-X applications efficiently.

### Plug-ins
- [Graphics](./introduction-to-plugin-related-libraries-en.md#graphics)
  - @ohos.display(display)
- [Media](./introduction-to-plugin-related-libraries-en.md#media)
  - @ohos.multimedia.image
- [File Management](./introduction-to-plugin-related-libraries-en.md#file-management)
  - @ohos.fileio
- [Resource Management](./introduction-to-plugin-related-libraries-en.md#resource-management)
  - @ohos.i18n
  - @ohos.resourceManager
- [Network Management](./introduction-to-plugin-related-libraries-en.md#network-management)
  - @ohos.net.http
  - @ohos.request
- [Basic System Capabilities](./introduction-to-plugin-related-libraries-en.md#basic-system-capabilities)
  - @ohos.hiTraceMeter
- [Device Management](./introduction-to-plugin-related-libraries-en.md#device-management)
  - @ohos.runningLock
- [Common Event and Notification](./introduction-to-plugin-related-libraries-en.md#common-event-and-notification)
  - @ohos.common_event
- [Ability Access Control](./introduction-to-plugin-related-libraries-en.md#ability-access-control)
  - @ohos.abilityAccessCtrl

## Directory Structure
```
plugins
|---ability_access_ctrl  # Ability Access Control
|---common_event         # Common event and notification
|---display              # Display
|---fileio               # File management
|---hilog                # HiLog
|---hitrace_meter        # Performance tracing
|---i18n                 # Internationalization (i18n)
|---interfaces           # Public APIs
|---multimedia_image     # Image Processing
|---ndk_raw_file         # rawfile
|---net                  # Network management
|---request              # Upload and download
|---resource_manager     # Resource management
|---running_lock         # Running lock
|---test                 # Test cases
|---test_plugin          # Sample plug-ins
```

## How to Develop
- [Android Plug-in Development](https://gitee.com/arkui-x/docs/blob/master/en/contribute/tutorial/how-to-use-napi-on-Android.md)
- [iOS Plug-in Development](https://gitee.com/arkui-x/docs/blob/master/en/contribute/tutorial/how-to-use-napi-on-iOS.md)
