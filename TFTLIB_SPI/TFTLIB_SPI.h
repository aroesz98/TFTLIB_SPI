/*
 * TFTLIB_SPI.h
 *
 *  Created on: Jan 10, 2022
 *  Updated on: Apr 17, 2022
 *      Author: asz
 */

#ifndef INC_TFTLIB_SPI_H_
#define INC_TFTLIB_SPI_H_

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

#include "spi.h"
#include "stm32f4xx_hal.h"
#include "algorithm"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fonts.h"

/**
 *Color of pen
 *If you want to use another color, you can choose one in RGB565 format.
 */
#define WHITE			0xFFFF
#define BLACK			0x0000
#define GRAY			0X8430
#define LIGHTGREY		0xD69A
#define DARKGREY		0x7BEF
#define RED				0xF800
#define GREEN			0x07E0
#define DARKGREEN   	0x03E0
#define BLUE			0x001F
#define LIGHTBLUE		0X7D7C
#define DARKBLUE		0X01CF
#define SKYBLUE			0x867D

#define YELLOW			0xFFE0
#define PURPLE			0x780F
#define MAGENTA			0xF81F
#define VIOLET			0x915C
#define CYAN			0x7FFF
#define DARKCYAN    	0x03EF
#define ORANGE			0xFB20

#define PINK        	0xFE19
#define BROWN       	0x9A60
#define GOLD        	0xFEA0
#define SILVER      	0xC618

/*MIXED*/
#define BRED			0XF81F
#define GRED			0XFFE0
#define GBLUE			0X07FF
#define BRRED			0XFC07
#define GRAYBLUE		0X5458
#define LGRAY			0XC618
#define LGRAYBLUE		0XA651
#define LBBLUE			0X2B12
#define NAVY        	0x000F
#define MAROON      	0x7800
#define OLIVE       	0x7BE0
#define GREENYELLOW 	0xB7E0

/* Control Registers and constant codes */
#define NOP				0x00
#define SWRESET			0x01
#define RDDID			0x04
#define RDDST			0x09

#define SLPIN			0x10
#define SLPOUT			0x11
#define PTLON			0x12
#define NORON			0x13

#define INVOFF			0x20
#define INVON			0x21
#define GAMMASET		0x26
#define DISPOFF			0x28
#define DISPON			0x29
#define CASET			0x2A
#define RASET			0x2B
#define RAMWR			0x2C
#define RAMRD			0x2E

#define COLMOD			0x3A
#define MADCTL			0x36

#define MADCTL_MH		0x04
#define MADCTL_ML		0x10
#define MADCTL_MV		0x20
#define MADCTL_MX		0x40
#define MADCTL_MY		0x80

#define MADCTL_RGB		0x00
#define MADCTL_BGR		0x08

#define FRMCTR1			0xB1
#define FRMCTR2			0xB2
#define DFUNCTR			0xB6
#define VSCRDEF			0x33
#define VSCRSADD		0x37

#define PWCTR1			0xC0
#define PWCTR2			0xC1
#define PWCTR3			0xC2
#define PWCTR4			0xC3
#define PWCTR5			0xC4
#define VMCTR1			0xC5
#define VMCTR2			0xC7

#define GMCTRP1			0xE0
#define GMCTRN1			0xE1

#define COLOR_MODE_16bit 0x55
#define COLOR_MODE_18bit 0x66

/* Basic operations */
template <typename T> static inline void
swap_coord(T& a, T& b) { T t = a; a = b; b = t;}

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

extern "C" char* sbrk(int incr);

constexpr float PixelAlphaGain   = 255.0;
constexpr float LoAlphaTheshold  = 64.0/255.0;
constexpr float HiAlphaTheshold  = 1.0 - LoAlphaTheshold;

enum class TFT_DRIVER : uint8_t
{
	ST7789				= 0x01,
	ILI9341				= 0x02,
};

