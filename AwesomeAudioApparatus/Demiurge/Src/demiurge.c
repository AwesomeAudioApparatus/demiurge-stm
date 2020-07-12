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

#include "stm32f4xx_hal.h"
#include "demiurge.h"
#include "logger.h"
#include "main.h"

#define TAG "SOUND"

static uint32_t gpio_a;
static uint32_t gpio_b;
static uint32_t gpio_c;

// Figure out later whether the hardware should be initialized in main.c or just moved here.
extern DAC_HandleTypeDef hdac;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern TIM_HandleTypeDef htim6;  // Timer for realtime clock.

static uint32_t sample_rate;
static uint32_t micros_per_tick;
static uint64_t timer_counter = 0;

#ifdef DEMIURGE_TICK_TIMING
static uint32_t tick_update = 0;
static volatile uint64_t tick_start = 0;
static volatile uint64_t tick_duration = 0;
static volatile uint64_t tick_interval = 0;
#endif

static volatile signal_t *sinks[DEMIURGE_MAX_SINKS];
float inputs[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float outputs[2] = {0.0f, 0.0f};


// Overrun increments means that the tick() took longer than the sample time.
// If you see this happening, either decrease the sample rate or optimize the tick()
// evaluation to take less time.
static uint32_t overrun = -3;  // 3 overruns happen during startup, and that is ok. Let's compensate for that.

uint64_t demiurge_current_time() {
   return timer_counter;
}

static void wait_timer() {
   // htim6 must be set to tick at microseconds.
   bool atleast_once = false;
   uint32_t passed;
   while (1) {
      volatile uint32_t current = htim6.Instance->CNT;
      passed = current - (uint32_t) timer_counter;
      if (passed >= micros_per_tick) {
         break;
      }
      atleast_once = true;
   }
   if (!atleast_once)
      overrun++;
   timer_counter += passed;
}

void demiurge_registerSink(signal_t *processor) {
   logI(TAG, "Registering Sink: %p", (void *) processor);
   configASSERT(processor != NULL)
   for (int i = 0; i < DEMIURGE_MAX_SINKS; i++) {
      if (sinks[i] == NULL) {
         sinks[i] = processor;
         logI(TAG, "Registering Sink: %d", i);
         break;
      }
   }
}

void demiurge_unregisterSink(signal_t *processor) {
   logI(TAG, "Unregistering Sink: %p", (void *) processor);
   configASSERT(processor != NULL)
   for (int i = 0; i < DEMIURGE_MAX_SINKS; i++) {
      if (sinks[i] == processor) {
         sinks[i] = NULL;
         logI(TAG, "Unregistering Sink: %d, %p", i, (void *) processor);
         break;
      }
   }
}


bool demiurge_gpio(int pin) {
   uint32_t gpios = 0;
   if (pin >= 0 && pin < 16)
      gpios = gpio_a;
   else if (pin >= 16 && pin < 32)
      gpios = gpio_b;
   else if (pin >= 32 && pin < 48)
      gpios = gpio_c;
   return (gpios >> pin & 1) != 0;
}

void demiurge_set_output(int number, float value) {
   configASSERT(number > 0 && number <= 2)
   outputs[number - 1] = value;
}
uint32_t demiurge_sample_rate()
{
   return sample_rate;
}
void demiurge_print_overview(const char *tag, signal_t *signal) {
#ifdef DEMIURGE_TICK_TIMING
   logI("TICK", "interval=%lld, duration=%lld, start=%lld, overrun=%d",
            tick_interval, tick_duration, tick_start, overrun);
#endif  //DEMIURGE_TICK_TIMING

   logI(tag, "Input: %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f",
        demiurge_input(1),
        demiurge_input(2),
        demiurge_input(3),
        demiurge_input(4),
        demiurge_input(5),
        demiurge_input(6),
        demiurge_input(7),
        demiurge_input(8)
   );
   logI(tag, "Output: %2.1f, %2.1f",
        demiurge_output(1),
        demiurge_output(2)
   );
#ifdef DEMIURGE_DEV
   logI(tag, "Extras: [%lld] : %2.1f - %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f, %2.1f",
            signal->last_calc,
            signal->cached,
            signal->extra1,
            signal->extra2,
            signal->extra3,
            signal->extra4,
            signal->extra5,
            signal->extra6,
            signal->extra7,
            signal->extra8
   );
#endif // DEMIURGE_DEV
}

float demiurge_input(int number) {
   configASSERT(number > 0 && number <= 8)
   return inputs[number - 1];
}

float demiurge_output(int number) {
   configASSERT(number > 0 && number <= DEMIURGE_MAX_SINKS)
   return outputs[number - 1];
}

static void readGpio() {
   gpio_a = GPIOA->IDR;
   gpio_b = GPIOB->IDR;
   gpio_c = GPIOC->IDR;
}

static void update_dac() {
   uint16_t ch1 = (uint16_t) ((10.0f - outputs[0]) * 204.8f);
   uint16_t ch2 = (uint16_t) ((10.0f - outputs[1]) * 204.8f);
   hdac.Instance->DHR12R1 = ch1;
   hdac.Instance->DHR12R2 = ch2;
}

static void read_adc() {
   inputs[0] = ((float) hadc1.Instance->DR) / 204.8f;
   inputs[1] = ((float) hadc2.Instance->DR) / 204.8f;
}

void demiurge_tick() {
   wait_timer();
   HAL_GPIO_WritePin(TP1_GPIO_Port, TP1_Pin,GPIO_PIN_RESET ); // TP1 - Test Point to check timing

   timer_counter = demiurge_current_time();
#ifdef DEMIURGE_TICK_TIMING
   tick_interval = timer_counter - tick_start;
   tick_start = timer_counter;
#endif
   // We are setting the outputs at the start of a cycle, to ensure that the interval is identical from cycle to cycle.
   update_dac();
   readGpio();
   read_adc();
   for (int i = 0; i < DEMIURGE_MAX_SINKS; i++) {
      signal_t *sink = sinks[i];
      if (sink != NULL) {
         sink->read_fn(sink, timer_counter);  // ignore return value
      }
   }
#ifdef DEMIURGE_TICK_TIMING
   if (tick_update > 200000) {
      tick_duration = timer_counter - tick_start;
      tick_update = 0;
   }
   tick_update++;
#endif
   HAL_GPIO_WritePin(TP1_GPIO_Port, TP1_Pin,GPIO_PIN_RESET );
}

static void demiurge_initialize() {
   logI(TAG, "Starting Demiurge...\n");

   for (int i = 0; i < DEMIURGE_MAX_SINKS; i++)
      sinks[i] = NULL;
   logI(TAG, "Initialized GPIO done");
}

void demiurge_start(uint64_t rate) {
   sample_rate = rate;
   demiurge_initialize();
   logI(TAG, "Demiurge Started.");
}

#undef TAG
