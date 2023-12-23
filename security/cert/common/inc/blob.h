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

#ifndef HCF_BLOB_H
#define HCF_BLOB_H

#include <stddef.h>
#include <stdint.h>

typedef struct HcfBlob HcfBlob;
struct HcfBlob {
    uint8_t *data;
    size_t len;
};

enum EncodingFormat {
    HCF_FORMAT_DER = 0,
    HCF_FORMAT_PEM = 1,
};

#ifdef __cplusplus
extern "C" {
#endif

void HcfBlobDataFree(HcfBlob *blob);
void HcfBlobDataClearAndFree(HcfBlob *blob);

#ifdef __cplusplus
}
#endif

#endif
