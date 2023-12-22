/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit Basic Services Kit
 */

import { AsyncCallback } from './@ohos.base';
import { Callback } from './@ohos.base';
import BaseContext from './application/BaseContext';

/**
 * upload and download
 *
 * @namespace request
 * @permission ohos.permission.INTERNET
 * @since 6
 */
/**
 * upload and download
 *
 * @namespace request
 * @since 10
 */
declare namespace request {
  /**
   * Error code 201 - the permissions check fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 201 - the permissions check fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_PERMISSION: number;
  /**
   * Error code 401 - the parameters check fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 401 - the parameters check fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_PARAMCHECK: number;
  /**
   * Error code 801 - call unsupported api.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 801 - call unsupported api.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_UNSUPPORTED: number;
  /**
   * Error code 13400001 - file operation error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 13400001 - file operation error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_FILEIO: number;
  /**
   * Error code 13400002 - bad file path.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 13400002 - bad file path.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_FILEPATH: number;
  /**
   * Error code 13400003 - task service ability error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 13400003 - task service ability error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_SERVICE: number;
  /**
   * Error code 13499999 - others error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 13499999 - others error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const EXCEPTION_OTHERS: number;

  /**
   * Code 0x00000001 - Bit flag indicating download is allowed when using the cellular network.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   */
  /**
   * Code 0x00000001 - Bit flag indicating download is allowed when using the cellular network.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const NETWORK_MOBILE: number;

  /**
   * Code 0x00010000 - Bit flag indicating download is allowed when using the WLAN.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   */
  /**
   * Code 0x00010000 - Bit flag indicating download is allowed when using the WLAN.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const NETWORK_WIFI: number;

  /**
   * Error code 0 - Indicates that the download cannot be resumed for network reasons.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 0 - Indicates that the download cannot be resumed for network reasons.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_CANNOT_RESUME: number;

  /**
   * Error code 1 - Indicates that no storage device, such as an SD card, is found.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 1 - Indicates that no storage device, such as an SD card, is found.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_DEVICE_NOT_FOUND: number;

  /**
   * Error code 2 - Indicates that files to be downloaded already exist, and that the download session cannot overwrite the existing files.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 2 - Indicates that files to be downloaded already exist, and that the download session cannot overwrite the existing files.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_FILE_ALREADY_EXISTS: number;

  /**
   * Error code 3 - Indicates that a file operation fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 3 - Indicates that a file operation fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_FILE_ERROR: number;

  /**
   * Error code 4 - Indicates that the HTTP transmission fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 4 - Indicates that the HTTP transmission fails.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_HTTP_DATA_ERROR: number;

  /**
   * Error code 5 - Indicates insufficient storage space.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 5 - Indicates insufficient storage space.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_INSUFFICIENT_SPACE: number;

  /**
   * Error code 6 - Indicates an error caused by too many network redirections.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 6 - Indicates an error caused by too many network redirections.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_TOO_MANY_REDIRECTS: number;

  /**
   * Error code 7 - Indicates an HTTP code that cannot be identified.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 7 - Indicates an HTTP code that cannot be identified.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_UNHANDLED_HTTP_CODE: number;

  /**
   * Error code 8 - Indicates an undefined error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Error code 8 - Indicates an undefined error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_UNKNOWN: number;

  /**
   * Error code 9 - Indicates network offline.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 9 - Indicates network offline.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_OFFLINE: number;

  /**
   * Error code 10 - Indicates network type configuration error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Error code 10 - Indicates network type configuration error.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const ERROR_UNSUPPORTED_NETWORK_TYPE: number;

  /**
   * Paused code 0 - Indicates that the download is paused and waiting for a WLAN connection,
   * because the file size exceeds the maximum allowed for a session using the cellular network.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Paused code 0 - Indicates that the download is paused and waiting for a WLAN connection,
   * because the file size exceeds the maximum allowed for a session using the cellular network.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const PAUSED_QUEUED_FOR_WIFI: number;

  /**
   * Paused code 1 - Indicates that the download is paused due to a network problem, for example, network disconnection.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Paused code 1 - Indicates that the download is paused due to a network problem, for example, network disconnection.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const PAUSED_WAITING_FOR_NETWORK: number;

  /**
   * Paused code 2 - Indicates that a network error occurs, and the download session will be retried.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Paused code 2 - Indicates that a network error occurs, and the download session will be retried.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const PAUSED_WAITING_TO_RETRY: number;

  /**
   * Paused code 3 - Indicates that the download is paused due to the user.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Paused code 3 - Indicates that the download is paused due to the user.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const PAUSED_BY_USER: number;

  /**
   * Paused code 4 - Indicates that the download is paused for some reasons.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Paused code 4 - Indicates that the download is paused for some reasons.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const PAUSED_UNKNOWN: number;

  /**
   * Session status code 0 - Indicates that the download session is completed.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Session status code 0 - Indicates that the download session is completed.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const SESSION_SUCCESSFUL: number;

  /**
   * Session status code 1 - Indicates that the download session is in progress.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Session status code 1 - Indicates that the download session is in progress.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const SESSION_RUNNING: number;

  /**
   * Session status code 2 - Indicates that the download session is being scheduled.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Session status code 2 - Indicates that the download session is being scheduled.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const SESSION_PENDING: number;

  /**
   * Session status code 3 - Indicates that the download session has been paused.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Session status code 3 - Indicates that the download session has been paused.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const SESSION_PAUSED: number;

  /**
   * Session status code 4 - Indicates that the download session has failed and will not be retried.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   */
  /**
   * Session status code 4 - Indicates that the download session has failed and will not be retried.
   *
   * @permission ohos.permission.INTERNET
   * @constant
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  const SESSION_FAILED: number;

  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { DownloadConfig } config Download config
   * @param { AsyncCallback<DownloadTask> } callback Indicate the callback function to receive DownloadTask.
   * @syscap SystemCapability.MiscServices.Download
   * @FAModelOnly
   * @since 6
   * @deprecated since 9
   * @useinstead ohos.request.downloadFile
   */
  function download(config: DownloadConfig, callback: AsyncCallback<DownloadTask>): void;

  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { DownloadConfig } config Download config
   * @param { AsyncCallback<DownloadTask> } callback Indicate the callback function to receive DownloadTask.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400001 - file operation error
   * @throws { BusinessError } 13400002 - bad file path
   * @throws { BusinessError } 13400003 - task service ability error
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { DownloadConfig } config Download config
   * @param { AsyncCallback<DownloadTask> } callback Indicate the callback function to receive DownloadTask.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400001 - file operation error
   * @throws { BusinessError } 13400002 - bad file path
   * @throws { BusinessError } 13400003 - task service ability error
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  function downloadFile(context: BaseContext, config: DownloadConfig, callback: AsyncCallback<DownloadTask>): void;

  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { DownloadConfig } config Download config
   * @returns { Promise<DownloadTask> } the promise returned by the function.
   * @syscap SystemCapability.MiscServices.Download
   * @FAModelOnly
   * @since 6
   * @deprecated since 9
   * @useinstead ohos.request.downloadFile
   */
  function download(config: DownloadConfig): Promise<DownloadTask>;

  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { DownloadConfig } config Download config
   * @returns { Promise<DownloadTask> } the promise returned by the function.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400001 - file operation error
   * @throws { BusinessError } 13400002 - bad file path
   * @throws { BusinessError } 13400003 - task service ability error
   * @syscap SystemCapability.MiscServices.Download
   * @since 9
   */
  /**
   * Starts a download task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { DownloadConfig } config Download config
   * @returns { Promise<DownloadTask> } the promise returned by the function.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400001 - file operation error
   * @throws { BusinessError } 13400002 - bad file path
   * @throws { BusinessError } 13400003 - task service ability error
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  function downloadFile(context: BaseContext, config: DownloadConfig): Promise<DownloadTask>;

  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { UploadConfig } config Upload config
   * @param { AsyncCallback<UploadTask> } callback Indicate the callback function to receive UploadTask.
   * @syscap SystemCapability.MiscServices.Upload
   * @FAModelOnly
   * @since 6
   * @deprecated since 9
   * @useinstead ohos.request.uploadFile
   */
  function upload(config: UploadConfig, callback: AsyncCallback<UploadTask>): void;

  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { UploadConfig } config Upload config
   * @param { AsyncCallback<UploadTask> } callback Indicate the callback function to receive UploadTask.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400002 - bad file path
   * @syscap SystemCapability.MiscServices.Upload
   * @since 9
   */
  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { UploadConfig } config Upload config
   * @param { AsyncCallback<UploadTask> } callback Indicate the callback function to receive UploadTask.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400002 - bad file path
   * @syscap SystemCapability.MiscServices.Upload
   * @crossplatform
   * @since 10
   */
  function uploadFile(context: BaseContext, config: UploadConfig, callback: AsyncCallback<UploadTask>): void;

  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { UploadConfig } config Upload config
   * @returns { Promise<UploadTask> } the promise returned by the function.
   * @syscap SystemCapability.MiscServices.Upload
   * @FAModelOnly
   * @since 6
   * @deprecated since 9
   * @useinstead ohos.request.uploadFile
   */
  function upload(config: UploadConfig): Promise<UploadTask>;

  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { UploadConfig } config Upload config
   * @returns { Promise<UploadTask> } the promise returned by the function.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400002 - bad file path
   * @syscap SystemCapability.MiscServices.Upload
   * @since 9
   */
  /**
   * Starts a upload task.
   *
   * @permission ohos.permission.INTERNET
   * @param { BaseContext } context Indicates the application BaseContext.
   * @param { UploadConfig } config Upload config
   * @returns { Promise<UploadTask> } the promise returned by the function.
   * @throws { BusinessError } 201 - the permissions check fails
   * @throws { BusinessError } 401 - the parameters check fails
   * @throws { BusinessError } 13400002 - bad file path
   * @syscap SystemCapability.MiscServices.Upload
   * @crossplatform
   * @since 10
   */
  function uploadFile(context: BaseContext, config: UploadConfig): Promise<UploadTask>;

