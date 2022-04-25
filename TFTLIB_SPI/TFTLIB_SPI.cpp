/*
 * TFTLIB_SPI.cpp
 *
 *  Created on: Jan 10, 2022
 *  Updated on: Apr 17, 2022
 *      Author: asz
 */


#include <TFTLIB_SPI.h>
#include "hw_drv.h"

using namespace std;

/***************************************************************************************
** Function name:           XPT2046_Touchscreen
** Description:             Constructor
***************************************************************************************/
XPT2046_Touchscreen::XPT2046_Touchscreen(SPI_HandleTypeDef &bus, GPIO_TypeDef& GPIO_CS_PORT, uint16_t GPIO_CS_PIN, GPIO_TypeDef& GPIO_IRQ_PORT, uint16_t GPIO_IRQ_PIN) {
	__bus = &bus;
	CS_PORT	 = &GPIO_CS_PORT;
	CS_PIN	 = GPIO_CS_PIN;
	IRQ_PORT	 = &GPIO_IRQ_PORT;
	IRQ_PIN	 = GPIO_IRQ_PIN;
	setRotation(1);
}

/***************************************************************************************
** Function name:           ~XPT2046_Touchscreen
** Description:             Destructor
***************************************************************************************/
XPT2046_Touchscreen::~XPT2046_Touchscreen() { }

/***************************************************************************************
** Function name:           pressed
** Description:             Check if touch is pressed
***************************************************************************************/
bool XPT2046_Touchscreen::pressed(void) {
	return (HAL_GPIO_ReadPin(IRQ_PORT, IRQ_PIN) == 0);
}

/***************************************************************************************
** Function name:           setRotation
** Description:             Set touch rotation
***************************************************************************************/
void XPT2046_Touchscreen::setRotation(uint8_t rotation) {
	rotation = rotation % 4;
	switch(rotation) {
		case 0:
			__scale_x = 240;
			__scale_y = 320;
			__read_x  = 0xD0;
			__read_y  = 0x90;
			__rotation = 0;
		break;
		case 1:
			__scale_x = 320;
			__scale_y = 240;
			__read_x  = 0x90;
			__read_y  = 0xD0;
			__rotation = 1;
		break;
		case 2:
			__scale_x = 240;
			__scale_y = 320;
			__read_x  = 0xD0;
			__read_y  = 0x90;
			__rotation = 2;
		break;
		case 3:
			__scale_x = 320;
			__scale_y = 240;
			__read_x  = 0x90;
			__read_y  = 0xD0;
			__rotation = 3;
		break;
	}
}

/***************************************************************************************
** Function name:           setSamplesNumber
** Description:             Set number of samples to get average coordinates
***************************************************************************************/
void XPT2046_Touchscreen::setSamplesNumber(uint8_t number_of_samples) {
	if(number_of_samples >= 16) __no_samples = 16;
	__no_samples = number_of_samples;
}

/***************************************************************************************
** Function name:           getRaw
** Description:             Get calibrated touch coordinates
***************************************************************************************/
bool XPT2046_Touchscreen::getTouch(int32_t* x, int32_t* y)
{
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    uint8_t samples = 0;
    uint16_t zorros = 0;

    if(!pressed()){
    	*x = 0;
    	*y = 0;
        return 0;
    }

    CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;

    for(uint8_t i = 0; i < __no_samples; i++)
    {
        samples++;

        uint8_t y_raw[2];
        uint8_t x_raw[2];

        HAL_SPI_Transmit(__bus, reinterpret_cast<uint8_t*>(&__read_y), 1, HAL_MAX_DELAY);
        HAL_SPI_TransmitReceive(__bus, reinterpret_cast<uint8_t*>(&zorros), y_raw, 2, HAL_MAX_DELAY);

        HAL_SPI_Transmit(__bus, reinterpret_cast<uint8_t*>(&__read_x), 1, HAL_MAX_DELAY);
        HAL_SPI_TransmitReceive(__bus, reinterpret_cast<uint8_t*>(&zorros), x_raw, 2, HAL_MAX_DELAY);

        sum_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        sum_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
    }

    CS_PORT->BSRR = (uint32_t)CS_PIN;

    if(samples < __no_samples) return false;

    int32_t raw_x = (sum_x / __no_samples);
    if(raw_x < __min_x) raw_x = __min_x;
    if(raw_x > __max_x) raw_x = __max_x;

    int32_t raw_y = (sum_y / __no_samples);
    if(raw_y < __min_y) raw_y = __min_y;
    if(raw_y > __max_y) raw_y = __max_y;

    switch(__rotation){
		case 0:
			*x = 240 - (raw_x - __min_x) * __scale_x / (__max_x - __min_x);
			*y = (raw_y - __min_y) * __scale_y / (__max_y - __min_y);
		break;
		case 1:
			*x = (raw_x - __min_x) * __scale_x / (__max_x - __min_x);
			*y = (raw_y - __min_y) * __scale_y / (__max_y - __min_y);
		break;
		case 2:
			*x = (raw_x - __min_x) * __scale_x / (__max_x - __min_x);
			*y = 320-(raw_y - __min_y) * __scale_y / (__max_y - __min_y);
    	break;
		case 3:
			*x = 320-(raw_x - __min_x) * __scale_x / (__max_x - __min_x);
			*y = 240-(raw_y - __min_y) * __scale_y / (__max_y - __min_y);
    	break;
    }

    return true;
}

/***************************************************************************************
** Function name:           getRaw
** Description:             Get touch raw value
***************************************************************************************/
bool XPT2046_Touchscreen::getRaw(int32_t* x, int32_t* y)
{
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    uint8_t samples = 0;
    uint16_t zorros = 0;

    if(!pressed()){
    	*x = 0;
    	*y = 0;
        return 0;
    }

    CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;

    for(uint8_t i = 0; i < __no_samples; i++)
    {
        samples++;

        uint8_t y_raw[2];
        uint8_t x_raw[2];

        HAL_SPI_Transmit(__bus, reinterpret_cast<uint8_t*>(&__read_y), 1, HAL_MAX_DELAY);
        HAL_SPI_TransmitReceive(__bus, reinterpret_cast<uint8_t*>(&zorros), y_raw, 2, HAL_MAX_DELAY);

        HAL_SPI_Transmit(__bus, reinterpret_cast<uint8_t*>(&__read_x), 1, HAL_MAX_DELAY);
        HAL_SPI_TransmitReceive(__bus, reinterpret_cast<uint8_t*>(&zorros), x_raw, 2, HAL_MAX_DELAY);


        sum_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        sum_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
    }

    CS_PORT->BSRR = (uint32_t)CS_PIN;

    if(samples < __no_samples) return false;

    uint32_t raw_x = (sum_x / __no_samples);
    uint32_t raw_y = (sum_y / __no_samples);

    if(__rotation == 1 || __rotation == 3) {
        *x = raw_y;
        *y = raw_x;
    }

    else{
        *x = raw_x;
        *y = raw_y;
    }
    return true;
}

