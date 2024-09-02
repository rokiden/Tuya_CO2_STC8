#ifndef _SENSOR_H
#define _SENSOR_H

#define SENSOR_FILTER_POW 7
#define SENSOR_VALUE_PERIOD 600
#define SENSOR_PERIOD ((uint16_t)SENSOR_VALUE_PERIOD * 100 / (1 << SENSOR_FILTER_POW))

__BIT sensor_value_ready();  // check new value available
uint32_t sensor_value_pop(); // take and invalidate

void sensor_tx_start();    // start request transmition
uint8_t sensor_tx_ready(); // check tx byte available
uint8_t sensor_tx();       // get tx byte

void sensor_rx(uint8_t c); // process received byte

void sensor_reset(); // reset state

#endif // _SENSOR_H
