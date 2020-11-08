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

/* Status of Disk Functions */
typedef uint8_t	DSTATUS;

/* Results of Disk Functions */
typedef enum {
    RES_OK = 0,		/* 0: Successful */
    RES_ERROR,		/* 1: R/W Error */
    RES_WRPRT,		/* 2: Write Protected */
    RES_NOTRDY,		/* 3: Not Ready */
    RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

DSTATUS sd_disk_initialize(ardu_sdcard_t *card);
DRESULT sd_disk_read(ardu_sdcard_t *card, uint8_t *buffer, unsigned long long sector, unsigned int count);
DRESULT sd_disk_write(ardu_sdcard_t *card, const uint8_t *buffer, unsigned long long sector, unsigned int count);
DRESULT sd_disk_ioctl(ardu_sdcard_t *card, uint8_t cmd,  unsigned long *buff);

#endif