class TFTLIB_SPI {
	private:
		uint8_t __rotation;
		int32_t _posx=0, _posy=0;
		int32_t __tx0 = 0, __ty0 = 0, __tx1 = 0, __ty1 = 0;
		SPI_HandleTypeDef* _bus;
		GPIO_TypeDef* CS_PORT;
		GPIO_TypeDef* DC_PORT;
		GPIO_TypeDef* RST_PORT;
		uint16_t CS_PIN;
		uint16_t DC_PIN;
		uint16_t RST_PIN;
		uint8_t _type;
		uint16_t __buffer_size = 1024;
		uint16_t *__buffer = new uint16_t[__buffer_size];
		FontDef *__font = &Font_11x18;
		uint16_t __text_fg = RED, __text_bg = BLACK;
		uint16_t SWAP_UINT16(uint16_t x) {x = (x >> 8) | (x << 8); return x;}

		int32_t _display_width  = 240;
		int32_t _display_height = 320;
		int32_t _width  = 240;
		int32_t _height = 320;

		void CS_L(void) { CS_PORT->BSRR = CS_PIN << 16U; }
		void CS_H(void) {  CS_PORT->BSRR = CS_PIN; }

		void DC_L(void) {  DC_PORT->BSRR = DC_PIN << 16U; }
		void DC_H(void) {  DC_PORT->BSRR = DC_PIN; }
	public:
		TFTLIB_SPI(SPI_HandleTypeDef &bus, TFT_DRIVER drv, GPIO_TypeDef *GPIO_DC_PORT, uint16_t GPIO_DC_PIN, GPIO_TypeDef *GPIO_CS_PORT, uint16_t GPIO_CS_PIN, GPIO_TypeDef *GPIO_RST_PORT, uint16_t GPIO_RST_PIN);
		~TFTLIB_SPI();

		inline void writeCommand(uint8_t cmd);

		void writeData(uint8_t *buff, size_t buff_size);
		void writeData_DMA(uint8_t *buff, size_t buff_size);
		inline void writeSmallData(uint8_t data);

		void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

		void pushPixels(const void* data_in, uint32_t len);
		void pushBlock(uint16_t color, uint32_t len);

		void setRotation(uint8_t m);
		void invertColors(uint8_t invert);
		void tearEffect(uint8_t tear);
		uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
		uint16_t color16to8(uint16_t c);
		uint16_t color8to16(uint8_t color);
		uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc);

		void init(void);

		void ARTtoggle();
		uint16_t width(void);
		uint16_t height(void);
		void fillScreen(uint16_t color);

		void drawPixel(int32_t x, int32_t y, uint16_t color);

		inline float wedgeLineDistance(float xpax, float ypay, float bax, float bay, float dr);
		inline void drawCircleHelper( int32_t x0, int32_t y0, int32_t rr, uint8_t cornername, uint16_t color);
		inline void fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint16_t color);
		inline void fillCircleHelperAA(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint16_t color);

		void drawFastHLine(int32_t x, int32_t y, int32_t w, uint16_t color);
		void drawFastVLine(int32_t x, int32_t y, int32_t w, uint16_t color);
		void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color);

		void drawWideLine(float ax, float ay, float bx, float by, float wd, uint16_t fg_color);
		void drawWideLine(float ax, float ay, float bx, float by, float wd, uint16_t fg_color, uint16_t bg_color);
		void drawWedgeLine(float ax, float ay, float bx, float by, float aw, float bw, uint16_t fg_color);
		void drawWedgeLine(float ax, float ay, float bx, float by, float aw, float bw, uint16_t fg_color, uint16_t bg_color);

		void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint16_t color);
		void drawTriangleAA(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t thickness, uint16_t color);

		void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
		void drawRectAA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

		void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color);

		void drawCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color);

		void drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color);

		/* Text functions. */
		void setCursor(int32_t x, int32_t y);
		void setTextColor(int32_t fg, int32_t bg);
		void setFont(FontDef &Font);
		void writeChar(int32_t x, int32_t y, char ch);
		void writeString(int32_t x, int32_t y, char *str);
		void print(char *ch);
		void println(char *ch);
		void println(uint8_t *ch);

		/* Extented Graphical functions. */
		void fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color);
		void fillTriangleAA( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color);

		void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
		void fillRectAA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

		void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color);
		void fillRoundRectAA(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color);

		void fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color);
		void fillCircleAA(float x, float y, float r, uint16_t color);

		void fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color);

		void drawImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
		void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bitmap, uint16_t color);

		uint32_t testFillScreen();
		uint32_t testText();
		uint32_t testLines(uint16_t color);
		uint32_t testFastLines(uint16_t color1, uint16_t color2);
		uint32_t testRects(uint16_t color);
		uint32_t testFilledRects(uint16_t color1, uint16_t color2);
		uint32_t testFilledCircles(uint8_t radius, uint16_t color);
		uint32_t testCircles(uint8_t radius, uint16_t color);
		uint32_t testTriangles();
		uint32_t testFilledTriangles();
		uint32_t testRoundRects();
		uint32_t testFilledRoundRects();
		void benchmark(void);
		void cpuConfig(void);
		int FreeRAM();
};

