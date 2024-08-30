#include "fw_hal.h"

void GPIO_Init(void)
{
    GPIO_P3_SetMode(GPIO_Pin_7, GPIO_Mode_InOut_OD);
    GPIO_SetPullUp(GPIO_Port_3, GPIO_Pin_7, HAL_State_ON);
}

int main(void)
{
    GPIO_Init();

    while(1)
    {
        P37 = RESET;
        SYS_Delay(500);
        P37 = SET;
        SYS_Delay(500);
    }
}
