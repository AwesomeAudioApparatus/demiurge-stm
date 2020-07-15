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

#include <main.h>
#include "clipping.h"
#include "demiurge.h"
#include "demi_asserts.h"
#include "oscillator.h"
#include "octave_per_volt.h"

void oscillator_init(oscillator_t *handle) {
   handle->me.read_fn = oscillator_saw;
   handle->me.data = handle;
   handle->me.post_fn = clip_none;
   handle->frequency = NULL;
   handle->amplitude = NULL;
   handle->trigger = NULL;
   handle->lastTrig = 0;
   handle->scale = 1.0;
}

void
oscillator_configure(oscillator_t *handle, signal_t *freqControl, signal_t *amplitudeControl, signal_t *trigControl) {
   oscillator_configure_frequency(handle, freqControl);
   oscillator_configure_amplitude(handle, amplitudeControl);
   oscillator_configure_trig(handle, trigControl);
}

void oscillator_configure_mode(oscillator_t *handle, oscillator_mode mode) {
   switch (mode) {
      case SAW:
         handle->me.read_fn = oscillator_saw;
         break;
      case SINE:
         handle->me.read_fn = oscillator_sine;
         break;
      case SQUARE:
         handle->me.read_fn = oscillator_square;
         break;
      case TRIANGLE:
         handle->me.read_fn = oscillator_triangle;
         break;
      default:
         break;
   }
}

void oscillator_configure_frequency(oscillator_t *handle, signal_t *control) {
   configASSERT(control != NULL)
   handle->frequency = control;
}

void oscillator_configure_amplitude(oscillator_t *handle, signal_t *control) {
   configASSERT(control != NULL)
   handle->amplitude = control;
}

void oscillator_configure_trig(oscillator_t *handle, signal_t *control) {
   configASSERT(control != NULL)
   handle->trigger = control;
}

static float angular_delta(const oscillator_t *osc, uint64_t time_in_us) {
   signal_t *freqControl = osc->frequency;
   float freq = 440;
   if (freqControl) {
      float voltage = freqControl->read_fn(freqControl, time_in_us);
      freq = octave_frequency_of(voltage);
#ifdef DEMIURGE_DEV
      handle->extra2 = voltage;
   }
   handle->extra3 = freq;
#else
   }
#endif
   float samples_per_second = (float) demiurge_sample_rate();
   float progression = freq / samples_per_second;
   return progression;
}

static float scale(oscillator_t *osc, uint64_t time_in_us) {
   float scale = 1.0f;
   if (osc->amplitude != NULL) {
      scale = (10.0f + osc->amplitude->read_fn(osc->amplitude, time_in_us)) / 4.0;
   }
   return scale / 2.0;
}

float oscillator_saw(signal_t *handle, uint64_t time_in_us) {
   if (time_in_us > handle->last_calc) {
      handle->last_calc = time_in_us;
      oscillator_t *osc = (oscillator_t *) handle->data;
      float delta = angular_delta(osc, time_in_us);
      float x = osc->angular_pos + delta;
      if( x > 1.0f )
         x = 0.0f;
      float out = x * osc->scale;
      out = handle->post_fn(out);
      osc->angular_pos = x;
#ifdef DEMIURGE_DEV
      handle->extra1 = out;
      handle->extra2 = x;
      handle->extra3 = delta;
#endif
      handle->cached = out;
      return out;
   }
   return handle->cached;
}

float oscillator_triangle(signal_t *handle, uint64_t time_in_us) {
}

float oscillator_sine(signal_t *handle, uint64_t time_in_us) {
   if (time_in_us > handle->last_calc) {
      handle->last_calc = time_in_us;
      oscillator_t *osc = (oscillator_t *) handle->data;
      float delta = angular_delta(osc, time_in_us);
      float x = osc->angular_pos + delta;
      if( x > 1.0f )
         x = 0.0f;
      float out = arm_sin_f32( M_TWOPI * x ) * osc->scale;
      out = handle->post_fn(out);
      osc->angular_pos = x;
#ifdef DEMIURGE_DEV
      handle->extra1 = out;
      handle->extra2 = x;
      handle->extra3 = delta;
#endif
      handle->cached = out;
      return out;
   }
   return handle->cached;
}

float oscillator_square(signal_t *handle, uint64_t time_in_us) {
   if (time_in_us > handle->last_calc) {
      handle->last_calc = time_in_us;
      oscillator_t *osc = (oscillator_t *) handle->data;
      float delta = angular_delta(osc, time_in_us);
      float x = osc->angular_pos + delta;
      if( x > 1.0f )
         x = 0.0f;
      float out = (x > 0.5f) * osc->scale;
      out = handle->post_fn(out);
      osc->angular_pos = x;
#ifdef DEMIURGE_DEV
      handle->extra1 = out;
      handle->extra2 = x;
      handle->extra3 = delta;
#endif
      handle->cached = out;
      return out;
   }
   return handle->cached;
}

