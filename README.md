# Seeed-Arduino-SD

## Introduction

An Arduino library for SD over SPI operation. It implements the needed structure for the Arduino/Ardupy wrapper. For detailed use of an SD card with Ardupy see [Seeed Ardupy SD Card](https://github.com/Crazyhackelkorn/seeed-ardupy-sdcard)

## Note
**This is not for use with Arduino.** If you want to use a SD card with Seeed Wio terminal or so, look at the original code [Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS). There is no filesystem included. Only the need hardware functions to init(), read(), write() and ioctl() to a sd card are implemented.

<!---
## API Reference

Won't come. Detailed use 


- boolean begin(uint8_t ssPin, SPIClass& sp, int hz) : config the SPI to control storage device

```c++
DEV.begin(SDCARD_SS_PIN,SDCARD_SPI,4000000UL)
// DEV.begin(104000000UL) //use qspi flash
```

- sdcard_type_t  cardType() : get SD card type 

**Note** : only work with SD card

```c++
    uint8_t cardType = DEV.cardType();
    if (cardType == CARD_NONE) {
        SERIAL.println("No SD card attached");
        return;
    }
```

- sfud_type_t   flashType() : get flash type

**Note** : only work with flash

```c++
    uint8_t flashType = DEV.flashType();
    if (flashType == FLASH_NONE) {
        SERIAL.println("No flash attached");
        return;
    }
```

- uint64_t cardSize(): get SD card size

**Note** : only work with SD card

```c++
    uint64_t cardSize = DEV.cardSize() / (1024 * 1024);
    SERIAL.print("SD Card Size: ");
    SERIAL.print((uint32_t)cardSize);
    SERIAL.println("MB");
```

- uint64_t    flashSize() : get flash size

**Note** : only work with flash

```c++
    uint32_t flashSize = DEV.flashSize() / (1024 * 1024);
    SERIAL.print("flash Size: ");
    SERIAL.print((uint32_t)flashSize);
    SERIAL.println("MB");
```

- uint64_t totalBytes(): return total Bytes of storage device

```c++
    uint32_t totalBytes = DEV.totalBytes();
    SERIAL.print("Total space: ");
    SERIAL.print(totalBytes / (1024 * 1024));
    SERIAL.println("MB");
```

- uint64_t usedBytes(): return used Bytes of storage device

```c++
    uint32_t usedBytes = DEV.usedBytes();
    SERIAL.print("Used space: ");
    SERIAL.print(usedBytes / (1024 * 1024));
    SERIAL.println("MB");
```
-->

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

