#!/bin/bash
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

cd $2

if [ -d "abseil-cpp" ];then
        rm -rf abseil-cpp
fi

tarlist=($(find $1 -name *.gz))

tarname=${tarlist[0]}

tar zxvf $tarname -C $2

dirname=${tarname%%.tar.gz}
dirname=${dirname##*/}
mv $dirname abseil-cpp

exit 0
