/*
  Copyright 2019, Awesome Audio Apparatus.

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

#include "led.h"
#include "demiurge.h"


// TODO: This code should be refactored so that all the hardware specifics goes to Demiurge::runtime(), and we can later devise a "driver" system if there are more boards being made.

void led_init(led_t *handle, int position) {
   configASSERT(position > 0 && position <= 4)
   handle->me.read_fn = led_read;
   handle->me.data = handle;
   handle->me.post_fn = clip_none;
   handle->registered = false;
//   handle->channel = LED_CHANNEL[position - 1];
   handle->input = NULL;
   handle->fade = NULL;
   handle->dutycycle = NULL;

}

void led_configure_input(led_t *handle, signal_t *input) {
   handle->input = input;
   if (!handle->registered) {
      demiurge_registerSink(&handle->me);
      handle->registered = true;
   }
}

void led_configure_fade(led_t *handle, signal_t *fade) {
   handle->fade = fade;
}

void led_configure_dutycycle(led_t *handle, signal_t *dutycycle) {
   handle->dutycycle = dutycycle;
}

//static void led_set(ledc_channel_t channel, uint32_t duty, int fade_time_ms) {
//   ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, channel, duty, fade_time_ms);
//   ledc_fade_start(LEDC_HIGH_SPEED_MODE, channel, LEDC_FADE_NO_WAIT);
//};

float led_read(signal_t *handle, uint64_t time) {
   led_t *led = (led_t *) handle->data;
   if (time > handle->last_calc) {
      handle->last_calc = time;
      signal_t *upstream = led->input;
      float result = handle->post_fn(upstream->read_fn(upstream, time));

// TODO: Fix the dynamic LEDs
//      uint32_t duty = (uint32_t) (result * 819.0f);
//      led_set(led->channel, duty, led->fade_time_ms);
//      led->duty = duty;
//      handle->cached = duty;
//      return duty;
   }
   return handle->cached;
}
