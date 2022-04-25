/*
 * st7789_drv.h
 *
 *  Created on: Jan 10, 2022
 *  Updated on: Apr 17, 2022
 *      Author: asz
 */

#ifndef INC_ST7789_DRV_H_
#define INC_ST7789_DRV_H_

{
	HAL_Delay(5);
	writeCommand(0x01);
	HAL_Delay(250);

    writeCommand(COLMOD);		//	Set color mode
    writeSmallData(COLOR_MODE_16bit);

  	writeCommand(FRMCTR2);				//	Porch control
	{
		uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
		writeData(data, sizeof(data));
	}
	setRotation(3);	//	MADCTL (Display Rotation)

	uint8_t init_sequence[8][2] = {
			//VGL -10.43 / VCOM 0.725 / LCMCTRL		/ VDVVRHEN	  / VRHS		/ VDVS		  / FRCTRL2		/ PWCTRL1
			//VGH  13.25 /			  / MX MH MY	/ CMDEN		  / GVDD 4.45	/ VDV 0		  / 105Hz		/ VDDS 2.19 AVCL -4.6 VDS 6.8
			{0xB7, 0x35}, {0xBB, 0x19}, {PWCTR1, 0x2C}, {PWCTR3, 0x01}, {PWCTR4, 0x12}, {PWCTR5, 0x20}, {0xC6, 0x02}, {0xD0, 0xA4}
	};

	for(uint8_t ini = 0; ini<8; ini++){
		writeCommand(init_sequence[ini][0]);				//	Power control
		writeSmallData(init_sequence[ini][1]);
	}

    writeSmallData (0xA1);			//	Default value
	/**************** Division line ****************/

	writeCommand(GMCTRP1);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
		writeData(data, sizeof(data));
	}

    writeCommand(GMCTRN1);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
		writeData(data, sizeof(data));
	}

	{
		uint8_t data[] = {INVON, SLPOUT, NORON, DISPON};
		for(uint8_t i = 0; i<sizeof(data)/sizeof(data[0]); i++){
			writeCommand(data[i]);
		}
	}
	HAL_Delay(50);
}

#endif /* INC_ST7789_DRV_H_ */