/***************************************************************************************
** Function name:           calibrateTouch
** Description:             Calibrate touch from 4 points
***************************************************************************************/
void XPT2046_Touchscreen::calibrateTouch(TFTLIB_SPI *tft) {
	uint8_t size = 10;
	uint16_t calData[8];
	int32_t x_tmp, y_tmp;
	uint16_t color_fg = MAGENTA, color_bg = BLACK;
	char print[32];
	tft->setFont(Font_7x10);
	tft->setCursor(0, 30);
	tft->fillScreen(BLACK);
	for(uint8_t i = 0; i<4; i++){
		tft->fillRect(0, 0, size+1, size+1, color_bg);
		tft->fillRect(0, tft->height()-size-1, size+1, size+1, color_bg);
		tft->fillRect(tft->width()-size-1, 0, size+1, size+1, color_bg);
		tft->fillRect(tft->width()-size-1, tft->height()-size-1, size+1, size+1, color_bg);

		if (i == 5) break; // used to clear the arrows

		switch (i) {
			case 0: // up left
				tft->drawLine(0, 0, 0, size, color_fg);
				tft->drawLine(0, 0, size, 0, color_fg);
				tft->drawLine(0, 0, size , size, color_fg);
			break;
			case 1: // bot left
				tft->drawLine(0, tft->height()-size-1, 0, tft->height()-1, color_fg);
				tft->drawLine(0, tft->height()-1, size, tft->height()-1, color_fg);
				tft->drawLine(size, tft->height()-size-1, 0, tft->height()-1 , color_fg);
			break;
			case 2: // up right
				tft->drawLine(tft->width()-size-1, 0, tft->width()-1, 0, color_fg);
				tft->drawLine(tft->width()-size-1, size, tft->width()-1, 0, color_fg);
				tft->drawLine(tft->width()-1, size, tft->width()-1, 0, color_fg);
			break;
			case 3: // bot right
				tft->drawLine(tft->width()-size-1, tft->height()-size-1, tft->width()-1, tft->height()-1, color_fg);
				tft->drawLine(tft->width()-1, tft->height()-1-size, tft->width()-1, tft->height()-1, color_fg);
				tft->drawLine(tft->width()-1-size, tft->height()-1, tft->width()-1, tft->height()-1, color_fg);
			break;
		}

		while(!getRaw(&x_tmp, &y_tmp));
		while(pressed());
		calData[i*2] = x_tmp;
		calData[i*2+1] = y_tmp;

		sprintf(print, "x%d = %ld y%d = %ld", i, x_tmp, i, y_tmp);
		tft->println(print);
		HAL_Delay(1000);
	}

	switch(__rotation){
		case 0:
			__min_x = (calData[4] + calData[6]) / 2; // calc min x
			__max_x = (calData[0] + calData[2]) / 2; // calc max x
			__min_y = (calData[1] + calData[5]) / 2; // calc min y
			__max_y = (calData[3] + calData[7]) / 2; // calc max y
		break;
		case 1:
			__min_x = (calData[0] + calData[4]) / 2; // calc min x
			__max_x = (calData[2] + calData[6]) / 2; // calc max x
			__min_y = (calData[1] + calData[3]) / 2; // calc min y
			__max_y = (calData[5] + calData[7]) / 2; // calc max y
		break;
		case 2:
			__min_x = (calData[0] + calData[2]) / 2; // calc min x
			__max_x = (calData[4] + calData[6]) / 2; // calc max x
			__min_y = (calData[3] + calData[7]) / 2; // calc min y
			__max_y = (calData[1] + calData[5]) / 2; // calc max y
		break;
		case 3:
			__min_x = (calData[2] + calData[6])/2; // calc min x
			__max_x = (calData[0] + calData[4])/2; // calc max x
			__min_y = (calData[5] + calData[7])/2; // calc min y
			__max_y = (calData[1] + calData[3])/2; // calc max y
		break;
	}

	  tft->writeString(0, 180, (char*)"Calibration completed!");
	  HAL_Delay(2000);
}

/***************************************************************************************
** Function name:           TFTLIB_SPI
** Description:             TFTLIB_SPI Constructor
***************************************************************************************/
TFTLIB_SPI::TFTLIB_SPI(SPI_HandleTypeDef &bus, TFT_DRIVER drv, GPIO_TypeDef *GPIO_DC_PORT, uint16_t GPIO_DC_PIN, GPIO_TypeDef *GPIO_CS_PORT, uint16_t GPIO_CS_PIN, GPIO_TypeDef *GPIO_RST_PORT, uint16_t GPIO_RST_PIN) {
	_bus = &bus;
	CS_PORT	 = GPIO_CS_PORT;
	CS_PIN	 = GPIO_CS_PIN;

	DC_PORT	 = GPIO_DC_PORT;
	DC_PIN	 = GPIO_DC_PIN;

	RST_PORT = GPIO_DC_PORT;
	RST_PIN	 = GPIO_DC_PIN;

	_type = (uint8_t)drv;
}

/***************************************************************************************
** Function name:           ~TFTLIB_SPI
** Description:             TFTLIB_SPI Destructor
***************************************************************************************/
TFTLIB_SPI::~TFTLIB_SPI() {
	delete[] __buffer;
}

uint16_t TFTLIB_SPI::width(void){
	return _width;
}

uint16_t TFTLIB_SPI::height(void){
	return _height;
}

/***************************************************************************************
** Function name:           init
** Description:             Init display with selected driver
***************************************************************************************/
void TFTLIB_SPI::init(void){
	if(_type == (uint8_t)TFT_DRIVER::ST7789) {
		#include "st7789_drv.h"
	}

	else if(_type == (uint8_t)TFT_DRIVER::ILI9341) {
		#include "ili9341_drv.h"
	}
	setRotation(3);
	fillScreen(GREEN);
}

