#include "OLED.h"
#include "stdlib.h"
#include "oledfont.h"
#include "IIC.h"
#include "SPI.h"

#ifdef OLED091

uint8_t OLED_GRAM[LCD_SIZE_WIDTH+16][LCD_SIZE_PAGES];

//发送一个字节
//mode:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
	union
	{
		struct
		{
			uint8_t mode;
			uint8_t data;
		} SSD1306IIC;
		uint8_t sendbyte[2];
	} sendbyte;

	sendbyte.SSD1306IIC.mode = mode;
	sendbyte.SSD1306IIC.data = dat;

	HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, sendbyte.sendbyte, sizeof(sendbyte), 10);
}

// 反色显示
void OLED_ColorTurn(uint8_t i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xA6, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xA7, OLED_CMD);
	}
}

void OLED_SetBrightness(uint8_t brightness)
{
	union
	{
		struct
		{
			uint8_t mode;
			uint8_t data[2];
		} SSD1306IIC;
		uint8_t sendbyte[3];
	} sendbyte;

	sendbyte.SSD1306IIC.mode = OLED_ADDR;
	sendbyte.SSD1306IIC.data[0] = 0x81; /*contract control*/
	sendbyte.SSD1306IIC.data[1] = brightness;

	HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, sendbyte.sendbyte, sizeof(sendbyte), 10);
}

//屏幕旋转180度
void OLED_DisplayTurn(uint8_t i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xC8, OLED_CMD); //正常显示
		OLED_WR_Byte(0xA1, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xC0, OLED_CMD); //反转显示
		OLED_WR_Byte(0xA0, OLED_CMD);
	}
}

//开启OLED显示
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); //电荷泵使能
	OLED_WR_Byte(0x14, OLED_CMD); //开启电荷泵
	OLED_WR_Byte(0xAF, OLED_CMD); //点亮屏幕
}

//关闭OLED显示
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); //电荷泵使能
	OLED_WR_Byte(0x10, OLED_CMD); //关闭电荷泵
	OLED_WR_Byte(0xAE, OLED_CMD); //关闭屏幕
}

//更新显存到OLED
void OLED_Refresh(void)
{
	uint8_t data[128];
	HAL_StatusTypeDef status;
	uint8_t i, n;
	for (i = 0; i < 4; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00, OLED_CMD);	  //设置低列起始地址
		OLED_WR_Byte(0x10, OLED_CMD);	  //设置高列起始地址

		for (n = 0; n < 128; n++)
		{
			data[n] = OLED_GRAM[n][i];
		}

		status = HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x40, 1, (uint8_t *)data, sizeof(data), 50);
		if (status == HAL_BUSY)
		{
			// I2C BUSY
			HAL_I2C_Init(&hi2c1);
		}
		else if (status == HAL_ERROR)
		{
			// I2C ERROR
			//LED_Blink(30, 500);
		}
	}
}
//清屏函数
void OLED_Clear(void)
{
	uint8_t i, n;
	for (i = 0; i < 4; i++)
	{
		for (n = 0; n < 128; n++)
		{
			OLED_GRAM[n][i] = 0; //清除所有数据
		}
	}
	OLED_Refresh(); //更新显示
}

//画点
//x:0~127
//y:0~63
//t:1 填充 0,清空
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
	uint8_t i, m, n;
	i = y / 8;
	m = y % 8;
	n = 1 << m;
	if (t)
	{
		OLED_GRAM[x][i] |= n;
	}
	else
	{
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
		OLED_GRAM[x][i] |= n;
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
	}
}

//画线
//x1,y1:起点坐标
//x2,y2:结束坐标
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; //计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; //画线起点坐标
	uCol = y1;
	if (delta_x > 0)
		incx = 1; //设置单步方向
	else if (delta_x == 0)
		incx = 0; //垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; //水平线
	else
	{
		incy = -1;
		delta_y = -delta_x;
	}
	if (delta_x > delta_y)
		distance = delta_x; //选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		OLED_DrawPoint(uRow, uCol, mode); //画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
