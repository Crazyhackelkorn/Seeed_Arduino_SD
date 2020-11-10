# Seeed-Arduino-SD

## Introduction

An Arduino library for SD over SPI operation. It implements the needed structure for the Arduino/Ardupy wrapper. For detailed use of an SD card with Ardupy see [Seeed Ardupy SD Card](https://github.com/Crazyhackelkorn/seeed-ardupy-sdcard)

## Note
**This is not for use with Arduino.** If you want to use a SD card with Seeed Wio terminal or so, look at the original code [Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS). There is no filesystem included. Only the need hardware functions to init(), read(), write() and ioctl() to a sd card are implemented.

## API Reference

For detailes, why this funktions are implemented, see [uos.AbstractBlockDev](https://docs.micropython.org/en/latest/library/uos.html#uos.AbstractBlockDev)

- SD (uint8_t cs, SPIClass *spi=&SPI, int hz=5000000):
	- Create SD object and SPI.begin()

- ~SD (): Destructor:
	- stops SPI
	
- uint32_t read(uint8_t *buffer, unsigned long long sector, unsigned int count):
	- read raw data from SD card

- uint32_t write(uint8_t *buffer, unsigned long long sector, unsigned int count):
	- write raw data to SD card

- uint32_t ioctl(uint8_t ops):
	- 1: initialise the device / get device status
	- 2: deinitialise/shutdown the device
	- 3: sync the device
	- 4: get a count of the number of blocks
	- 5: get the number of bytes in a block

- sdcard_type_t sdcard_type():
    - 0: CARD_NONE,
    - 1: CARD_MMC,
    - 2: CARD_SD,
    - 3: CARD_SDHC,
    - 4: CARD_UNKNOWN

----

This software was forked from [Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS)

This software was written by seeed studio and modifyed by Crazyhackelkorn<br>
and is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check License.txt for more information.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed Studio is an open hardware facilitation company based in Shenzhen, China. <br>
Benefiting from local manufacture power and convenient global logistic system, <br>
we integrate resources to serve new era of innovation. Seeed also works with <br>
global distributors and partners to push open hardware movement.<br>

