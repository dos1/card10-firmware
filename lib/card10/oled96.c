// OLED SSD1306 using the I2C interface
// Written by Larry Bank (bitbank@pobox.com)
// Project started 1/15/2017
//
// The I2C writes (through a file handle) can be single or multiple bytes.
// The write mode stays in effect throughout each call to write()
// To write commands to the OLED controller, start a byte sequence with 0x00,
// to write data, start a byte sequence with 0x40,
// The OLED controller is set to "page mode". This divides the display
// into 8 128x8 "pages" or strips. Each data write advances the output
// automatically to the next address. The bytes are arranged such that the LSB
// is the topmost pixel and the MSB is the bottom.
// The font data comes from another source and must be rotated 90 degrees
// (at init time) to match the orientation of the bits on the display memory.
// A copy of the display memory is maintained by this code so that single pixel
// writes can occur without having to read from the display controller.

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "oled96.h"
#include "i2c.h"

#define I2C_DEVICE	    MXC_I2C0_BUS0

extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static uint8_t addr;

static void oledWriteCommand(unsigned char);
//
// Initializes the OLED controller into "page mode"
// Prepares the font data for the orientation of the display
//
int oledInit(int iAddr, int bFlip, int bInvert)
{
const unsigned char initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa0,0xa1,0xc0,0xc8,
			0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
			0xaf,0x20,0x02};
unsigned char uc[4];

    addr = iAddr;

    I2C_MasterWrite(I2C_DEVICE, addr << 1, initbuf, sizeof(initbuf), 0);

	if (bInvert)
	{
		uc[0] = 0; // command
		uc[1] = 0xa7; // invert command
        I2C_MasterWrite(I2C_DEVICE, addr << 1, uc, 2, 0);
	}
	if (bFlip) // rotate display 180
	{
		uc[0] = 0; // command
		uc[1] = 0xa0;
        I2C_MasterWrite(I2C_DEVICE, addr << 1, uc, 2, 0);
		uc[1] = 0xc0;
        I2C_MasterWrite(I2C_DEVICE, addr << 1, uc, 2, 0);
	}
	return 0;
} /* oledInit() */

// Sends a command to turn off the OLED display
// Closes the I2C file handle
void oledShutdown()
{
		oledWriteCommand(0xaE); // turn off OLED
}

// Send a single byte command to the OLED controller
static void oledWriteCommand(unsigned char c)
{
unsigned char buf[2];

	buf[0] = 0x00; // command introducer
	buf[1] = c;
    I2C_MasterWrite(I2C_DEVICE, addr << 1, buf, 2, 0);
} /* oledWriteCommand() */

static void oledWriteCommand2(unsigned char c, unsigned char d)
{
unsigned char buf[3];

	buf[0] = 0x00;
	buf[1] = c;
	buf[2] = d;
    I2C_MasterWrite(I2C_DEVICE, addr << 1, buf, 3, 0);
} /* oledWriteCommand2() */

int oledSetContrast(unsigned char ucContrast)
{
	oledWriteCommand2(0x81, ucContrast);
	return 0;
} /* oledSetContrast() */

//static void oledWriteData(unsigned char c)
//{
//unsigned char buf[2];
//        buf[0] = 0x40; // data introducer
//        buf[1] = c;
//        write(file_i2c, buf, 2);
//} /* oledWriteData() */

// Send commands to position the "cursor" to the given
// row and column
static void oledSetPosition(int x, int y)
{
	oledWriteCommand(0xb0 | y); // go to page Y
	oledWriteCommand(0x00 | (x & 0xf)); // // lower col addr
	oledWriteCommand(0x10 | ((x >> 4) & 0xf)); // upper col addr
	iScreenOffset = (y*128)+x;
}

//static void oledWrite(unsigned char c)
//{
//	write(file_i2c, &c, 1);
//} /* oledWrite() */

// Write a block of pixel data to the OLED
// Length can be anything from 1 to 1024 (whole display)
static void oledWriteDataBlock(unsigned char *ucBuf, int iLen)
{
unsigned char ucTemp[129];

	ucTemp[0] = 0x40; // data command
	memcpy(&ucTemp[1], ucBuf, iLen);
    I2C_MasterWrite(I2C_DEVICE, addr << 1, ucTemp, iLen+1, 0);
	// Keep a copy in local buffer
	memcpy(&ucScreen[iScreenOffset], ucBuf, iLen);
	iScreenOffset += iLen;
}

// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
int oledSetPixel(int x, int y, unsigned char ucColor)
{
int i;
unsigned char uc, ucOld;

	i = ((y >> 3) * 128) + x;
	if (i < 0 || i > 1023) // off the screen
		return -1;
	uc = ucOld = ucScreen[i];
	uc &= ~(0x1 << (y & 7));
	if (ucColor)
	{
		uc |= (0x1 << (y & 7));
	}
	if (uc != ucOld) // pixel changed
	{
		oledSetPosition(x, y>>3);
		oledWriteDataBlock(&uc, 1);
	}
	return 0;
} /* oledSetPixel() */
//
// Draw a string of small (8x8), large (16x24), or very small (6x8)  characters
// At the given col+row
// The X position is in character widths (8 or 16)
// The Y position is in memory pages (8 lines each)
//
int oledWriteString(int x, int y, char *szMsg, int iSize)
{
int i, iLen;
unsigned char *s;

	if (iSize < FONT_NORMAL || iSize > FONT_SMALL)
		return -1;

	iLen = strlen(szMsg);
	if (iSize == FONT_BIG) // draw 16x32 font
	{
		if (iLen+x > 8) iLen = 8-x;
		if (iLen < 0) return -1;
		x *= 16;
		for (i=0; i<iLen; i++)
		{
			s = &ucFont[9728 + (unsigned char)szMsg[i]*64];
			oledSetPosition(x+(i*16), y);
			oledWriteDataBlock(s, 16);
			oledSetPosition(x+(i*16), y+1);
			oledWriteDataBlock(s+16, 16);	
			oledSetPosition(x+(i*16), y+2);
			oledWriteDataBlock(s+32, 16);	
//			oledSetPosition(x+(i*16), y+3);
//			oledWriteDataBlock(s+48, 16);	
		}
	}
	else if (iSize == FONT_NORMAL) // draw 8x8 font
	{
		oledSetPosition(x*8, y);
		if (iLen + x > 16) iLen = 16 - x; // can't display it
		if (iLen < 0)return -1;

		for (i=0; i<iLen; i++)
		{
			s = &ucFont[(unsigned char)szMsg[i] * 8];
			oledWriteDataBlock(s, 8); // write character pattern
		}	
	}
	else // 6x8
	{
		oledSetPosition(x*6, y);
		if (iLen + x > 21) iLen = 21 - x;
		if (iLen < 0) return -1;
		for (i=0; i<iLen; i++)
		{
			s = &ucSmallFont[(unsigned char)szMsg[i]*6];
			oledWriteDataBlock(s, 6);
		}
	}
	return 0;
} /* oledWriteString() */

// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
int oledFill(unsigned char ucData)
{
int y;
unsigned char temp[128];

	memset(temp, ucData, 128);
	for (y=0; y<8; y++)
	{
		oledSetPosition(0,y); // set to (0,Y)
		oledWriteDataBlock(temp, 128); // fill with data byte
	} // for y
	return 0;
} /* oledFill() */

int oledset(uint8_t *content)
{
    int y;
	for (y=0; y<8; y++) {
		oledSetPosition(0,y);
		oledWriteDataBlock(&content[128*y], 128);
	}
}