//x,y:圆心坐标
//r:圆的半径
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
	int a, b, num;
	a = 0;
	b = r;
	while (2 * b * b >= r * r)
	{
		OLED_DrawPoint(x + a, y - b, 1);
		OLED_DrawPoint(x - a, y - b, 1);
		OLED_DrawPoint(x - a, y + b, 1);
		OLED_DrawPoint(x + a, y + b, 1);

		OLED_DrawPoint(x + b, y + a, 1);
		OLED_DrawPoint(x + b, y - a, 1);
		OLED_DrawPoint(x - b, y - a, 1);
		OLED_DrawPoint(x - b, y + a, 1);

		a++;
		num = (a * a + b * b) - r * r; //计算画的点离圆心的距离
		if (num > 0)
		{
			b--;
			a--;
		}
	}
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size1:选择字体 6x8/6x12/8x16/12x24
//mode:0,反色显示;1,正常显示
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode)
{
	uint8_t i, m, temp, size2, chr1;
	uint8_t x0 = x, y0 = y;
	if (size1 == 8)
		size2 = 6;
	else
		size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2); //得到字体一个字符对应点阵集所占的字节数
	chr1 = chr - ' ';											   //计算偏移后的值
	for (i = 0; i < size2; i++)
	{
		if (size1 == 8)
		{
			temp = asc2_0806[chr1][i];
		} //调用0806字体
		else if (size1 == 12)
		{
			temp = asc2_1206[chr1][i];
		} //调用1206字体
		else if (size1 == 16)
		{
			temp = asc2_1608[chr1][i];
		} //调用1608字体
		else if (size1 == 24)
		{
			temp = asc2_2412[chr1][i];
		} //调用2412字体
		else
			return;
		for (m = 0; m < 8; m++)
		{
			if (temp & 0x01)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp >>= 1;
			y++;
		}
		x++;
		if ((size1 != 8) && ((x - x0) == size1 / 2))
		{
			x = x0;
			y0 = y0 + 8;
		}
		y = y0;
	}
}

//显示字符串
//x,y:起点坐标
//size1:字体大小
//*chr:字符串起始地址
//mode:0,反色显示;1,正常显示
void OLED_ShowString(uint8_t x, uint8_t y, char chr[], uint8_t size1, uint8_t mode)
{
	while ((*chr >= ' ') && (*chr <= '~')) //判断是不是非法字符!
	{
		OLED_ShowChar(x, y, *chr, size1, mode);
		if (size1 == 8)
			x += 6;
		else
			x += size1 / 2;
		chr++;
	}
}

//m^n
uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
	{
		result *= m;
	}
	return result;
}

//显示数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//mode:0,反色显示;1,正常显示
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode)
{
	uint8_t t, temp, m = 0;
	if (size1 == 8)
		m = 2;
	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10;
		if (temp == 0)
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
		}
		else
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
		}
	}
}

//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
//mode:0,反色显示;1,正常显示
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode)
{
	uint8_t m,temp;
	uint8_t x0=x,y0=y;
	uint16_t i,size3=(size1/8+((size1%8)?1:0))*size1;  //得到字体一个字符对应点阵集所占的字节数
	for(i=0;i<size3;i++)
	{
		if(size1==16)
				{temp=ZH_CN[num][i];}//调用16*16字体
//		else if(size1==24)
//				{temp=Hzk2[num][i];}//调用24*24字体
//		else if(size1==32)       
//				{temp=Hzk3[num][i];}//调用32*32字体
//		else if(size1==64)
//				{temp=Hzk4[num][i];}//调用64*64字体
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((x-x0)==size1)
		{x=x0;y0=y0+8;}
		y=y0;
	}
}

