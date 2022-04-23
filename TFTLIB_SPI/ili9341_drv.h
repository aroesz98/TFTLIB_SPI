/*
 * ili9341_drv.h
 *
 *  Created on: Jan 10, 2022
 *  Updated on: Apr 17, 2022
 *      Author: asz
 */

{
	HAL_Delay(5);
	writeCommand(0x01);
	HAL_Delay(150);

	// POWER CONTROL A
	writeCommand(0xCB);
	{
		uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
		writeData(data, sizeof(data));
	}

	// POWER CONTROL B
	writeCommand(0xCF);
	{
		uint8_t data[] = { 0x00, 0xC1, 0x30 };
		writeData(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL A
	writeCommand(0xE8);
	{
		uint8_t data[] = { 0x85, 0x00, 0x78 };
		writeData(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL B
	writeCommand(0xEA);
	{
		uint8_t data[] = { 0x00, 0x00 };
		writeData(data, sizeof(data));
	}

	// POWER ON SEQUENCE CONTROL
	writeCommand(0xED);
	{
		uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
		writeData(data, sizeof(data));
	}

	// PUMP RATIO CONTROL
	writeCommand(0xF7);
	{
		uint8_t data[] = { 0x20 };
		writeData(data, sizeof(data));
	}

	// POWER CONTROL,VRH[5:0]
	writeCommand(PWCTR1);
	{
		uint8_t data[] = { 0x23 };
		writeData(data, sizeof(data));
	}

	// POWER CONTROL,SAP[2:0];BT[3:0]
	writeCommand(PWCTR2);
	{
		uint8_t data[] = { 0x10 };
		writeData(data, sizeof(data));
	}

	// VCM CONTROL
	writeCommand(PWCTR5);
	{
		uint8_t data[] = { 0x3E, 0x28 };
		writeData(data, sizeof(data));
	}

	// VCM CONTROL 2
	writeCommand(VMCTR2);
	{
		uint8_t data[] = { 0x86 };
		writeData(data, sizeof(data));
	}

	// MEMORY ACCESS CONTROL
	setRotation(3);

	// PIXEL FORMAT
	writeCommand(COLMOD);
	{
		uint8_t data[] = {COLOR_MODE_16bit};
		writeData(data, sizeof(data));
	}

	// FRAME RATIO CONTROL, STANDARD RGB COLOR
	writeCommand(FRMCTR1);
	{
		uint8_t data[] = { 0x00, 0x18 };
		writeData(data, sizeof(data));
	}

	// DISPLAY FUNCTION CONTROL
	writeCommand(DFUNCTR);
	{
		uint8_t data[] = { 0x08, 0x82, 0x27 };
		writeData(data, sizeof(data));
	}

	// 3GAMMA FUNCTION DISABLE
	writeCommand(0xF2);
	{
		uint8_t data[] = { 0x00 };
		writeData(data, sizeof(data));
	}

	// GAMMA CURVE SELECTED
	writeCommand(GAMMASET);
	{
		uint8_t data[] = { 0x01 };
		writeData(data, sizeof(data));
	}

	// POSITIVE GAMMA CORRECTION
	writeCommand(GMCTRP1);
	{
		uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
						   0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
		writeData(data, sizeof(data));
	}

	// NEGATIVE GAMMA CORRECTION
	writeCommand(GMCTRN1);
	{
		uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
						   0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
		writeData(data, sizeof(data));
	}

	// EXIT SLEEP
	writeCommand(SLPOUT);
	HAL_Delay(120);

	// TURN ON DISPLAY
	writeCommand(DISPON);
}