  /**
   * DownloadConfig data Structure
   *
   * @interface DownloadConfig
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   * @name DownloadConfig
   */
  /**
   * DownloadConfig data Structure
   *
   * @typedef DownloadConfig
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   * @name DownloadConfig
   */
  interface DownloadConfig {
    /**
     * Resource address.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Resource address.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    url: string;
    /**
     * Adds an HTTP or HTTPS header to be included with the download request.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Adds an HTTP or HTTPS header to be included with the download request.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?Object }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    header?: Object;
    /**
     * Allows download under a metered connection.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Allows download under a metered connection.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?boolean }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    enableMetered?: boolean;
    /**
     * Allows download in a roaming network.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Allows download in a roaming network.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?boolean }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    enableRoaming?: boolean;
    /**
     * Sets the description of a download session.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Sets the description of a download session.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    description?: string;
    /**
     * Sets the network type allowed for download.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Sets the network type allowed for download.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    networkType?: number;
    /**
     * Sets the path for downloads.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * Sets the path for downloads.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    filePath?: string;
    /**
     * Sets a download session title.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Sets a download session title.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    title?: string;
    /**
     * Allow download background task notifications.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Allow download background task notifications.
     *
     * @permission ohos.permission.INTERNET
     * @type { ?boolean }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    background?: boolean;
  }

  /**
   * DownloadInfo data Structure
   *
   * @interface DownloadInfo
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @name DownloadInfo
   */
  /**
   * DownloadInfo data Structure
   *
   * @typedef DownloadInfo
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  interface DownloadInfo {
    /**
     * the description of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the description of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    description: string;
    /**
     * the real-time downloads size (in bytes).
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the real-time downloads size (in bytes).
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    downloadedBytes: number;
    /**
     * the ID of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the ID of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    downloadId: number;
    /**
     * a download failure cause, which can be any DownloadSession.ERROR_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * a download failure cause, which can be any DownloadSession.ERROR_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    failedReason: number;
    /**
     * the name of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the name of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    fileName: string;
    /**
     * the URI of a stored file.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the URI of a stored file.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    filePath: string;
    /**
     * the reason why a session is paused, which can be any DownloadSession.PAUSED_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the reason why a session is paused, which can be any DownloadSession.PAUSED_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    pausedReason: number;
    /**
     * the download status code, which can be any DownloadSession.SESSION_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the download status code, which can be any DownloadSession.SESSION_* constant.
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    status: number;
    /**
     * the URI of files to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the URI of files to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    targetURI: string;
    /**
     * the title of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the title of a file to be downloaded.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    downloadTitle: string;
    /**
     * the total size of files to be downloaded (in bytes).
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * the total size of files to be downloaded (in bytes).
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    downloadTotalBytes: number;
  }

  /**
   * Download task interface
   *
   * @interface DownloadTask
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   */
  /**
   * Download task interface
   *
   * @typedef DownloadTask
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  interface DownloadTask {
    /**
     * Called when the current download session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the download task progress.
     * @param { function } callback
     *        The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Called when the current download session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the download task progress.
     * @param { function } callback
     *        The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    on(type: 'progress', callback: (receivedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current download session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the download task progress.
     * @param { function } [callback]
     *        The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Called when the current download session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the download task progress.
     * @param { function } [callback]
     *        The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    off(type: 'progress', callback?: (receivedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current download session complete pause or remove.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'pause' | 'remove' } type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param { function } callback The callback function for the download complete pause or remove change event.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * Called when the current download session complete pause or remove.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'pause' | 'remove' } type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param { function } callback The callback function for the download complete pause or remove change event.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    on(type: 'complete' | 'pause' | 'remove', callback: () => void): void;

    /**
     * Called when the current download session complete pause or remove.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'pause' | 'remove' } type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param { function } [callback] The callback function for the download complete pause or remove change event.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * Called when the current download session complete pause or remove.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'pause' | 'remove' } type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param { function } [callback] The callback function for the download complete pause or remove change event.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    off(type: 'complete' | 'pause' | 'remove', callback?: () => void): void;

    /**
     * Called when the current download session fails.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'fail' } type Indicates the download session type, fail: download task has failed.
     * @param { function } callback The callback function for the download fail change event
     *        err The error code for download task.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * Called when the current download session fails.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'fail' } type Indicates the download session type, fail: download task has failed.
     * @param { function } callback The callback function for the download fail change event
     *        err The error code for download task.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    on(type: 'fail', callback: (err: number) => void): void;

    /**
     * Called when the current download session fails.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'fail' } type Indicates the download session type, fail: download task has failed.
     * @param { function } [callback] Indicate the callback function to receive err.
     *        err The error code for download task.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     */
    /**
     * Called when the current download session fails.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'fail' } type Indicates the download session type, fail: download task has failed.
     * @param { function } [callback] Indicate the callback function to receive err.
     *        err The error code for download task.
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    off(type: 'fail', callback?: (err: number) => void): void;

    /**
     * Deletes a download session and the downloaded files.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback Indicates asynchronous invoking Result.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @deprecated since 9
     * @useinstead ohos.request.delete
     */
    remove(callback: AsyncCallback<boolean>): void;

