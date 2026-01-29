/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.plugin.clipboard;

/**
 * Pasteboard constants.
 *
 * @since 1
 */
public class PasteboardConstants {
    /**
     * Distributed data manager system ID.
     */
    public static final int DIST_DATA_MGR_SYS_ID = 0xd;

    /**
     * Pasteboard ID.
     */
    public static final int PASTEBOARD_ID = 8;

    /**
     * Base value of error code.
     */
    public static final int E_OK_BASE_VALUE = (DIST_DATA_MGR_SYS_ID << 21) | (PASTEBOARD_ID << 16);

    /**
     * Error code.
     */
    public enum PasteboardError {
        E_OK(E_OK_BASE_VALUE),
        INVALID_RETURN_VALUE_ERROR(E_OK_BASE_VALUE + 1),
        INVALID_PARAM_ERROR(E_OK_BASE_VALUE + 2),
        SERIALIZATION_ERROR(E_OK_BASE_VALUE + 3),
        DESERIALIZATION_ERROR(E_OK_BASE_VALUE + 4),
        OBTAIN_SERVER_SA_ERROR(E_OK_BASE_VALUE + 5),
        OTHER_ERROR(E_OK_BASE_VALUE + 6),
        CROSS_BORDER_ERROR(E_OK_BASE_VALUE + 7),
        PERMISSION_VERIFICATION_ERROR(E_OK_BASE_VALUE + 8),
        PARAM_ERROR(E_OK_BASE_VALUE + 9),
        TIMEOUT_ERROR(E_OK_BASE_VALUE + 10),
        CANCELED(E_OK_BASE_VALUE + 11),
        EXCEEDING_LIMIT_EXCEPTION(E_OK_BASE_VALUE + 12),
        TASK_PROCESSING(E_OK_BASE_VALUE + 13),
        PROHIBIT_COPY(E_OK_BASE_VALUE + 14),
        UNKNOWN_ERROR(E_OK_BASE_VALUE + 15),
        BACKUP_EXCEPTION(E_OK_BASE_VALUE + 16),
        REMOTE_EXCEPTION(E_OK_BASE_VALUE + 17),
        INVALID_DATA_ERROR(E_OK_BASE_VALUE + 18),
        NO_DATA_ERROR(E_OK_BASE_VALUE + 19),
        INVALID_USERID_ERROR(E_OK_BASE_VALUE + 20),
        REMOTE_TASK_ERROR(E_OK_BASE_VALUE + 21),
        INVALID_EVENT_ERROR(E_OK_BASE_VALUE + 22),
        GET_REMOTE_DATA_ERROR(E_OK_BASE_VALUE + 23),
        SEND_BROADCAST_ERROR(E_OK_BASE_VALUE + 24),
        SYNC_DATA_ERROR(E_OK_BASE_VALUE + 25),
        URI_GRANT_ERROR(E_OK_BASE_VALUE + 26),
        DP_LOAD_SERVICE_ERROR(E_OK_BASE_VALUE + 27),
        INVALID_OPTION_ERROR(E_OK_BASE_VALUE + 28),
        INVALID_OPERATION_ERROR(E_OK_BASE_VALUE + 29),
        BUTT_ERROR(E_OK_BASE_VALUE + 30),
        NO_TRUST_DEVICE_ERROR(E_OK_BASE_VALUE + 31),
        NO_USER_DATA_ERROR(E_OK_BASE_VALUE + 32),
        DATA_EXPIRED_ERROR(E_OK_BASE_VALUE + 33),
        CREATE_DATASHARE_SERVICE_ERROR(E_OK_BASE_VALUE + 34),
        QUERY_SETTING_NO_DATA_ERROR(E_OK_BASE_VALUE + 35),
        GET_LOCAL_DEVICE_ID_ERROR(E_OK_BASE_VALUE + 36),
        LOCAL_SWITCH_NOT_TURNED_ON(E_OK_BASE_VALUE + 37),
        DATA_ENCODE_ERROR(E_OK_BASE_VALUE + 38),
        NO_DELAY_GETTER(E_OK_BASE_VALUE + 39),
        PLUGIN_IS_NULL(E_OK_BASE_VALUE + 40),
        PLUGIN_EVENT_EMPTY(E_OK_BASE_VALUE + 41),
        GET_LOCAL_DATA(E_OK_BASE_VALUE + 42),
        INVALID_EVENT_ACCOUNT(E_OK_BASE_VALUE + 43),
        INVALID_EVENT_STATUS(E_OK_BASE_VALUE + 44),
        PROGRESS_PASTE_TIME_OUT(E_OK_BASE_VALUE + 45),
        PROGRESS_CANCEL_PASTE(E_OK_BASE_VALUE + 46),
        COPY_FILE_ERROR(E_OK_BASE_VALUE + 47),
        PROGRESS_START_ERROR(E_OK_BASE_VALUE + 48),
        PROGRESS_ABNORMAL(E_OK_BASE_VALUE + 49),
        PRPGRESS_CANCEL_SUCCESS(E_OK_BASE_VALUE + 50),
        GET_ENTRY_VALUE_FAILED(E_OK_BASE_VALUE + 51),
        REBUILD_HTML_FAILED(E_OK_BASE_VALUE + 52),
        INVALID_DATA_ID(E_OK_BASE_VALUE + 53),
        INVALID_RECORD_ID(E_OK_BASE_VALUE + 54),
        INVALID_MIMETYPE(E_OK_BASE_VALUE + 55),
        MALLOC_FAILED(E_OK_BASE_VALUE + 56),
        GET_SAMGR_FAILED(E_OK_BASE_VALUE + 57),
        RESOURCE_APPLY_TIMEOUT(E_OK_BASE_VALUE + 58),
        RESOURCE_APPLYING(E_OK_BASE_VALUE + 59),
        RESOURCE_APPLY_NOT_FIND(E_OK_BASE_VALUE + 60),
        CHECK_DESCRIPTOR_ERROR(E_OK_BASE_VALUE + 61),
        GET_SAME_REMOTE_DATA(E_OK_BASE_VALUE + 62),
        DLOPEN_FAILED(E_OK_BASE_VALUE + 63),
        GET_BUNDLE_MGR_FAILED(E_OK_BASE_VALUE + 64),
        GET_BOOTTIME_FAILED(E_OK_BASE_VALUE + 65),
        NOT_SUPPORT(E_OK_BASE_VALUE + 66),
        ADD_OBSERVER_FAILED(E_OK_BASE_VALUE + 67),
        INVALID_DATA_SIZE(E_OK_BASE_VALUE + 68),
        INVALID_TOKEN_ID(E_OK_BASE_VALUE + 69),
        INVALID_URI_ERROR(E_OK_BASE_VALUE + 70);

        private final int value;

        /**
         * Constructor.
         *
         * @param value value
         */
        PasteboardError(int value) {
            this.value = value;
        }

        /**
         * Get value.
         *
         * @return value
         */
        public int getValue() {
            return value;
        }

        /**
         * Get error from value.
         *
         * @param value value
         * @return error
         */
        public static PasteboardError fromValue(int value) {
            for (PasteboardError error : PasteboardError.values()) {
                if (error.value == value) {
                    return error;
                }
            }
            return null;
        }
    }
}