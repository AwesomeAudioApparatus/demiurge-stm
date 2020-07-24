/*
  Copyright 2019-2020, Awesome Audio Apparatus.

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

#include "demiurge.h"

audio_inport_t in1;
audio_inport_t in2;

control_pair_t pair1;
control_pair_t pair2;

audio_outport_t out1;
audio_outport_t out2;

mixer_t mixer;

/*
 * A two port Mixer, with CV control
 */
void setup() {
   mixer_init(&mixer, 2);
   control_pair_init(&pair1, 1);
   control_pair_init(&pair2, 2);
   audio_inport_init(&in1, 3);
   audio_inport_init(&in2, 4);
   mixer_configure_input(&mixer, 1, &in1.me, &pair1.me);
   mixer_configure_input(&mixer, 2, &in2.me, &pair2.me);

   audio_outport_init(&out1, 1);
   audio_outport_init(&out2, 2);
   audio_outport_configure_input(&out1, &mixer.me);
}

void loop() {
}