//x,y：起点坐标
//sizex,sizey,图片长宽
//BMP[]：要写入的图片数组
//mode:0,反色显示;1,正常显示
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode)
{
	uint16_t j = 0;
	uint8_t i, n, temp, m;
	uint8_t x0 = x, y0 = y;
	sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
	for (n = 0; n < sizey; n++)
	{
		for (i = 0; i < sizex; i++)
		{
			temp = BMP[j];
			j++;
			for (m = 0; m < 8; m++)
			{
				if (temp & 0x01)
					OLED_DrawPoint(x, y, mode);
				else
					OLED_DrawPoint(x, y, !mode);
				temp >>= 1;
				y++;
			}
			x++;
			if ((x - x0) == sizex)
			{
				x = x0;
				y0 = y0 + 8;
			}
			y = y0;
		}
	}
}
void Test_OLED(void)
{
    OLED_Init();
    OLED_SetBrightness(128);
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_ShowString(0,4,"HANYING",16,1);
    OLED_Refresh();
}
//OLED的初始化
void OLED_Init(void)
{
	HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_SET);
	HAL_Delay(50);
	
	OLED_WR_Byte(0xAE, OLED_CMD); /*display off*/
	OLED_WR_Byte(0x00, OLED_CMD); /*set lower column address*/
	OLED_WR_Byte(0x10, OLED_CMD); /*set higher column address*/
	OLED_WR_Byte(0x00, OLED_CMD); /*set display start line*/
	OLED_WR_Byte(0xB0, OLED_CMD); /*set page address*/
	OLED_WR_Byte(0x81, OLED_CMD); /*contract control*/
	OLED_WR_Byte(0xff, OLED_CMD); /*128*/
	OLED_WR_Byte(0xA1, OLED_CMD); /*set segment remap*/
	OLED_WR_Byte(0xA6, OLED_CMD); /*normal / reverse*/
	OLED_WR_Byte(0xA8, OLED_CMD); /*multiplex ratio*/
	OLED_WR_Byte(0x1F, OLED_CMD); /*duty = 1/32*/
	OLED_WR_Byte(0xC8, OLED_CMD); /*Com scan direction*/
	OLED_WR_Byte(0xD3, OLED_CMD); /*set display offset*/
	OLED_WR_Byte(0x00, OLED_CMD);
	OLED_WR_Byte(0xD5, OLED_CMD); /*set osc division*/
	OLED_WR_Byte(0x80, OLED_CMD);
	OLED_WR_Byte(0xD9, OLED_CMD); /*set pre-charge period*/
	OLED_WR_Byte(0x1f, OLED_CMD);
	OLED_WR_Byte(0xDA, OLED_CMD); /*set COM pins*/
	OLED_WR_Byte(0x00, OLED_CMD);
	OLED_WR_Byte(0xdb, OLED_CMD); /*set vcomh*/
	OLED_WR_Byte(0x40, OLED_CMD);
	OLED_WR_Byte(0x8d, OLED_CMD); /*set charge pump enable*/
	OLED_WR_Byte(0x14, OLED_CMD);
	OLED_Clear();
	OLED_WR_Byte(0xAF, OLED_CMD); /*display ON*/
}
#elif defined  OLED096

#define X_WIDTH 132
#define Y_WIDTH 64

/*!
* @brief    OLED初始化
*
* @param    无
*
* @return   无
*
* @note     如果修改管脚 需要修改初始化的管脚 
*
* @see      OLED_Init();
*
* @date     2019/6/13 星期四
*/
void OLED_Init(void)
{
  OLED_RES_Clr();
  HAL_Delay(200);
  OLED_RES_Set();

  OLED_WrCmd(0xae);//--turn off oled panel
  OLED_WrCmd(0x00);//---set low column address
  OLED_WrCmd(0x10);//---set high column address
  OLED_WrCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  OLED_WrCmd(0x81);//--set contrast control register
  OLED_WrCmd(0xcf); // Set SEG Output Current Brightness
  OLED_WrCmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
  OLED_WrCmd(0xc8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
  OLED_WrCmd(0xa6);//--set normal display
  OLED_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  OLED_WrCmd(0x3f);//--1/64 duty
  OLED_WrCmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  OLED_WrCmd(0x00);//-not offset
  OLED_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  OLED_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
  OLED_WrCmd(0xd9);//--set pre-charge period
  OLED_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  OLED_WrCmd(0xda);//--set com pins hardware configuration
  OLED_WrCmd(0x12);
  OLED_WrCmd(0xdb);//--set vcomh
  OLED_WrCmd(0x40);//Set VCOM Deselect Level
  OLED_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  OLED_WrCmd(0x02);//
  OLED_WrCmd(0x8d);//--set Charge Pump enable/disable
  OLED_WrCmd(0x14);//--set(0x10) disable
  OLED_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  OLED_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7)
  OLED_WrCmd(0xaf);//--turn on oled panel
  OLED_CLS();      //初始清屏
  OLED_Set_Pos(0,0);
  
} 

