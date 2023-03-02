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

#include <esp_camera.h>
#include <esp_log.h>
#include <esp_system.h>

#include "camera_app.hpp"
#include "camera_pins.h"

const char *Camera_app::TAG = "camera_app:";

Camera_app::Camera_app(void)
{
    this->config.ledc_channel = LEDC_CHANNEL_0;
    this->config.ledc_timer = LEDC_TIMER_0;

    this->config.pin_d0 = Y2_GPIO_NUM;
    this->config.pin_d1 = Y3_GPIO_NUM;
    this->config.pin_d2 = Y4_GPIO_NUM;
    this->config.pin_d3 = Y5_GPIO_NUM;
    this->config.pin_d4 = Y6_GPIO_NUM;
    this->config.pin_d5 = Y7_GPIO_NUM;
    this->config.pin_d6 = Y8_GPIO_NUM;
    this->config.pin_d7 = Y9_GPIO_NUM;

    this->config.pin_xclk = XCLK_GPIO_NUM;
    this->config.pin_pclk = PCLK_GPIO_NUM;
    this->config.pin_vsync = VSYNC_GPIO_NUM;
    this->config.pin_href = HREF_GPIO_NUM;
    this->config.pin_sccb_sda = SIOD_GPIO_NUM; // pin_sscb_sda has been deprecated so using pin_sccb_sda
    this->config.pin_sccb_scl = SIOC_GPIO_NUM; // pin_sscb_scl has been deprecated so using pin_sccb_scl
    this->config.pin_pwdn = PWDN_GPIO_NUM;
    this->config.pin_reset = RESET_GPIO_NUM;

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    this->config.xclk_freq_hz = 20000000;
    this->config.pixel_format = PIXFORMAT_JPEG;

    this->config.fb_count = 1; // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    this->config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // Default Quality
    this->config.frame_size = FRAMESIZE_SVGA;
    this->config.jpeg_quality = 12; // 0-63, for OV series camera sensors, lower number means higher quality
}

esp_err_t Camera_app::init()
{
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        esp_restart();
    }
    return err;
}

void Camera_app::set_frame_size(framesize_t frame_size)
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, frame_size);
}

void Camera_app::set_img_quality(int quality)
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_quality(s, quality);
}

camera_fb_t *Camera_app::capture()
{
    camera_fb_t *pic = esp_camera_fb_get();
    // use pic->buf to access the image
    ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);

    if (pic->format != PIXFORMAT_JPEG)
        return NULL;

    return pic;
}

Camera_app::~Camera_app()
{
    esp_err_t err = esp_camera_deinit();
    ESP_LOGI(TAG, "Camera deinit...");
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Camera deinit failed with error 0x%x", err);
}
