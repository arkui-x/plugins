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
import { AsyncCallback } from './basic';

/**
 * upload and download
 * 
 * @import request from '@ohos.request';
 * @permission ohos.permission.INTERNET
 */
declare namespace request {

  /**
   * Indicates that files to be downloaded already exist, and that the download session cannot overwrite the existing files.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_FILE_ALREADY_EXISTS: number;

  /**
   * Indicates that a file operation fails.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_FILE_ERROR: number;

  /**
   * Indicates that the HTTP transmission fails.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_HTTP_DATA_ERROR: number;

  /**
   * Indicates insufficient storage space.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_INSUFFICIENT_SPACE: number;

  /**
   * Indicates an HTTP code that cannot be identified.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_UNHANDLED_HTTP_CODE: number;

  /**
   * Indicates an undefined error.
   * @syscap SystemCapability.MiscServices.Download
   * @since 7
   * @permission ohos.permission.INTERNET
   */
  const ERROR_UNKNOWN: number;

  /**
   * Starts a download session.
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   * @param config download config
   * @param callback Indicate the callback function to receive DownloadTask.
   * @permission ohos.permission.INTERNET
   * @return -
   * @FAModelOnly
   */
  function download(config: DownloadConfig, callback: AsyncCallback<DownloadTask>): void;

  /**
   * Starts a download session.
   * @syscap SystemCapability.MiscServices.Download
   * @since 6
   * @param config download config
   * @permission ohos.permission.INTERNET
   * @return -
   * @FAModelOnly
   */
  function download(config: DownloadConfig): Promise<DownloadTask>;


  /**
   * Starts a upload session.
   * @syscap SystemCapability.MiscServices.Upload
   * @since 6
   * @param config upload config
   * @param callback Indicate the callback function to receive UploadTask.
   * @permission ohos.permission.INTERNET
   * @return -
   * @FAModelOnly
   */
  function upload(config: UploadConfig, callback: AsyncCallback<UploadTask>): void;


  /**
   * Starts a upload session.
   * @syscap SystemCapability.MiscServices.Upload
   * @since 6
   * @param config upload config
   * @permission ohos.permission.INTERNET
   * @return -
   * @FAModelOnly
   */
  function upload(config: UploadConfig): Promise<UploadTask>;

  interface TaskState {
    /**
     * Upload file path.
     *
     * @since 9
     * @permission ohos.permission.INTERNET
     */
    path: string;
    /**
     * Upload task return value.
     *
     * @since 9
     * @permission ohos.permission.INTERNET
     */
    responseCode: number;
    /**
     * Upload task information.
     *
     * @since 9
     * @permission ohos.permission.INTERNET
     */
    message: string;

  }

  /**
   * DownloadConfig data Structure
   *
   * @name DownloadConfig
   * @since 6
   * @syscap SystemCapability.MiscServices.Download
   * @permission ohos.permission.INTERNET
   */
  interface DownloadConfig {
    /**
     * Resource address.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    url: string;
    /**
     * Adds an HTTP or HTTPS header to be included with the download request.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    header?: Object;
    /**
     * Sets the description of a download session.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    description?: string;
    /**
     * Sets the path for downloads.
     *
     * @since 7
     * @permission ohos.permission.INTERNET
     */
    filePath?: string;
    /**
     * Sets a download session title.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    title?: string;
  }

  interface DownloadTask {
    /**
     * Called when the current download session is in process.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @param type progress Indicates the download task progress.
     * @param callback The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    on(type: 'progress', callback: (receivedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current download session is in process.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @param type progress Indicates the download task progress.
     * @param callback The callback function for the download progress change event
     *        receivedSize the length of downloaded data, in bytes
     *        totalSize he length of data expected to be downloaded, in bytes.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    off(type: 'progress', callback?: (receivedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current download session complete、pause or remove.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @param type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param callback The callback function for the download complete、pause or remove change event.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    on(type: 'complete' | 'remove', callback: () => void): void;

    /**
     * Called when the current download session complete、pause or remove.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @param type Indicates the download session event type
     *        complete: download task completed,
     *        pause: download task stopped,
     *        remove: download task deleted.
     * @param callback The callback function for the download complete、pause or remove change event.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    off(type: 'complete' | 'remove', callback?: () => void): void;

    /**
     * Called when the current download session fails.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @param type Indicates the download session type, fail: download task has failed.
     * @param callback The callback function for the download fail change event
     *        err The error code for download task.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    on(type: 'fail', callback: (err: number) => void): void;

    /**
     * Called when the current download session fails.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @param type Indicates the download session type, fail: download task has failed.
     * @param callback Indicate the callback function to receive err.
     *        err The error code for download task.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    off(type: 'fail', callback?: (err: number) => void): void;

    /**
     * Deletes a download session and the downloaded files.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @param callback Indicates asynchronous invoking Result.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    remove(callback: AsyncCallback<boolean>): void;

    /**
     * Deletes a download session and the downloaded files.
     * @syscap SystemCapability.MiscServices.Download
     * @since 6
     * @permission ohos.permission.INTERNET
     * @return -
     */
    remove(): Promise<boolean>;
  }

