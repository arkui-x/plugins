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

public interface Reason {
    String REASON_OK = "REASON_OK";
    int REASON_OK_CODE = 0;
    String TASK_SURVIVAL_ONE_MONTH = "TASK_SURVIVAL_ONE_MONTH";
    int TASK_SURVIVAL_ONE_MONTH_CODE = 1;
    String WAITTING_NETWORK_ONE_DAY = "WAITTING_NETWORK_ONE_DAY";
    int WAITTING_NETWORK_ONE_DAY_CODE = 2;
    String STOPPED_NEW_FRONT_TASK = "STOPPED_NEW_FRONT_TASK";
    int STOPPED_NEW_FRONT_TASK_CODE = 3;
    String RUNNING_TASK_MEET_LIMITS = "RUNNING_TASK_MEET_LIMITS";
    int RUNNING_TASK_MEET_LIMITS_CODE = 4;
    String USER_OPERATION = "USER_OPERATION";
    int USER_OPERATION_CODE = 5;
    String APP_BACKGROUND_OR_TERMINATE = "APP_BACKGROUND_OR_TERMINATE";
    int APP_BACKGROUND_OR_TERMINATE_CODE = 6;
    String NETWORK_OFFLINE = "NETWORK_OFFLINE";
    int NETWORK_OFFLINE_CODE = 7;
    String UNSUPPORTED_NETWORK_TYPE = "UNSUPPORTED_NETWORK_TYPE";
    int UNSUPPORTED_NETWORK_TYPE_CODE = 8;
    String BUILD_CLIENT_FAILED = "BUILD_CLIENT_FAILED";
    int BUILD_CLIENT_FAILED_CODE = 9;
    String BUILD_REQUEST_FAILED = "BUILD_REQUEST_FAILED";
    int BUILD_REQUEST_FAILED_CODE = 10;
    String GET_FILESIZE_FAILED = "GET_FILESIZE_FAILED";
    int GET_FILESIZE_FAILED_CODE = 11;
    String CONTINUOUS_TASK_TIMEOUT = "CONTINUOUS_TASK_TIMEOUT";
    int CONTINUOUS_TASK_TIMEOUT_CODE = 12;
    String CONNECT_ERROR = "CONNECT_ERROR";
    int CONNECT_ERROR_CODE = 13;
    String REQUEST_ERROR = "REQUEST_ERROR";
    int REQUEST_ERROR_CODE = 14;
    String UPLOAD_FILE_ERROR = "UPLOAD_FILE_ERROR";
    int UPLOAD_FILE_ERROR_CODE = 15;
    String REDIRECT_ERROR = "REDIRECT_ERROR";
    int REDIRECT_ERROR_CODE = 16;
    String PROTOCOL_ERROR = "PROTOCOL_ERROR";
    int PROTOCOL_ERROR_CODE = 17;
    String IO_ERROR = "IO_ERROR";
    int IO_ERROR_CODE = 18;
    String UNSUPPORT_RANGE_REQUEST = "UNSUPPORT_RANGE_REQUEST";
    int UNSUPPORT_RANGE_REQUEST_CODE = 19;
    String OTHERS_ERROR = "OTHERS_ERROR";
    int OTHERS_ERROR_CODE = 20;
}