/***************************************************************************************
** Function name:           writeCommand
** Description:             Writing command to display
***************************************************************************************/
inline void TFTLIB_SPI::writeCommand(uint8_t cmd)
{
	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN << 16U;
	SPI_Transmit(_bus, &cmd, sizeof(cmd), HAL_MAX_DELAY);
	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           writeData
** Description:             SPI writing for small blocks of data
***************************************************************************************/
void TFTLIB_SPI::writeData(uint8_t *buff, size_t buff_size)
{
	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;
	while (buff_size > 0) {
		uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
		SPI_Transmit(_bus, buff, chunk_size, HAL_MAX_DELAY);
		buff += chunk_size;
		buff_size -= chunk_size;
	}
	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           writeData_DMA
** Description:             Fast SPI writing for big buffers
***************************************************************************************/

void TFTLIB_SPI::writeData_DMA(uint8_t *buff, size_t size)
{
	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;

	uint32_t chunk_size = 0;
	while (size > 0) {
		chunk_size = size > 65535 ? 65535 : size;
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)buff, chunk_size);
		buff += chunk_size;
		size -= chunk_size;
		while (_bus->State != HAL_SPI_STATE_READY);
	}

	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           writeSmallData
** Description:             SPI writing 8bit data
***************************************************************************************/
inline void TFTLIB_SPI::writeSmallData(uint8_t data)
{
	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;
	SPI_Transmit(_bus, &data, sizeof(data), HAL_MAX_DELAY);
	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           setRotation
** Description:             Set the rotation direction of the display
***************************************************************************************/
void TFTLIB_SPI::setRotation(uint8_t m)
{
	m = m % 4;
	__rotation = m;

	if(_type == (uint8_t)TFT_DRIVER::ST7789){
		writeCommand(MADCTL);
		switch (m) {
		case 0:
			writeSmallData(MADCTL_RGB);
			_width  = _display_width;
			_height = _display_height;
			break;
		case 1:
			writeSmallData(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
			_width  = _display_height;
			_height = _display_width;
			break;
		case 2:
			writeSmallData(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
			_width  = _display_width;
			_height = _display_height;
			break;
		case 3:
			writeSmallData(MADCTL_MV | MADCTL_MY | MADCTL_RGB);
			_width  = _display_height;
			_height = _display_width;
			break;
		default:
			break;
		}
	}

	else if(_type == (uint8_t)TFT_DRIVER::ILI9341){
		writeCommand(MADCTL);
		switch (m) {
		case 0:
			writeSmallData(MADCTL_MX | MADCTL_BGR);
			_width  = _display_width;
			_height = _display_height;
			break;
		case 1:
			writeSmallData(MADCTL_MV | MADCTL_BGR);
			_width  = _display_height;
			_height = _display_width;
			break;
		case 2:
			writeSmallData(MADCTL_MY | MADCTL_BGR);
			_width  = _display_width;
			_height = _display_height;
			break;
		case 3:
			writeSmallData(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = _display_height;
			_height = _display_width;
			break;
		default:
			break;
		}
	}
}

/***************************************************************************************
** Function name:           invertColors
** Description:             Inverted colors mode
***************************************************************************************/
void TFTLIB_SPI::invertColors(uint8_t invert)
{
	writeCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
}

/***************************************************************************************
** Function name:           tearEffect
** Description:             Open/Close tearing effect line
***************************************************************************************/
void TFTLIB_SPI::tearEffect(uint8_t tear)
{
	//Tear Effect -->   ON     OFF
	writeCommand(tear ? 0x35 : 0x34);
}

/***************************************************************************************
** Function name:           color565
** Description:             Convert value RGB888 to RGB565
***************************************************************************************/
uint16_t TFTLIB_SPI::color565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	return color;
}

/***************************************************************************************
** Function name:           color16to8
** Description:             Convert 16bit palette to 8bit palette
***************************************************************************************/
uint16_t TFTLIB_SPI::color16to8(uint16_t c)
{
  return ((c & 0xE000)>>8) | ((c & 0x0700)>>6) | ((c & 0x0018)>>3);
}

/***************************************************************************************
** Function name:           color8to16
** Description:             Convert 8bit palette to 16bit palette
***************************************************************************************/
uint16_t TFTLIB_SPI::color8to16(uint8_t color)
{
  uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table
  uint16_t color16 = 0;

  //        =====Green=====     ===============Red==============
  color16  = (color & 0x1C)<<6 | (color & 0xC0)<<5 | (color & 0xE0)<<8;
  //        =====Green=====    =======Blue======
  color16 |= (color & 0x1C)<<3 | blue[color & 0x03];

  return color16;
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Mix fgc & bgc with selected alpha channel(255 = full bgc)
***************************************************************************************/
uint16_t TFTLIB_SPI::alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
{
	// For speed use fixed point maths and rounding to permit a power of 2 division
	uint16_t fgR = ((fgc >> 10) & 0x3E) + 1;
	uint16_t fgG = ((fgc >>  4) & 0x7E) + 1;
	uint16_t fgB = ((fgc <<  1) & 0x3E) + 1;

	uint16_t bgR = ((bgc >> 10) & 0x3E) + 1;
	uint16_t bgG = ((bgc >>  4) & 0x7E) + 1;
	uint16_t bgB = ((bgc <<  1) & 0x3E) + 1;

	// Shift right 1 to drop rounding bit and shift right 8 to divide by 256
	uint16_t r = (((fgR * alpha) + (bgR * (255 - alpha))) >> 9);
	uint16_t g = (((fgG * alpha) + (bgG * (255 - alpha))) >> 9);
	uint16_t b = (((fgB * alpha) + (bgB * (255 - alpha))) >> 9);

	return (r << 11) | (g << 5) | (b << 0);
}

/***************************************************************************************
** Function name:           setWindow
** Description:             Set start/end address of drawed window
***************************************************************************************/
void TFTLIB_SPI::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	if(x0 < 0 || x0 >= _width || x1 < 0 || x1 >= _width || y0 < 0 || y0 >= _height || y1 < 0 || y1 >= _height) return;

	/* Column Address set */
	if(__tx0 != x0 || __tx1 != x1) {
		uint16_t col[2] = { SWAP_UINT16(x0), SWAP_UINT16(x1) };
		writeCommand(CASET);
		writeData((uint8_t*)col, 4);
		__tx0 = x0;
		__tx1 = x1;
	}

	/* Row Address set */
	if(__ty0 != y0 || __ty1 != y1) {
		uint16_t row[2] = { SWAP_UINT16(y0), SWAP_UINT16(y1) };
		writeCommand(RASET);
		writeData((uint8_t*)row, 4);
		__ty0 = y0;
		__ty1 = y1;
	}

	/* Write to RAM */
	writeCommand(RAMWR);
}

/***************************************************************************************
** Function name:           pushPixels
** Description:             Write pixels from pointer (for JPEG Decoding)
***************************************************************************************/
void TFTLIB_SPI::pushPixels(const void* data_in, uint32_t len){
	uint16_t *data = (uint16_t*)data_in;
	uint32_t size = len;
	uint32_t ptr=0;

	while (size > 0) {
		uint32_t chunk_size = size > __buffer_size ? __buffer_size : size;
		ptr = 0;

		while(len--){
			if(ptr >= chunk_size) break;
			__buffer[ptr] = *data;
			data++;
			ptr++;
		}

		for(uint32_t i=0; i<chunk_size; i++){
			__buffer[i] = SWAP_UINT16(__buffer[i]);
		}

		writeData_DMA((uint8_t*)__buffer, chunk_size*2);
		size -= chunk_size;
	}
}

/***************************************************************************************
** Function name:           pushBlock
** Description:             Push block of data divided on chunk with size of buffer
***************************************************************************************/
void TFTLIB_SPI::pushBlock(uint16_t color, uint32_t len = 1){
	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;
	uint16_t last_chunk_size=0;
	while (len-- > 0) {
		uint16_t chunk_size = len > __buffer_size ? __buffer_size : len;
		if(chunk_size != last_chunk_size){
			last_chunk_size = chunk_size;
			fill_n(__buffer, chunk_size, SWAP_UINT16(color));
		}
		HAL_SPI_Transmit_DMA(_bus, reinterpret_cast<uint8_t*>(__buffer), chunk_size*2);
		len -= chunk_size;
		while(HAL_DMA_GetState(_bus->hdmatx) != HAL_DMA_STATE_READY);
	}
	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           fillScreen
** Description:             Fast color fillig screen function
***************************************************************************************/
void TFTLIB_SPI::fillScreen(uint16_t color)
{
	uint32_t buff_size = 0, chunk_size = 0;

	fill_n(__buffer, __buffer_size, SWAP_UINT16(color));
	setWindow(0, 0, _width-1, _height - 1);

	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;

	chunk_size = __buffer_size;
	buff_size = _width*_height;

	while (buff_size > 0) {
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, chunk_size*2);
		buff_size -= chunk_size;
		while(HAL_DMA_GetState(_bus->hdmatx) != HAL_DMA_STATE_READY);
	}

	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           cpuConfig
** Description:             Print MCU clock configuration
***************************************************************************************/
void TFTLIB_SPI::cpuConfig(void){
	fillScreen(BLACK);
	char prt[32];
	uint32_t hclk = HAL_RCC_GetHCLKFreq();
	uint32_t apb1 = HAL_RCC_GetPCLK1Freq();
	uint32_t apb2 = HAL_RCC_GetPCLK2Freq();
	uint32_t sysclk = HAL_RCC_GetSysClockFreq();

	uint8_t psc = 0x02 & ((_bus->Instance->CR1) >> 3);
	psc = pow(2, psc+1);

	setTextColor(RED, BLACK);
	setFont(Font_11x18);
	setCursor(0, 30);

	sprintf(prt, "HCLK: %lu", hclk);
	println(prt);
	sprintf(prt, "APB1: %lu", apb1);
	println(prt);
	sprintf(prt, "APB2: %lu", apb2);
	println(prt);
	sprintf(prt, "SYSCLK: %lu", sysclk);
	println(prt);
	sprintf(prt, "SPI Psc: %u", psc);
	println(prt);
	/*if(_bus == &hspi1) sprintf(prt, "SPI Freq: %lu", (apb2 / psc));
	else sprintf(prt, "SPI Freq: %lu", (apb1 / psc));*/
	println(prt);
}

/***************************************************************************************
** Function name:           FreeRAM
** Description:             Return free RAM
***************************************************************************************/
int TFTLIB_SPI::FreeRAM() {
  register uint32_t sp asm("sp");
  return reinterpret_cast<char*>(sp) - reinterpret_cast<char*>(sbrk(0));
}

/***************************************************************************************
** Function name:           Toggle ART Accelerator
** Description:             Turn on / Turn off ART Accelerator
***************************************************************************************/
void TFTLIB_SPI::ARTtoggle() {
	setTextColor(RED, BLACK);
	setFont(Font_11x18);
	setCursor(0, 0);
	if((FLASH->ACR & FLASH_ACR_ICEN)!=FLASH_ACR_ICEN) { // art disabled
		/* enable the ART accelerator */
		/* enable prefetch buffer */
		FLASH->ACR |= FLASH_ACR_PRFTEN;
		/* Enable flash instruction cache */
		FLASH->ACR |= FLASH_ACR_ICEN;
		/* Enable flash data cache */
		FLASH->ACR |= FLASH_ACR_DCEN;
		asm("wfi"); //wait for a systick interrupt i.e. delay(1)
		fillScreen(BLACK);
		setCursor(0, 0);
		println((char*)"ART enabled");
	}

	else {
		/* disable the ART accelerator */
		/* disable flash instruction cache */
		FLASH->ACR &= ~FLASH_ACR_ICEN;
		/* disable flash data cache */
		FLASH->ACR &= ~FLASH_ACR_DCEN;
		/* enable prefetch buffer */
		FLASH->ACR &= ~FLASH_ACR_PRFTEN;
		asm("wfi"); //wait for a systick interrupt, i.e. delay(1)
		fillScreen(BLACK);
		setCursor(0, 0);
		println((char*)"ART disabled");
	}
}

/***************************************************************************************
** Function name:           drawPixel
** Description:             Draw single pixel at coords x&y
***************************************************************************************/
void TFTLIB_SPI::drawPixel(int32_t x, int32_t y, uint16_t color)
{
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))	return;

	__buffer[0] = SWAP_UINT16(color);
	setWindow(x, y, x, y);

	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;

	SPI_Transmit(_bus, reinterpret_cast<uint8_t*>(__buffer), 2, HAL_MAX_DELAY);

	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           drawFastHLine
** Description:             Fast drawing Horizontal Line
***************************************************************************************/
inline void TFTLIB_SPI::drawFastHLine(int32_t x, int32_t y, int32_t w, uint16_t color) {
	if(x < 0 || x > _width || y < 0 || y > _height || x + w > _width) return;

	fill_n(__buffer, w, SWAP_UINT16(color));

	setWindow(x, y, x + w - 1, y);
	if(w>(_width/4)) writeData_DMA((uint8_t*)__buffer, w*2);
	else writeData((uint8_t*)__buffer, w*2);
}

/***************************************************************************************
** Function name:           drawFastVLine
** Description:             Drawing Vertical Line
***************************************************************************************/
inline void TFTLIB_SPI::drawFastVLine(int32_t x, int32_t y, int32_t h, uint16_t color) {
	if(x < 0 || x > _width || y < 0 || y > _height || y + h > _height) return;

	fill_n(__buffer, h, SWAP_UINT16(color));

	setWindow(x, y, x, y + h - 1);
	if(h>(_height/4)) writeData_DMA((uint8_t*)__buffer, h*2);
	else writeData((uint8_t*)__buffer, h*2);
}

/***************************************************************************************
** Function name:           drawCircleHelper
** Description:             Support function for drawRoundRect()
***************************************************************************************/
inline void TFTLIB_SPI::drawCircleHelper( int32_t x0, int32_t y0, int32_t rr, uint8_t cornername, uint16_t color) {
	if (rr <= 0) return;
	int32_t f     = 1 - rr;
	int32_t ddF_x = 1;
	int32_t ddF_y = -2 * rr;
	int32_t xe    = 0;
	int32_t xs    = 0;
	int32_t len   = 0;

	while (xe < rr--) {
		while (f < 0) {
		  ++xe;
		  f += (ddF_x += 2);
		}

		f += (ddF_y += 2);

		if (xe-xs==1) {
			if (cornername & 0x1) { // left top
				drawPixel(x0 - xe, y0 - rr, color);
				drawPixel(x0 - rr, y0 - xe, color);
			}

			if (cornername & 0x2) { // right top
				drawPixel(x0 + rr    , y0 - xe, color);
				drawPixel(x0 + xs + 1, y0 - rr, color);
			}

			if (cornername & 0x4) { // right bottom
				drawPixel(x0 + xs + 1, y0 + rr    , color);
				drawPixel(x0 + rr, y0 + xs + 1, color);
			}

			if (cornername & 0x8) { // left bottom
				drawPixel(x0 - rr, y0 + xs + 1, color);
				drawPixel(x0 - xe, y0 + rr    , color);
			}
		}

		else {
			len = xe - xs++;
			if (cornername & 0x1) { // left top
				drawFastHLine(x0 - xe, y0 - rr, len, color);
				drawFastVLine(x0 - rr, y0 - xe, len, color);
			}

			if (cornername & 0x2) { // right top
				drawFastVLine(x0 + rr, y0 - xe, len, color);
				drawFastHLine(x0 + xs, y0 - rr, len, color);
			}

			if (cornername & 0x4) { // right bottom
				drawFastHLine(x0 + xs, y0 + rr, len, color);
				drawFastVLine(x0 + rr, y0 + xs, len, color);
			}

			if (cornername & 0x8) { // left bottom
				drawFastVLine(x0 - rr, y0 + xs, len, color);
				drawFastHLine(x0 - xe, y0 + rr, len, color);
			}
		}
	xs = xe;
	}
}

/***************************************************************************************
** Function name:           wideWedgeDistance
** Description:             Support function for drawWedgeLine
***************************************************************************************/
inline float TFTLIB_SPI::wedgeLineDistance(float xpax, float ypay, float bax, float bay, float dr) {
	float h = fmaxf(fminf((xpax * bax + ypay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
	float dx = xpax - bax * h, dy = ypay - bay * h;
	return sqrtf(dx * dx + dy * dy) + h * dr;
}

/***************************************************************************************
** Function name:           fillCircleHelper
** Description:             Support function for fillRoundRect()
***************************************************************************************/
inline void TFTLIB_SPI::fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint16_t color) {
	if(x0 < 0 || y0 < 0 || x0 > _width || y0 > _height || x0 + r > _width || y0 + r > _height) return;

	int32_t f     = 1 - r;
	int32_t ddF_x = 1;
	int32_t ddF_y = -r - r;
	int32_t y     = 0;

	delta++;

	while (y < r) {
	if (f >= 0) {
	if (cornername & 0x1) drawFastHLine(x0 - y, y0 + r, y + y + delta, color);
	ddF_y += 2;
	f     += ddF_y;
	if (cornername & 0x2) drawFastHLine(x0 - y, y0 - r, y + y + delta, color);
	r--;
	}

	y++;
	if (cornername & 0x1) drawFastHLine(x0 - r, y0 + y, r + r + delta, color);
	ddF_x += 2;
	f     += ddF_x;
	if (cornername & 0x2) drawFastHLine(x0 - r, y0 - y, r + r + delta, color);
	}
}

/***************************************************************************************
** Function name:           fillCircleHelperAA
** Description:             Support function for fillRoundRectAA()
***************************************************************************************/
inline void TFTLIB_SPI::fillCircleHelperAA(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint16_t color) {
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y     = 0;

  delta++;

  while (y < r) {
    if (f >= 0) {
      if (cornername & 0x1) drawWideLine(x0 - y, y0 + r, x0 + y + delta, y0 + r, 1, color);
      ddF_y += 2;
      f     += ddF_y;
      if (cornername & 0x2) drawWideLine(x0 - y, y0 - r, x0 + y + delta, y0 - r, 1, color);
      r--;
    }

    y++;
    if (cornername & 0x1) drawWideLine(x0 - r, y0 + y, x0 + r + delta, y0 + y, 1, color);
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x2) drawWideLine(x0 - r, y0 - y, x0 + r + delta, y0 - y, 1, color);
  }
}

/***************************************************************************************
** Function name:           drawLine
** Description:             Draw a line with single color
***************************************************************************************/
void TFTLIB_SPI::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color)
{
	if(x0 < 0 || x0 > _width || y0 < 0 || y0 > _height || x1 < 0 || x1 > _width || y1 < 0 || y1 > _height) return;
	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap_coord(x0, y0);
		swap_coord(x1, y1);
	}

	if (x0 > x1) {
		swap_coord(x0, x1);
		swap_coord(y0, y1);
	}

	int32_t dx = x1 - x0, dy = abs(y1 - y0);

	int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

	if (y0 < y1) ystep = 1;

	// Split into steep and not steep for FastH/V separation
	if (steep) {
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				if (dlen == 1) drawPixel(y0, xs, color);
				else drawFastVLine(y0, xs, dlen, color);
				dlen = 0;
				y0 += ystep; xs = x0 + 1;
				err += dx;
			}
		}
		if (dlen) drawFastVLine(y0, xs, dlen, color);
	}
	else {
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				if (dlen == 1) drawPixel(xs, y0, color);
				else drawFastHLine(xs, y0, dlen, color);
				dlen = 0;
				y0 += ystep; xs = x0 + 1;
				err += dx;
			}
		}
		if (dlen) drawFastHLine(xs, y0, dlen, color);
	}
}

