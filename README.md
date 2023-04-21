# ArkUI-X项目跨平台插件

## 简介
【插件贡献温馨提示】感谢您关注ArkUI-X项目，如果您希望提交插件/Plugin，请提交到[Plugins仓](https://gitee.com/arkui-x/plugins)。再次感谢您的关注！

### 内容简介
为帮助开发者快速构建ArkUI-X项目，我们提供了一系列的插件，即Plugin。

### 插件列表
- [图形图像](./introduction-to-plugin-related-libraries.md#图形图像)
  - @ohos.display(屏幕属性)
- [媒体](./introduction-to-plugin-related-libraries.md#媒体)
  - @ohos.meltimedia.image(图片处理)
- [文件管理](./introduction-to-plugin-related-libraries.md#文件管理)
  - @ohos.fileio(文件管理)
- [资源管理](./introduction-to-plugin-related-libraries.md#资源管理)
  - @ohos.i18n(国际化-i18n)
  - @ohos.resourceManager(资源管理)
- [网络管理](./introduction-to-plugin-related-libraries.md#网络管理)
  - @ohos.net.http(数据请求)
  - @ohos.request(上传下载)
- [系统基础能力](./introduction-to-plugin-related-libraries.md#系统基础能力)
  - @ohos.hiTraceMeter(性能打点)
- [设备管理](./introduction-to-plugin-related-libraries.md#设备管理)
  - @ohos.runningLock(Runninglock锁)
- [公共事件与通知](./introduction-to-plugin-related-libraries.md#公共事件与通知)
  - @ohos.common_event(公共事件与通知)
- [程序访问控制](./introduction-to-plugin-related-libraries.md#程序访问控制)
  - @ohos.abilityAccessCtrl(程序访问控制)

## 目录
```
plugins
|---ability_access_ctrl  # 程序访问控制
|---common_event         # 公共事件与通知
|---display              # 屏幕属性
|---fileio               # 文件管理
|---hilog                # 日志打印
|---hitrace_meter        # 性能打点
|---i18n                 # 国际化-i18n
|---interfaces           # 公共接口
|---multimedia_image     # 图片处理
|---ndk_raw_file         # rawfile
|---net                  # 网络管理
|---request              # 上传下载
|---resource_manager     # 资源管理
|---running_lock         # Runninglock锁
|---test                 # 测试用例
|---test_plugin          # 示例插件
```

## 开发指导
- [Android平台插件开发](https://gitee.com/arkui-x/docs/blob/master/zh-cn/contribute/tutorial/how-to-use-napi-on-Android.md)
- [iOS平台插件开发](https://gitee.com/arkui-x/docs/blob/master/zh-cn/contribute/tutorial/how-to-use-napi-on-iOS.md)