    /**
     * Deletes a download session and the downloaded files.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @deprecated since 9
     * @useinstead ohos.request.delete
     */
    remove(): Promise<boolean>;

    /**
     * Pause a download session.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<void> } callback Indicates asynchronous invoking Result.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.suspend
     */
    pause(callback: AsyncCallback<void>): void;

    /**
     * Pause a download session.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<void> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.suspend
     */
    pause(): Promise<void>;

    /**
     * Resume a paused download session.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<void> } callback Indicates asynchronous invoking Result.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.restore
     */
    resume(callback: AsyncCallback<void>): void;

    /**
     * Resume a paused download session.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<void> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.restore
     */
    resume(): Promise<void>;

    /**
     * Queries download information of a session, which is defined in DownloadSession.DownloadInfo.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<DownloadInfo> } callback Indicate the callback function to receive download info.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.getTaskInfo
     */
    query(callback: AsyncCallback<DownloadInfo>): void;

    /**
     * Queries download information of a session, which is defined in DownloadSession.DownloadInfo.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<DownloadInfo> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.getTaskInfo
     */
    query(): Promise<DownloadInfo>;

    /**
     * Queries the MIME type of the download file.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<string> } callback Indicate the callback function to receive download file MIME type.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.getTaskMimeType
     */
    queryMimeType(callback: AsyncCallback<string>): void;

    /**
     * Queries the MIME type of the download file.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<string> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @deprecated since 9
     * @useinstead ohos.request.getTaskMimeType
     */
    queryMimeType(): Promise<string>;

    /**
     * Delete the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Delete the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    delete(callback: AsyncCallback<boolean>): void;

    /**
     * Delete the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Delete the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    delete(): Promise<boolean>;

    /**
     * Suspend the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Suspend the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    suspend(callback: AsyncCallback<boolean>): void;

    /**
     * Suspend the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Suspend the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    suspend(): Promise<boolean>;

    /**
     * Restore the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Restore the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    restore(callback: AsyncCallback<boolean>): void;

    /**
     * Restore the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Restore the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    restore(): Promise<boolean>;

    /**
     * Get the download task info
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<DownloadInfo> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Get the download task info
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<DownloadInfo> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    getTaskInfo(callback: AsyncCallback<DownloadInfo>): void;

    /**
     * Get the download task info
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<DownloadInfo> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Get the download task info
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<DownloadInfo> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    getTaskInfo(): Promise<DownloadInfo>;

    /**
     * Get mimetype of the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<string> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Get mimetype of the download task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<string> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    getTaskMimeType(callback: AsyncCallback<string>): void;

    /**
     * Get mimetype of the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<string> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @since 9
     */
    /**
     * Get mimetype of the download task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<string> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    getTaskMimeType(): Promise<string>;
  }

  /**
   * File data Structure
   *
   * @interface File
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   * @name File
   */
  /**
   * File data Structure
   *
   * @typedef File
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  interface File {
    /**
     * When multipart is submitted, the file name in the request header.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * When multipart is submitted, the file name in the request header.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    filename: string;
    /**
     * When multipart is submitted, the name of the form item. The default is file.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * When multipart is submitted, the name of the form item. The default is file.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    name: string;
    /**
     * The local storage path of the file (please refer to the storage directory definition for path usage).
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * The local storage path of the file (please refer to the storage directory definition for path usage).
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    uri: string;
    /**
     * The content type of the file is obtained by default according to the suffix of the file name or path.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * The content type of the file is obtained by default according to the suffix of the file name or path.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    type: string;
  }

  /**
   * RequestData data Structure
   *
   * @interface RequestData
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   * @name RequestData
   */
  /**
   * RequestData data Structure
   *
   * @typedef RequestData
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  interface RequestData {
    /**
     * Represents the name of the form element.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Represents the name of the form element.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    name: string;
    /**
     * Represents the value of the form element.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     */
    /**
     * Represents the value of the form element.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Download
     * @crossplatform
     * @since 10
     */
    value: string;
  }

  /**
   * UploadConfig data Structure
   *
   * @interface UploadConfig
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Upload
   * @since 6
   * @name UploadConfig
   */
  /**
   * UploadConfig data Structure
   *
   * @typedef UploadConfig
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Upload
   * @crossplatform
   * @since 10
   */
  interface UploadConfig {
    /**
     * Resource address.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * Resource address.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    url: string;
    /**
     * Adds an HTTP or HTTPS header to be included with the upload request.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * Adds an HTTP or HTTPS header to be included with the upload request.
     *
     * @permission ohos.permission.INTERNET
     * @type { Object }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    header: Object;
    /**
     * Request method: POST, PUT. The default POST.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * Request method: POST, PUT. The default POST.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    method: string;
    /**
     * The index of paths for a task.
     * Usually used for a continuous job.
     * The default is 0.
     *
     * @type { ?number }
     * @syscap SystemCapability.MiscServices.Upload
     * @since 11
     */
    index?: number;
    /**
     * The start point of a file.
     * Usually used for a continuous job.
     * It will start read at the point in upload.
     * The default is 0.
     *
     * @type { ?number }
     * @syscap SystemCapability.MiscServices.Upload
     * @since 11
     */
    begins?: number;
    /**
     * The end point of a file.
     * Usually used for a continuous job.
     * It will end read at the point in upload.
     * The default is -1 indicating the end of the data for upload.
     *
     * @type { ?number }
     * @syscap SystemCapability.MiscServices.Upload
     * @since 11
     */
    ends?: number;
    /**
     * A list of files to be uploaded. Please use multipart/form-data to submit.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * A list of files to be uploaded. Please use multipart/form-data to submit.
     *
     * @permission ohos.permission.INTERNET
     * @type { Array<File> }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    files: Array<File>;
    /**
     * The requested form data.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * The requested form data.
     *
     * @permission ohos.permission.INTERNET
     * @type { Array<RequestData> }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    data: Array<RequestData>;
  }

  /**
   * TaskState data Structure
   *
   * @interface TaskState
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Upload
   * @since 9
   * @name TaskState
   */
  /**
   * TaskState data Structure
   *
   * @typedef TaskState
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Upload
   * @crossplatform
   * @since 10
   */
  interface TaskState {
    /**
     * Upload file path.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Upload file path.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    path: string;
    /**
     * Upload task return value.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Upload task return value.
     *
     * @permission ohos.permission.INTERNET
     * @type { number }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    responseCode: number;
    /**
     * Upload task information.
     *
     * @permission ohos.permission.INTERNET
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Upload task information.
     *
     * @permission ohos.permission.INTERNET
     * @type { string }
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    message: string;
  }

  /**
   * Upload task interface
   *
   * @interface UploadTask
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   */
  /**
   * Upload task interface
   *
   * @typedef UploadTask
   * @permission ohos.permission.INTERNET
   * @syscap SystemCapability.MiscServices.Download
   * @crossplatform
   * @since 10
   */
  interface UploadTask {
    /**
     * Called when the current upload session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the upload task progress.
     * @param { function } callback
     *        The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * Called when the current upload session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the upload task progress.
     * @param { function } callback
     *        The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    on(type: 'progress', callback: (uploadedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current upload session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the upload task progress.
     * @param { function } [callback]
     *        The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     */
    /**
     * Called when the current upload session is in process.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'progress' } type progress Indicates the upload task progress.
     * @param { function } [callback]
     *        The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    off(type: 'progress', callback?: (uploadedSize: number, totalSize: number) => void): void;

    /**
     * Called when the header of the current upload session has been received.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'headerReceive' } type headerReceive Indicates the upload task headed receive.
     * @param { function } callback The callback function for the HTTP Response Header event
     *        header HTTP Response Header returned by the developer server.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 7
     */
    /**
     * Called when the header of the current upload session has been received.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'headerReceive' } type headerReceive Indicates the upload task headed receive.
     * @param { function } callback The callback function for the HTTP Response Header event
     *        header HTTP Response Header returned by the developer server.
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    on(type: 'headerReceive', callback: (header: object) => void): void;

    /**
     * Called when the header of the current upload session has been received.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'headerReceive' } type headerReceive Indicates the upload task headed receive.
     * @param { function } [callback] The callback function for the HTTP Response Header event
     *        header HTTP Response Header returned by the developer server.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 7
     */
    /**
     * Called when the header of the current upload session has been received.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'headerReceive' } type headerReceive Indicates the upload task headed receive.
     * @param { function } [callback] The callback function for the HTTP Response Header event
     *        header HTTP Response Header returned by the developer server.
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    off(type: 'headerReceive', callback?: (header: object) => void): void;

    /**
     * Called when the current upload session complete or fail.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'fail' } type Indicates the upload session event type
     *        complete: upload task completed
     *        fail: upload task failed
     * @param { Callback<Array<TaskState>> } callback The callback function for the upload complete or fail change event.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Called when the current upload session complete or fail.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'fail' } type Indicates the upload session event type
     *        complete: upload task completed
     *        fail: upload task failed
     * @param { Callback<Array<TaskState>> } callback The callback function for the upload complete or fail change event.
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    on(type: 'complete' | 'fail', callback: Callback<Array<TaskState>>): void;

    /**
     * Called when the current upload session complete or fail.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'fail' } type Indicates the upload session event type
     *        complete: upload task completed
     *         fail: upload task failed
     * @param { Callback<Array<TaskState>> } [callback]
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Called when the current upload session complete or fail.
     *
     * @permission ohos.permission.INTERNET
     * @param { 'complete' | 'fail' } type Indicates the upload session event type
     *        complete: upload task completed
     *         fail: upload task failed
     * @param { Callback<Array<TaskState>> } [callback]
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    off(type: 'complete' | 'fail', callback?: Callback<Array<TaskState>>): void;

    /**
     * Deletes a upload session.
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback Indicates asynchronous invoking Result.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @deprecated since 9
     * @useinstead ohos.request.delete
     */
    remove(callback: AsyncCallback<boolean>): void;