/***************************************************************************************
** Function name:           drawWideLine
** Description:             Draw anti-aliased line with single color
***************************************************************************************/
void TFTLIB_SPI::drawWideLine(float ax, float ay, float bx, float by, float wd, uint16_t fg_color) {
	drawWedgeLine( ax, ay, bx, by, wd/2.0, wd/2.0, fg_color, 0xFFFF);
}

/***************************************************************************************
** Function name:           drawWideLine
** Description:             Draw anti-aliased line with single color
***************************************************************************************/
void TFTLIB_SPI::drawWideLine(float ax, float ay, float bx, float by, float wd, uint16_t fg_color, uint16_t bg_color) {
	drawWedgeLine( ax, ay, bx, by, wd/2.0, wd/2.0, fg_color, bg_color);
}

/***************************************************************************************
** Function name:           drawWedgeLine
** Description:             Draw anti-aliased line with single color
***************************************************************************************/
void TFTLIB_SPI::drawWedgeLine(float ax, float ay, float bx, float by, float ar, float br, uint16_t fg_color) {
	drawWedgeLine(ax, ay, bx, by, ar, br, fg_color, 0xFFFF);
}

/***************************************************************************************
** Function name:           drawWedgeLine
** Description:             Draw anti-aliased line with single color
***************************************************************************************/
void TFTLIB_SPI::drawWedgeLine(float ax, float ay, float bx, float by, float ar, float br, uint16_t fg_color, uint16_t bg_color) {
	if ( (abs(ax - bx) < 0.01f) && (abs(ay - by) < 0.01f) ) bx += 0.01f;  // Avoid divide by zero

	// Find line bounding box
	int32_t x0 = (int32_t)floorf(fminf(ax-ar, bx-br));
	int32_t x1 = (int32_t) ceilf(fmaxf(ax+ar, bx+br));
	int32_t y0 = (int32_t)floorf(fminf(ay-ar, by-br));
	int32_t y1 = (int32_t) ceilf(fmaxf(ay+ar, by+br));

	// Establish x start and y start
	int32_t ys = ay;
	if ((ax-ar)>(bx-br)) ys = by;

	float rdt = ar - br; // Radius delta
	float alpha = 1.0f;
	ar += 0.5;

	float xpax, ypay, bax = bx - ax, bay = by - ay;

	int32_t xs = x0;
	// Scan bounding box from ys down, calculate pixel intensity from distance to line
	for (int32_t yp = ys; yp <= y1; yp++) {
		bool swin = true;  // Flag to start new window area
		bool endX = false; // Flag to skip pixels
		ypay = yp - ay;
		for (int32_t xp = xs; xp <= x1; xp++) {
			if (endX) if (alpha <= LoAlphaTheshold) break;  // Skip right side
			xpax = xp - ax;
			alpha = ar - wedgeLineDistance(xpax, ypay, bax, bay, rdt);
			if (alpha <= LoAlphaTheshold ) continue;
			// Track edge to minimise calculations
			if (!endX) {
				endX = true;
				xs = xp;
			}

			if (alpha > HiAlphaTheshold) {
				if (swin) {
					setWindow(xp, yp, width()-1, yp);
					swin = false;
				}
				pushBlock(fg_color);
				continue;
			}

			//Blend color with background and plot
			if(bg_color == 0xFFFF) {
				swin = true;
			}

			if (swin) {
				setWindow(xp, yp, width()-1, yp);
				swin = false;
			}
			pushBlock(alphaBlend((uint8_t)(alpha * PixelAlphaGain), fg_color, bg_color));
		}
	}

	// Reset x start to left side of box
	xs = x0;
	// Scan bounding box from ys-1 up, calculate pixel intensity from distance to line
	for (int32_t yp = ys-1; yp >= y0; yp--) {
		bool swin = true;  // Flag to start new window area
		bool endX = false; // Flag to skip pixels
		ypay = yp - ay;
		for (int32_t xp = xs; xp <= x1; xp++) {
			if (endX) if (alpha <= LoAlphaTheshold) break;  // Skip right side of drawn line
			xpax = xp - ax;
			alpha = ar - wedgeLineDistance(xpax, ypay, bax, bay, rdt);
			if (alpha <= LoAlphaTheshold ) continue;
			// Track line boundary
			if (!endX) {
				endX = true;
				xs = xp;
			}
			if (alpha > HiAlphaTheshold) {
				if (swin) {
					setWindow(xp, yp, width()-1, yp);
					swin = false;
				}
				pushBlock(fg_color);
				continue;
			}

			//Blend color with background and plot
			if(bg_color == 0xFFFF) {
				swin = true;
			}
			if (swin) {
				setWindow(xp, yp, width()-1, yp);
				swin = false;
			}
			pushBlock(alphaBlend((uint8_t)(alpha * PixelAlphaGain), fg_color, bg_color));
		}
	}
}

