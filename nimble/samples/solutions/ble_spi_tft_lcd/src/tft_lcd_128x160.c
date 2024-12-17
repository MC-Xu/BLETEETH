
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "tft_lcd_128x160.h"
#include "PanSeries.h"
#include "pan_gpio.h"
#include "pan_sys.h"

#include "pan_hal.h"

const uint8_t english_6x12_text[] = {
	0x00,0x00,0x0E,0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00,0x00,/*"0",0*/
	0x00,0x00,0x04,0x06,0x04,0x04,0x04,0x04,0x04,0x0E,0x00,0x00,/*"1",1*/
	0x00,0x00,0x0E,0x11,0x11,0x08,0x04,0x02,0x01,0x1F,0x00,0x00,/*"2",2*/
	0x00,0x00,0x0E,0x11,0x10,0x0C,0x10,0x10,0x11,0x0E,0x00,0x00,/*"3",3*/
	0x00,0x00,0x08,0x0C,0x0C,0x0A,0x09,0x1F,0x08,0x1C,0x00,0x00,/*"4",4*/
	0x00,0x00,0x1F,0x01,0x01,0x0F,0x11,0x10,0x11,0x0E,0x00,0x00,/*"5",5*/
	0x00,0x00,0x0C,0x12,0x01,0x0D,0x13,0x11,0x11,0x0E,0x00,0x00,/*"6",6*/
	0x00,0x00,0x1E,0x10,0x08,0x08,0x04,0x04,0x04,0x04,0x00,0x00,/*"7",7*/
	0x00,0x00,0x0E,0x11,0x11,0x0E,0x11,0x11,0x11,0x0E,0x00,0x00,/*"8",8*/
	0x00,0x00,0x0E,0x11,0x11,0x19,0x16,0x10,0x09,0x06,0x00,0x00,/*"9",9*/
	0x00,0x00,0x04,0x04,0x0C,0x0A,0x0A,0x1E,0x12,0x33,0x00,0x00,/*"A",10*/
	0x00,0x00,0x0F,0x12,0x12,0x0E,0x12,0x12,0x12,0x0F,0x00,0x00,/*"B",11*/
	0x00,0x00,0x1E,0x11,0x01,0x01,0x01,0x01,0x11,0x0E,0x00,0x00,/*"C",12*/
	0x00,0x00,0x0F,0x12,0x12,0x12,0x12,0x12,0x12,0x0F,0x00,0x00,/*"D",13*/
	0x00,0x00,0x1F,0x12,0x0A,0x0E,0x0A,0x02,0x12,0x1F,0x00,0x00,/*"E",14*/
	0x00,0x00,0x1F,0x12,0x0A,0x0E,0x0A,0x02,0x02,0x07,0x00,0x00,/*"F",15*/
	0x00,0x00,0x1C,0x12,0x01,0x01,0x39,0x11,0x12,0x0C,0x00,0x00,/*"G",16*/
	0x00,0x00,0x33,0x12,0x12,0x1E,0x12,0x12,0x12,0x33,0x00,0x00,/*"H",17*/
	0x00,0x00,0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x1F,0x00,0x00,/*"I",18*/
	0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x09,0x07,/*"J",19*/
	0x00,0x00,0x37,0x12,0x0A,0x06,0x0A,0x12,0x12,0x37,0x00,0x00,/*"K",20*/
	0x00,0x00,0x07,0x02,0x02,0x02,0x02,0x02,0x22,0x3F,0x00,0x00,/*"L",21*/
	0x00,0x00,0x3B,0x1B,0x1B,0x1B,0x15,0x15,0x15,0x35,0x00,0x00,/*"M",22*/
	0x00,0x00,0x3B,0x12,0x16,0x16,0x1A,0x1A,0x12,0x17,0x00,0x00,/*"N",23*/
	0x00,0x00,0x0E,0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00,0x00,/*"O",24*/
	0x00,0x00,0x0F,0x12,0x12,0x0E,0x02,0x02,0x02,0x07,0x00,0x00,/*"P",25*/
	0x00,0x00,0x0E,0x11,0x11,0x11,0x11,0x17,0x19,0x0E,0x18,0x00,/*"Q",26*/
	0x00,0x00,0x0F,0x12,0x12,0x0E,0x0A,0x12,0x12,0x37,0x00,0x00,/*"R",27*/
	0x00,0x00,0x1E,0x11,0x01,0x06,0x08,0x10,0x11,0x0F,0x00,0x00,/*"S",28*/
	0x00,0x00,0x1F,0x15,0x04,0x04,0x04,0x04,0x04,0x0E,0x00,0x00,/*"T",29*/
	0x00,0x00,0x33,0x12,0x12,0x12,0x12,0x12,0x12,0x0C,0x00,0x00,/*"U",30*/
	0x00,0x00,0x33,0x12,0x12,0x0A,0x0A,0x0C,0x04,0x04,0x00,0x00,/*"V",31*/
	0x00,0x00,0x15,0x15,0x15,0x15,0x0E,0x0A,0x0A,0x0A,0x00,0x00,/*"W",32*/
	0x00,0x00,0x1B,0x0A,0x0A,0x04,0x04,0x0A,0x0A,0x1B,0x00,0x00,/*"X",33*/
	0x00,0x00,0x1B,0x0A,0x0A,0x0A,0x04,0x04,0x04,0x0E,0x00,0x00,/*"Y",34*/
	0x00,0x00,0x1F,0x09,0x08,0x04,0x04,0x02,0x12,0x1F,0x00,0x00,/*"Z",35*/
	0x00,0x00,0x00,0x00,0x00,0x0C,0x12,0x1C,0x12,0x3C,0x00,0x00,/*"a",36*/
	0x00,0x03,0x02,0x02,0x02,0x0E,0x12,0x12,0x12,0x0E,0x00,0x00,/*"b",37*/
	0x00,0x00,0x00,0x00,0x00,0x1C,0x12,0x02,0x12,0x0C,0x00,0x00,/*"c",38*/
	0x00,0x18,0x10,0x10,0x10,0x1C,0x12,0x12,0x12,0x3C,0x00,0x00,/*"d",39*/
	0x00,0x00,0x00,0x00,0x00,0x0C,0x12,0x1E,0x02,0x1C,0x00,0x00,/*"e",40*/
	0x00,0x18,0x24,0x04,0x04,0x1E,0x04,0x04,0x04,0x1E,0x00,0x00,/*"f",41*/
	0x00,0x00,0x00,0x00,0x00,0x3C,0x12,0x0C,0x02,0x1C,0x22,0x1C,/*"g",42*/
	0x00,0x03,0x02,0x02,0x02,0x0E,0x12,0x12,0x12,0x37,0x00,0x00,/*"h",43*/
	0x00,0x04,0x04,0x00,0x00,0x06,0x04,0x04,0x04,0x0E,0x00,0x00,/*"i",44*/
	0x00,0x08,0x08,0x00,0x00,0x0C,0x08,0x08,0x08,0x08,0x08,0x07,/*"j",45*/
	0x00,0x03,0x02,0x02,0x02,0x1A,0x0A,0x06,0x0A,0x13,0x00,0x00,/*"k",46*/
	0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x1F,0x00,0x00,/*"l",47*/
	0x00,0x00,0x00,0x00,0x00,0x0F,0x15,0x15,0x15,0x15,0x00,0x00,/*"m",48*/
	0x00,0x00,0x00,0x00,0x00,0x0F,0x12,0x12,0x12,0x37,0x00,0x00,/*"n",49*/
	0x00,0x00,0x00,0x00,0x00,0x0C,0x12,0x12,0x12,0x0C,0x00,0x00,/*"o",50*/
	0x00,0x00,0x00,0x00,0x00,0x0F,0x12,0x12,0x12,0x0E,0x02,0x07,/*"p",51*/
	0x00,0x00,0x00,0x00,0x00,0x1C,0x12,0x12,0x12,0x1C,0x10,0x38,/*"q",52*/
	0x00,0x00,0x00,0x00,0x00,0x1B,0x06,0x02,0x02,0x07,0x00,0x00,/*"r",53*/
	0x00,0x00,0x00,0x00,0x00,0x1E,0x02,0x0C,0x10,0x1E,0x00,0x00,/*"s",54*/
	0x00,0x00,0x00,0x04,0x04,0x1E,0x04,0x04,0x04,0x1C,0x00,0x00,/*"t",55*/
	0x00,0x00,0x00,0x00,0x00,0x1B,0x12,0x12,0x12,0x3C,0x00,0x00,/*"u",56*/
	0x00,0x00,0x00,0x00,0x00,0x1B,0x0A,0x0A,0x04,0x04,0x00,0x00,/*"v",57*/
	0x00,0x00,0x00,0x00,0x00,0x15,0x15,0x0E,0x0A,0x0A,0x00,0x00,/*"w",58*/
	0x00,0x00,0x00,0x00,0x00,0x1B,0x0A,0x04,0x0A,0x1B,0x00,0x00,/*"x",59*/
	0x00,0x00,0x00,0x00,0x00,0x33,0x12,0x12,0x0C,0x08,0x04,0x03,/*"y",60*/
	0x00,0x00,0x00,0x00,0x00,0x1E,0x08,0x04,0x04,0x1E,0x00,0x00,/*"z",61*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,/*".",62*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,0x00,0x00,/*"-",63*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,/*"_",64*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*" ",65*/
	0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x04,0x00,0x00,/*":",66*/
	0x1C,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x1C,0x00,/*"[",67*/
	0x0E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0E,0x00,/*"]",68*/
	0x00,0x00,0x00,0x04,0x15,0x0E,0x0E,0x15,0x04,0x00,0x00,0x00/*"*",69*/
};

