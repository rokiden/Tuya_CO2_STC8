#ifndef _ZM_H
#define _ZM_H

void zm_reset();
void zm_rx(uint8_t c);

__BIT zm_tx_ready();
uint8_t zm_tx();

#endif // _ZM_H