/*!
* @brief    OLED画点命令
*
* @param    data ：要画的点
*
* @return   无
*
* @note     无
*
* @see      内部调用
*
* @date     2019/6/13 星期四
*/
void OLED_WrDat(unsigned char data)
{
    OLED_DC_Set();
    OLED_CS_Clr();
    HAL_SPI_Transmit(&hspi1,&data,1,200);
    OLED_CS_Set();
    OLED_DC_Set();
}

/*!
* @brief    OLED命令
*
* @param    cmd ：命令
*
* @return   无
*
* @note     无
*
* @see      内部调用
*
* @date     2019/6/13 星期四
*/
void OLED_WrCmd(unsigned char cmd)
{
    OLED_DC_Clr();
    OLED_CS_Clr();
    HAL_SPI_Transmit(&hspi1,&cmd,1,200);
    OLED_CS_Set();
    OLED_DC_Set();
}

/*!
* @brief    OLED设置点
*
* @param    x ：坐标   
* @param    y ：坐标
*
* @return   无
*
* @note     左上角为0,0  右下角 127,63
*
* @see      内部调用
*
* @date     2019/6/13 星期四
*/
void OLED_Set_Pos(unsigned char x, unsigned char y)
{ 
  OLED_WrCmd(0xb0+y);
  OLED_WrCmd(((x&0xf0)>>4)|0x10);
  OLED_WrCmd((x&0x0f));
} 

/*!
* @brief    OLED全亮
*
* @param    无   
*
* @return   无
*
* @note     无
*
* @see      OLED_Fill(); //OLED全亮 用于检测坏点
*
* @date     2019/6/13 星期四
*/
void OLED_Fill(void)
{
  unsigned char y,x;
  
  for(y=0;y<8;y++)
  {
    OLED_WrCmd(0xb0+y);
    OLED_WrCmd(0x01);
    OLED_WrCmd(0x10);
    for(x=0;x<X_WIDTH;x++)
      OLED_WrDat(0xff);
  }
}

/*!
* @brief    OLED清屏
*
* @param    无  
*
* @return   无
*
* @note     无
*
* @see      OLED_CLS(); //清屏
*
* @date     2019/6/13 星期四
*/
void OLED_CLS(void)
{
  unsigned char y,x;	
  for(y=0;y<8;y++)
  {
    OLED_WrCmd(0xb0+y);
    OLED_WrCmd(0x01);
    OLED_WrCmd(0x10);
    for(x=0;x<X_WIDTH;x++)
      OLED_WrDat(0);
  }
}



/*!
* @brief    画一个点
*
* @param    x  : 0-127 
* @param    y  : 0-63
*
* @return   无
*
* @note     OLED是按字节显示的 一个字节8位，代表8个纵向像素点，
* @note     因此如果画点 OLED_PutPixel(0,0); OLED_PutPixel(0,1); 只会显示点(0, 1) (0,0)则在画点(0,1)时清除掉
*
* @see      OLED_PutPixel(0, 0); //画一个点 左上角为0,0  右下角 127,63
*
* @date     2019/6/13 星期四
*/
void OLED_PutPixel(unsigned char x,unsigned char y)
{
  unsigned char data1;  //data1当前点的数据 
  
  OLED_Set_Pos(x,(unsigned char)(y>>3));
  data1 =(unsigned char)(0x01<<(y%8)); 	
  OLED_WrCmd((unsigned char)(0xb0+(y>>3)));
  OLED_WrCmd((unsigned char)(((x&0xf0)>>4)|0x10));
  OLED_WrCmd((unsigned char)((x&0x0f)|0x00));
  OLED_WrDat(data1);
}

