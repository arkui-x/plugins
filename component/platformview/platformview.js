/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

const __PlatformView__ = requireInternal('platformview');
class PlatformView extends JSViewAbstract {
    static create(id, data, type) {
        const finalType = (type === undefined || type === null) ? 0 : type;
        if (data === undefined || data === null) {
            __PlatformView__.create(id, finalType);
        } else {
            __PlatformView__.create(id, finalType, data);
        }
    }
    
    static onAppear(value) {
        __Common__.onAppear(value);
    }

    static onDisAppear(value) {
        __Common__.onDisAppear(value);
    }

    static rotate(value) {
        __PlatformView__.rotate(value);
        JSViewAbstract.rotate(value);
    }

    static scale(value) {
        __PlatformView__.scale(value);
        JSViewAbstract.scale(value);
    }

    static translate(value) {
        __PlatformView__.translate(value);
        JSViewAbstract.translate(value);
    }

    static transform(value) {
        __PlatformView__.transform(value);
        JSViewAbstract.transform(value);
    }

    static transform3D(value) {
        __PlatformView__.transform(value);
        JSViewAbstract.transform(value);
    }
}

export let PlatformViewType;
(function (PlatformViewType) {
  PlatformViewType[PlatformViewType.TEXTURE_TYPE = 0] = 'TEXTURE_TYPE';
  PlatformViewType[PlatformViewType.SURFACE_TYPE = 1] = 'SURFACE_TYPE';
})(PlatformViewType || (PlatformViewType = {}));
PlatformView.PlatformViewType = PlatformViewType;
export default PlatformView;
