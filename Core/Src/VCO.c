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

#include <demiurge.h>

#define TAG "VCO"

control_pair_t pair1;  // frequency

control_pair_t pair2;  // frequency offset
offset_t offset_freq;

control_pair_t pair3;

audio_outport_t out1;
audio_outport_t out2;

oscillator_t oscillator;

float offset10( float value ) {
   return value + 10;
}
/*
 * Simple VCO with sine wave on both outputs.
 */
void vco_setup() {
   control_pair_init(&pair1, 1);
   control_pair_init(&pair2, 2);
   offset_init(&offset_freq);
   offset_configure_input(&offset_freq, &pair1.me);
   offset_configure_control(&offset_freq, &pair2.me);

   control_pair_init(&pair3, 3);
   oscillator_init(&oscillator);
   oscillator_configure_mode(&oscillator, TRIANGLE);
   oscillator_configure_frequency(&oscillator, &offset_freq.me);
   oscillator_configure_attentuation(&oscillator, &pair3.me);
   audio_outport_init(&out1, 1);
   audio_outport_init(&out2, 2);
   audio_outport_configure_input(&out1, &oscillator.me);
   audio_outport_configure_input(&out2, &oscillator.me);
}

void vco_loop() {

}

#undef TAG