/*!
* @brief    清除8个点
*
* @param    x  : 0-127
* @param    y  : 0-63
*
* @return   无
*
* @note     OLED是按字节显示的 一个字节8位，代表8个纵向像素点，
*
* @see      OLED_ClrDot(0, 0); //清除(0,0)-(0,7)8个点 左上角为0,0  右下角 127,63
*
* @date     2019/6/13 星期四
*/
void OLED_ClrDot(unsigned char x,unsigned char y)
{
  OLED_Set_Pos(x,(unsigned char)(y>>3));
  OLED_WrCmd((unsigned char)(0xb0+(y>>3)));
  OLED_WrCmd((unsigned char)(((x&0xf0)>>4)|0x10));
  OLED_WrCmd((unsigned char)((x&0x0f)|0x00));
  OLED_WrDat(0x00);
} 


/*!
* @brief    画矩形
*
* @param    x1  : 0-127   左上角坐标（x1,y1）
* @param    y1  : 0-63
* @param    x2  : 0-127   右下角坐标（x2，y2）
* @param    y2  : 0-63
* @param    gif : 是否开启动画效果
*
* @return   无
*
* @note     无
*
* @see      OLED_Rectangle(0, 0, 127,62,0);   //绘制矩形
*
* @date     2019/6/13 星期四
*/
void OLED_Rectangle(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char gif)
{	
  unsigned char n; 
  OLED_Set_Pos(x1,y1>>3);
  for(n=x1;n<=x2;n++)
  {
    OLED_WrDat(0x01<<(y1%8));
    if(gif == 1) 	
        HAL_Delay(10);
  }  
  OLED_Set_Pos(x1,y2>>3);
  for(n=x1;n<=x2;n++)
  {
    OLED_WrDat(0x01<<(y2%8));
    if(gif == 1) 	
        HAL_Delay(10);
  }
  
  for(n = y1>>3; n <= y2>>3; n++)
  {
    OLED_Set_Pos(x1, n);
    OLED_WrDat(0xff);
    if(gif == 1) 	
        HAL_Delay(50);
  }
  
  for(n = y1>>3; n <= y2>>3; n++)
  {
    OLED_Set_Pos(x2, n);
    OLED_WrDat(0xff);
    if(gif == 1) 	
        HAL_Delay(50);
  }
}  


/*!
* @brief    写入一组标准ASCII字符串
*
* @param    x  : 0-127   
* @param    y  : 0-7
* @param    ch : 要显示的字符串
*
* @return   无
*
* @note     无
*
* @see      OLED_P6x8Str(0,0,"OLED 3.");  //显示字符串
*
* @date     2019/6/13 星期四
*/
void OLED_P6x8Str(unsigned char x,unsigned char y, char ch[])
{
  unsigned char c=0,i=0,j=0;      
  while (ch[j]!='\0')
  {    
    c =ch[j]-32;
    if(x>126){x=0;y++;}
    OLED_Set_Pos(x,y);
    for(i=0;i<6;i++)     
      OLED_WrDat(F6x8[c][i]);
    x+=6;
    j++;
  }
}


