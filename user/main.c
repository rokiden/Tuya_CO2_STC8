#include "fifo.h"
#include "fw_hal.h"
#include "led.h"
#include "sensor.h"
#include "zm.h"

/*
                             ┌─────────┐
                             │         │
                 Rxd  P1.0 ──┤ 1    16 ├── P3.7  Led
Zigbee ◄─► UART2             │         │
                 Txd  P1.1 ──┤ 2    15 ├── P3.6
                             │         │
                 Rxd  P1.6 ──┤ 3    14 ├── P3.5
Sensor ◄─► UART1             │         │
                 Txd  P1.7 ──┤ 4    13 ├── P3.4
                             │         │
                      P5.4 ──┤ 5    12 ├── P3.3
                             │         │
                           ──┤ 6    11 ├── P3.2
                             │         │
       Button to GND  P5.5 ──┤ 7    10 ├── P3.1  Txd
                             │         │             UART1 ◄─► ISP
                           ──┤ 8     9 ├── P3.0  Rxd
                             │         │
                             └─────────┘

*/

#define BUTTON P55
#define BUTTON_CONF()                                                                              \
  GPIO_P5_SetMode(GPIO_Pin_5, GPIO_Mode_Input_HIP);                                                \
  GPIO_SetPullUp(GPIO_Port_5, GPIO_Pin_5, HAL_State_ON)
#define BUTTON_DEBOUNCE 250

#define FIFO_U1_RX_size 32
#define FIFO_U2_RX_size 16

Fifo_init(FIFO_U1_RX);
Fifo_init(FIFO_U2_RX);

static volatile __BIT u1_tx_busy = 0;
static volatile __BIT u2_tx_busy = 0;

// check u1_tx_busy before call
inline void u1_tx_start(uint8_t c) {
  u1_tx_busy = 1;
  UART1_WriteBuffer(c);
}
inline void u2_tx_start(uint8_t c) {
  u2_tx_busy = 1;
  UART2_WriteBuffer(c);
}

INTERRUPT(UART1_Routine, EXTI_VectUART1) {
  uint8_t c;
  if (RI) {
    UART1_ClearRxInterrupt();
    c = SBUF;
    Fifo_push(FIFO_U1_RX, c);
  }
  if (TI) {
    UART1_ClearTxInterrupt();
    u1_tx_busy = 0;
  }
}

INTERRUPT(UART2_Routine, EXTI_VectUART2) {
  uint8_t c;
  if ((S2CON & (1 << 0))) { // S2RI
    UART2_ClearRxInterrupt();
    c = S2BUF;
    Fifo_push(FIFO_U2_RX, c);
  }
  if ((S2CON & (1 << 1))) { // S2TI
    UART2_ClearTxInterrupt();
    u2_tx_busy = 0;
  }
}

static volatile __BIT t0_tick = 0;

INTERRUPT(Timer0_Routine, EXTI_VectTimer0) { t0_tick = 1; }

int main(void) {
  // GPIO Led
  led_init();

  // GPIO Button
  BUTTON_CONF();

  // GPIO UART1
  GPIO_P1_SetMode(GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_InOut_QBD);

  // GPIO UART2
  GPIO_P1_SetMode(GPIO_Pin_0 | GPIO_Pin_1, GPIO_Mode_InOut_QBD);

  // UART1
  UART1_SwitchPort(UART1_AlterPort_P16_P17);
  UART1_Config8bitUart(UART1_BaudSource_Timer1, HAL_State_ON, 9600);
  UART1_SetRxState(HAL_State_ON);
  EXTI_UART1_SetIntState(HAL_State_ON);

  // UART2
  UART2_SwitchPort(UART2_AlterPort_P10_P11);
  UART2_Set8bitUART();
  UART2_Config(HAL_State_ON, 9600);
  UART2_SetRxState(HAL_State_ON);
  EXTI_UART2_SetIntState(HAL_State_ON);

  // Timer0
  TIM_Timer0_Config(HAL_State_OFF, TIM_TimerMode_16BitAuto, 100);
  EXTI_Timer0_SetIntState(HAL_State_ON);
  TIM_Timer0_SetRunState(HAL_State_ON);

  // Enable Interrupts
  EXTI_Global_SetIntState(HAL_State_ON);

  // Main
  uint8_t c;

  led(1, 1);

  uint8_t btn_debounce = 0;
  __BIT btn_ack = 0;

  zm_reset();
  sensor_reset();

  while (1) {
    if (t0_tick) {
      t0_tick = 0;
      led_intr();
      if (BUTTON == RESET) {
        if (btn_debounce < BUTTON_DEBOUNCE)
          btn_debounce++;
      } else {
        btn_debounce = 0;
        btn_ack = 0;
      }
    }
    if (Fifo_has_data(FIFO_U1_RX)) {
      Fifo_pop(FIFO_U1_RX, c);
      zm_rx(c);
      // led(1, 0);
    }
    if (zm_tx_ready() && !u1_tx_busy) {
      u1_tx_start(zm_tx());
    }
    if (Fifo_has_data(FIFO_U2_RX)) {
      Fifo_pop(FIFO_U2_RX, c);
      sensor_rx(c);
    }
    if (sensor_tx_ready() && !u2_tx_busy) {
      u2_tx_start(sensor_tx());
    }
    if (sensor_value_ready()) {
      sensor_value_pop();
    }
  }
}