  /**
   * File data Structure
   *
   * @name File
   * @since 6
   * @syscap SystemCapability.MiscServices.Download
   * @permission ohos.permission.INTERNET
   */
  interface File {
    /**
     * When multipart is submitted, the file name in the request header.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    filename: string;
    /**
     * When multipart is submitted, the name of the form item. The default is file.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    name: string;
    /**
     * The local storage path of the file (please refer to the storage directory definition for path usage).
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    uri: string;
    /**
     * The content type of the file is obtained by default according to the suffix of the file name or path.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    type: string;
  }

  /**
   * RequestData data Structure
   *
   * @name RequestData
   * @since 6
   * @syscap SystemCapability.MiscServices.Download
   * @permission ohos.permission.INTERNET
   */
  interface RequestData {
    /**
     * Represents the name of the form element.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    name: string;
    /**
     * Represents the value of the form element.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    value: string;
  }

  /**
   * UploadConfig data Structure
   *
   * @name UploadConfig
   * @since 6
   * @syscap SystemCapability.MiscServices.Upload
   * @permission ohos.permission.INTERNET
   */
  interface UploadConfig {
    /**
     * Resource address.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    url: string;
    /**
     * Adds an HTTP or HTTPS header to be included with the upload request.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    header: Object;
    /**
     * Request method: POST, PUT. The default POST.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    method: string;
    /**
     * A list of files to be uploaded. Please use multipart/form-data to submit.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    files: Array<File>;
    /**
     * The requested form data.
     *
     * @since 6
     * @permission ohos.permission.INTERNET
     */
    data: Array<RequestData>;
  }

  interface UploadTask {
    /**
     * Called when the current upload session is in process.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @param type progress Indicates the upload task progress.
     * @param callback The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    on(type: 'progress', callback: (uploadedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current upload session is in process.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @param type progress Indicates the upload task progress.
     * @param callback The callback function for the upload progress change event
     *        uploadedSize The length of uploaded data, in bytes
     *        totalSize The length of data expected to be uploaded, in bytes.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    off(type: 'progress', callback?: (uploadedSize: number, totalSize: number) => void): void;

    /**
     * Called when the current download session fails.
     * @syscap SystemCapability.MiscServices.Download
     * @since 7
     * @param type Indicates the download session type, fail: download task has failed.
     * @param callback The callback function for the download fail change event
     *        err The error code for download task.
     * @permission ohos.permission.INTERNET
     * @return -
     */
     on(type: 'fail' | 'complete', callback: (err: Array<TaskState>) => void): void;

     /**
      * Called when the current download session fails.
      * @syscap SystemCapability.MiscServices.Download
      * @since 7
      * @param type Indicates the download session type, fail: download task has failed.
      * @param callback Indicate the callback function to receive err.
      *        err The error code for download task.
      * @permission ohos.permission.INTERNET
      * @return -
      */
     off(type: 'fail' | 'complete', callback?: (err: Array<TaskState>) => void): void;

    /**
     * Deletes a upload session.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @param callback Indicates asynchronous invoking Result.
     * @permission ohos.permission.INTERNET
     * @return -
     */
    remove(callback: AsyncCallback<boolean>): void;

    /**
     * Deletes a upload session.
     * @syscap SystemCapability.MiscServices.Upload
     * @since 6
     * @permission ohos.permission.INTERNET
     * @return -
     */
    remove(): Promise<boolean>;
  }
}

export default request;