/*!
* @brief    写入一组标准ASCII字符串
*
* @param    x  : 0-127   
* @param    y  : 0-6
* @param    ch : 要显示的字符串
*
* @return   无
*
* @note     一个字符高16  OLED高度64  
*
* @see      OLED_P8x16Str(0,0,"OLED 3.");  //显示字符串
*
* @date     2019/6/13 星期四
*/
void OLED_P8x16Str(unsigned char x,unsigned char y, char ch[])
{
  unsigned char c=0,i=0,j=0;
  
  while (ch[j]!='\0')
  {    
    c =ch[j]-32;
    if(x>120){x=0;y++;}
    OLED_Set_Pos(x,y);
    for(i=0;i<8;i++)     
      OLED_WrDat(F8X16[c*16+i]);
    OLED_Set_Pos(x,y+1);
    for(i=0;i<8;i++)     
      OLED_WrDat(F8X16[c*16+i+8]);
    x+=8;
    j++;
  }
}

/*!
* @brief    输出汉字字符串
*
* @param    x  : 0-127   
* @param    y  : 0-6
* @param    ch : 要显示的字符串
*
* @return   无
*
* @note     一个字符高16  OLED高度64   字库需要自己制作
*
* @see      OLED_P14x16Str(0,4,"寒樱散华");   //显示汉字
*
* @date     2019/6/13 星期四
*/
void OLED_P14x16Str(unsigned char x,unsigned char y, unsigned char ch[])
{
  unsigned char wm=0,ii = 0;
  unsigned int adder=1; 
  
  while(ch[ii] != '\0')
  {
    wm = 0;
    adder = 1;
    while(F14x16_Idx[wm] > 127)
    {
      if(F14x16_Idx[wm] == (unsigned char)ch[ii])
      {
        if(F14x16_Idx[wm + 1] == ch[ii + 1])
        {
          adder = wm * 14;
          break;
        }
      }
      wm += 2;			
    }
    if(x>118){x=0;y++;}
    OLED_Set_Pos(x , y);
    if(adder != 1)// 显示汉字					
    {
      OLED_Set_Pos(x , y);
      for(wm = 0;wm < 14;wm++)               
      {
        OLED_WrDat(F14x16[adder]);
        adder += 1;
      }      
      OLED_Set_Pos(x,y + 1);
      for(wm = 0;wm < 14;wm++)          
      {
        OLED_WrDat(F14x16[adder]);
        adder += 1;
      }   		
    }
    else			  //显示空白字符			
    {
      ii += 1;
      OLED_Set_Pos(x,y);
      for(wm = 0;wm < 16;wm++)
      {
        OLED_WrDat(0);
      }
      OLED_Set_Pos(x,y + 1);
      for(wm = 0;wm < 16;wm++)
      {   		
        OLED_WrDat(0);
      }
    }
    x += 14;
    ii += 2;
  }
}



/*!
* @brief    显示BMP图片 最大 128×64
*
* @param    x0  : 0-127   左上角坐标（x0,y0）
* @param    y0  : 0-63
* @param    wide: 1-127   图片宽度
* @param    high: 1-64    图片高度
* @param    bmp : 要显示的图片
*
* @return   无
*
* @note     图片需要用软件转化
*
* @see      OLED_Show_BMP(0, 0, 104, 64, longqiu104x64); //画龙邱loge
*
* @date     2019/6/13 星期四
*/
void OLED_Show_BMP(unsigned char x0,unsigned char y0,unsigned char wide,unsigned char high,const unsigned char * bmp)
{ 	
  unsigned int ii=0;
  unsigned char x,y;
  unsigned char x1,y1;
  if(wide + x0 > 128 || high + y0 > 64)
  {
    return;  //超出显示范围
  }
  y1 = (y0+high-1)/8;
  x1 = x0 + wide;
  for(y=y0/8;y<=y1;y++)
  {
    OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
    {      
      OLED_WrDat(bmp[ii++]);
    }
  }
}

/*!
* @brief    显示龙邱图片128×64
*
* @param    无 
*
* @return   无
*
* @note     无
*
* @see      OLED_Show_LQLogo();
*
* @date     2019/6/13 星期四
*/
void OLED_Show_LQLogo(void)
{ 	
  unsigned int ii=0;
  unsigned char x,y;       
  
  for(y=0;y<8;y++)
  {
    OLED_Set_Pos(16,y);
    for(x=14;x<118;x++)
    {      
      OLED_WrDat(longqiu104x64[ii++]);
    }
  }
}




