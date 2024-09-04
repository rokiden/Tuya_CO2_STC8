#include "fw_hal.h"

#include "led.h"
#include "zm.h"

#define RX_DATA_CAPACITY 32

enum rx_state_enum {
  RXS_HDR0,
  RXS_HDR1,
  RXS_VER,
  RXS_SEQ0,
  RXS_SEQ1,
  RXS_CMD,
  RXS_LEN0,
  RXS_LEN1,
  RXS_DATA,
  RXS_CS
};

static uint8_t __DATA rx_state;
static uint8_t __DATA rx_cmd;
static uint16_t __DATA rx_len;
static uint8_t __DATA rx_csum;
static uint8_t __DATA rx_data_size;
static uint8_t __XDATA rx_data[RX_DATA_CAPACITY];

inline void zm_rx_reset() { rx_state = RXS_HDR0; }

void zm_rx(uint8_t c) {
  switch (rx_state) {
  case RXS_HDR0:
    if (c == 0x55)
      rx_state = RXS_HDR1;
    rx_csum = 0;
    break;
  case RXS_HDR1:
    if (c == 0xAA)
      rx_state = RXS_VER;
    else
      zm_rx_reset();
    break;
  case RXS_VER:
    if (c == 0x02)
      rx_state = RXS_SEQ0;
    else
      zm_rx_reset();
    break;
  case RXS_SEQ0:
    rx_state = RXS_SEQ1;
    break;
  case RXS_SEQ1:
    rx_state = RXS_CMD;
    break;
  case RXS_CMD:
    rx_cmd = c;
    rx_state = RXS_LEN0;
    break;
  case RXS_LEN0:
    rx_len = c << 8;
    rx_state = RXS_LEN1;
    break;
  case RXS_LEN1:
    rx_len |= c;
    if (rx_len)
      rx_state = RXS_DATA;
    else
      rx_state = RXS_CS;
    rx_data_size = 0;
    break;
  case RXS_DATA:
    if (rx_data_size < RX_DATA_CAPACITY)
      rx_data[rx_data_size++] = c;
    else
      led_err(LED_ERR_ZM_RX_DATA);
    rx_len--;
    if (!rx_len)
      rx_state = RXS_CS;
    break;
  case RXS_CS:
    if (c != rx_csum)
      led_err(LED_ERR_ZM_RX_CS);
    zm_rx_reset();
    break;
  default:
    led_err(LED_ERR_ZM_RX_STATE);
  }
  rx_csum += c;
}

void zm_reset() { zm_rx_reset(); }

__BIT zm_tx_ready(){
  return 0;
}
uint8_t zm_tx(){
  return 0;
}