/***************************************************************************************
** Function name:           drawTriangle
** Description:             Draw a triangle with single color
***************************************************************************************/
void TFTLIB_SPI::drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint16_t color)
{
	drawLine(x1, y1, x2, y2, color);
	drawLine(x2, y2, x3, y3, color);
	drawLine(x3, y3, x1, y1, color);
}

/***************************************************************************************
** Function name:           drawTriangleAA
** Description:             Draw anti-aliased triangle with single color and specified thickness
***************************************************************************************/
void TFTLIB_SPI::drawTriangleAA(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t thickness, uint16_t color) {
	drawWideLine(x1, y1, x2, y2, thickness, color);
	drawWideLine(x2, y2, x3, y3, thickness, color);
	drawWideLine(x3, y3, x1, y1, thickness, color);
}

/***************************************************************************************
** Function name:           drawRect
** Description:             Draw rectangle with single color
***************************************************************************************/
void TFTLIB_SPI::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
{
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;
	drawFastHLine(x, y, w, color);
	drawFastHLine(x, y + h, w, color);
	drawFastVLine(x, y, h, color);
	drawFastVLine(x + w, y, h, color);
}

/***************************************************************************************
** Function name:           drawRectAA
** Description:             Draw anti-aliased rectangle with single color
***************************************************************************************/
void TFTLIB_SPI::drawRectAA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
{
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;
	drawWideLine(x, y, x + w, y, 1, color);
	drawWideLine(x, y, x, y + h, 1, color);
	drawWideLine(x + w, y, x + w, y + h, 1, color);
	drawWideLine(x, y + h, x + w, y + h, 1, color);
}

/***************************************************************************************
** Function name:           drawRoundRect
** Description:             Draw a rectangle with rounded corners
***************************************************************************************/
void TFTLIB_SPI::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color)
{
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;
	drawFastHLine(x + r  , y    , w - r - r, color); // Top
	drawFastHLine(x + r  , y + h - 1, w - r - r, color); // Bottom
	drawFastVLine(x    , y + r  , h - r - r, color); // Left
	drawFastVLine(x + w - 1, y + r  , h - r - r, color); // Right

	// draw four corners
	drawCircleHelper(x + r    , y + r    , r, 1, color);
	drawCircleHelper(x + w - r - 1, y + r    , r, 2, color);
	drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	drawCircleHelper(x + r    , y + h - r - 1, r, 8, color);
}

/***************************************************************************************
** Function name:           drawCircle
** Description:             Draw a circle with single color
***************************************************************************************/
void TFTLIB_SPI::drawCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color)
{
	if(x0 - r < 0 || x0 + r > _width || y0 - r < 0 || y0 + r > _height)	return;

	int32_t f     = 1 - r;
	int32_t ddF_y = -2 * r;
	int32_t ddF_x = 1;
	int32_t xs    = -1;
	int32_t xe    = 0;
	int32_t len   = 0;

	bool first = true;
	do {
		while (f < 0) {
			++xe;
			f += (ddF_x += 2);
		}
		f += (ddF_y += 2);

		if (xe-xs>1) {
			if (first) {
				len = 2*(xe - xs)-1;
				drawFastHLine(x0 - xe, y0 + r, len, color);
				drawFastHLine(x0 - xe, y0 - r, len, color);
				drawFastVLine(x0 + r, y0 - xe, len, color);
				drawFastVLine(x0 - r, y0 - xe, len, color);
				first = false;
			}
			else {
				len = xe - xs++;
				drawFastHLine(x0 - xe, y0 + r, len, color);
				drawFastHLine(x0 - xe, y0 - r, len, color);
				drawFastHLine(x0 + xs, y0 - r, len, color);
				drawFastHLine(x0 + xs, y0 + r, len, color);

				drawFastVLine(x0 + r, y0 + xs, len, color);
				drawFastVLine(x0 + r, y0 - xe, len, color);
				drawFastVLine(x0 - r, y0 - xe, len, color);
				drawFastVLine(x0 - r, y0 + xs, len, color);
			}
		}
		else {
			++xs;
			drawPixel(x0 - xe, y0 + r, color);
			drawPixel(x0 - xe, y0 - r, color);
			drawPixel(x0 + xs, y0 - r, color);
			drawPixel(x0 + xs, y0 + r, color);

			drawPixel(x0 + r, y0 + xs, color);
			drawPixel(x0 + r, y0 - xe, color);
			drawPixel(x0 - r, y0 - xe, color);
			drawPixel(x0 - r, y0 + xs, color);
		}
		xs = xe;
	} while (xe < --r);
}

/***************************************************************************************
** Function name:           drawEllipse
** Description:             Draw an ellipse with single color
***************************************************************************************/
void TFTLIB_SPI::drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
	if(x0 - rx < 0 || x0 + rx > _width || y0 - ry < 0 || y0 + ry > _height || rx < 2 || ry < 2) return;
	int32_t x, y;
	int32_t rx2 = rx * rx;
	int32_t ry2 = ry * ry;
	int32_t fx2 = 4 * rx2;
	int32_t fy2 = 4 * ry2;
	int32_t s;

	for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + x, y0 - y, color);
		if (s >= 0) {
			s += fx2 * (1 - y);
			y--;
		}
		s += ry2 * ((4 * x) + 6);
	}

	for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + x, y0 - y, color);
		if (s >= 0) {
			s += fy2 * (1 - x);
			x--;
		}
		s += rx2 * ((4 * y) + 6);
	}
}

/***************************************************************************************
** Function name:           fillTriangle
** Description:             Draw filled triangle with fixed color
***************************************************************************************/
void TFTLIB_SPI::fillTriangle ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
  int32_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap_coord(y0, y1); swap_coord(x0, x1);
  }
  if (y1 > y2) {
    swap_coord(y2, y1); swap_coord(x2, x1);
  }
  if (y0 > y1) {
    swap_coord(y0, y1); swap_coord(x0, x1);
  }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)      a = x1;
    else if (x1 > b) b = x1;
    if (x2 < a)      a = x2;
    else if (x2 > b) b = x2;
    drawFastHLine(a, y0, b - a + 1, color);
    return;
  }

  int32_t
  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1,
  sa   = 0,
  sb   = 0;

  if (y1 == y2) last = y1;
  else last = y1 - 1;

  for (y = y0; y <= last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b) swap_coord(a, b);
    drawFastHLine(a, y, b - a + 1, color);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b) swap_coord(a, b);
    drawFastHLine(a, y, b - a + 1, color);
  }
}