//string_text_t string_display_text;
/**
 *******************************************************************************
 ** \addtogroup TFT_LCD_128x160
 ******************************************************************************/
//@{

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/



/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef IP_101x
#define LCD_CLK_HIGH() 		(P14 = 1)//CLK
#define LCD_CLK_LOW() 		(P14 = 0)

#define LCD_DO_HIGH() 		(P11 = 1)//DOUT
#define LCD_DO_LOW() 		(P11 = 0)

#define LCD_CS_HIGH()  		(P15 = 1)//CS
#define LCD_CS_LOW()  		(P15 = 0)

#define LCD_DC_HIGH()  		(P22 = 1)//DC
#define LCD_DC_LOW()  		(P22 = 0)

#define LCD_RST_HIGH()  	(P23 = 1)//RST
#define LCD_RST_LOW()  		(P23 = 0)

#else
#define LCD_CLK_HIGH() 		(P04 = 1)//CLK
#define LCD_CLK_LOW() 		(P04 = 0)

#define LCD_DO_HIGH() 		(P11 = 1)//DOUT
#define LCD_DO_LOW() 		(P11 = 0)

#define LCD_RST_HIGH()  	(P06 = 1)//RST
#define LCD_RST_LOW()  		(P06 = 0)

#define LCD_DC_HIGH()  		(P05 = 1)//DC
#define LCD_DC_LOW()  		(P05 = 0)
 		     
