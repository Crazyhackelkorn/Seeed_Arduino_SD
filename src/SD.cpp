#include "Arduino.h"
#include "SD.h"

SD::SD (uint8_t cs, SPIClass *spi, int hz)
{
    s_card.frequency = hz;
    s_card.spi = spi;
    s_card.ssPin = cs;

    s_card.supports_crc = true;
    s_card.type = CARD_NONE;
    s_card.status = STA_NOINIT;

    pinMode(s_card.ssPin, OUTPUT);
    digitalWrite(s_card.ssPin, HIGH);
	
	s_card.spi->begin();
}

SD::~SD ()
{
	s_card.spi->end();
}

uint32_t SD::read(uint8_t *buffer, unsigned long long sector, unsigned int count)
{
	return sd_disk_read(&s_card, buffer, sector, count);
}

uint32_t SD::write(uint8_t *buffer, unsigned long long sector, unsigned int count)
{
	return sd_disk_write(&s_card, buffer, sector, count);
}

uint32_t SD::ioctl(uint8_t ops, void* arg)
{
	uint32_t res=0;
	unsigned long buf=0;
	res=sd_disk_ioctl(&s_card,ops,&buf);
	if(res==0)
		return buf;
	else
		return res;
}

sdcard_type_t SD::sdcard_type()
{
	return s_card.type;
}