/***************************************************************************************
** Function name:           fillTriangleAA
** Description:             Draw anti-aliased filled triangle with fixed color
***************************************************************************************/
void TFTLIB_SPI::fillTriangleAA( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
  int32_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap_coord(y0, y1); swap_coord(x0, x1);
  }
  if (y1 > y2) {
    swap_coord(y2, y1); swap_coord(x2, x1);
  }
  if (y0 > y1) {
    swap_coord(y0, y1); swap_coord(x0, x1);
  }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)      a = x1;
    else if (x1 > b) b = x1;
    if (x2 < a)      a = x2;
    else if (x2 > b) b = x2;
    drawWideLine(a, y0, b + 1, y0, 1, color);
    return;
  }

  int32_t
  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1,
  sa   = 0,
  sb   = 0;

  if (y1 == y2) last = y1;
  else last = y1 - 1;

  for (y = y0; y <= last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b) swap_coord(a, b);
    drawWideLine(a, y, b + 1, y, 1, color);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b) swap_coord(a, b);
    drawWideLine(a, y, b + 1, y, 1, color);
  }
}

/***************************************************************************************
** Function name:           fillRect
** Description:             Draw a filled rectangle with fixed color
***************************************************************************************/
void TFTLIB_SPI::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
{
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;

	uint32_t buff_size = 0;
	setWindow(x, y, x + w - 1, y + h - 1);

	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;

	buff_size = w * h;

	fill_n(__buffer, __buffer_size, SWAP_UINT16(color));

	while (buff_size > (8 * __buffer_size)) {
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);

		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		buff_size -= (__buffer_size * 8);
	}

	while (buff_size > __buffer_size) {
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, __buffer_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		buff_size -= __buffer_size;
	}

	while (buff_size > 0) {
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, buff_size*2);
		while (_bus->State != HAL_SPI_STATE_READY);
		buff_size -= buff_size;
	}
	CS_PORT->BSRR = (uint32_t)CS_PIN;
}

/***************************************************************************************
** Function name:           fillRectAA
** Description:             Draw anti-aliased filled rectangle with fixed color
***************************************************************************************/
void TFTLIB_SPI::fillRectAA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
{
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;

	uint32_t chunk_size = 0, buff_size = 0;
	setWindow(x, y, x + w - 1, y + h - 1);

	CS_PORT->BSRR = (uint32_t)CS_PIN << 16U;
	DC_PORT->BSRR = (uint32_t)DC_PIN;

	if(w * h >  __buffer_size){
		chunk_size = w;
		fill_n(__buffer, w, SWAP_UINT16(color));
	}

	else{
		chunk_size = w * h;
		fill_n(__buffer, w*h, SWAP_UINT16(color));
	}

	buff_size = w * h;

	while (buff_size > 0) {
		HAL_SPI_Transmit_DMA(_bus, (uint8_t*)__buffer, chunk_size*2);
		buff_size -= chunk_size;
		while (_bus->State != HAL_SPI_STATE_READY);
	}

	CS_PORT->BSRR = (uint32_t)CS_PIN;

	drawWideLine(x, y, x + w - 1, y, 1, color);
	drawWideLine(x, y+h-1, x+w-1, y+h-1, 1, color);

	drawWideLine(x, y, x, y + h - 1, 1, color);
	drawWideLine(x + w, y, x + w, y + h - 1, 1, color);
}

/***************************************************************************************
** Function name:           fillRoundRect
** Description:             Draw a filled rectangle with rounded corners & single color
***************************************************************************************/
void TFTLIB_SPI::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color) {
  fillRect(x, y + r, w, h - r - r, color);

  fillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  fillCircleHelper(x + r    , y + r, r, 2, w - r - r - 1, color);
}

/***************************************************************************************
** Function name:           fillRoundRectAA
** Description:             Draw a filled rectangle with rounded corners & single color
***************************************************************************************/
void TFTLIB_SPI::fillRoundRectAA(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint16_t color) {
  fillRectAA(x, y + r, w, h - r - r, color);

  fillCircleHelperAA(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  fillCircleHelperAA(x + r    , y + r, r, 2, w - r - r - 1, color);
}

/***************************************************************************************
** Function name:           fillCircle
** Description:             Draw a filled circle with single color
***************************************************************************************/
void TFTLIB_SPI::fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color){
	if(x - r < 0 || x + r > _width || y - r < 0 || y + r > _height)	return;
	int16_t  xs  = 0;
	int16_t  dx = 1;
	int16_t  dy = r+r;
	int16_t  p  = -(r>>1);

	drawFastHLine(x - r, y, dy+1, color);

	while(x < r) {
		if(p >= 0) {
			drawFastHLine(x - xs, y + r, dx, color);
			dy-=2;
			p-=dy;

			drawFastHLine(x - xs, y - r, dx, color);
			r--;
		}

		x++;
		drawFastHLine(x - r, y + xs, dy+1, color);

		dx+=2;
		p+=dx;
		drawFastHLine(x - r, y - xs, dy+1, color);
	}
}

/***************************************************************************************
** Function name:           fillCircleAA
** Description:             Draw anti-aliased filled circle with fixed color
***************************************************************************************/
void TFTLIB_SPI::fillCircleAA(float x, float y, float r, uint16_t color) {
	if(x - r < 0 || x + r > _width || y - r < 0 || y + r > _height)	return;
	drawWedgeLine(x, y, x, y, r, r, color, 0xFFFF);
}

/***************************************************************************************
** Function name:           fillEllipse
** Description:             Draw a filled ellipse with single color
***************************************************************************************/
void TFTLIB_SPI::fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
	if(x0 - rx < 0 || x0 + rx > _width || y0 - ry < 0 || y0 + ry > _height || rx < 2 || ry < 2) return;
	int32_t x, y;
	int32_t rx2 = rx * rx;
	int32_t ry2 = ry * ry;
	int32_t fx2 = 4 * rx2;
	int32_t fy2 = 4 * ry2;
	int32_t s;

	for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
		drawFastHLine(x0 - x, y0 - y, x + x + 1, color);
		drawFastHLine(x0 - x, y0 + y, x + x + 1, color);

		if (s >= 0) {
			s += fx2 * (1 - y);
			y--;
		}
		s += ry2 * ((4 * x) + 6);
	}

	for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
		drawFastHLine(x0 - x, y0 - y, x + x + 1, color);
		drawFastHLine(x0 - x, y0 + y, x + x + 1, color);

		if (s >= 0) {
			s += fy2 * (1 - x);
			x--;
		}
		s += rx2 * ((4 * y) + 6);
	}
}

/***************************************************************************************
** Function name:           drawImage
** Description:             Draw image at coords x&y
***************************************************************************************/
void TFTLIB_SPI::drawImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data) {
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;

	setWindow(x, y, x + w - 1, y + h - 1);
	writeData_DMA((uint8_t *)data, w * h * 2);
}

/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw bitmap from array with fixed color
***************************************************************************************/
void TFTLIB_SPI::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bitmap, uint16_t color) {
	if(x < 0 || y < 0 || w < 0 || h < 0 || x > _width || y > _height || x + w > _width || y + h > _height) return;

	int32_t i, j, byteWidth = (w + 7) / 8;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++ ) {
			if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
				drawPixel(x + i, y + j, color);
			}
		}
	}
}

/***************************************************************************************
** Function name:           setCursor
** Description:             Set text cursor at x&y. Used for print/println function
***************************************************************************************/
void TFTLIB_SPI::setCursor(int32_t x, int32_t y){
	if(x > _width || y > _height) return;
	_posx = x;
	_posy = y;
}

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set text cursor at x&y. Used for print/println function
***************************************************************************************/
void TFTLIB_SPI::setTextColor(int32_t fg, int32_t bg){
	__text_fg = fg;
	__text_bg = bg;
}

/***************************************************************************************
** Function name:           setFont
** Description:             Set text cursor at x&y. Used for print/println function
***************************************************************************************/
void TFTLIB_SPI::setFont(FontDef &Font){
	__font = &Font;
}

/***************************************************************************************
** Function name:           writeChar
** Description:             Print character at coords x&y with selected font
***************************************************************************************/
void TFTLIB_SPI::writeChar(int32_t x, int32_t y, char ch) {
	int32_t i=0, b=0, j=0;

	if(x + __font->width > _width || y + __font->height > _height) return;
	setWindow(x, y, x + __font->width - 1, y + __font->height - 1);

	for (i = 0; i < __font->height; i++) {
		b = __font->data[(ch - 32) * __font->height + i];
		for (j = 0; j < __font->width; j++) {
			if ((b << j) & 0x8000) {
				__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_fg);
			}
			else {
				__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_bg);
			}
		}
	}
	writeData_DMA((uint8_t*)__buffer, __font->width * __font->height * 2);
}

/***************************************************************************************
** Function name:           writeString
** Description:             Print string at coords x&y with selected font
***************************************************************************************/
void TFTLIB_SPI::writeString(int32_t x, int32_t y, char *ch) {

	while(*ch){
		if(strcmp(reinterpret_cast<const char*>(&ch), " ") == 0){
			x += __font->width;
			ch++;
			return;
		}

		else {
			if(x + __font->width > _width || y + __font->height > _height) return;
			int32_t i=0, b=0, j=0;
			setWindow(x, y, x + __font->width - 1, y + __font->height - 1);

			for (i = 0; i < __font->height; i++) {
				b = __font->data[(*ch - 32) * __font->height + i];
				for (j = 0; j < __font->width; j++) {
					if ((b << j) & 0x8000) {
						__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_fg);
					}
					else {
						__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_bg);
					}
				}
			}
			writeData_DMA((uint8_t*)__buffer, __font->width * __font->height * 2);
			x += __font->width;
			ch++;
		}
	}
}

