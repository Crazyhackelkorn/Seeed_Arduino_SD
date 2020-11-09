/**
 * The MIT License (MIT)
 *
 * Author: Stephan Hartl
 *
 * Copyright (C) 2020 Stephan Hartl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

uint32_t SD::ioctl(uint8_t ops)
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