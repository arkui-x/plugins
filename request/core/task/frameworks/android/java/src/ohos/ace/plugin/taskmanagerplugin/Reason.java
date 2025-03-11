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

package ohos.ace.plugin.taskmanagerplugin;

/**
 * Reason of task stop
 *
 * @since 2023-05-31
 */
public interface Reason {
    /**
     * Task is stopped normally
     */
    String REASON_OK = "REASON_OK";

    /**
     * Task is stopped because it has been running for more than one month
     */
    int REASON_OK_CODE = 0;

    /**
     * Task is stopped because it has been waiting for network for more than one day
     */
    String TASK_SURVIVAL_ONE_MONTH = "TASK_SURVIVAL_ONE_MONTH";

    /**
     * Task is stopped because it has been waiting for network for more than one day
     */
    int TASK_SURVIVAL_ONE_MONTH_CODE = 1;

    /**
     * Task is stopped because it has been waiting for network for more than one day
     */
    String WAITTING_NETWORK_ONE_DAY = "WAITTING_NETWORK_ONE_DAY";

    /**
     * Task is stopped because it has been waiting for network for more than one day
     */
    int WAITTING_NETWORK_ONE_DAY_CODE = 2;

    /**
     * Task is stopped because a new front task is running
     */
    String STOPPED_NEW_FRONT_TASK = "STOPPED_NEW_FRONT_TASK";

    /**
     * Task is stopped because a new front task is running
     */
    int STOPPED_NEW_FRONT_TASK_CODE = 3;

    /**
     * Task is stopped because the number of running tasks has reached the limit
     */
    String RUNNING_TASK_MEET_LIMITS = "RUNNING_TASK_MEET_LIMITS";

    /**
     * Task is stopped because the number of running tasks has reached the limit
     */
    int RUNNING_TASK_MEET_LIMITS_CODE = 4;

    /**
     * Task is stopped because of user operation
     */
    String USER_OPERATION = "USER_OPERATION";

    /**
     * Task is stopped because of user operation
     */
    int USER_OPERATION_CODE = 5;

    /**
     * Task is stopped because the app is in the background or terminated
     */
    String APP_BACKGROUND_OR_TERMINATE = "APP_BACKGROUND_OR_TERMINATE";

    /**
     * Task is stopped because the app is in the background or terminated
     */
    int APP_BACKGROUND_OR_TERMINATE_CODE = 6;

    /**
     * Task is stopped because the network is offline
     */
    String NETWORK_OFFLINE = "NETWORK_OFFLINE";

    /**
     * Task is stopped because the network is offline
     */
    int NETWORK_OFFLINE_CODE = 7;

    /**
     * Task is stopped because the network type is not supported
     */
    String UNSUPPORTED_NETWORK_TYPE = "UNSUPPORTED_NETWORK_TYPE";

    /**
     * Task is stopped because the network type is not supported
     */
    int UNSUPPORTED_NETWORK_TYPE_CODE = 8;

    /**
     * Task is stopped because the client is not built
     */
    String BUILD_CLIENT_FAILED = "BUILD_CLIENT_FAILED";

    /**
     * Task is stopped because the client is not built
     */
    int BUILD_CLIENT_FAILED_CODE = 9;

    /**
     * Task is stopped because the request is not built
     */
    String BUILD_REQUEST_FAILED = "BUILD_REQUEST_FAILED";

    /**
     * Task is stopped because the request is not built
     */
    int BUILD_REQUEST_FAILED_CODE = 10;

    /**
     * Task is stopped because the file size is not obtained
     */
    String GET_FILESIZE_FAILED = "GET_FILESIZE_FAILED";

    /**
     * Task is stopped because the file size is not obtained
     */
    int GET_FILESIZE_FAILED_CODE = 11;

    /**
     * Task is stopped because the continuous task timeout
     */
    String CONTINUOUS_TASK_TIMEOUT = "CONTINUOUS_TASK_TIMEOUT";

    /**
     * Task is stopped because the continuous task timeout
     */
    int CONTINUOUS_TASK_TIMEOUT_CODE = 12;

    /**
     * Task is stopped because of connection error
     */
    String CONNECT_ERROR = "CONNECT_ERROR";

    /**
     * Task is stopped because of connection error
     */
    int CONNECT_ERROR_CODE = 13;

    /**
     * Task is stopped because of request error
     */
    String REQUEST_ERROR = "REQUEST_ERROR";

    /**
     * Task is stopped because of request error
     */
    int REQUEST_ERROR_CODE = 14;

    /**
     * Task is stopped because of upload file error
     */
    String UPLOAD_FILE_ERROR = "UPLOAD_FILE_ERROR";

    /**
     * Task is stopped because of upload file error
     */
    int UPLOAD_FILE_ERROR_CODE = 15;

    /**
     * Task is stopped because of redirect error
     */
    String REDIRECT_ERROR = "REDIRECT_ERROR";

    /**
     * Task is stopped because of redirect error
     */
    int REDIRECT_ERROR_CODE = 16;

    /**
     * Task is stopped because of protocol error
     */
    String PROTOCOL_ERROR = "PROTOCOL_ERROR";

    /**
     * Task is stopped because of protocol error
     */
    int PROTOCOL_ERROR_CODE = 17;

    /**
     * Task is stopped because of IO error
     */
    String IO_ERROR = "IO_ERROR";

    /**
     * Task is stopped because of IO error code
     */
    int IO_ERROR_CODE = 18;

    /**
     * Task is stopped because of unsupported range request
     */
    String UNSUPPORT_RANGE_REQUEST = "UNSUPPORT_RANGE_REQUEST";

    /**
     * Task is stopped because of unsupported range request code
     */
    int UNSUPPORT_RANGE_REQUEST_CODE = 19;

    /**
     * Task is stopped because of others error
     */
    String OTHERS_ERROR = "OTHERS_ERROR";

    /**
     * Task is stopped because of others error
     */
    int OTHERS_ERROR_CODE = 20;
}