#define LCD_CS_HIGH()  		(P15 = 1)//CS
#define LCD_CS_LOW()  		(P15 = 0)
#endif

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief ????? LCD DC ?? RST PIN ?? 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void init_lcd_dc_and_rst_pin(void)
{
	#ifdef IP_101x
	SYS_SET_MFP(P1, 4, GPIO);
	SYS_SET_MFP(P1, 1, GPIO);
	SYS_SET_MFP(P1, 5, GPIO);
	SYS_SET_MFP(P2, 2, GPIO);
	SYS_SET_MFP(P2, 3, GPIO);
	
	GPIO_SetMode(P1, BIT5|BIT4|BIT1, GPIO_MODE_OUTPUT);
	GPIO_SetMode(P2, BIT2|BIT3, GPIO_MODE_OUTPUT);
	#else
	SYS_SET_MFP(P0, 4, GPIO);
    SYS_SET_MFP(P0, 5, GPIO);
    SYS_SET_MFP(P1, 1, GPIO);
	SYS_SET_MFP(P1, 5, GPIO);
	SYS_SET_MFP(P0, 6, GPIO);

	GPIO_SetMode(P0, BIT4|BIT5|BIT6, GPIO_MODE_OUTPUT);
    GPIO_SetMode(P1, BIT5|BIT1, GPIO_MODE_OUTPUT);
	#endif

	LCD_CLK_LOW();
	LCD_DO_LOW();
	LCD_CS_HIGH();
	LCD_RST_HIGH();
}


/**
 *******************************************************************************
 ** \brief ????? TFT LCD 128x160 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void reset_lcd(void)
{
	/* LCD RST Low */
	LCD_RST_LOW();
	/* Wait 100 ms */
	TIMER_Delay(TIMER0, 100000);
	/* LCD RST Hight */
	LCD_RST_HIGH(); 
	/* Wait 100 ms */
	TIMER_Delay(TIMER0, 100000);
}

void hal_bsp_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_SPI0, ENABLE);

	#ifdef IP_101x
	SYS_SET_MFP(P1, 4, SPI0_CLK);
	SYS_SET_MFP(P1, 1, SPI0_MOSI);
	SYS_SET_MFP(P1, 5, SPI0_CS);
	
	GPIO_EnableDigitalPath(P1, BIT4);   //SPI CLK is input pin while act as master
	GPIO_EnableDigitalPath(P1, BIT1);   //SPI MOSI is input pin while act as slave
	GPIO_EnableDigitalPath(P1, BIT5);   //SPI CS is input pin while act as master
	#else
	SYS_SET_MFP(P1, 1, SPI0_MOSI);
	SYS_SET_MFP(P1, 5, SPI0_CS);
	SYS_SET_MFP(P0, 4, SPI0_CLK);

	GPIO_EnableDigitalPath(P0, BIT4);   //SPI CLK is input pin while act as master
	GPIO_EnableDigitalPath(P1, BIT5);   //SPI CS is input pin while act as master
	GPIO_EnableDigitalPath(P1, BIT1);   //SPI MOSI is input pin while act as slave
	#endif

    SPI_InitOptDef spiInitStruct = {
        .format = SPI_FormatMotorola,
        .dataSize = SPI_DataFrame_8b,
        .clkPhase = SPI_ClockPhaseSecondEdge,
        .clkPolarity = SPI_ClockPolarityLow,
        .baudrateDiv = SPI_BaudRateDiv_32
    };
	
	spiInitStruct.role = SPI_RoleMaster;
	SPI0_OBJ.initObj = spiInitStruct;
	HAL_SPI_Init(&SPI0_OBJ);
}

#if 0
void SpiWriteReadByte(uint8_t dat)
{	
	uint8_t i;			  
	  
	for(i=0;i<8;i++)
	{			  
		if(dat&0x80)
		   LCD_DO_HIGH();
		else 
		   LCD_DO_LOW();
		LCD_CLK_HIGH();
		SYS_delay_10nop(1);
		LCD_CLK_LOW();
		dat<<=1;   
	}
}
#else
void SpiWriteReadByte(uint8_t dat)
{	
	HAL_SPI_SendData(&SPI0_OBJ, (uint16_t*)&dat, 1,0);
}
#endif
/**
 *******************************************************************************
 ** \brief TFT LCD ????????
 **
 ** \param [in] u8Cmd ????????
 **
 ** \retval None
 **
 ******************************************************************************/
void tft_lcd_write_cmd(uint8_t u8Cmd)
{
	/* LCD DC Low */
	LCD_DC_LOW();
	/* LCD CS Enable */
//	LCD_CS_LOW();
	/* LCD Send Cmd Data */
	SpiWriteReadByte(u8Cmd);
	/* LCD CS Disable */
//	LCD_CS_HIGH();
}

/**
 *******************************************************************************
 ** \brief TFT LCD ???? 16 ��????
 **
 ** \param [in] u16Data ????? 16 ��????
 **
 ** \retval None
 **
 ******************************************************************************/
void tft_lcd_write_data(uint16_t u16Data)
{
	/* LCD DC HIGH */
	LCD_DC_HIGH();
	/* LCD CS Enable */
//	LCD_CS_LOW();
	/* LCD Send [15:8] Data */
	SpiWriteReadByte(u16Data >> 8);
	/* LCD Send [7:0] Data */
	SpiWriteReadByte(u16Data);
	/* LCD CS Disable */
//	LCD_CS_HIGH();
}