    /**
     * Deletes a upload session.
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @deprecated since 9
     * @useinstead ohos.request.delete
     */
    remove(): Promise<boolean>;

    /**
     * Delete the upload task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Delete the upload task
     *
     * @permission ohos.permission.INTERNET
     * @param { AsyncCallback<boolean> } callback
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    delete(callback: AsyncCallback<boolean>): void;

    /**
     * Delete the upload task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Upload
     * @since 9
     */
    /**
     * Delete the upload task
     *
     * @permission ohos.permission.INTERNET
     * @returns { Promise<boolean> } the promise returned by the function.
     * @throws { BusinessError } 201 - the permissions check fails
     * @throws { BusinessError } 401 - the parameters check fails
     * @syscap SystemCapability.MiscServices.Upload
     * @crossplatform
     * @since 10
     */
    delete(): Promise<boolean>;
  }

  /**
   * The request agent api.
   * Supports "background" and "frontend" tasks as while.
   * Though "background" and "frontend" here do not the same with process's concept.
   * All tasks will be executed at request manager service and recorded.
   * Background tasks is for concurrent transfer, such as caching videos for a later play.
   * Frontend tasks is for instant transfer, such as submitting forms for a consumption bill.
   * Background tasks use notification to tell user tasks' status information.
   * Frontend tasks use callback to tell caller tasks' status information.
   * Background has some automatically restore mechanism.
   * Frontend tasks controlled by caller.
   * Uses `multipart/form-data` in client request for upload.
   * A `Content-Disposition: attachment; filename=<filename>` response from server leads to download.
   * More details, please see the architecture documents of the request subsystem.
   *
   * @namespace agent
   * @syscap SystemCapability.Request.FileTransferAgent
   * @since 10
   */
  /**
   * The request agent api.
   * Supports "background" and "frontend" tasks as while.
   * Though "background" and "frontend" here do not the same with process's concept.
   * All tasks will be executed at request manager service and recorded.
   * Background tasks is for concurrent transfer, such as caching videos for a later play.
   * Frontend tasks is for instant transfer, such as submitting forms for a consumption bill.
   * Background tasks use notification to tell user tasks' status information.
   * Frontend tasks use callback to tell caller tasks' status information.
   * Background has some automatically restore mechanism.
   * Frontend tasks controlled by caller.
   * Uses `multipart/form-data` in client request for upload.
   * A `Content-Disposition: attachment; filename=<filename>` response from server leads to download.
   * More details, please see the architecture documents of the request subsystem.
   * Only front-end mode is supported in cross-platform scenarios.
   *
   * @namespace agent
   * @syscap SystemCapability.Request.FileTransferAgent
   * @crossplatform
   * @atomicservice
   * @since 11
   */
  namespace agent {
    /**
     * The action options.
     *
     * @enum { number } Action
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The action options.
     *
     * @enum { number } Action
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    enum Action {
      /**
       * Indicates download task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates download task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      DOWNLOAD,
      /**
       * Indicates upload task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates upload task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      UPLOAD
    }

    /**
     * The mode options.
     *
     * @enum { number } Mode
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The mode options.
     *
     * @enum { number } Mode
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    enum Mode {
      /**
       * Indicates background task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates background task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @atomicservice
       * @since 11
       */
      BACKGROUND,
      /**
       * Indicates foreground task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates foreground task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      FOREGROUND
    }

    /**
     * The network options.
     *
     * @enum { number } Network
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The network options.
     *
     * @enum { number } Network
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    enum Network {
      /**
       * Indicates no restriction on network type.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates no restriction on network type.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      ANY,
      /**
       * Indicates Wi-Fi only.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates Wi-Fi only.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      WIFI,
      /**
       * Indicates cellular only.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates cellular only.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      CELLULAR
    }

    /**
     * Broadcast events for the request.
     *
     * @enum { string } BroadcastEvent
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 11
     */
    enum BroadcastEvent {
      /**
       * Completion event for the task.
       * The code in the commonEventData can only be "0x40"(COMPLETE) or "0x41"(FAILED), same as "State".
       * The data in the commonEventData contains the id of the task.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      COMPLETE = 'ohos.request.event.COMPLETE'
    }

    /**
     * The file information for a form item.
     *
     * @typedef FileSpec
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The file information for a form item.
     *
     * @typedef FileSpec
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    interface FileSpec {
      /**
       * A relative path string, like "./xxx/yyy/zzz.html", "xxx/yyy/zzz.html", in the caller's cache directory.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * A relative path string, like "./xxx/yyy/zzz.html", "xxx/yyy/zzz.html", in the caller's cache directory.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      path: string;
      /**
       * The MIME type of the file.
       * The default is obtained by the suffix of the filename.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The MIME type of the file.
       * The default is obtained by the suffix of the filename.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      mimeType?: string;
      /**
       * The filename, the default is obtained by path.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The filename, the default is obtained by path.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      filename?: string;
      /**
       * The extras for the file information.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The extras for the file information.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      extras?: object;
    }

    /**
     * The form item information for a task.
     *
     * @typedef FormItem
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The form item information for a task.
     *
     * @typedef FormItem
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    interface FormItem {
      /**
       * The item's name.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The item's name.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      name: string;
      /**
       * The item's value.
       *
       * @type { string | FileSpec | Array<FileSpec> }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The item's value.
       *
       * @type { string | FileSpec | Array<FileSpec> }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      value: string | FileSpec | Array<FileSpec>;
    }

    /**
     * The configurations for a task.
     * Using a flexible configuration for clear upload and download functions.
     * If without emphasis, an option is for any task.
     *
     * @typedef Config
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The configurations for a task.
     * Using a flexible configuration for clear upload and download functions.
     * If without emphasis, an option is for any task.
     *
     * @typedef Config
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    interface Config {
      /**
       * The task action, upload or download.
       *
       * @type { Action }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task action, upload or download.
       *
       * @type { Action }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      action: Action;
      /**
       * The Universal Resource Locator for a task.
       * The maximum length is 2048 characters.
       * Using raw `url` option, even url parameters in it.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The Universal Resource Locator for a task.
       * The maximum length is 2048 characters.
       * Using raw `url` option, even url parameters in it.
       *
       * @type { string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      url: string;
      /**
       * The title for a task, give a meaningful title please.
       * The maximum length is 256 characters.
       * The default is the same with its action.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The title for a task, give a meaningful title please.
       * The maximum length is 256 characters.
       * The default is the same with its action.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      title?: string;
      /**
       * The details for a task.
       * The maximum length is 1024 characters.
       * The default is empty string.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The details for a task.
       * The maximum length is 1024 characters.
       * The default is empty string.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      description?: string;
      /**
       * Indicates task's mode.
       * The default is background.
       * For frontend task, it has callbacks.
       * For background task, it has notifications and fallback.
       *
       * @type { ?Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates task's mode.
       * The default is background.
       * For frontend task, it has callbacks.
       * For background task, it has notifications and fallback.
       * The cross-platform default is FOREGROUND.
       *
       * @type { ?Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */

      mode?: Mode;
      /**
       * The solution choice when path already exists during download.
       * Currently support:
       * true, rewrite the existed file.
       * false, go to fail.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The solution choice when path already exists during download.
       * Currently support:
       * true, rewrite the existed file.
       * false, go to fail.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      overwrite?: boolean;
      /**
       * The HTTP standard method for upload or download: GET/POST/PUT.
       * Case insensitive.
       * For upload, use PUT/POST, the default is PUT.
       * For download, use GET/POST, the default is GET.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The HTTP standard method for upload or download: GET/POST/PUT.
       * Case insensitive.
       * For upload, use PUT/POST, the default is PUT.
       * For download, use GET/POST, the default is GET.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      method?: string;
      /**
       * The HTTP headers.
       * For upload request, the `Content-Type` is forced to `multipart/form-data`.
       * For download request, the default `Content-Type` is `application/json`.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The HTTP headers.
       * For upload request, the `Content-Type` is forced to `multipart/form-data`.
       * For download request, the default `Content-Type` is `application/json`.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      headers?: object;
      /**
       * The arguments, it can be any text, uses json usually.
       * For download, it can be raw string, the default is empty string.
       * For upload, it can be form items, the default is a empty form.
       * there must be one `FileSpec` item at least or will be a parameter error.
       *
       * @type { ?(string | Array<FormItem>) }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The arguments, it can be any text, uses json usually.
       * For download, it can be raw string, the default is empty string.
       * For upload, it can be form items, the default is a empty form.
       * there must be one `FileSpec` item at least or will be a parameter error.
       *
       * @type { ?(string | Array<FormItem>) }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      data?: string | Array<FormItem>;
      /**
       * The path to save the downloaded file, the default is "./".
       * Currently support:
       * 1: relative path, like "./xxx/yyy/zzz.html", "xxx/yyy/zzz.html", under caller's cache folder.
       * 2: uri path, like "datashare://bundle/xxx/yyy/zzz.html", the data provider must allow the caller's access.
       *
       * @type { ?string }
       * @default ./
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The path to save the downloaded file, the default is "./".
       * Currently support:
       * 1: relative path, like "./xxx/yyy/zzz.html", "xxx/yyy/zzz.html", under caller's cache folder.
       * 2: uri path, like "datashare://bundle/xxx/yyy/zzz.html", the data provider must allow the caller's access.
       *
       * @type { ?string }
       * @default ./
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      saveas?: string;
      /**
       * The network.
       *
       * @type { ?Network }
       * @default Network.ANY
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The network.
       *
       * @type { ?Network }
       * @default Network.ANY
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      network?: Network;
      /**
       * Allows work in metered network or not.
       * The default is false.
       *
       * @type { ?boolean }
       * @default false
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Allows work in metered network or not.
       * The default is false.
       *
       * @type { ?boolean }
       * @default false
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      metered?: boolean;
      /**
       * Allows work in roaming network or not.
       * The default is true.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Allows work in roaming network or not.
       * The default is true.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      roaming?: boolean;
      /**
       * Enable automatic retry or not for the background task.
       * The frontend task is always fast-fail.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Enable automatic retry or not for the background task.
       * The frontend task is always fast-fail.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @atomicservice
       * @since 11
       */
      retry?: boolean;
      /**
       * Allows redirect or not.
       * The default is yes.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Allows redirect or not.
       * The default is yes.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      redirect?: boolean;
      /**
       * The index of paths for a task.
       * Usually used for a continuous job.
       * The default is 0.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The index of paths for a task.
       * Usually used for a continuous job.
       * The default is 0.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      index?: number;
      /**
       * The start point of a file.
       * Usually used for a continuous job.
       * It will set the "Range" header in download.
       * It will start read at the point in upload.
       * The default is 0.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The start point of a file.
       * Usually used for a continuous job.
       * It will set the "Range" header in download.
       * It will start read at the point in upload.
       * The default is 0.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      begins?: number;
      /**
       * The end point of a file.
       * Usually used for a continuous job.
       * It will set The "Range" header in download.
       * It will end read at the point in upload.
       * The default is -1 indicating the end of the data for upload or download.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The end point of a file.
       * Usually used for a continuous job.
       * It will set The "Range" header in download.
       * It will end read at the point in upload.
       * The default is -1 indicating the end of the data for upload or download.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      ends?: number;
      /**
       * The policy of the progress notification for background task.
       * If false: only completed or failed notification, the default.
       * If true, emits every progress, completed or failed notifications.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The policy of the progress notification for background task.
       * If false: only completed or failed notification, the default.
       * If true, emits every progress, completed or failed notifications.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @atomicservice
       * @since 11
       */
      gauge?: boolean;
      /**
       * Breaks when fail to fetch filesize before upload/download or not.
       * Uses filesize for a precise gauge.
       * The default is not, set size as -1 indicating the case.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Breaks when fail to fetch filesize before upload/download or not.
       * Uses filesize for a precise gauge.
       * The default is not, set size as -1 indicating the case.
       *
       * @type { ?boolean }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      precise?: boolean;
      /**
       * For in-application layer isolation.
       * If given:
       *   the minimum is 8 bytes.
       *   the maximum is 2048 bytes.
       * Creates a task with token, then must provide it during normal query.
       * So saves the token carefully, it can not be retrieved by query.
       * Or leave it empty.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * For in-application layer isolation.
       * If given:
       *   the minimum is 8 bytes.
       *   the maximum is 2048 bytes.
       * Creates a task with token, then must provide it during normal query.
       * So saves the token carefully, it can not be retrieved by query.
       * Or leave it empty.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      token?: string;
      /**
       * The priority of this task.
       * Front-end tasks have higher priority than back-end tasks.
       * In tasks of the same mode, the smaller the number, the higher the priority.
       * The default is 0.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      priority?: number;
      /**
       * The extras for the configuration.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The extras for the configuration.
       *
       * @type { ?object }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      extras?: object;
    }

    /**
     * Indicate the current state of the task.
     *
     * @enum { number } State
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Indicate the current state of the task.
     *
     * @enum { number } State
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    enum State {
      /**
       * Indicates a task created by `new Task(Config)`.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a task created by `new Task(Config)`.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      INITIALIZED = 0x00,
      /**
       * Indicates a task lack of resources or conditions to run or retry.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a task lack of resources or conditions to run or retry.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      WAITING = 0x10,
      /**
       * Indicates a task in processing now.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a task in processing now.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      RUNNING = 0x20,
      /**
       * Indicates a task failed once at least and in processing again now.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a task failed once at least and in processing again now.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      RETRYING = 0x21,
      /**
       * Indicates a paused task which tends to be resumed for continuous work.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a paused task which tends to be resumed for continuous work.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      PAUSED = 0x30,
      /**
       * Indicates a stopped task which must be started again.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a stopped task which must be started again.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      STOPPED = 0x31,
      /**
       * Indicates a completed task which finish its data transfer.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a completed task which finish its data transfer.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      COMPLETED = 0x40,
      /**
       * Indicates a failed task which interrupted by some error.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a failed task which interrupted by some error.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      FAILED = 0x41,
      /**
       * Indicates a removed task which can not be processed again.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates a removed task which can not be processed again.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      REMOVED = 0x50
    }

    /**
     * The progress data structure.
     * Upload allows multiple files per upload task.
     * Only one file in a download task.
     * So using a unified data structure for progress.
     * Generally:
     * 1: sum(sizes) is total files size of the task.
     * 2: float(processed)/sizes[counter] is the progress for the current processing file.
     * 3: float(sum(sizes[:index])+processed)/sum(sizes) is the summary progress for a task.
     * If fetch file size in failure, the size of the file in sizes will be set as -1.
     *
     * @typedef Progress
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The progress data structure.
     * Upload allows multiple files per upload task.
     * Only one file in a download task.
     * So using a unified data structure for progress.
     * Generally:
     * 1: sum(sizes) is total files size of the task.
     * 2: float(processed)/sizes[counter] is the progress for the current processing file.
     * 3: float(sum(sizes[:index])+processed)/sum(sizes) is the summary progress for a task.
     * If fetch file size in failure, the size of the file in sizes will be set as -1.
     *
     * @typedef Progress
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    interface Progress {
      /**
       * The current state of the task.
       *
       * @type { State }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The current state of the task.
       *
       * @type { State }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly state: State;
      /**
       * The current processing file index in a task.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The current processing file index in a task.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly index: number;
      /**
       * The processed data size for the current file in a task.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The processed data size for the current file in a task.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly processed: number;
      /**
       * The sizes of files in a task.
       *
       * @type { Array<number> }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The sizes of files in a task.
       *
       * @type { Array<number> }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly sizes: Array<number>;
      /**
       * The extras for an interaction.
       * Such as headers and body of response from server.
       * But when the Content-Disposition header responded, the body will be into the uri of its attachment only, the body here is empty.
       * {"headers": {"key": v}, "body": "contents"}.
       *
       * @type { ?object }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The extras for an interaction.
       * Such as headers and body of response from server.
       * But when the Content-Disposition header responded, the body will be into the uri of its attachment only, the body here is empty.
       * {"headers": {"key": v}, "body": "contents"}.
       *
       * @type { ?object }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly extras?: object;
    }

    /**
     * Indicates the reason for the failure.
     *
     * @enum { number }
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Indicates the reason for the failure.
     *
     * @enum { number }
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    enum Faults {
      /**
       * Indicates others failure.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates others failure.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      OTHERS = 0xFF,
      /**
       * Indicates network disconnection.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates network disconnection.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      DISCONNECTED = 0x00,
      /**
       * Indicates task timeout.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates task timeout.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      TIMEOUT = 0x10,
      /**
       * Indicates protocol error, such as 5xx response from server.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates protocol error, such as 5xx response from server.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      PROTOCOL = 0x20,
      /**
       * Indicates filesystem io error, such as open/seek/read/write/close.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Indicates filesystem io error, such as open/seek/read/write/close.
       *
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      FSIO = 0x40
    }

    /**
     * The filter data structure.
     * Used for search, given fields works as **LOGICAL AND**.
     * Invalid value may cause a parameter error.
     *
     * @typedef Filter
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The filter data structure.
     * Used for search, given fields works as **LOGICAL AND**.
     * Invalid value may cause a parameter error.
     *
     * @typedef Filter
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    interface Filter {
      /**
       * Specify the package name of an application.
       * Only for advanced search, common search will be fixed to the caller.
       * A "*" means any bundle.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @systemapi Hide this for inner system use.
       * @since 10
       */
      bundle?: string;
      /**
       * Specify the end Unix timestamp.
       * The default is the moment of calling.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify the end Unix timestamp.
       * The default is the moment of calling.
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      before?: number;
      /**
       * Specify the start Unix timestamp.
       * The default is "`before` - 24 hours".
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify the start Unix timestamp.
       * The default is "`before` - 24 hours".
       *
       * @type { ?number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      after?: number;
      /**
       * Specify the state of tasks.
       * The default is any state.
       *
       * @type { ?State }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify the state of tasks.
       * The default is any state.
       *
       * @type { ?State }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      state?: State;
      /**
       * Specify the action of tasks, "upload" or "download", case insensitive.
       * The default is upload and download.
       *
       * @type { ?Action }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify the action of tasks, "upload" or "download", case insensitive.
       * The default is upload and download.
       *
       * @type { ?Action }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      action?: Action;
      /**
       * Specify task's mode.
       * The default is frontend and background.
       *
       * @type { ?Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify task's mode.
       * The default is frontend.
       *
       * @type { ?Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      mode?: Mode;
    }

    /**
     * The task information data structure for query results.
     * Provides common query and advanced query, visible range of fields is different.
     *
     * @typedef TaskInfo
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The task information data structure for query results.
     * Provides common query and advanced query, visible range of fields is different.
     *
     * @typedef TaskInfo
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    interface TaskInfo {
      /**
       * The UID of an application.
       * For system query only.
       *
       * @type { ?string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @systemapi Hide this for inner system use.
       * @since 10
       */
      readonly uid?: string;
      /**
       * The bundle name.
       * For system query only.
       *
       * @type { ?string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @systemapi Hide this for inner system use.
       * @since 10
       */
      readonly bundle?: string;
      /**
       * The path to save the downloaded file.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The path to save the downloaded file.
       *
       * @type { ?string }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly saveas?: string;
      /**
       * The url of a task.
       * For `${ show }` and `${ touch }`.
       * It is empty string in `${ query }`.
       *
       * @type { ?string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The url of a task.
       * For `${ show }` and `${ touch }`.
       *
       * @type { ?string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly url?: string;
      /**
       * The arguments.
       * For `${ show }` and `${ touch }`.
       * It is empty string in `${ query }`.
       *
       * @type { ?(string | Array<FormItem>) }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The arguments.
       * For `${ show }` and `${ touch }`.
       *
       * @type { ?(string | Array<FormItem>) }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly data?: string | Array<FormItem>;
      /**
       * The task id.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task id.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly tid: string;
      /**
       * The task title.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task title.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly title: string;
      /**
       * The task details.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task details.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly description: string;
      /**
       * The task action.
       *
       * @type { Action }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task action.
       *
       * @type { Action }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly action: Action;
      /**
       * Specify task mode.
       * The default is frontend and background.
       *
       * @type { Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Specify task mode.
       * The default is frontend.
       *
       * @type { Mode }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly mode: Mode;
      /**
       * The priority of this task.
       * Front-end tasks have higher priority than back-end tasks.
       * In tasks of the same mode, the smaller the number, the higher the priority.
       * The default is 0.
       *
       * @type { number }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      readonly priority: number;
      /**
       * The MIME type in the configuration of the task.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The MIME type in the configuration of the task.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly mimeType: string;
      /**
       * An instance of `Progress` for a task.
       *
       * @type { Progress }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * An instance of `Progress` for a task.
       *
       * @type { Progress }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly progress: Progress;
      /**
       * The progress notification policy of a background task.
       *
       * @type { boolean }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      readonly gauge: boolean;
      /**
       * The creating date and time of a task in Unix timestamp.
       * It is generated by system of current device.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The creating date and time of a task in Unix timestamp.
       * It is generated by system of current device.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly ctime: number;
      /**
       * The modified date and time of a task in Unix timestamp.
       * It is generated by system of current device.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The modified date and time of a task in Unix timestamp.
       * It is generated by system of current device.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly mtime: number;
      /**
       * The retry switch of a task.
       * Just for background, frontend always disabled.
       *
       * @type { boolean }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      readonly retry: boolean;
      /**
       * The tried times of a task.
       *
       * @type { number }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      readonly tries: number;
      /**
       * The faults case of a task.
       *
       * @type { Faults }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The faults case of a task.
       *
       * @type { Faults }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly faults: Faults;
      /**
       * The reason of a waiting/failed/stopped/paused task.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The reason of a waiting/failed/stopped/paused task.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly reason: string;
      /**
       * The extras of a task.
       * For background, the last response from server.
       * For frontend, nothing now.
       *
       * @type { ?object }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The extras of a task.
       * For frontend, nothing now.
       *
       * @type { ?object }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @since 11
       */
      readonly extras?: object;
    }