class XPT2046_Touchscreen {
	private:
		uint16_t	__scale_x=320,
					__scale_y=240,
					__rotation=0;

		uint8_t		__read_x=0xD0,
					__read_y=0x90,
					__no_samples=16;

		int32_t		__min_x = 1600,
					__max_x = 29600,
					__min_y = 2400,
					__max_y = 30700;

		SPI_HandleTypeDef* __bus;

		GPIO_TypeDef* CS_PORT;
		GPIO_TypeDef* IRQ_PORT;

		uint16_t CS_PIN;
		uint16_t IRQ_PIN;

	public:
		XPT2046_Touchscreen(SPI_HandleTypeDef &bus, GPIO_TypeDef &GPIO_CS_PORT, uint16_t GPIO_CS_PIN, GPIO_TypeDef &GPIO_IRQ_PORT, uint16_t GPIO_IRQ_PIN);
		~XPT2046_Touchscreen();

		bool pressed(void);
		void setRotation(uint8_t rotation);
		void setSamplesNumber(uint8_t number_of_samples);
		bool getTouch(int32_t* x, int32_t* y);
		bool getRaw(int32_t* x, int32_t* y);
		void calibrateTouch(TFTLIB_SPI *tft);
};

class Button {
	private:
		TFTLIB_SPI *__tft;
		XPT2046_Touchscreen *__ts;
		FontDef *__font;
		int16_t  __x1, __y1; // Coordinates of top-left corner of button
		int16_t  __xd, __yd; // Button text datum offsets (wrt center of button)
		uint16_t __w, __h, __text_w, __text_h;   // Width and height of button
		uint16_t __outlinecolor, __fillcolor, __textcolor;
		char     __label[20]; // Button text is 9 chars maximum unless long_name used
		bool  curr_state, last_state; // Button states

	public:
		Button(void);
		~Button(void);
		// "Classic" initButton() uses center & size
		void	initButton(TFTLIB_SPI *tft, XPT2046_Touchscreen *ts, int16_t x,
		int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
		uint16_t textcolor, char *label, FontDef* Font);

		// New/alt initButton() uses upper-left corner & size
		void     initButtonUL(TFTLIB_SPI *tft, XPT2046_Touchscreen *ts, int16_t x1,
		int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
		uint16_t textcolor, char *label, FontDef* Font);

		using Callback = void (*) (void);

		void setLabelOffset(int16_t x_delta, int16_t y_delta);
		void drawButton(bool inverted = false, char* long_name = (char*)"");
		bool contains(int16_t x, int16_t y);
		void process(char* label_pressed, char* label_release, Callback btn_func, Callback end_func, XPT2046_Touchscreen &ts);

		void press(bool p);
		bool isPressed();
		bool wasPressed();
		bool wasReleased();
};

#pragma GCC pop_options

#endif