/**
 *******************************************************************************
 ** \brief TFT LCD ????????
 **
 ** \param [in] pU8Datas ??????????????
 **
 ** \param [in] len ???????????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t tft_lcd_write_datas(uint8_t *pU8Datas, uint16_t len)
{
	uint8_t u8Ret = 0;
	uint16_t i = 0;

	if(pU8Datas && len > 0){
		/* LCD DC HIGH */
		LCD_DC_HIGH();
		/* LCD CS Enable */
//		LCD_CS_LOW();
		for(i = 0; i < len; i++){
			/* LCD Send Data */
			SpiWriteReadByte(pU8Datas[i]);
		}
		/* LCD CS Disable */
//		LCD_CS_HIGH();

		u8Ret = 1;
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????
 **
 ** \param [in] pU8Datas ??????????????
 **
 ** \param [in] len ???????????
 **
 ** \retval None
 **
 ******************************************************************************/
uint8_t tft_lcd_set_region(uint8_t u8X, uint8_t u8Y, uint8_t u8W, uint8_t u8H)
{
	uint8_t u8Ret = 0;
	uint8_t u8Buf[4];

	if(u8X < TFT_LCD_DEF_X_MAX){
		if(u8Y < TFT_LCD_DEF_Y_MAX){
			if(u8X + u8W > TFT_LCD_DEF_X_MAX){
				u8W = TFT_LCD_DEF_X_MAX - u8X;
			}
			if(u8Y + u8H > TFT_LCD_DEF_Y_MAX){
				u8H = TFT_LCD_DEF_Y_MAX - u8Y;
			}
			// Set X
			u8Buf[0] = 0x02;
			u8Buf[1] = u8X + 2;
			u8Buf[2] = 0x02;
			u8Buf[3] = u8X + u8W + 1;
			tft_lcd_write_cmd(0x2A); // Set X Cmd
			tft_lcd_write_datas(u8Buf, 4); // Set X Param
			
			// Set Y
			u8Buf[0] = 0x01;
			u8Buf[1] = u8Y + 1;
			u8Buf[2] = 0x01;
			u8Buf[3] = u8Y + u8H;
			tft_lcd_write_cmd(0x2B); // Set Y Cmd
			tft_lcd_write_datas(u8Buf, 4); // Set Y Param

			tft_lcd_write_cmd(0x2C);

			u8Ret = 1;
		}
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ???????
 **
 ** \param [in] u16Color ???????
 **
 ** \retval None
 **
 ******************************************************************************/
void tft_lcd_view_all_color(uint16_t u16Color)
{
	uint8_t u8x = 0, u8y = 0;
	
	tft_lcd_set_region(u8x, u8y, TFT_LCD_DEF_X_MAX, TFT_LCD_DEF_Y_MAX);
	for(u8y = 0; u8y < TFT_LCD_DEF_Y_MAX; u8y++){
		for(u8x = 0; u8x < TFT_LCD_DEF_X_MAX; u8x++){
			tft_lcd_write_data(u16Color);
		}
	}
}


/**
 *******************************************************************************
 ** \brief ??? TFT LCD ??????????
 **
 ** \param ??
 **
 ** \retval ??????????????
 **
 ******************************************************************************/
uint16_t tft_lcd_screen_width(void)
{
	uint16_t width = TFT_LCD_DEF_X_MAX;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax 	
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	width = TFT_LCD_DEF_Y_MAX;
#endif
	
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	width = TFT_LCD_DEF_Y_MAX;
#endif
	
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	width = TFT_LCD_DEF_X_MAX;
#endif
	
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	width = TFT_LCD_DEF_X_MAX;
#endif

	return width;
}


/**
 *******************************************************************************
 ** \brief ??? TFT LCD ?????????
 **
 ** \param ??
 **
 ** \retval ?????????????
 **
 ******************************************************************************/
uint16_t tft_lcd_screen_hight(void)
{
	uint16_t hight = TFT_LCD_DEF_Y_MAX;

// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax 	
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
	hight = TFT_LCD_DEF_X_MAX;
#endif
		
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
	hight = TFT_LCD_DEF_X_MAX;
#endif
		
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
	hight = TFT_LCD_DEF_Y_MAX;
#endif
		
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
	hight = TFT_LCD_DEF_Y_MAX;
#endif

	return hight;
}



/**
 *******************************************************************************
 ** \brief ???? TFT LCD ????????????????
 **
 **---------------------------------------------
 **| TFT Screen  |   Txt View Scan Mode        |
 **|             |      x -->                  |
 **| LCD 160x128 |    0,0------ +1 ------>128  |
 **| 0,0->128    |   y | \                     |
 **|  | \  |	 |    +1  \                    |
 **|  |  \ |	 |    \|/  \                   |
 **| 160--->	 |     |                       |
 **|             |    160                      |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] pU8Data ????????????????????????? 
 **
 ** \param [in] u16Color ??????
 **
 ** \param [in] u16bkColor ???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t tft_lcd_draw_once_text(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pU8Data, 
	uint16_t u16Color, uint16_t u16BkColor)
{
	uint8_t u8Ret = 0;
	uint8_t u8WbyteNum = 0, u8WrmBit = 0, u8Bit = 0;
	uint8_t u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16DataIdx = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pU8Data && u8W > 0 && u8H > 0){
		u8WbyteNum = u8W / 8;
		u8WrmBit = u8W % 8;
		if(u8WrmBit){
			u8WbyteNum += 1;
		}
		/* ??? LCD ????????? */
		u16ScreenW = tft_lcd_screen_width();
		u16ScreenH = tft_lcd_screen_hight();
		/* ?��????????????????? */
		if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
			u8Width = u8H;
			u8Hight = u8W;
			tft_lcd_set_region(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					if(u8Y < u8WrmBit){
						u8Bit = u8WrmBit - u8Y - 1;
					}
					else{
						u8Bit = 7 - ((u8Y - u8WrmBit) % 8);
					}
					u16DataIdx = (u8X * u8WbyteNum) + u8WbyteNum - 1;
					if(u8Y){
						if(u8WrmBit){
							u16DataIdx -= (u8Y + 8 - u8WrmBit) / 8;
						}
						else{
							u16DataIdx -= u8Y / 8;
						}
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						tft_lcd_write_data(u16Color);
					}
					else{
						tft_lcd_write_data(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
			u8Width = u8H;
			u8Hight = u8W;
			tft_lcd_set_region(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u8Bit = u8Y % 8;
					u16DataIdx = u8Width - u8X - 1;
					u16DataIdx *= u8WbyteNum;
					if(u8Y){
						u16DataIdx += (u8Y / 8);
					}
					else{
						u16DataIdx += u8Y;
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						tft_lcd_write_data(u16Color);
					}
					else{
						tft_lcd_write_data(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
			u8Width = u8W;
			u8Hight = u8H;
			tft_lcd_set_region(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u16DataIdx = (u8Hight - u8Y) * u8WbyteNum - 1;
					if(u8X){
						if(u8WrmBit){
							u16DataIdx -= (u8X + 8 - u8WrmBit) / 8;
						}
						else{
							u16DataIdx -= u8X / 8;
						}
					}
					if(u8X < u8WrmBit){
						u8Bit = u8WrmBit - u8X - 1;
					}
					else{
						u8Bit = 7 - ((u8X - u8WrmBit) % 8);
					}
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						tft_lcd_write_data(u16Color);
					}
					else{
						tft_lcd_write_data(u16BkColor);
					}
				}
			}
#endif
			
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
			u8Width = u8W;
			u8Hight = u8H;
			tft_lcd_set_region(u8Px, u8Py, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					u16DataIdx = u8Y * u8WbyteNum;
					if(u8X){
						u16DataIdx += u8X / 8;
					}
					u8Bit = u8X % 8;
					if(pU8Data[u16DataIdx] & (1 << u8Bit)){
						tft_lcd_write_data(u16Color);
					}
					else{
						tft_lcd_write_data(u16BkColor);
					}
				}
			}
#endif
			u8Ret = 1;
		}

	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????????????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] pU8Data ????????????????????????? 
 **
 ** \param [in] u16Color ??????
 **
 ** \param [in] u16bkColor ???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t tft_lcd_draw_line_text(uint8_t u8X, uint8_t u8Y, uint8_t u8Fw, uint8_t u8Fh,
	const uint8_t *pU8TextDatas, uint16_t u16TextNum, 
	uint16_t u16FtColor, uint16_t u16BkColor
	)
{
	uint8_t u8TextX = 0, u8TextY = 0, u8Ret = 0;
	uint16_t u16OnceTextDatasSize = 0, u16TextX = 0, u16TextY = 0;
	uint16_t u16TextIdx = 0, u16TextDatasIdx = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pU8TextDatas && u16TextNum > 0){
		/* ??? LCD ????????? */
		u16ScreenW = tft_lcd_screen_width();
		u16ScreenH = tft_lcd_screen_hight();
		// calc once text datas size
		u16OnceTextDatasSize = (u8Fw / 8);
		if(u8Fw % 8){
			u16OnceTextDatasSize ++;
		}
		u16OnceTextDatasSize *= u8Fh;
		u16TextX = u8X;
		u16TextY = u8Y;
		for(u16TextIdx = 0; u16TextIdx < u16TextNum; u16TextIdx++){
			/* ????��????????��?? */
			u8TextX = u16TextX;
			u8TextY = u16TextY;
			
			u16TextX += u8Fw;
			if(u16TextX > u16ScreenW){
				/* ??????? */
				u16TextX = 0;
				u8TextX = u16TextX;
				u16TextY += u8Fh;
				if(u8TextY > u16ScreenH){
					/* ?????????? */
					break;
				}
				u8TextY = u16TextY;
			}
			if(u16TextY + u8Fh > u16ScreenH){
				/* ?????????? */
				break;
			}
			
			// calc datas start index
			u16TextDatasIdx = u16TextIdx * u16OnceTextDatasSize;
			tft_lcd_draw_once_text(u8TextX, u8TextY, u8Fw, u8Fh, pU8TextDatas + u16TextDatasIdx, u16FtColor, u16BkColor);
		}
		if(u16TextIdx >= u16TextNum){
			u8Ret = 1;
		}
	}

	return u8Ret;
}	


/**
 *******************************************************************************
 ** \brief ???? TFT LCD 16 ???16 ��???([7:0],[15:8]) 
 ** R:5 bit [15:11], G:6 bit[10:5] B:5 bit[4:0]
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8Px LCD ???????????��??
 **
 ** \param [in] u8Py LCD ???????????��??
 **
 ** \param [in] u8W ??????????????
 **
 ** \param [in] u8H ?????????????
 **
 ** \param [in] pImage ???????????????????  2 * (u8W * u8H) ???
 **             ?????????????? 16 ��??????????8��???????????8��???????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t tft_lcd_draw_u16color_image(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pImage)
{
	uint8_t u8Ret = 0;
	uint8_t u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16Idx = 0, u16Color = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pImage){
		/* ??? LCD ????????? */
		u16ScreenW = tft_lcd_screen_width();
		u16ScreenH = tft_lcd_screen_hight();
		/* ?��????????????????? */
		if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
			u8Width = u8H;
			u8Hight = u8W;
			tft_lcd_set_region(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8X;
					u16Idx *= u8Hight;
					u16Idx += (u8Hight - u8Y);
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					tft_lcd_write_data(u16Color);
				}
			}
#endif
		
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
			u8Width = u8H;
			u8Hight = u8W;
			tft_lcd_set_region(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8Width - u8X - 1;
					u16Idx *= u8Hight;
					u16Idx += u8Y;
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					tft_lcd_write_data(u16Color);
				}
			}
