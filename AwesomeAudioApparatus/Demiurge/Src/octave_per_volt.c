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


#include <stdint.h>
#include "octave_per_volt.h"

float octave_frequency_of(float voltage) {
   // TODO: power of 2 can be optimized on 'float' by adding 1 to exponent.
   if( voltage > 10 || voltage < -10 )
   {
      return 400;
   }
   return 27.5 * pow2(voltage);
}

float octave_voltage_of(float frequency) {
   return log2f(frequency / 27.5);
}
