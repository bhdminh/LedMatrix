#pragma once
#include "DMD_STM32a.h"
#include <SPI.h>
#define DMD_SPI_CLOCK_18MHZ     18000000
#define DMD_SPI_CLOCK_9MHZ      9000000
#define DMD_SPI_CLOCK_4_5MHZ    4500000
#define DMD_SPI_CLOCK_2_2MHZ    2300000
#define DMD_SPI_CLOCK_1MHZ     1000000

#if defined(__STM32F1__)
#define DMD_SPI_CLOCK DMD_SPI_CLOCK_9MHZ
#define DMD_USE_DMA	1
#elif defined(__AVR_ATmega328P__)
#define DMD_SPI_CLOCK SPI_CLOCK_DIV4
#endif


class DMD_MonoChrome_SPI :
	public DMD
{
public:
	DMD_MonoChrome_SPI(byte _pin_A, byte _pin_B, byte _pin_nOE, byte _pin_SCLK, 
		               byte panelsWide, byte panelsHigh, SPIClass _spi,
		bool d_buf = false, byte dmd_pixel_x = 32, byte dmd_pixel_y = 16);
	
	~DMD_MonoChrome_SPI();

	virtual void init(uint16_t scan_interval = 2000);
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
	
	void scanDisplayBySPI();
	virtual void shiftScreen(int8_t step);
	

	
#if defined(__STM32F1__)
	void scanDisplayByDMA();
	void latchDMA();
	uint8_t spi_num = 0;
#endif
	
private:
	byte pin_DMD_R_DATA;   // is SPI Master Out 
	byte pin_DMD_CLK;      // is SPI Clock 
	uint16_t rowsize, row1, row2, row3;

	SPIClass SPI_DMD;
#if defined(__STM32F1__)
	dma_channel  spiTxDmaChannel;
	dma_dev* spiDmaDev;
	uint8_t *dmd_dma_buf;
	uint8_t *rx_dma_buf;

	
#endif

	
};