/***************************************************************************************
** Function name:           print
** Description:             Print string with selected font
***************************************************************************************/
void TFTLIB_SPI::print(char *ch) {
	int32_t i=0, b=0, j=0;

	while(*ch){
		if(strcmp(reinterpret_cast<const char*>(&ch), " ") == 0){
			_posx += __font->width;
			ch++;
			return;
		}

		setWindow(_posx, _posy, _posx + __font->width - 1, _posy + __font->height - 1);
		for (i = 0; i < __font->height; i++) {
			b = __font->data[(*ch - 32) * __font->height + i];
			for (j = 0; j < __font->width; j++) {
				if ((b << j) & 0x8000) {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_fg);
				}
				else {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_bg);
				}
			}
		}
		writeData_DMA((uint8_t*)__buffer, __font->width * __font->height * 2);
		_posx += __font->width;
		ch++;
	}
}

/***************************************************************************************
** Function name:           println
** Description:             Print string with selected font
***************************************************************************************/
void TFTLIB_SPI::println(uint8_t *ch)
{
	int32_t cur_x = _posx;
	int32_t i=0, b=0, j=0;

	while(*ch){
		if(*ch < 32 || *ch > 128 || *ch == 0) return;
		if(cur_x + __font->width > _width) setCursor(0, _posy + __font->height);

		setWindow(cur_x, _posy, cur_x + __font->width - 1, _posy + __font->height - 1);

		for (i = 0; i < __font->height; i++) {
			b = __font->data[(*ch - 32) * __font->height + i];
			for (j = 0; j < __font->width; j++) {
				if ((b << j) & 0x8000) {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_fg);
				}
				else {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_bg);
				}
			}
		}
		writeData_DMA((uint8_t*)__buffer, __font->width * __font->height * 2);
		cur_x += __font->width;
		ch++;
	}

	if(_posy + __font->height > _height)
		_posy = 0;
	else
		_posy += __font->height;
}

void TFTLIB_SPI::println(char *ch)
{
	int32_t cur_x = _posx;
	int32_t i=0, b=0, j=0;

	while(*ch){
		if(*ch < 32 || *ch > 128 || *ch == 0) return;
		if(cur_x + __font->width > _width) setCursor(0, _posy + __font->height);

		setWindow(cur_x, _posy, cur_x + __font->width - 1, _posy + __font->height - 1);

		for (i = 0; i < __font->height; i++) {
			b = __font->data[(*ch - 32) * __font->height + i];
			for (j = 0; j < __font->width; j++) {
				if ((b << j) & 0x8000) {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_fg);
				}
				else {
					__buffer[(i*__font->width) + j] = SWAP_UINT16(__text_bg);
				}
			}
		}
		writeData_DMA((uint8_t*)__buffer, __font->width * __font->height * 2);
		cur_x += __font->width;
		ch++;
	}

	if(_posy + __font->height > _height)
		_posy = 0;
	else
		_posy += __font->height;
}