#endif
		
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
			u8Width = u8W;
			u8Hight = u8H;
			tft_lcd_set_region(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = (u8Hight - u8Y - 1);
					u16Idx *= u8Width;
					u16Idx += (u8Width - u8X - 1);
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					tft_lcd_write_data(u16Color);
				}
			}
#endif
		
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
			u8Width = u8W;
			u8Hight = u8H;
			tft_lcd_set_region(u8Px, u8Py, u8Width, u8Hight);
			for(u8Y = 0; u8Y < u8Hight; u8Y ++){
				for(u8X = 0; u8X < u8Width; u8X ++){
					// calc index
					u16Idx = u8Y;
					u16Idx *= u8Width;
					u16Idx += u8X;
					u16Idx *= 2;
					// get color
					u16Color = (pImage[u16Idx + 1] << 8) + pImage[u16Idx];
					// draw 
					tft_lcd_write_data(u16Color);
				}
			}
#endif
			u8Ret = 1;
		}
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ??????????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] u16Color ??????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/
uint8_t tft_lcd_draw_range(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint16_t u16Color)
{
	uint8_t u8Ret = 0, u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	/* ??? LCD ????????? */
	u16ScreenW = tft_lcd_screen_width();
	u16ScreenH = tft_lcd_screen_hight();
	/* ?��????????????????? */
	if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		u8Width = u8H;
		u8Hight = u8W;
		tft_lcd_set_region(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				tft_lcd_write_data(u16Color);
			}
		}
