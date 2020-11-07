#ifndef SEEED_SDCARD_HAL_H
#define SEEED_SDCARD_HAL_H

#include "SPI.h"
#include "Arduino.h"

typedef enum {
    CARD_NONE,
    CARD_MMC,
    CARD_SD,
    CARD_SDHC,
    CARD_UNKNOWN
} sdcard_type_t;

typedef struct {
    uint8_t ssPin;
    SPIClass* spi;
    int frequency;
    sdcard_type_t type;
    unsigned long sectors;
    bool supports_crc;
    int status;
} ardu_sdcard_t;


#endif