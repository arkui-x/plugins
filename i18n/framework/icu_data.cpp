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
 
#include "icu_data.h"

#include "ucmndata.h"
#include "unicode/utypes.h"
#include "unicode/udata.h"

#include <mutex>

extern "C" const DataHeader U_DATA_API U_ICUDATA_ENTRY_POINT;

namespace {
static int status = 0;
std::mutex dataMutex;
}

/**
 * set icu data files to hwDirectory
 */
void InitIcuData()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    if (status != 0) {
        return;
    }
	UErrorCode err;
    udata_setCommonDataAfterClean(&U_ICUDATA_ENTRY_POINT, &err);
    status = 1;
}
