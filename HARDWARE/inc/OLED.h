#ifndef __OLED_H
#define __OLED_H

#include "main.h"

//#define OLED091
//#define OLED096

#ifdef OLED091

#define OLED_RES_Pin GPIO_PIN_5
#define OLED_RES_GPIO_Port GPIOB

#define OLED_ADDR (0x78)
#define OLED_CMD  (0x00) //写命令
#define OLED_DATA (0x40) //写数据

#define LCD_SIZE_WIDTH (128)
#define LCD_SIZE_HEIGHT (32)
#define LCD_SIZE_COLUMNS (LCD_SIZE_WIDTH)
#define LCD_SIZE_PAGES (LCD_SIZE_HEIGHT / 8)

void OLED_WR_Byte(uint8_t dat, uint8_t mode);
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_SetBrightness(uint8_t brightness);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode);
void OLED_ShowChar6x8(uint8_t x, uint8_t y, uint8_t chr, uint8_t mode);
void OLED_ShowString(uint8_t x, uint8_t y, char chr[], uint8_t size1, uint8_t mode);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode);
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode);
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode);
void Test_OLED(void);
void OLED_Init(void);

#elif defined  OLED096

#define OLED_RES_Clr() HAL_GPIO_WritePin(SPI1_RES_GPIO_Port,SPI1_RES_Pin,GPIO_PIN_RESET)//RES
#define OLED_RES_Set() HAL_GPIO_WritePin(SPI1_RES_GPIO_Port,SPI1_RES_Pin,GPIO_PIN_SET)
#define OLED_DC_Clr()  HAL_GPIO_WritePin(SPI1_DC_GPIO_Port,SPI1_DC_Pin,GPIO_PIN_RESET)//DC
#define OLED_DC_Set()  HAL_GPIO_WritePin(SPI1_DC_GPIO_Port,SPI1_DC_Pin,GPIO_PIN_SET)
#define OLED_CS_Clr()  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET)//CS
#define OLED_CS_Set()  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET)

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
void OLED_Init(void);

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
void OLED_Fill(void);

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
void OLED_CLS(void);

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
void OLED_PutPixel(unsigned char x,unsigned char y);

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
void OLED_ClrDot(unsigned char x,unsigned char y);

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
void OLED_Rectangle(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char gif);

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
void OLED_P6x8Str(unsigned char x,unsigned char y, char ch[]);

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
void OLED_P8x16Str(unsigned char x,unsigned char y, char ch[]);

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
void OLED_P14x16Str(unsigned char x,unsigned char y,unsigned char ch[]);



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
void OLED_Show_BMP(unsigned char x0,unsigned char y0,unsigned char wide,unsigned char high,const unsigned char * bmp);

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
void OLED_Show_LQLogo(void);

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
void OLED_Show_Frame80(void);

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
void OLED_Show_Frame94(void);

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
void OLED_Road(unsigned short high, unsigned short wide, unsigned char *Pixle);

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
void OLED_WrCmd(unsigned char cmd);

/*!
  * @brief    OLED设置点
  *
  * @param    x ：坐标   
  * @param    y ：坐标
  *
  * @return   无
  *
  * @note     无
  *
  * @see      内部调用
  *
  * @date     2019/6/13 星期四
  */
void OLED_Set_Pos(unsigned char x, unsigned char y);
	
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
void OLED_Show_Dec(uint8_t x, uint8_t y, int32_t number,uint8_t len,uint8_t size);
void OLED_Show_Folat(uint8_t x, uint8_t y, float number,uint8_t len,uint8_t size);
void Test_OLED(void);
#endif

#endif