#endif
					
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		u8Width = u8H;
		u8Hight = u8W;
		tft_lcd_set_region(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				tft_lcd_write_data(u16Color);
			}
		}
#endif
					
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
		u8Width = u8W;
		u8Hight = u8H;
		tft_lcd_set_region(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				tft_lcd_write_data(u16Color);
			}
		}
#endif
					
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
		u8Width = u8W;
		u8Hight = u8H;
		tft_lcd_set_region(u8Px, u8Py, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				tft_lcd_write_data(u16Color);
			}
		}
#endif
		u8Ret = 1;
	}

	return u8Ret;
}


/**
 *******************************************************************************
 ** \brief ???? TFT LCD ???????
 **
 **---------------------------------------------
 **| TFT Screen  |	 Txt View Scan Mode 	   |
 **|			 |		x -->				   |
 **| LCD 160x128 |	  0,0------ +1 ------>128  |
 **| 0,0->128	 |	 y | \					   |
 **|  | \  |	 |	  +1  \ 				   |
 **|  |  \ |	 |	  \|/  \				   |
 **| 160--->	 |	   |					   |
 **|			 |	  160					   |
 **---------------------------------------------
 ** \param [in] u8X LCD ???????????��??
 **
 ** \param [in] u8Y LCD ???????????��??
 **
 ** \param [in] u8W ???????????????
 **
 ** \param [in] u8H ??????????????
 **
 ** \param [in] u8Direction ???????????
 **             0: --> ????????????????????????????????????????
 **             1: --> ??????????????????????????????????
 **             2: | ?��??????????????????????????????????????
 **             3: | ?��????????????????????????????????
 **
 ** \param [in] u16Color ?????????
 **
 ** \param [in] u16EmptyColor ????????
 **
 ** \param [in] u16Value ????????
 **
 ** \param [in] u16AllValue ??????
 **
 ** \retval 1: ?????0: ???
 **
 ******************************************************************************/

uint8_t tft_lcd_draw_percent(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint8_t u8Direction, 
	uint16_t u16Color, uint16_t u16EmptyColor,
	uint32_t u32Value, uint32_t u32AllValue
	)
{
	uint8_t u8Ret = 0, u8X = 0, u8Y = 0, u8Width = 0, u8Hight = 0;
	uint16_t u16PointNum = 0, u16ValuePoint = 0, u16Cont = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;
	uint32_t u32Percent = 0;

	/* ??? LCD ????????? */
	u16ScreenW = tft_lcd_screen_width();
	u16ScreenH = tft_lcd_screen_hight();
	/* ?��????????????????? */
	if(u8Px + u8W <= u16ScreenW && u8Py + u8H <= u16ScreenH){
		/* ?????????????? */
		u16PointNum = u8W;
		u16PointNum *= u8H;
		for(;;){
			if(u32AllValue > (0xFFFFFFFF / TFT_LCD_MAX_POINT)){
				/* ??��???? */
				u32AllValue /= 10;
				if(u32Value > 0){
					u32Value /= 10;
				}
			}
			else{
				/* ???????????????��?? */
				break;
			}
		}
		u32Percent = u32Value;
		u32Percent *= u16PointNum;
		u32Percent /= u32AllValue;
		u16ValuePoint = u32Percent;
		if(u8Direction == 1){
			u8Hight = u16ValuePoint % u8H;
			u16ValuePoint = (u16ValuePoint / u8H) * u8H;
			if(u8Hight > u8H / 2){
				u16ValuePoint += u8H;
			}
		}
		else if(u8Direction == 3){
			u8Width = u16ValuePoint % u8W;
			u16ValuePoint = (u16ValuePoint / u8W) * u8W;
			if(u8Width > u8W / 2){
				u16ValuePoint += u8W;
			}
		}
			
// x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#if (TFT_LCD_VIEW == TFT_LCD_ROW_DEF)
		u8Width = u8H;
		u8Hight = u8W;
		tft_lcd_set_region(u8Py, u16ScreenW - u8Px - u8W, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8X + 1);
				}
				else{ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Y + 1);
				}
				u16Cont = u16PointNum - u16Cont;
				if(u16ValuePoint > u16Cont){
					tft_lcd_write_data(u16Color);
				}

				else{
					tft_lcd_write_data(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#if (TFT_LCD_VIEW == TFT_LCD_ROW_TURN)
		u8Width = u8H;
		u8Hight = u8W;
		tft_lcd_set_region(u16ScreenH - u8Py - u8H, u8Px, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
				}
				else{ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Hight - u8Y - 1);
				}
				if(u16ValuePoint >= u16Cont){
					tft_lcd_write_data(u16Color);
				}

				else{
					tft_lcd_write_data(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#if (TFT_LCD_VIEW == TFT_LCD_COL_DEF)
		u8Width = u8W;
		u8Hight = u8H;
		tft_lcd_set_region(u16ScreenW - u8Px - u8W, u16ScreenH - u8Py - u8H, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Hight - u8Y - 1);
					u16Cont = u16PointNum - u16Cont;
				}
				else{ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
				}
				if(u16ValuePoint >= u16Cont){
					tft_lcd_write_data(u16Color);
				}

				else{
					tft_lcd_write_data(u16EmptyColor);
				}
			}
		}
