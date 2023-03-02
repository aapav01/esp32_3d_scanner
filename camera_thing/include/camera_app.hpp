/*
 Copyright 2023 Apavayan <info@apavayan.com>
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
      https://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#ifndef _CAMERA_APP_HPP_
#define _CAMERA_APP_HPP_

#include <esp_camera.h>

#define CAMERA_MODEL_AI_THINKER

class Camera_app
{
private:
    camera_config_t config;
    static const char *TAG;

public:
    Camera_app(void);
    ~Camera_app();
    esp_err_t init();
    void set_frame_size(framesize_t);
    void set_img_quality(int);
    camera_fb_t *capture();
};
#endif
