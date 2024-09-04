#ifndef _ZM_H
#define _ZM_H

#define ZM_STATUS_NOT_PAIRED 0x00
#define ZM_STATUS_PAIRED 0x01
#define ZM_STATUS_EXC 0x02
#define ZM_STATUS_BEING 0x03
#define ZM_STATUS_UNKNOWN 0xFF

void zm_rx(uint8_t c);

__BIT zm_tx_ready();
uint8_t zm_tx();

__BIT zm_send_ready();
void zm_send_value(uint32_t val);
void zm_send_pair();

uint8_t zm_network_status();
void zm_reset();

#endif // _ZM_H
