#ifndef _LED_H
#define _LED_H

#define LED_PORT 3
#define LED_PIN 7

#define LED_INTRS_ON_OFF 10
#define LED_INTRS_PAUSE 100

enum led_err_enum {
  LED_ERR_SENSOR_RX_DATA = 1,
  LED_ERR_SENSOR_RX_CS0,
  LED_ERR_SENSOR_RX_CS1,
  LED_ERR_SENSOR_RX_STATE,
  LED_ERR_ZM_RX_DATA,
  LED_ERR_ZM_RX_CS,
  LED_ERR_ZM_RX_STATE,
  LED_ERR_ZM_TX_STATE,
  LED_ERR_ZM_PKT_BUSY,
  LED_ERR_ZM_PKT_BAD,
};

void led_init();
void led_intr();
void led_err(uint8_t code);
void led(uint8_t pulses, __BIT inf);

#endif // _LED_H
