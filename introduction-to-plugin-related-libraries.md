# ArkUI-X项目跨平台插件相关库

## 图形图像
- @ohos.display(屏幕属性)

  | 描述文件(d.ts)     | 插件库                                     | 依赖库                        |
  | ------------------ | ------------------------------------------ | ----------------------------- |
  | @ohos.display.d.ts | libdisplay.so<br />ace_display_android.jar | libace_plugin_util_android.so |
  
    
## 媒体
- @ohos.meltimedia.image(图片处理)

  | 描述文件(d.ts)              | 插件库                | 依赖库                                                       |
  | --------------------------- | --------------------- | ------------------------------------------------------------ |
  | @ohos.multimedia.image.d.ts | libmultimediaimage.so | libexif.so<br />libimage.so<br />libimage_native.so<br />libimage_utils.so<br />libmultimedia_image.so<br />libpixelconvertadapter.so<br />libpluginmanager.so |
  
    
## 文件管理
- @ohos.fileio(文件管理)

  | 描述文件(d.ts)    | 插件库       | 依赖库                        |
  | ----------------- | ------------ | ----------------------------- |
  | @ohos.fileio.d.ts | libfileio.so | libace_plugin_util_android.so |
  

## 资源管理
- @ohos.i18n(国际化-i18n)

  | 描述文件(d.ts)  | 插件库                                      | 依赖库                                                       |
  | --------------- | ------------------------------------------- | ------------------------------------------------------------ |
  | @ohos.i18n.d.ts | libi18n.so<br />ace_i18n_plugin_android.jar | libhmicui18n.so<br />libhmicuuc.so<br />libace_plugin_util_android.so |
  

- @ohos.resourceManager(资源管理)

  | 描述文件(d.ts)             | 插件库                | 依赖库                                                      |
  | -------------------------- | --------------------- | ----------------------------------------------------------- |
  | @ohos.resourceManager.d.ts | libresourcemanager.so | libglobal_resmgr.so<br />libhmicui18n.so<br />libhmicuuc.so |
  
    
## 网络管理
- @ohos.net.http(数据请求)

  | 描述文件(d.ts)      | 插件库        | 依赖库                                                       |
  | ------------------- | ------------- | ------------------------------------------------------------ |
  | @ohos.net.http.d.ts | libnet_http.so | libace_plugin_util_android.so<br />libgettext.so<br />libgio.so<br />libglib.so<br />libglibpcre.so<br />libgmodule.so<br />libgobject.so<br />libjsoncpp.so<br />libffi.so |
  

- @ohos.request(上传下载)

  | 描述文件(d.ts)     | 插件库        | 依赖库                        |
  | ------------------ | ------------- | ----------------------------- |
  | @ohos.request.d.ts | librequest.so | libace_plugin_util_android.so |
  
    
## 系统基础能力
  
- @ohos.hiTraceMeter(性能打点)

  | 描述文件(d.ts)          | 插件库             | 依赖库 |
  | ----------------------- | ------------------ | ------ |
  | @ohos.hiTraceMeter.d.ts | libhitracemeter.so |        |
  
    
## 设备管理
  
- @ohos.runningLock(Runninglock锁)

  | 描述文件(d.ts)         | 插件库                                             | 依赖库                        |
  | ---------------------- | -------------------------------------------------- | ----------------------------- |
  | @ohos.runningLock.d.ts | librunninglock.so<br />ace_runninglock_android.jar | libace_plugin_util_android.so |


## 公共事件与通知
  
- @ohos.common_event(公共事件与通知)

  | 描述文件(d.ts)         | 插件库                                              | 依赖库                        |
  | ---------------------- | --------------------------------------------------- | ----------------------------- |
  | @ohos.commonEvent.d.ts | libcommonevent.so<br />ace_common_event_android.jar | libace_plugin_util_android.so |

## 程序访问控制
  
- @ohos.abilityAccessCtrl(程序访问控制)

  | 描述文件(d.ts)         | 插件库                                              | 依赖库                        |
  | ---------------------- | --------------------------------------------------- | ----------------------------- |
  | @ohos.abilityAccessCtrl.d.ts | libabilityaccessctrl.so<br />ace_ability_access_ctrl_android.jar | libace_plugin_util_android.so |
