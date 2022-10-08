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

package ohos.ace.plugin.displayplugin;

/**
 * DisplayInfo
 *
 * @since 1
 */
public class DisplayInfo {
    public int id;
    public String name;
    public boolean alive;
    public int state;
    public float refreshRate;
    public int rotation;
    public int width;
    public int height;
    public int densityDPI;
    public float densityPixels;
    public float scaledDensity;
    public float xDPI;
    public float yDPI;
}
