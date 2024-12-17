
/******************************************************************************/
/** \file tft_lcd_128x160.h
 ******************************************************************************/
#ifndef __TFT_LCD_128X160_H
#define __TFT_LCD_128X160_H

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "PanSeries.h"

#ifdef __cplusplus
 extern "C" {
#endif


/**
 *******************************************************************************
 ** \defgroup DdlUtilityGroup Device Driver Library Gpio (GPIO CONTROL)
 **
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

//----------------------------------------------
// | TFT Screen  |   Lcd View Scan Mode        |
// |             |      x -->                  |
// | LCD 160x128 |    0,0------ +1 ------>128  |
// | 0,0->128    |   y | \                     |
// |  | \  |	 |    +1  \                    |
// |  |  \ |	 |    \|/  \                   |
// | 160--->	 |     |                       |
// |             |    160                      |
//----------------------------------------------
#define TFT_LCD_DEF_X_MAX			160
#define TFT_LCD_DEF_Y_MAX			128
#define TFT_LCD_MAX_POINT			(TFT_LCD_DEF_X_MAX * TFT_LCD_DEF_Y_MAX)

#define TFT_LCD_ROW_DEF				0 // x-> (LCD)Ymax -> Ymin; y-> (LCD)Xmin -> Xmax
#define TFT_LCD_ROW_TURN			1 // x-> (LCD)Ymin -> Ymax; y-> (LCD)Xmax -> Xmin
#define TFT_LCD_COL_DEF				2 // x-> (LCD)Xmin -> Xmax; y-> (LCD)Ymin -> Ymax
#define TFT_LCD_COL_TURN			3 // x-> (LCD)Xmax -> Xmin; y-> (LCD)Ymax -> Ymin

#define TFT_LCD_VIEW				(TFT_LCD_ROW_DEF)

// TFT_COLOR RGB
#define GET_TFT_COLOR(r,g,b)		(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))

// color
#define TFT_COLOR_RED				GET_TFT_COLOR(0x1F,0x00,0x00)
#define TFT_COLOR_GREEN				GET_TFT_COLOR(0x00,0x3F,0x00)
#define TFT_COLOR_BLUE				GET_TFT_COLOR(0x00,0x00,0x1F)
#define TFT_COLOR_BLACK				GET_TFT_COLOR(0x00,0x00,0x00)
#define TFT_COLOR_WHITE				GET_TFT_COLOR(0x1F,0x3F,0x1F)
#define TFT_COLOR_YELLOW			GET_TFT_COLOR(0x1F,0x3F,0x00)
#define TFT_COLOR_GRAPE				GET_TFT_COLOR(0x10,0x33,0x1F)
#define TFT_COLOR_PURPLE			GET_TFT_COLOR(0x1F,0x00,0x1F)

#define TFT_TITLE_COLOR				GET_TFT_COLOR(0x15,0x36,0x1C)
#define TFT_STATIC_TEXT_COLOR		GET_TFT_COLOR(0x19,0x26,0x03)


#define ENGLISH_6X12_TEXT_SIZE		12
#define ENGLISH_6X12_TEXT_W			6
#define ENGLISH_6X12_TEXT_H			12

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
void init_tft_lcd_128x160(void);

void tft_lcd_view_all_color(uint16_t u16Color);

uint8_t tft_lcd_draw_once_text(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pU8Data, 
	uint16_t u16Color, uint16_t u16BkColor);

uint8_t tft_lcd_draw_line_text(uint8_t u8X, uint8_t u8Y, 
	uint8_t u8Fw, uint8_t u8Fh, 
	const uint8_t *pU8TextDatas, uint16_t u16TextNum, 
	uint16_t u16FtColor, uint16_t u16BkColor);

uint8_t tft_lcd_draw_u16color_image(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	const uint8_t *pImage);

uint8_t tft_lcd_draw_range(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint16_t u16Color);

uint8_t tft_lcd_draw_percent(uint8_t u8Px, uint8_t u8Py, uint8_t u8W, uint8_t u8H, 
	uint8_t u8Direction, 
	uint16_t u16Color, uint16_t u16EmptyColor,
 	uint32_t u326Value, uint32_t u32AllValue);

uint16_t tft_lcd_screen_width(void);

uint16_t tft_lcd_screen_hight(void);

void display_lcd_text(uint8_t *data);
	
#ifdef __cplusplus
}
#endif

#endif /* __TFT_LCD_128X160_H */


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/