    /**
     * The task entry.
     * New task' status is "initialized" and enqueue.
     * Can `start` a initialized task.
     * Can `pause` a waiting/running/retrying background task.
     * Can `resume` a paused background task.
     * Can `stop` a running/waiting/retrying task and dequeue it.
     *
     * @typedef Task
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * The task entry.
     * New task' status is "initialized" and enqueue.
     * Can `start` a initialized task.
     * Can `pause` a waiting/running/retrying background task.
     * Can `resume` a paused background task.
     * Can `stop` a running/waiting/retrying task and dequeue it.
     *
     * @typedef Task
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    interface Task {
      /**
       * The task id, unique on system.
       * Generated automatically by system.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The task id, unique on system.
       * Generated automatically by system.
       *
       * @type { string }
       * @readonly
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      readonly tid: string;
      /**
       * The configurations for the task.
       *
       * @type { Config }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * The configurations for the task.
       *
       * @type { Config }
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      config: Config;
      /**
       * Enable the specified callback for a frontend task.
       *
       * @param { 'progress' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Enables the specified callback.
       *
       * @param { 'progress' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      on(event: 'progress', callback: (progress: Progress) => void): void;
      /**
       * Disable the specified callback for a frontend task.
       *
       * @param { 'progress' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Disables the specified callback.
       *
       * @param { 'progress' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      off(event: 'progress', callback?: (progress: Progress) => void): void;
      /**
       * Enable the specified callback for a frontend task.
       *
       * @param { 'completed' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Enables the specified callback.
       *
       * @param { 'completed' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      on(event: 'completed', callback: (progress: Progress) => void): void;
      /**
       * Disable the specified callback for a frontend task.
       *
       * @param { 'completed' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Disables the specified callback.
       *
       * @param { 'completed' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      off(event: 'completed', callback?: (progress: Progress) => void): void;
      /**
       * Enable the specified callback for a frontend task.
       *
       * @param { 'failed' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Enables the specified callback.
       *
       * @param { 'failed' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      on(event: 'failed', callback: (progress: Progress) => void): void;
      /**
       * Disable the specified callback for a frontend task.
       *
       * @param { 'failed' } event event types.
       * @param { function } callback callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Disables the specified callback.
       *
       * @param { 'failed' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      off(event: 'failed', callback?: (progress: Progress) => void): void;
      /**
       * Enables the specified callback.
       *
       * @param { 'pause' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      on(event: 'pause', callback: (progress: Progress) => void): void;
      /**
       * Disables the specified callback.
       *
       * @param { 'pause' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      off(event: 'pause', callback?: (progress: Progress) => void): void;
      /**
       * Enables the specified callback.
       *
       * @param { 'resume' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      on(event: 'resume', callback: (progress: Progress) => void): void;
      /**
       * Disables the specified callback.
       *
       * @param { 'resume' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      off(event: 'resume', callback?: (progress: Progress) => void): void;
      /**
       * Enables the specified callback.
       *
       * @param { 'remove' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      on(event: 'remove', callback: (progress: Progress) => void): void;
      /**
       * Disables the specified callback.
       *
       * @param { 'remove' } event - event types.
       * @param { function } callback - callback function with a `Progress` argument.
       * @throws { BusinessError } 401 - Parameter error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      off(event: 'remove', callback?: (progress: Progress) => void): void;
      /**
       * Starts the task.
       *
       * @permission ohos.permission.INTERNET
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Starts the task.
       *
       * @permission ohos.permission.INTERNET
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      start(callback: AsyncCallback<void>): void;
      /**
       * Starts the task.
       *
       * @permission ohos.permission.INTERNET
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Starts the task.
       *
       * @permission ohos.permission.INTERNET
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      start(): Promise<void>;
      /**
       * Pauses the background task.
       *
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Pauses the task.
       *
       * @param { AsyncCallback<void> } callback - callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      pause(callback: AsyncCallback<void>): void;
      /**
       * Pauses the background task.
       *
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Pauses the task.
       *
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      pause(): Promise<void>;
      /**
       * Resumes the background task.
       *
       * @permission ohos.permission.INTERNET
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Resumes the task.
       *
       * @permission ohos.permission.INTERNET
       * @param { AsyncCallback<void> } callback - callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      resume(callback: AsyncCallback<void>): void;
      /**
       * Resumes the background task.
       *
       * @permission ohos.permission.INTERNET
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900005 - task mode error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Resumes the task.
       *
       * @permission ohos.permission.INTERNET
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 201 - Permission denied.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 11
       */
      resume(): Promise<void>;
      /**
       * Stops the task.
       *
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Stops the task.
       *
       * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating the calling result.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      stop(callback: AsyncCallback<void>): void;
      /**
       * Stops the task.
       *
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @since 10
       */
      /**
       * Stops the task.
       *
       * @returns { Promise<void> } the promise returned by the function.
       * @throws { BusinessError } 13400003 - task service ability error.
       * @throws { BusinessError } 21900007 - task state error.
       * @syscap SystemCapability.Request.FileTransferAgent
       * @crossplatform
       * @atomicservice
       * @since 11
       */
      stop(): Promise<void>;
    }