/*!
* @brief    画边线 OV7725显示图像时 可以用这个画边线
*
* @param    无 
*
* @return   无
*
* @note     无
*
* @see      OLED_Show_Frame80();
*
* @date     2019/6/13 星期四
*/
void OLED_Show_Frame80(void)
{ 	
  OLED_Set_Pos(23,1);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,2);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,3);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,4);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,5);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,6);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(23,7);
  OLED_WrDat(0xFF);
  
  OLED_Set_Pos(104,1);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,2);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,3);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,4);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,5);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,6);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(104,7);
  OLED_WrDat(0xFF);
}

/*!
* @brief    画边线 MT9V034显示图像时 可以用这个画边线
*
* @param    无 
*
* @return   无
*
* @note     无
*
* @see      OLED_Show_Frame94();
*
* @date     2019/6/13 星期四
*/
void OLED_Show_Frame94(void)
{ 	
  //OLED_Set_Pos(13,0);//第0行，第13列
  //OLED_WrDat(0xFF); //画竖线
  OLED_Set_Pos(16,1);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,2);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,3);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,4);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,5);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,6);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(16,7);
  OLED_WrDat(0xFF);
  
  //OLED_Set_Pos(114,0);
  //OLED_WrDat(0xFF);
  OLED_Set_Pos(111,1);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,2);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,3);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,4);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,5);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,6);
  OLED_WrDat(0xFF);
  OLED_Set_Pos(111,7);
  OLED_WrDat(0xFF);
}

/**
* @brief    OLED 二值化图像显示
*
* @param    high ： 显示图像高度
* @param    wide ： 显示图像宽度
* @param    Pixle： 显示图像数据地址
*
* @return   无
*
* @note     无
*
* @see      OLED_Road(OLEDH, OLEDW, (unsigned char *)Pixle);
*
* @date     2019/6/25 星期二
*/
void OLED_Road(unsigned short high, unsigned short wide, unsigned char *Pixle)
{ 	 
  unsigned char i = 0, j = 0,temp=0;
  unsigned char wide_start = (128 - wide)/2;
  for(i=0;i<high-7;i+=8)
  {
    
    OLED_Set_Pos(wide_start,i/8+1);
    
    for(j=0;j<wide;j++) 
    { 
      temp=0;
      if(Pixle[(0+i)*wide + j]) temp|=1;
      if(Pixle[(1+i)*wide + j]) temp|=2;
      if(Pixle[(2+i)*wide + j]) temp|=4;
      if(Pixle[(3+i)*wide + j]) temp|=8;
      if(Pixle[(4+i)*wide + j]) temp|=0x10;
      if(Pixle[(5+i)*wide + j]) temp|=0x20;
      if(Pixle[(6+i)*wide + j]) temp|=0x40;
      if(Pixle[(7+i)*wide + j]) temp|=0x80;
      OLED_WrDat(temp);
    }
  }  
}


/*******************************************************************************
 *  函数名称：OLED_DispDecAt(OLED_FONT font,uint8_t line,uint8_t x,int32_t number,uint8_t len)
 *  功能说明：OLED显示一个的整形数字
 *  参数说明：OLED_FONT font：字体（OLED_FONT枚举）
 uint8_t y：行位置（0-7）
 uint8_t x   ：列位置（0-127）
 int32_t number：要显示的数字
 uint8_t len   ：显示的位数
 *  函数返回：无
 *  使用示例：
 ********************************************************************************/