/***************************************************************************************************************************
** 												Test functions for display benchmark
****************************************************************************************************************************/
uint32_t TFTLIB_SPI::testFillScreen() {
	unsigned long start = HAL_GetTick();
	fillScreen(BLACK);
	fillScreen(RED);
	fillScreen(GREEN);
	fillScreen(BLUE);
	fillScreen(BLACK);
	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testText() {
	fillScreen(BLACK);

	unsigned long start = HAL_GetTick();
	setFont(Font_7x10);
	setTextColor(WHITE, BLACK);
	writeString(0, 0, (char*)"Hello World!");

	setFont(Font_11x18);
	setTextColor(YELLOW, BLACK);
	writeString(0, 10, (char*)"1234.56");

	setTextColor(RED, BLACK);
	writeString(0, 28, (char*)"0xDEADBEEF");

	setFont(Font_16x26);
	setTextColor(GREEN, BLACK);
	writeString(0, 64, (char*)"Groop");

	setFont(Font_11x18);
	setTextColor(MAGENTA, BLACK);
	writeString(0, 90, (char*)"I implore thee,");

	setFont(Font_7x10);
	setTextColor(LGRAY, BLACK);
	writeString(0, 108, (char*)"And hooptiously drangle me");
	writeString(0, 118, (char*)"with crinkly bindlewurdles,");
	writeString(0, 128, (char*)"Or I will rend thee");
	writeString(0, 138, (char*)"in the gobberwarts");
	writeString(0, 148, (char*)"with my blurglecruncheon,");
	writeString(0, 158, (char*)"see if I don't!");
	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testLines(uint16_t color) {
	unsigned long start, t;
	int           x1, y1, x2, y2,
				w = _width,
				h = _height;

	fillScreen(BLACK);

	x1 = y1 = 0;
	y2    = h - 1;
	start = HAL_GetTick();
	for (x2 = 0; x2 < w; x2 += 6) drawLine(x1, y1, x2, y2, color);
	x2    = w - 1;
	for (y2 = 0; y2 < h; y2 += 6) drawLine(x1, y1, x2, y2, color);
	t     = HAL_GetTick() - start; // fillScreen doesn't count against timing

	fillScreen(BLACK);

	x1    = w - 1;
	y1    = 0;
	y2    = h - 1;
	start = HAL_GetTick();
	for (x2 = 0; x2 < w; x2 += 6) drawLine(x1, y1, x2, y2, color);
	x2    = 0;
	for (y2 = 0; y2 < h; y2 += 6) drawLine(x1, y1, x2, y2, color);
	t    += HAL_GetTick() - start;

	fillScreen(BLACK);

	x1    = 0;
	y1    = h - 1;
	y2    = 0;
	start = HAL_GetTick();
	for (x2 = 0; x2 < w; x2 += 6) drawLine(x1, y1, x2, y2, color);
	x2    = w - 1;
	for (y2 = 0; y2 < h; y2 += 6) drawLine(x1, y1, x2, y2, color);
	t    += HAL_GetTick() - start;

	fillScreen(BLACK);

	x1    = w - 1;
	y1    = h - 1;
	y2    = 0;
	start = HAL_GetTick();
	for (x2 = 0; x2 < w; x2 += 6) drawLine(x1, y1, x2, y2, color);
	x2    = 0;
	for (y2 = 0; y2 < h; y2 += 6) drawLine(x1, y1, x2, y2, color);

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testFastLines(uint16_t color1, uint16_t color2) {
	unsigned long start;
	int           x, y, w = _width, h = _height;

	fillScreen(BLACK);
	start = HAL_GetTick();
	for (y = 0; y < h; y += 5) drawFastHLine(0, y, w, color1);
	for (x = 0; x < w; x += 5) drawFastVLine(x, 0, h, color2);

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testRects(uint16_t color) {
	unsigned long start;
	int           n, i, i2,
				cx = _width  / 2,
				cy = _height / 2;

	fillScreen(BLACK);
	n     = min(_width, _height);
	start = HAL_GetTick();
	for (i = 2; i < n; i += 6) {
		i2 = i / 2;
		drawRect(cx - i2, cy - i2, i, i, color);
	}

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testFilledRects(uint16_t color1, uint16_t color2) {
	unsigned long start, t = 0;
	int           n, i, i2,
				cx = _width  / 2 - 1,
				cy = _height / 2 - 1;

	fillScreen(BLACK);
	n = min(_width, _height);
	for (i = n - 1; i > 0; i -= 6) {
		i2    = i / 2;
		start = HAL_GetTick();
		fillRect(cx - i2, cy - i2, i, i, color1);
		t    += HAL_GetTick() - start;
		// Outlines are not included in timing results
		drawRect(cx - i2, cy - i2, i, i, color2);
	}
	return t;
}

uint32_t TFTLIB_SPI::testFilledCircles(uint8_t radius, uint16_t color) {
	unsigned long start;
	int x, y, w = _width, h = _height, r2 = radius * 2;

	fillScreen(BLACK);
	start = HAL_GetTick();
	for (x = radius; x < w; x += r2) {
		for (y = radius; y < h; y += r2) {
			fillCircle(x, y, radius, color);
		}
	}
	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testCircles(uint8_t radius, uint16_t color) {
	unsigned long start;
	int           x, y, r2 = radius * 2,
					  w = _width  + radius,
					  h = _height + radius;

	// Screen is not cleared for this one -- this is
	// intentional and does not affect the reported time.
	start = HAL_GetTick();
	for (x = 0; x < w; x += r2) {
		for (y = 0; y < h; y += r2) {
			drawCircle(x, y, radius, color);
		}
	}

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testTriangles() {
	unsigned long start;
	int           n, i, cx = _width  / 2 - 1,
				  cy = _height / 2 - 1;

	fillScreen(BLACK);
	n     = min(cx, cy);
	start = HAL_GetTick();

	for (i = 0; i < n; i += 5) {
		drawTriangle(
		cx    , cy - i, // peak
		cx - i, cy + i, // bottom left
		cx + i, cy + i, // bottom right
		color565(0, 0, i));
	}

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testFilledTriangles() {
	unsigned long start, t = 0;
	int           i, cx = _width  / 2 - 1,
				   cy = _height / 2 - 1;

	fillScreen(BLACK);
	start = HAL_GetTick();
	for (i = min(cx, cy); i > 10; i -= 5) {
		start = HAL_GetTick();
		fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i, color565(0, i, i));
		t += HAL_GetTick() - start;
		drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i, color565(i, i, 0));
	}

	return t;
}

uint32_t TFTLIB_SPI::testRoundRects() {
	unsigned long start;
	int           w, i, i2,
				cx = _width  / 2 - 1,
				cy = _height / 2 - 1;

	fillScreen(BLACK);
	w     = min(_height, _width);
	start = HAL_GetTick();
	for (i = 0; i < w; i += 6) {
		i2 = i / 2;
		drawRoundRect(cx - i2, cy - i2, i, i, i / 8, color565(i, 0, 0));
	}

	return HAL_GetTick() - start;
}

uint32_t TFTLIB_SPI::testFilledRoundRects() {
	unsigned long start;
	int           i, i2,
				cx = _width  / 2 - 1,
				cy = _height / 2 - 1;

	fillScreen(BLACK);
	start = HAL_GetTick();
	for (i = min(_width , _height); i > 20; i -= 6) {
		i2 = i / 2;
		fillRoundRect(cx - i2, cy - i2, i, i, i / 8, color565(0, i, 0));
	}

	return HAL_GetTick() - start;
}

void TFTLIB_SPI::benchmark(void){
	char buffer[32];
	uint32_t t1 = testFillScreen();
	uint32_t t2 = testText();

	uint32_t t3 = testLines(CYAN);
	uint32_t t4 = testFastLines(RED, BLUE);

	uint32_t t5 = testRects(GREEN);
	uint32_t t6 = testFilledRects(YELLOW, MAGENTA);

	uint32_t t7 = testCircles(10, WHITE);
	uint32_t t8 = testFilledCircles(10, MAGENTA);

	uint32_t t9 = testTriangles();
	uint32_t t10 = testFilledTriangles();

	uint32_t t11 = testRoundRects();
	uint32_t t12 = testFilledRoundRects();

	uint32_t total = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;

	fillScreen(BLACK);

	setFont(Font_11x18);
	setTextColor(RED, BLACK);
	sprintf(buffer, "Fillscreen:         %lums", t1);
	writeString(0, 0, buffer);

	sprintf(buffer, "Text:               %lums", t2);
	writeString(0, 18, buffer);

	sprintf(buffer, "Lines:              %lums", t3);
	writeString(0, 36, buffer);

	sprintf(buffer, "Hor/Vert Lines:     %lums", t4);
	writeString(0, 54, buffer);

	sprintf(buffer, "Rect(outline):      %lums", t5);
	writeString(0, 72, buffer);

	sprintf(buffer, "Rect(filled):       %lums", t6);
	writeString(0, 90, buffer);

	sprintf(buffer, "Circ(outline):      %lums", t7);
	writeString(0, 108, buffer);

	sprintf(buffer, "Circ(filled):       %lums", t8);
	writeString(0, 126, buffer);

	sprintf(buffer, "Tri(outline):       %lums", t9);
	writeString(0, 144, buffer);

	sprintf(buffer, "Tri(filled):        %lums", t10);
	writeString(0, 162, buffer);

	sprintf(buffer, "Round Rects:        %lums", t11);
	writeString(0, 180, buffer);

	sprintf(buffer, "Filled Round Rects: %lums", t12);
	writeString(0, 198, buffer);

	sprintf(buffer, "Time total:         %lums", total);
	writeString(0, 216, buffer);

	HAL_Delay(4000);
}

/***************************************************************************************
** Function name:           Button
** Description:             Constructor
***************************************************************************************/
Button::Button(void) {
	__tft       = 0;
	__xd        = 0;
	__yd        = 5;
}

/***************************************************************************************
** Function name:           ~Button
** Description:             Destructor
***************************************************************************************/
Button::~Button(void) { }

/***************************************************************************************
** Function name:           initButton
** Description:             Initialize centered button
***************************************************************************************/
void Button::initButton(
	TFTLIB_SPI *tft, XPT2046_Touchscreen *ts, int16_t x, int16_t y, uint16_t w, uint16_t h,
	uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, FontDef* Font)
{
	initButtonUL(tft, ts, x - (w / 2), y - (h / 2), w, h, outline, fill, textcolor, label, Font);
}

/***************************************************************************************
** Function name:           initButton
** Description:             Classic initialize button
***************************************************************************************/
void Button::initButtonUL(
	TFTLIB_SPI *tft, XPT2046_Touchscreen *ts, int16_t x1, int16_t y1, uint16_t w, uint16_t h,
	uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, FontDef* Font)
{
	__x1				= x1;
	__y1				= y1;
	__w					= w;
	__h					= h;
	__outlinecolor		= outline;
	__fillcolor			= fill;
	__textcolor			= textcolor;
	__tft				= tft;
	__ts				= ts;
	__font = Font;
	__text_w			= __font->width;
	__text_h			= __font->height;
	strncpy(__label, label, 19);
}

/***************************************************************************************
** Function name:           setLabelOffset
** Description:             Adjust text datum on x, y coordinates
***************************************************************************************/
void Button::setLabelOffset(int16_t x_delta, int16_t y_delta) {
	__xd        = x_delta;
	__yd        = y_delta;
}

/***************************************************************************************
** Function name:           drawButton
** Description:             Draw button with label
***************************************************************************************/
void Button::drawButton(bool inverted, char* long_name) {
	uint16_t fill, outline, text;
	uint16_t text_size = sizeof(long_name);

	if(!inverted) {
		fill    = __fillcolor;
		outline = __outlinecolor;
		text    = __textcolor;
	}

	else {
		fill    = __textcolor;
		outline = __outlinecolor;
		text    = __fillcolor;
		}

	uint8_t r = min(__w, __h) / 2; // Corner radius
	__tft->fillRoundRect(__x1, __y1, __w, __h, r, fill);
	__tft->drawRoundRect(__x1, __y1, __w, __h, r, outline);
	__tft->setTextColor(text, fill);
	__tft->setFont(*__font);
	if (strcmp(long_name, "") == 0) __tft->writeString(__x1 + (__w/2) + __xd - ((text_size*__text_w)/2), __y1 + (__h/2) - 4 + __yd - (__text_h/2), __label/*, *_font, text, fill*/);
	else  __tft->writeString(__x1 + (__w/2) + __xd - ((text_size*__text_w)/2), __y1 + (__h/2) - 4 + __yd - (__text_h/2), long_name/*, *_font, text, fill*/);
}

/***************************************************************************************
** Function name:           contains
** Description:             Check if button is pressed in correct coordinates
***************************************************************************************/
bool Button::contains(int16_t x, int16_t y) {
	return ((x >= __x1) && (x < (__x1 + __w)) && (y >= __y1) && (y < (__y1 + __h)));
}

/***************************************************************************************
** Function name:           process
** Description:             New process function with callbacks to set task our button
***************************************************************************************/
void Button::process(char* label_pressed, char* label_release, Callback btn_func, Callback end_func, XPT2046_Touchscreen &ts){
	int32_t x = 0, y = 0;
	ts.getTouch(&x, &y);
	bool pressed_btn;

	char info[32];
	sprintf(info, "X = %ld, Y = %ld, RAM:%d    ", x, y, __tft->FreeRAM());
	__tft->setTextColor(RED, WHITE);
	__tft->writeString(0, 0, info/*, Font_11x18, YELLOW, BLUE*/);

	if (ts.pressed() && contains(x, y)){
		press(true);
		pressed_btn = 1;
		if (wasPressed()) drawButton(true, label_pressed);
	}

	else{
		press(false);
		drawButton(false, label_release);
	}


	if (wasReleased() && pressed_btn){
		btn_func();
		end_func();
		pressed_btn = 0;
	}
}

/***************************************************************************************
** Function name:           press
** Description:             Change actually button state
***************************************************************************************/
void Button::press(bool p) {
	last_state = curr_state;
	curr_state = p;
}

bool Button::isPressed()    { return curr_state; }
bool Button::wasPressed()  { return (curr_state && !last_state); }
bool Button::wasReleased() { return (!curr_state && last_state); }
