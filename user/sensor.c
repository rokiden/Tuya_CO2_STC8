#include "fw_hal.h"

#include "led.h"
#include "sensor.h"

// ==== Value and filter ====

static __BIT value_ready;
static uint32_t __XDATA value, value_acc;
static uint8_t __DATA value_acc_num;

__BIT sensor_value_ready() { return value_ready; }
uint32_t sensor_value_pop() {
  value_ready = 0;
  return value;
}

inline void sensor_filter(uint32_t raw_val) {
  value_acc += raw_val;
  value_acc_num += 1;

  if (value_acc_num == 1 << SENSOR_FILTER_POW) {
    value = value_acc >> SENSOR_FILTER_POW;
    value_ready = 1;
    value_acc_num = 0;
    value_acc = 0;
  }
}

// ==== TX ====

static const uint8_t req[] = {0x42, 0x4d, 0xa0, 0x00, 0x03, 0x00, 0x00, 0x01, 0x32};

static __BIT tx_ready;
static uint8_t __DATA tx_sent;

inline void sensor_tx_reset() {
  tx_sent = 0;
  tx_ready = 0;
}

void sensor_tx_start() { tx_ready = 1; }

uint8_t sensor_tx_ready() { return tx_ready; }

uint8_t sensor_tx() {
  uint8_t c;
  c = req[tx_sent++];
  if (tx_sent == sizeof(req)) {
    tx_sent = 0;
    tx_ready = 0;
  }
  return c;
}

// ==== RX ====

#define RX_DATA_CAPACITY 5

enum rx_state_enum {
  RXS_HDR0,
  RXS_HDR1,
  RXS_INST,
  RXS_CMD0,
  RXS_CMD1,
  RXS_LEN0,
  RXS_LEN1,
  RXS_DATA,
  RXS_CS0,
  RXS_CS1
};

static uint8_t __DATA rx_state;
static uint16_t __DATA rx_cmd;
static uint16_t __DATA rx_len;
static uint16_t __DATA rx_csum;
static uint8_t __DATA rx_data_size;
static uint8_t __XDATA rx_data[RX_DATA_CAPACITY];

inline void sensor_rx_reset() { rx_state = RXS_HDR0; }

inline void sensor_rx_packet() {
  if (rx_cmd == 0x3 && rx_data_size == 5 && rx_data[4] == 0) {
    uint32_t data = 0;
    data += (uint32_t)rx_data[3] << 0;
    data += (uint32_t)rx_data[2] << 8;
    data += (uint32_t)rx_data[1] << 16;
    data += (uint32_t)rx_data[0] << 24;
    sensor_filter(data);
  }
}

void sensor_rx(uint8_t c) {
  switch (rx_state) {
  case RXS_HDR0:
    if (c == 0x42)
      rx_state = RXS_HDR1;
    rx_csum = 0;
    break;
  case RXS_HDR1:
    if (c == 0x4d)
      rx_state = RXS_INST;
    else
      sensor_rx_reset();
    break;
  case RXS_INST:
    if (c == 0xA0)
      rx_state = RXS_CMD0;
    else
      sensor_rx_reset();
    break;
  case RXS_CMD0:
    rx_cmd = c << 8;
    rx_state = RXS_CMD1;
    break;
  case RXS_CMD1:
    rx_cmd |= c;
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
      rx_state = RXS_CS0;
    rx_data_size = 0;
    break;
  case RXS_DATA:
    if (rx_data_size < RX_DATA_CAPACITY)
      rx_data[rx_data_size++] = c;
    else
      led_err(3);
    rx_len--;
    if (!rx_len)
      rx_state = RXS_CS0;
    break;
  case RXS_CS0:
    if (c != (rx_csum >> 8))
      led_err(4);
    else
      rx_state = RXS_CS1;
    break;
  case RXS_CS1:
    if (c != (rx_csum & 0xff))
      led_err(5);
    else
      sensor_rx_packet();
    sensor_rx_reset();
    break;
  }
  if (rx_state < RXS_CS0)
    rx_csum += c;
}

// ==== Common ====

void sensor_reset() {
  value_ready = 0;
  value_acc = 0;
  value_acc_num = 0;
  sensor_tx_reset();
  sensor_rx_reset();
}
