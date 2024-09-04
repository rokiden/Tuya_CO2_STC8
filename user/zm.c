#include "fw_hal.h"

#include "led.h"
#include "zm.h"

inline void zm_rx_packet();

// ==== RX ====

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
    zm_rx_packet();
    zm_rx_reset();
    break;
  default:
    led_err(LED_ERR_ZM_RX_STATE);
  }
  rx_csum += c;
}

// ==== TX ====

enum tx_state_enum {
  TXS_HDR0,
  TXS_HDR1,
  TXS_VER,
  TXS_SEQ0,
  TXS_SEQ1,
  TXS_CMD,
  TXS_LEN0,
  TXS_LEN1,
  TXS_DATA,
  TXS_CS
};

static __BIT tx_en;
static uint8_t __DATA tx_cmd;
static uint16_t __DATA tx_len;
static uint8_t *tx_data;

static uint8_t __DATA tx_state;
static uint16_t __DATA tx_seq;
static uint8_t __DATA tx_data_sent;
static uint8_t __DATA tx_csum;

inline void zm_tx_reset() {
  tx_en = 0;
  tx_state = TXS_HDR0;
  tx_seq = 0;
}

__BIT zm_tx_ready() { return tx_en; }

uint8_t zm_tx() {
  uint8_t c;
  switch (tx_state) {
  case TXS_HDR0:
    c = 0x55;
    tx_state = TXS_HDR1;
    tx_csum = 0;
    tx_data_sent = 0;
    tx_seq += 1;
    if (tx_seq > 0xFFF0)
      tx_seq = 0;
    break;
  case TXS_HDR1:
    c = 0xAA;
    tx_state = TXS_VER;
    break;
  case TXS_VER:
    c = 0x02;
    tx_state = TXS_SEQ0;
    break;
  case TXS_SEQ0:
    c = tx_seq >> 8;
    tx_state = TXS_SEQ1;
    break;
  case TXS_SEQ1:
    c = tx_seq & 0xff;
    tx_state = TXS_CMD;
    break;
  case TXS_CMD:
    c = tx_cmd;
    tx_state = TXS_LEN0;
    break;
  case TXS_LEN0:
    c = tx_len >> 8;
    tx_state = TXS_LEN1;
    break;
  case TXS_LEN1:
    c = tx_len & 0xff;
    if (tx_len)
      tx_state = TXS_DATA;
    else
      tx_state = TXS_CS;
    rx_data_size = 0;
    break;
  case TXS_DATA:
    c = tx_data[tx_data_sent++];
    if (tx_len == tx_data_sent)
      tx_state = TXS_CS;
    break;
  case TXS_CS:
    c = tx_csum;
    tx_en = 0;
    tx_state = TXS_HDR0;
    break;
  default:
    led_err(LED_ERR_ZM_TX_STATE);
    c = 0;
  }
  tx_csum += c;
  return c;
}

inline void zm_tx_start(uint8_t cmd, uint8_t *data, uint16_t len) {
  tx_cmd = cmd;
  tx_data = data;
  tx_len = len;
  tx_en = 1;
}

// ==== Common ====

#define CMD_QPI 0x01
#define CMD_RNS 0x02
#define CMD_CZM 0x03
#define CMD_RSP 0x06

static uint8_t __DATA network_status;

static const uint8_t tx_data_prod_info[] = "{\"p\":\"ogkdpgy2\",\"v\":\"1.0.0\"}";

inline void zm_rx_packet() {
  switch (rx_cmd) {
  case CMD_QPI:
    if (tx_en)
      led_err(LED_ERR_ZM_PKT_BUSY);
    zm_tx_start(CMD_QPI, (uint8_t *)tx_data_prod_info, sizeof(tx_data_prod_info) - 1);
    break;
  case CMD_RNS:
    if (tx_en)
      led_err(LED_ERR_ZM_PKT_BUSY);
    if (rx_data_size != 1)
      led_err(LED_ERR_ZM_PKT_BAD);
    network_status = rx_data[0];
    zm_tx_start(CMD_RNS, 0, 0);
    break;
  }
}

__BIT zm_send_ready() { return !tx_en; }

static uint8_t __XDATA tx_data_value[8];
void zm_send_value(uint32_t val) {
  tx_data_value[4] = (val >> 24) & 0xff;
  tx_data_value[5] = (val >> 16) & 0xff;
  tx_data_value[6] = (val >> 8) & 0xff;
  tx_data_value[7] = (val >> 0) & 0xff;
  zm_tx_start(CMD_RSP, (uint8_t *)tx_data_value, sizeof(tx_data_value));
}

static const uint8_t tx_data_pair[] = {0x01};
void zm_send_pair() {
  network_status = ZM_STATUS_UNKNOWN;
  zm_tx_start(CMD_CZM, (uint8_t *)tx_data_pair, sizeof(tx_data_pair));
}

uint8_t zm_network_status() { return network_status; }

void zm_reset() {
  tx_data_value[0] = 0x02;
  tx_data_value[1] = 0x02;
  tx_data_value[2] = 0x00;
  tx_data_value[3] = 0x04;
  network_status = ZM_STATUS_UNKNOWN;
  zm_tx_reset();
  zm_rx_reset();
}
