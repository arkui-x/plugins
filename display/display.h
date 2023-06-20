/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_DISPLAY_DISPLAY_H
#define PLUGINS_DISPLAY_DISPLAY_H

#include <memory>
#include <string>

#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
enum class DisplayState {
    /**
     * Unknown.
     */
    STATE_UNKNOWN = 0,
    /**
     * Screen off.
     */
    STATE_OFF,
    /**
     * Screen on.
     */
    STATE_ON,
    /**
     * Doze, but it will update for some important system messages.
     */
    STATE_DOZE,
    /**
     * Doze and not update.
     */
    STATE_DOZE_SUSPEND,
    /**
     * VR node.
     */
    STATE_VR,
    /**
     * Screen on and not update.
     */
    STATE_ON_SUSPEND,
};

struct DisplayInfo {
    /**
     * Unique ID of the physical display
     *
     */
    int32_t id;

    /**
     * Name of the physical display, which is used for debugging
     *
     */
    std::string name;

    /**
     * The display is alive.
     */
    bool alive;

    /**
     * The state of display.
     */
    DisplayState state;

    /**
     * Refresh rate, in Hz.
     */
    int32_t refreshRate;

    /**
     * Rotation degrees of the display.
     */
    int32_t rotation;

    /**
     * Display width, which is the logical width of the original screen when the rotation angle is 0.
     * The value remains unchanged even if the display screen is rotated.
     *
     */
    int32_t width;

    /**
     * Display height, which is the logical height of the original screen when the rotation angle is 0.
     * The value remains unchanged even if the display screen is rotated.
     *
     */
    int32_t height;

    /**
     * Display resolution.
     */
    int32_t densityDPI;

    /**
     * Display density, in pixels. The value for a low-resolution display is 1.0.
     */
    int32_t densityPixels;

    /**
     * Text scale density of the display.
     */
    int32_t scaledDensity;

    /**
     * DPI on the x-axis.
     *
     */
    int32_t xDPI;

    /**
     * DPI on the y-axis.
     *
     */
    int32_t yDPI;
};

class Display {
public:
    Display() = default;
    virtual ~Display() = default;
    static std::unique_ptr<Display> Create();
    virtual void GetDefaultDisplay(AsyncCallbackInfo *ptr) = 0;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_DISPLAY_DISPLAY_H