#endif
					
// x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin
#if (TFT_LCD_VIEW == TFT_LCD_COL_TURN)
		u8Width = u8W;
		u8Hight = u8H;
		tft_lcd_set_region(u8Px, u8Py, u8Width, u8Hight);
		for(u8Y = 0; u8Y < u8Hight; u8Y ++){
			for(u8X = 0; u8X < u8Width; u8X ++){
				if(u8Direction <= 1){ // ????
					u16Cont = u8X;
					u16Cont *= u8Hight;
					u16Cont += (u8Y + 1);
				}
				else{ // ????
					u16Cont = u8Y;
					u16Cont *= u8Width;
					u16Cont += (u8Width - u8X - 1);
					u16Cont = u16PointNum - u16Cont;
				}
				if(u16ValuePoint >= u16Cont){
					tft_lcd_write_data(u16Color);
				}

				else{
					tft_lcd_write_data(u16EmptyColor);
				}
			}
		}
#endif
		u8Ret = 1;
	}

	return u8Ret;
}





/**
 *******************************************************************************
 ** \brief  ��� 6x12 ��Ӣ���ַ�����ʾ
 **
 ** \param  u8x [in] ��ʼ��������
 **
 ** \param  u8y [in] ��ʼ��������
 **
 ** \param  pStr [in] ��ʾ�ַ���ָ��
 **
 ** \param  strColor [in] ��ʾ�ַ�����ɫ
 **
 ** \param  bkColor [in] ��ʾ������ɫ
 **
 ** \retval ��
 **
 ******************************************************************************/
void lcd_draw_english_6x12_str(uint8_t u8x, uint8_t u8y, uint8_t *pStr, 
	uint16_t strColor, uint16_t bkColor)
{
	uint16_t u16StrIdx = 0, u16StrDataIdx = 0, u16w = 0, u16h = 0;
	uint16_t u16ScreenW = 0, u16ScreenH = 0;

	if(pStr){
		u16ScreenW = tft_lcd_screen_width();
		u16ScreenH = tft_lcd_screen_hight();
		u16w = u8x;
		u16h = u8y;
		for(u16StrIdx = 0; pStr[u16StrIdx]; u16StrIdx++){
			u8x = u16w;
			u8y = u16h;
			
			u16w += ENGLISH_6X12_TEXT_W;
			if(u16w > u16ScreenW){
				/* ������ʾ */
				u16w = 0;
				u8x = u16w;
				u16h += ENGLISH_6X12_TEXT_H;
				if(u16h > u16ScreenH){
					/* ������Ļ��ʾ */
					break;
				}
				u8y = u16h;
			}
			if(u8y + ENGLISH_6X12_TEXT_H > u16ScreenH){
				/* ������Ļ��ʾ */
				break;
			}
			
			/* ��ѯ�ַ�������λ�� */
			if(pStr[u16StrIdx] >= '0' && pStr[u16StrIdx] <= '9'){
				u16StrDataIdx = pStr[u16StrIdx] - '0';
			}
			else if(pStr[u16StrIdx] >= 'A' && pStr[u16StrIdx] <= 'Z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'A';
				u16StrDataIdx += 10;
			}
			else if(pStr[u16StrIdx] >= 'a' && pStr[u16StrIdx] <= 'z'){
				u16StrDataIdx = pStr[u16StrIdx] - 'a';
				u16StrDataIdx += 36;
			}
			else{
				switch(pStr[u16StrIdx]){
				case '.':
					u16StrDataIdx = 62;
					break;
				case '-':
					u16StrDataIdx = 63;
					break;
				case '_':
					u16StrDataIdx = 64;
					break;
				case ' ':
					u16StrDataIdx = 65;
					break;
				case ':':
					u16StrDataIdx = 66;
					break;
				case '[':
					u16StrDataIdx = 67;
					break;
				case ']':
					u16StrDataIdx = 68;
					break;
				default:
					u16StrDataIdx = 69;
					break;
				}
			}
			
			u16StrDataIdx *= ENGLISH_6X12_TEXT_SIZE;
			/* ��ʾ��ѯ�����ַ� */
 			tft_lcd_draw_once_text(u8x, u8y, 
				ENGLISH_6X12_TEXT_W, ENGLISH_6X12_TEXT_H, 
				english_6x12_text + u16StrDataIdx, 
				strColor, bkColor
				);
		}
	}
}

