/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "blob.h"

#include <securec.h>
#include "cf_memory.h"
#include "cf_log.h"

void HcfBlobDataFree(HcfBlob *blob)
{
    if ((blob == NULL) || (blob->data == NULL)) {
        return;
    }
    CfFree(blob->data);
    blob->data = NULL;
    blob->len = 0;
}

void HcfBlobDataClearAndFree(HcfBlob *blob)
{
    if ((blob == NULL) || (blob->data == NULL)) {
        LOGD("The input blob is null, no need to free.");
        return;
    }
    (void)memset_s(blob->data, blob->len, 0, blob->len);
    CfFree(blob->data);
    blob->data = NULL;
    blob->len = 0;
}
