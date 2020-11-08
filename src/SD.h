#ifndef SD_H
#define SD_H

#include "Arduino.h"
#include "SPI.h"
#include "Seeed_sdcard_hal.h"

class SD
{
public:
	SD (uint8_t cs, SPIClass *spi=&SPI, int hz=5000000);
	virtual ~SD ();
	
	uint32_t read(uint8_t *buffer, unsigned long long sector, unsigned int count);
	uint32_t write(uint8_t *buffer, unsigned long long sector, unsigned int count);
	uint32_t ioctl(uint8_t ops);
	sdcard_type_t sdcard_type();

private:
	/* data */
	ardu_sdcard_t s_card;
};

#endif