void OLED_Show_Dec(uint8_t x, uint8_t y, int32_t number,
                    uint8_t len,uint8_t size)
{
    char buf[30];
    int8_t p, flag = 0;
    uint32_t temp;
    if (number < 0)
    {
        number = -number;
        flag = 1;
    }
    temp = 10;
    for (p = len - 1; p >= 0; p--)
    {
        buf[p] = ((number % temp) / (temp / 10)) + '0';
        if (number < temp)
            break;
        temp = temp * 10;
    }
    if (flag)
    {
        buf[--p] = '-';
    }

    while (p-- > 0)
    {
        buf[p] = ' ';
    }

    buf[len] = 0;

      if(size == 1)
			{
        OLED_P6x8Str(x, y, buf);
			}
			if(size == 2)
			{
        OLED_P8x16Str(x, y, buf);
			}
}
/*******************************************************************************
 *  函数名称：OLED_DispFolatAt(OLED_FONT font,uint8_t line,uint8_t x,float number,uint8_t len)
 *  功能说明：OLED显示一个的浮点数
 *  参数说明：OLED_FONT font：字体（OLED_FONT枚举）
 uint8_t line：行位置（0-7）
 uint8_t x   ：列位置（0-127）
 float number：要显示的浮点数
 uint8_t len ：显示的位数(不计算小数点)
 uint8_t      : 数字尺寸
 *  函数返回：无
 *  使用示例：
 ********************************************************************************/
void OLED_Show_Folat(uint8_t x, uint8_t y, float number,
                      uint8_t len,uint8_t size)
{
    const float _fround[6] = { 0.49, 0.05, 0.005, 0.0005, 0.00005, 0.000005 };
    unsigned char i = 0, d;
    char Str[20];
    char *str = Str;
    float scale = 1.0;

    if (number < 0.0f)
    {
        number = -number;
        *str++ = '-';
    };
    if (len > 5)
        len = 5;
    number = number + _fround[len];
    while (number >= scale)
    {
        scale = scale * 10.0f;
        ++i;
    };
    if (i == 0)
        *str++ = '0';
    else
    {
        while (i--)
        {
            scale = (float) ((int) (0.5f + scale / 10.0f));
            d = (unsigned char) (number / scale);
            *str++ = d + '0';
            number = number - scale * d;
        };
    }
    if (len == 0)
    {
        *str = 0;
			if(size == 1)
			{
        OLED_P6x8Str(x, y, Str);
			}
			if(size == 2)
			{
        OLED_P8x16Str(x, y, Str);
			}
        return;
    };
    *str++ = '.';
    while (len--)
    {
        number = number * 10.0f;
        d = (unsigned char) number;
        *str++ = d + '0';
        number = number - d;
    }
    *str = 0;
    	if(size == 1)
			{
        OLED_P6x8Str(x, y, Str);
			}
			if(size == 2)
			{
        OLED_P8x16Str(x, y, Str);
			}

}


/**
* @brief    OLED 测试函数
*
* @param    无
*
* @return   无
*
* @note     无
*
* @example  Test_OLED();
*
* @date     2019/6/25 星期二
*/
void Test_OLED(void)
{      
  OLED_Init();                          //OLED初始化
  OLED_Show_LQLogo();                   //显示LOGO
  HAL_Delay(2000);
  OLED_CLS();
  
  while (1)
  {  
    OLED_P6x8Str(0,7,"OLED 3.");         //字符串
    HAL_Delay(1000);
    OLED_Rectangle(0, 0, 127,62,0);      //绘制矩形
    OLED_Rectangle(24,8,103,55,1);
    OLED_Rectangle(32,16,80,47,1);
    OLED_Rectangle(40,24,72,39,1);
    HAL_Delay(1000);
    OLED_CLS();                          //清屏
    
    OLED_P8x16Str(0, 0,"OLED 4.");       //显示字符串
    OLED_P14x16Str(0,4,(unsigned char*)"寒樱散华");      //显示汉字
    OLED_PutPixel(120,60);
    HAL_Delay(1000);
    OLED_CLS();                          //清屏
    
    OLED_Fill();                         //填充
    HAL_Delay(1000);
    OLED_CLS();                          //清屏
    //延时
    HAL_Delay(50);
  }
}

#endif
