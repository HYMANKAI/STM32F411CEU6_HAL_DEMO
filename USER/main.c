#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "main.h"
#include "BOARD.h"
#include "IIC.h"
#include "SPI.h"
#include "OLED.h"
#include "MPU6050.h"
#include "ICM20602.h"
#include "TIMER.h"

short AD_Value;
float Temperature;
float Vol_Value;

int main(void)
{   
    HAL_Init();
    
    SystemClock_Config();
    SYSTICK_init(100);
    
//	SCB->VTOR = FLASH_BASE | 0x4000; // ��������̿�ͷʱ�ӳ�ʼ��֮��
    
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_USART1_INIT(115200);					//��ʼ������
    // MX_TIM1_Init();
    
    while (true)
    {
        LED_RVS();
        HAL_Delay(500);
    }
}
//��ʱ��1�жϷ�����
void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}


//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&htim1))
    {
        LED_RVS();        //LED1��ת
    }
}
