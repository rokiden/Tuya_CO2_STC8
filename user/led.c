#include "fw_hal.h"

#include "led.h"

// Preprocessor concat
#define PPCAT_NX(A, B, C) A##B##C
#define PPCAT(A, B, C) PPCAT_NX(A, B, C)

// Enums
#define LED_PORT_ENUM PPCAT(GPIO_Port_, LED_PORT, )
#define LED_PIN_ENUM PPCAT(GPIO_Pin_, LED_PIN, )

// Funcs
#define LED_SETMODE(mode) PPCAT(GPIO_P, LED_PORT, _SetMode)(LED_PIN_ENUM, mode)
#define LED_SETPU(state) GPIO_SetPullUp(LED_PORT_ENUM, LED_PIN_ENUM, state)
#define LED_SETVAL(val) PPCAT(P, LED_PORT, LED_PIN) = val

//
static uint8_t blink_pulses;
static __BIT blink_inf;

static uint8_t blink_cntr_intr;
static uint8_t blink_cntr_pulse;

static __BIT blink_led;
static __BIT blink_pause;

void led_intr() {
  if (blink_pulses) {
    if (blink_cntr_intr == 0) // start of period
      LED_SETVAL(blink_led);

    if (blink_cntr_intr ==
        (blink_pause ? INTRS_PAUSE : INTRS_ON_OFF)) { // end of period
      blink_cntr_intr = 0;

      if (blink_pause) { // end of pause
        blink_pause = 0;
      } else if (blink_led) { // end of on
        blink_cntr_pulse++;
        if (blink_cntr_pulse == blink_pulses) { // all pulses done
          blink_cntr_pulse = 0;
          if (blink_inf)
            blink_pause = 1;
          else { // job done, turn off, cleanup
            blink_pulses = 0;
            LED_SETVAL(0);
          }
        }
      }
      blink_led = !(blink_pause || blink_led);
    } else
      blink_cntr_intr++;
  }
}

inline void led_reset() {
  blink_cntr_intr = 0;
  blink_cntr_pulse = 0;
  blink_pause = 0;
}

void led_init() {
  LED_SETMODE(GPIO_Mode_InOut_OD);
  LED_SETPU(HAL_State_ON);
  LED_SETVAL(0);
  blink_pulses = 0;
  led_reset();
}

void led(uint8_t pulses, __BIT inf) {
  blink_pulses = 0;
  led_reset();
  blink_led = 1;
  blink_inf = inf;
  blink_pulses = pulses;
}

void led_err(uint8_t code) {
  led(code, 1);
  while (1) {
  };
}