    /**
     * Creates a task for upload or download and enqueue it.
     * Only foreground application can create the frontend task.
     * It can deal only one frontend task at a time.
     * A in processing frontend task will be forced to stop when its application had switched to background.
     * A new frontend task will interrupt a existed in processing frontend task.
     * The background task is highly recommended.
     *
     * @permission ohos.permission.INTERNET
     * @param { BaseContext } context context of the caller.
     * @param { Config } config configurations for a task.
     * @param { AsyncCallback<Task> } callback indicate the callback function to receive Task.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400001 - file operation error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900004 - application task queue full error.
     * @throws { BusinessError } 21900005 - task mode error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Creates a task for upload or download and enqueue it.
     * When an application enters the background, the frontend tasks associated.
     * with it will gradually be paused until the application returns to the foreground. 
     *
     * @permission ohos.permission.INTERNET
     * @param { BaseContext } context context of the caller.
     * @param { Config } config configurations for a task.
     * @param { AsyncCallback<Task> } callback indicate the callback function to receive Task.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400001 - file operation error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900004 - application task queue full error.
     * @throws { BusinessError } 21900005 - task mode error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    function create(context: BaseContext, config: Config, callback: AsyncCallback<Task>): void;

    /**
     * Creates a task for upload or download and enqueue it.
     * Only foreground application can create the frontend task.
     * It can deal only one frontend task at a time.
     * A in processing frontend task will be forced to stop when its application had switched to background.
     * A new frontend task will interrupt a existed in processing frontend task.
     * The background task is highly recommended.
     *
     * @permission ohos.permission.INTERNET
     * @param { BaseContext } context context of the caller.
     * @param { Config } config configurations for a task.
     * @returns { Promise<Task> } the promise returned by the function.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400001 - file operation error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900004 - application task queue full error.
     * @throws { BusinessError } 21900005 - task mode error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Creates a task for upload or download and enqueue it.
     * When an application enters the background, the frontend tasks associated.
     * with it will gradually be paused until the application returns to the foreground.
     *
     * @permission ohos.permission.INTERNET
     * @param { BaseContext } context context of the caller.
     * @param { Config } config configurations for a task.
     * @returns { Promise<Task> } the promise returned by the function.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400001 - file operation error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900004 - application task queue full error.
     * @throws { BusinessError } 21900005 - task mode error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    function create(context: BaseContext, config: Config): Promise<Task>;

    /**
     * Gets the task with the specified id.
     *
     * @param { BaseContext } context - context of the caller.
     * @param { string } id - the id of the task.
     * @param { string } token - the token of the task.
     * @returns { Promise<Task> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 11
     */
    function getTask(context: BaseContext, id: string, token?: string): Promise<Task>;

