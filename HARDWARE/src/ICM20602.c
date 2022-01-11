#include "ICM20602.h"
#include "SPI.h"
#include "usart.h"

static 	float _accel_scale;
static	float _gyro_scale;
 
static uint8_t tx, rx;
static uint8_t tx_buff[14];

uint8_t icm20602_read_buffer(uint8_t const regAddr, uint8_t *pData, uint8_t len)
{
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_RESET);
  tx = regAddr | 0x80;
  tx_buff[0] = tx;
  HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 55);
  HAL_SPI_TransmitReceive(&hspi2, tx_buff, pData, len, 55);
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_SET);
  return 0;
}
 
 
 
uint8_t icm20602_write_reg(uint8_t reg,uint8_t val)
{
 
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_RESET);
	tx = reg & 0x7F;
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 55);
    tx = val;
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 55);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_SET);
 
	return 0;
}
 
 
uint8_t icm20602_read_reg(uint8_t reg)
{
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_RESET);
	tx = reg | 0x80;
	HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 55);
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 55);//���Ӧ������㷢һ��������
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin,GPIO_PIN_SET);//Դ���뷢�͵���ff
	
	return rx;
}
 
uint8_t icm20602_init()
{
	if(icm20602_write_reg(ICM20_PWR_MGMT_1,0x80))	//��λ����λ��λ0x41,˯��ģʽ��
	{
		
        printf("icm_20602 reset fail\r\n");
		return 1;
	}
	
	HAL_Delay(50);
	icm20602_write_reg(ICM20_PWR_MGMT_1,0x01);		//�ر�˯�ߣ��Զ�ѡ��ʱ��
	HAL_Delay(50);
	
	printf("icm_20602 id=%x\r\n",icm20602_read_reg(ICM20_WHO_AM_I));//��ȡID
	
	
	icm20602_write_reg(ICM20_SMPLRT_DIV,0);			//��Ƶ��=Ϊ0+1�������������Ϊ�ڲ���������
	icm20602_write_reg(ICM20_CONFIG,DLPF_BW_20);	//GYRO��ͨ�˲�����
	icm20602_write_reg(ICM20_ACCEL_CONFIG2,ACCEL_AVER_4|ACCEL_DLPF_BW_21);	//ACCEL��ͨ�˲�����
	
	
	icm20602_set_accel_fullscale(ICM20_ACCEL_FS_8G);
	icm20602_set_gyro_fullscale(ICM20_GYRO_FS_2000);
	
 
	HAL_Delay(100);
	printf("icm20602 init pass\r\n\r\n");
	
	return 0;
}
 
 
 
 
 
uint8_t icm20602_set_gyro_fullscale(uint8_t fs)
{
	switch(fs)
	{
		case ICM20_GYRO_FS_250:
			_gyro_scale = 1.0f/131.068f;	//32767/250
		break;
		case ICM20_GYRO_FS_500:
			_gyro_scale = 1.0f/65.534f;
		break;
		case ICM20_GYRO_FS_1000:
			_gyro_scale = 1.0f/32.767f;
		break;
		case ICM20_GYRO_FS_2000:
			_gyro_scale = 1.0f/16.4f;
		break;
		default:
			fs = ICM20_GYRO_FS_2000;
			_gyro_scale = 1.0f/16.3835f;
		break;
 
	}
	
	return icm20602_write_reg(ICM20_GYRO_CONFIG,fs);
	
}
 
 
uint8_t icm20602_set_accel_fullscale(uint8_t fs)
{
	switch(fs)
	{
		case ICM20_ACCEL_FS_2G:
			_accel_scale = 1.0f/16348.0f;
		break;
		case ICM20_ACCEL_FS_4G:
			_accel_scale = 1.0f/8192.0f;
		break;
		case ICM20_ACCEL_FS_8G:
			_accel_scale = 1.0f/4096.0f;
		break;
		case ICM20_ACCEL_FS_16G:
			_accel_scale = 1.0f/2048.0f;
		break;
		default:
			fs = ICM20_ACCEL_FS_8G;
			_accel_scale = 1.0f/4096.0f;
		break;
 
	}
	return icm20602_write_reg(ICM20_ACCEL_CONFIG,fs);
}
 
 
 
 
uint8_t icm20602_get_accel_adc(int16_t *accel)
{
	uint8_t buf[6];
	if(icm20602_read_buffer(ICM20_ACCEL_XOUT_H,buf,6))return 1;
	
	accel[0] = ((int16_t)buf[0]<<8) + buf[1];
	accel[1] = ((int16_t)buf[2]<<8) + buf[3];
	accel[2] = ((int16_t)buf[4]<<8) + buf[5];
	return 0;
}
 
 
 
uint8_t icm20602_get_gyro_adc(int16_t *gyro)
{
	uint8_t buf[6];
	if(icm20602_read_buffer(ICM20_GYRO_XOUT_H,buf,6))return 1;
	gyro[0] = (buf[0]<<8) + buf[1];
	gyro[1] = (buf[2]<<8) + buf[3];
	gyro[2] = (buf[4]<<8) + buf[5];
	return 0;
}
uint8_t icm20602_get_gyro(float *gyro)
{
	int16_t gyro_adc[3];
	if(icm20602_get_gyro_adc(gyro_adc))return 1;
	
	gyro[0] = _gyro_scale * gyro_adc[0];
	gyro[1] = _gyro_scale * gyro_adc[1];
	gyro[2] = _gyro_scale * gyro_adc[2];	
	return 0;
}
uint8_t icm20602_get_accel(float *accel)
{
	int16_t accel_adc[3];
	if(icm20602_get_accel_adc(accel_adc))return 1;
	accel[0] = _accel_scale * accel_adc[0];
	accel[1] = _accel_scale * accel_adc[1];
	accel[2] = _accel_scale * accel_adc[2];	
	return 0;
}
 
float icm20602_get_temp()
{
	int16_t temp_adc;
	uint8_t buf[2];
	if(icm20602_read_buffer(ICM20_TEMP_OUT_H,buf,2))return 0.0f;
 
	temp_adc = (buf[0]<<8)+buf[1];
 
	return (25.0f + (float)temp_adc/326.8f);
}

#include "BOARD.h"
#include "usart.h"
void Test_ICM20602(void)
{
    short gyro[3], accel[3];
    float temp;
    icm20602_init();
    while (1)
    {
        LED_RVS();
        icm20602_get_gyro_adc(gyro);
        icm20602_get_accel_adc(accel);
        temp = icm20602_get_temp();
        printf("%d %d %d\r\n",accel[0],accel[1],accel[2]);
        printf("%d %d %d\r\n",gyro[0],gyro[1],gyro[2]);
        printf("%.2f\r\n",temp);
        HAL_Delay(500);
    }
}
