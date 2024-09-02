#include "fw_hal.h"

#include "led.h"
#include "sensor.h"

static uint8_t __CODE req[] = {0x42, 0x4d, 0xa0, 0x00, 0x03,
                               0x00, 0x00, 0x01, 0x32};

static __BIT tx_ready = 1;
static uint8_t tx_sent;

uint8_t sensor_tx_ready() { return tx_ready; }

uint8_t sensor_tx() {
  uint8_t c;
  c = req[tx_sent++];
  if (tx_sent == sizeof(req))
    tx_ready = 0;
  return c;
}