    /**
     * Removes specified task belongs to the caller.
     * The task will be forced to stop if in processing.
     *
     * @param { string } id the task id.
     * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating success or not.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Removes specified task belongs to the caller.
     * The task will be forced to stop if in processing.
     *
     * @param { string } id the task id.
     * @param { AsyncCallback<void> } callback callback function with a boolean argument indicating success or not.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    function remove(id: string, callback: AsyncCallback<void>): void;

    /**
     * Removes specified task belongs to the caller.
     * The task will be forced to stop if in processing.
     *
     * @param { string } id the task id.
     * @returns { Promise<void> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Removes specified task belongs to the caller.
     * The task will be forced to stop if in processing.
     *
     * @param { string } id the task id.
     * @returns { Promise<void> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    function remove(id: string): Promise<void>;

    /**
     * Shows specified task details belongs to the caller.
     *
     * @param { string } id the task id.
     * @param { AsyncCallback<TaskInfo> } callback callback function with a `TaskInfo` argument for informations of the current task.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Shows specified task details belongs to the caller.
     *
     * @param { string } id the task id.
     * @param { AsyncCallback<TaskInfo> } callback callback function with a `TaskInfo` argument for informations of the current task.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function show(id: string, callback: AsyncCallback<TaskInfo>): void;

    /**
     * Shows specified task details belongs to the caller.
     *
     * @param { string } id the task id.
     * @returns { Promise<TaskInfo> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Shows specified task details belongs to the caller.
     *
     * @param { string } id the task id.
     * @returns { Promise<TaskInfo> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function show(id: string): Promise<TaskInfo>;

    /**
     * Touches specified task with token.
     *
     * @param { string } id the task id.
     * @param { string } token the in-application isolation key.
     * @param { AsyncCallback<TaskInfo> } callback callback function with a `TaskInfo` argument for informations of the current task.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Touches specified task with token.
     *
     * @param { string } id the task id.
     * @param { string } token the in-application isolation key.
     * @param { AsyncCallback<TaskInfo> } callback callback function with a `TaskInfo` argument for informations of the current task.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function touch(id: string, token: string, callback: AsyncCallback<TaskInfo>): void;

    /**
     * Touches specified task with token.
     *
     * @param { string } id the task id.
     * @param { string } token the in-application isolation key.
     * @returns { Promise<TaskInfo> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Touches specified task with token.
     *
     * @param { string } id the task id.
     * @param { string } token the in-application isolation key.
     * @returns { Promise<TaskInfo> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function touch(id: string, token: string): Promise<TaskInfo>;

    /**
     * Searches tasks, for system.
     *
     * @param { AsyncCallback<Array<string>> } callback callback function with a `Array<string>` argument contains task ids match filter.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Searches tasks, for system.
     *
     * @param { AsyncCallback<Array<string>> } callback callback function with a `Array<string>` argument contains task ids match filter.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function search(callback: AsyncCallback<Array<string>>): void;

    /**
     * Searches tasks, for system.
     *
     * @param { Filter } filter an instance of `Filter`.
     * @param { AsyncCallback<Array<string>> } callback callback function with a `Array<string>` argument contains task ids match filter.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Searches tasks, for system.
     *
     * @param { Filter } filter an instance of `Filter`.
     * @param { AsyncCallback<Array<string>> } callback callback function with a `Array<string>` argument contains task ids match filter.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 10
     */
    function search(filter: Filter, callback: AsyncCallback<Array<string>>): void;

    /**
     * Searches tasks, for system.
     *
     * @param { Filter } filter an instance of `Filter`.
     * @returns { Promise<Array<string>> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @since 10
     */
    /**
     * Searches tasks, for system.
     *
     * @param { Filter } filter an instance of `Filter`.
     * @returns { Promise<Array<string>> } the promise returned by the function.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @crossplatform
     * @since 11
     */
    function search(filter?: Filter): Promise<Array<string>>;

    /**
     * Queries specified task details.
     *
     * @permission ohos.permission.DOWNLOAD_SESSION_MANAGER or ohos.permission.UPLOAD_SESSION_MANAGER
     * @param { string } id the task id.
     * @param { AsyncCallback<TaskInfo> } callback callback function with a `TaskInfo` argument for informations of the current task.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 202 - permission verification failed, application which is not a system application uses system API.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @systemapi Hide this for inner system use.
     * @since 10
     */
    function query(id: string, callback: AsyncCallback<TaskInfo>): void;

    /**
     * Queries specified task details.
     *
     * @permission ohos.permission.DOWNLOAD_SESSION_MANAGER or ohos.permission.UPLOAD_SESSION_MANAGER
     * @param { string } id the task id.
     * @returns { Promise<TaskInfo> } the promise returned by the function.
     * @throws { BusinessError } 201 - permission denied.
     * @throws { BusinessError } 202 - permission verification failed, application which is not a system application uses system API.
     * @throws { BusinessError } 401 - parameter error.
     * @throws { BusinessError } 13400003 - task service ability error.
     * @throws { BusinessError } 21900006 - task not found error.
     * @syscap SystemCapability.Request.FileTransferAgent
     * @systemapi Hide this for inner system use.
     * @since 10
     */
    function query(id: string): Promise<TaskInfo>;
  }
}

export default request;
