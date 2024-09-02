#ifndef _LED_H
#define _LED_H

#define LED_PORT 3
#define LED_PIN 7

#define INTRS_ON_OFF 10
#define INTRS_PAUSE 90

void led_init();
void led_intr();
void led_err(uint8_t code);
void led(uint8_t pulses, __BIT inf);

#endif // _LED_H