void display_lcd_text(uint8_t *data)
{
	/* Set TFT LCD View */
	tft_lcd_view_all_color(TFT_COLOR_WHITE);

	lcd_draw_english_6x12_str(6, 80, data, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
}

/**
 *******************************************************************************
 ** \brief ????? TFT LCD 128x160 
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void init_tft_lcd_128x160(void)
{
	uint8_t i = 0, u8Buf[16];
	
	
	SYS_TEST("Check P04/P05/P06/P11/15 connected ,oled start.\n");
	/* ????? TFT LCD DC RST PIN */
	init_lcd_dc_and_rst_pin();

	/* ??�� TFT LCD DC RST PIN */
	reset_lcd();

	hal_bsp_init();
	
	/* ???? TFT LCD ??? SLEEP ?? */
	tft_lcd_write_cmd(0x11);

	/* ???? TFT LCD ?????? */
	i = 0;
	u8Buf[i++] = 0x01;
	u8Buf[i++] = 0x2C;
	u8Buf[i++] = 0x2D;
	u8Buf[i++] = 0x01;
	u8Buf[i++] = 0x2C;
	u8Buf[i++] = 0x2D;
	tft_lcd_write_cmd(0xB1);
	tft_lcd_write_datas(u8Buf, 3);
	tft_lcd_write_cmd(0xB2);
	tft_lcd_write_datas(u8Buf, 3);
	tft_lcd_write_cmd(0xB3);
	tft_lcd_write_datas(u8Buf, 6);

	
	/* ???? TFT LCD ?��?? */
	i = 0;
	u8Buf[i++] = 0x07;
	tft_lcd_write_cmd(0xB4);
	tft_lcd_write_datas(u8Buf, i);
	/* ???? TFT LCD Power Sequence */
	i = 0;
	u8Buf[i++] = 0xA2;
	u8Buf[i++] = 0x02;
	u8Buf[i++] = 0x84;
	tft_lcd_write_cmd(0xC0);
	tft_lcd_write_datas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0xC5;
	tft_lcd_write_cmd(0xC1);
	tft_lcd_write_datas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x0A;
	u8Buf[i++] = 0x00;
	tft_lcd_write_cmd(0xC2);
	tft_lcd_write_datas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x8A;
	u8Buf[i++] = 0x2A;
	tft_lcd_write_cmd(0xC3);
	tft_lcd_write_datas(u8Buf, i);
	i = 0;
	u8Buf[i++] = 0x8A;
	u8Buf[i++] = 0xEE;
	tft_lcd_write_cmd(0xC4);
	tft_lcd_write_datas(u8Buf, i);

	// VCOM
	i = 0;
	u8Buf[i++] = 0x0E;
	tft_lcd_write_cmd(0xC5);
	tft_lcd_write_datas(u8Buf, i);
	//MX, MY, RGB mode
	i = 0;
	u8Buf[i++] = 0xA0;
	u8Buf[i++] = 0xA8;
	tft_lcd_write_cmd(0x36);
	tft_lcd_write_datas(u8Buf, i);
	//ST7735R Gamma Sequence
	i = 0;
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x1a; 
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x18; 
	u8Buf[i++] = 0x2f; 
	u8Buf[i++] = 0x28; 
	u8Buf[i++] = 0x20; 
	u8Buf[i++] = 0x22; 
	u8Buf[i++] = 0x1f; 
	u8Buf[i++] = 0x1b; 
	u8Buf[i++] = 0x23; 
	u8Buf[i++] = 0x37; 
	u8Buf[i++] = 0x00; 	
	u8Buf[i++] = 0x07; 
	u8Buf[i++] = 0x02; 
	u8Buf[i++] = 0x10;
	tft_lcd_write_cmd(0xe0);
	tft_lcd_write_datas(u8Buf, i);

	i = 0;
	u8Buf[i++] = 0x0f; 
	u8Buf[i++] = 0x1b; 
	u8Buf[i++] = 0x17; 
	u8Buf[i++] = 0x33; 
	u8Buf[i++] = 0x2c; 
	u8Buf[i++] = 0x29; 
	u8Buf[i++] = 0x2e; 
	u8Buf[i++] = 0x30; 
	u8Buf[i++] = 0x30; 
	u8Buf[i++] = 0x39; 
	u8Buf[i++] = 0x3f;  
	u8Buf[i++] = 0x03;
	tft_lcd_write_cmd(0xe1);
	tft_lcd_write_datas(u8Buf, i);
	
	i = 0;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x02;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x9f;
	tft_lcd_write_cmd(0x2a);
	tft_lcd_write_datas(u8Buf, i);
	
	i = 0;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x03;
	u8Buf[i++] = 0x00;
	u8Buf[i++] = 0x7f;
	tft_lcd_write_cmd(0x2b);
	tft_lcd_write_datas(u8Buf, i);
	//Enable test command
	i = 0;
	u8Buf[i++] = 0x01;
	tft_lcd_write_cmd(0xF0);
	tft_lcd_write_datas(u8Buf, i);
	//Disable ram power save mode
	i = 0;
	u8Buf[i++] = 0x00;
	tft_lcd_write_cmd(0xF6);
	tft_lcd_write_datas(u8Buf, i);
	//65k mode 
	i = 0;
	u8Buf[i++] = 0x05;
	tft_lcd_write_cmd(0x3A);
	tft_lcd_write_datas(u8Buf, i);
	//Display on
	tft_lcd_write_cmd(0x29);
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
