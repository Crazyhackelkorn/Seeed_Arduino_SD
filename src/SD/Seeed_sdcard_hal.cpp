#include "Seeed_sdcard_hal.h"
#include "Arduino.h"
#include "SPI.h"

extern "C"
{
    char CRC7(const char *data, int length);
    unsigned short CRC16(const char *data, int length);
}

typedef enum
{
    GO_IDLE_STATE = 0,
    SEND_OP_COND = 1,
    SEND_CID = 2,
    SEND_RELATIVE_ADDR = 3,
    SEND_SWITCH_FUNC = 6,
    SEND_IF_COND = 8,
    SEND_CSD = 9,
    STOP_TRANSMISSION = 12,
    SEND_STATUS = 13,
    SET_BLOCKLEN = 16,
    READ_BLOCK_SINGLE = 17,
    READ_BLOCK_MULTIPLE = 18,
    SEND_NUM_WR_BLOCKS = 22,
    SET_WR_BLK_ERASE_COUNT = 23,
    WRITE_BLOCK_SINGLE = 24,
    WRITE_BLOCK_MULTIPLE = 25,
    APP_OP_COND = 41,
    APP_CLR_CARD_DETECT = 42,
    APP_CMD = 55,
    READ_OCR = 58,
    CRC_ON_OFF = 59
} ardu_sdcard_command_t;

namespace
{

    struct AcquireSPI
    {
        ardu_sdcard_t *card;
        explicit AcquireSPI(ardu_sdcard_t *card)
            : card(card)
        {
            card->spi->beginTransaction(SPISettings(card->frequency, MSBFIRST, SPI_MODE0));
        }
        AcquireSPI(ardu_sdcard_t *card, int frequency)
            : card(card)
        {
            card->spi->beginTransaction(SPISettings(card->frequency, MSBFIRST, SPI_MODE0));
        }
        ~AcquireSPI()
        {
            card->spi->endTransaction();
        }

    private:
        AcquireSPI(AcquireSPI const &);
        AcquireSPI &operator=(AcquireSPI const &);
    };

} // namespace

/*
    SD SPI
 * */

bool sdWait(ardu_sdcard_t *card, int timeout)
{
    char resp;
    uint32_t start = millis();

    do
    {
        resp = card->spi->transfer(0xFF);
    } while (resp == 0x00 && (millis() - start) < (unsigned int)timeout);

    return (resp > 0x00);
}

void sdStop(ardu_sdcard_t *card)
{
    card->spi->transfer(0xFD);
}

void sdDeselectCard(ardu_sdcard_t *card)
{
    digitalWrite(card->ssPin, HIGH);
}

bool sdSelectCard(ardu_sdcard_t *card)
{
    digitalWrite(card->ssPin, LOW);
    sdWait(card, 300);
    return true;
}

bool sdReadBytes(ardu_sdcard_t *card, char *buffer, int length)
{
    char token;
    unsigned short crc;
    char *p = buffer;

    uint32_t start = millis();
    do
    {
        token = card->spi->transfer(0xFF);
    } while (token == 0xFF && (millis() - start) < 500);

    if (token != 0xFE)
    {
        return false;
    }

    for (; p < buffer + length; p++)
    {
        *p = card->spi->transfer(0xFF);
    }
    crc = card->spi->transfer16(0xFFFF);
    return (!card->supports_crc || crc == CRC16(buffer, length));
}

char sdWriteBytes(ardu_sdcard_t *card, const char *buffer, char token)
{
    unsigned short crc = (card->supports_crc) ? CRC16(buffer, 512) : 0xFFFF;
    if (!sdWait(card, 500))
    {
        return false;
    }
    card->spi->transfer(token);
    card->spi->transfer((uint8_t *)buffer, 512);
    card->spi->transfer16(crc);
    return (card->spi->transfer(0xFF) & 0x1F);
}

char sdCommand(ardu_sdcard_t *card, char cmd, unsigned int arg, unsigned int *resp)
{

    char token;

    for (int f = 0; f < 3; f++)
    {
        if (cmd == SEND_NUM_WR_BLOCKS || cmd == SET_WR_BLK_ERASE_COUNT || cmd == APP_OP_COND || cmd == APP_CLR_CARD_DETECT)
        {
            token = sdCommand(card, APP_CMD, 0, NULL);
            sdDeselectCard(card);
            if (token > 1)
            {
                return token;
            }
            if (!sdSelectCard(card))
            {
                return 0xFF;
            }
        }

        char cmdPacket[7];
        cmdPacket[0] = cmd | 0x40;
        cmdPacket[1] = arg >> 24;
        cmdPacket[2] = arg >> 16;
        cmdPacket[3] = arg >> 8;
        cmdPacket[4] = arg;
        if (card->supports_crc || cmd == GO_IDLE_STATE || cmd == SEND_IF_COND)
        {
            cmdPacket[5] = (CRC7(cmdPacket, 5) << 1) | 0x01;
        }
        else
        {
            cmdPacket[5] = 0x01;
        }
        cmdPacket[6] = 0xFF;

        card->spi->transfer((uint8_t *)cmdPacket, (cmd == STOP_TRANSMISSION) ? 7 : 6);

        for (int i = 0; i < 9; i++)
        {
            token = card->spi->transfer(0xFF);
            if (!(token & 0x80))
            {
                break;
            }
        }

        if (token == 0xFF)
        {
            sdDeselectCard(card);
            delay(100);
            sdSelectCard(card);
            continue;
        }
        else if (token & 0x08)
        {
            sdDeselectCard(card);
            delay(100);
            sdSelectCard(card);
            continue;
        }
        else if (token > 1)
        {
            break;
        }

        if (cmd == SEND_STATUS && resp)
        {
            *resp = card->spi->transfer(0xFF);
        }
        else if ((cmd == SEND_IF_COND || cmd == READ_OCR) && resp)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                uint8_t temp = card->spi->transfer(0xFF);
                *resp = *resp * 256 + temp;
            }
        }
        break;
    }
    return token;
}

/*
    SPI SDCARD Communication
 * */

char sdTransaction(ardu_sdcard_t *card, char cmd, unsigned int arg, unsigned int *resp)
{
    if (!sdSelectCard(card))
    {
        return 0xFF;
    }
    char token = sdCommand(card, cmd, arg, resp);
    sdDeselectCard(card);
    return token;
}

bool sdReadSector(ardu_sdcard_t *card, char *buffer, unsigned long long sector)
{
    for (int f = 0; f < 3; f++)
    {
        if (!sdSelectCard(card))
        {
            break;
        }
        if (!sdCommand(card, READ_BLOCK_SINGLE, (card->type == CARD_SDHC) ? sector : sector << 9, NULL))
        {
            bool success = sdReadBytes(card, buffer, 512);
            sdDeselectCard(card);
            if (success)
            {
                return true;
            }
        }
        else
        {
            break;
        }
    }
    sdDeselectCard(card);
    return false;
}

bool sdReadSectors(ardu_sdcard_t *card, char *buffer, unsigned long long sector, int count)
{
    for (int f = 0; f < 3;)
    {
        if (!sdSelectCard(card))
        {
            break;
        }

        if (!sdCommand(card, READ_BLOCK_MULTIPLE, (card->type == CARD_SDHC) ? sector : sector << 9, NULL))
        {
            do
            {
                if (!sdReadBytes(card, buffer, 512))
                {
                    f++;
                    break;
                }

                sector++;
                buffer += 512;
                f = 0;
            } while (--count);

            if (sdCommand(card, STOP_TRANSMISSION, 0, NULL))
            {
                break;
            }

            sdDeselectCard(card);
            if (count == 0)
            {
                return true;
            }
        }
        else
        {
            break;
        }
    }
    sdDeselectCard(card);
    return false;
}

bool sdWriteSector(ardu_sdcard_t *card, const char *buffer, unsigned long long sector)
{
    for (int f = 0; f < 3; f++)
    {
        if (!sdSelectCard(card))
        {
            break;
        }
        if (!sdCommand(card, WRITE_BLOCK_SINGLE, (card->type == CARD_SDHC) ? sector : sector << 9, NULL))
        {

            char token = sdWriteBytes(card, buffer, 0xFE);
            sdDeselectCard(card);

            if (token == 0x0A)
            {
                continue;
            }
            else if (token == 0x0C)
            {
                return false;
            }

            unsigned int resp;
            if (sdTransaction(card, SEND_STATUS, 0, &resp) || resp)
            {
                return false;
            }
            return true;
        }
        else
        {
            break;
        }
    }
    sdDeselectCard(card);
    return false;
}

bool sdWriteSectors(ardu_sdcard_t *card, const char *buffer, unsigned long long sector, int count)
{
    char token;
    const char *currentBuffer = buffer;
    unsigned long long currentSector = sector;
    int currentCount = count;

    for (int f = 0; f < 3;)
    {
        if (card->type != CARD_MMC)
        {
            if (sdTransaction(card, SET_WR_BLK_ERASE_COUNT, currentCount, NULL))
            {
                break;
            }
        }

        if (!sdSelectCard(card))
        {
            break;
        }

        if (!sdCommand(card, WRITE_BLOCK_MULTIPLE, (card->type == CARD_SDHC) ? currentSector : currentSector << 9, NULL))
        {
            do
            {
                token = sdWriteBytes(card, currentBuffer, 0xFC);
                if (token != 0x05)
                {
                    f++;
                    break;
                }
                currentBuffer += 512;
                f = 0;
            } while (--currentCount);

            if (!sdWait(card, 500))
            {
                break;
            }

            if (currentCount == 0)
            {
                sdStop(card);
                sdDeselectCard(card);

                unsigned int resp;
                if (sdTransaction(card, SEND_STATUS, 0, &resp) || resp)
                {
                    return false;
                }
                return true;
            }
            else
            {
                if (sdCommand(card, STOP_TRANSMISSION, 0, NULL))
                {
                    break;
                }

                sdDeselectCard(card);

                if (token == 0x0A)
                {
                    unsigned int writtenBlocks = 0;
                    if (card->type != CARD_MMC && sdSelectCard(card))
                    {
                        if (!sdCommand(card, SEND_NUM_WR_BLOCKS, 0, NULL))
                        {
                            char acmdData[4];
                            if (sdReadBytes(card, acmdData, 4))
                            {
                                writtenBlocks = acmdData[0] << 24;
                                writtenBlocks |= acmdData[1] << 16;
                                writtenBlocks |= acmdData[2] << 8;
                                writtenBlocks |= acmdData[3];
                            }
                        }
                        sdDeselectCard(card);
                    }
                    currentBuffer = buffer + (writtenBlocks << 9);
                    currentSector = sector + writtenBlocks;
                    currentCount = count - writtenBlocks;
                    continue;
                }
                else
                {
                    return false;
                }
            }
        }
        else
        {
            break;
        }
    }
    sdDeselectCard(card);
    return false;
}

unsigned long sdGetSectorsCount(ardu_sdcard_t *card)
{
    for (int f = 0; f < 3; f++)
    {
        if (!sdSelectCard(card))
        {
            break;
        }

        if (!sdCommand(card, SEND_CSD, 0, NULL))
        {
            char csd[16];
            bool success = sdReadBytes(card, csd, 16);
            sdDeselectCard(card);
            if (success)
            {
                if ((csd[0] >> 6) == 0x01)
                {
                    unsigned long size = (((unsigned long)(csd[7] & 0x3F) << 16) | ((unsigned long)csd[8] << 8) | csd[9]) + 1;
                    return size << 10;
                }
                unsigned long size = (((unsigned long)(csd[6] & 0x03) << 10) | ((unsigned long)csd[7] << 2) | ((csd[8] & 0xC0) >> 6)) + 1;
                size <<= ((
                              ((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7)) +
                          2);
                size <<= (csd[5] & 0x0F);
                return size >> 9;
            }
        }
        else
        {
            break;
        }
    }

    sdDeselectCard(card);
    return 0;
}

/*
    vFAT API/
 * */

DSTATUS sd_disk_initialize(ardu_sdcard_t *card)
{
    char token;
    unsigned int resp;
    unsigned int start;
    if (!(card->status & STA_NOINIT))
    {
        return card->status;
    }

    AcquireSPI card_locked(card, 400000);

    digitalWrite(card->ssPin, HIGH);
    for (uint8_t i = 0; i < 20; i++)
    {
        card->spi->transfer(0XFF);
    }

    if (sdTransaction(card, GO_IDLE_STATE, 0, NULL) != 1)
    {
        goto unknown_card;
    }

    token = sdTransaction(card, CRC_ON_OFF, 0, NULL);
    if (token == 0x5)
    {
        card->supports_crc = false;
    }
    else if (token != 1)
    {
        goto unknown_card;
    }
    card->supports_crc = false;

    if (sdTransaction(card, SEND_IF_COND, 0x1AA, &resp) == 1)
    {
        if ((resp & 0xFFF) != 0x1AA)
        {
            goto unknown_card;
        }

        if (sdTransaction(card, READ_OCR, 0, &resp) != 1 || !(resp & (1 << 20)))
        {
            goto unknown_card;
        }

        start = millis();
        do
        {
            token = sdTransaction(card, APP_OP_COND, 0x40000000, NULL);
        } while (token == 1 && (millis() - start) < 1000);

        if (token)
        {
            goto unknown_card;
        }

        if (!sdTransaction(card, READ_OCR, 0, &resp))
        {
            if (resp & (1 << 30))
            {
                card->type = CARD_SDHC;
            }
            else
            {
                card->type = CARD_SD;
            }
        }
        else
        {
            goto unknown_card;
        }
    }
    else
    {
        if (sdTransaction(card, READ_OCR, 0, &resp) != 1 || !(resp & (1 << 20)))
        {
            goto unknown_card;
        }

        start = millis();
        do
        {
            token = sdTransaction(card, APP_OP_COND, 0x100000, NULL);
        } while (token == 0x01 && (millis() - start) < 1000);

        if (!token)
        {
            card->type = CARD_SD;
        }
        else
        {
            start = millis();
            do
            {
                token = sdTransaction(card, SEND_OP_COND, 0x100000, NULL);
            } while (token != 0x00 && (millis() - start) < 1000);

            if (token == 0x00)
            {
                card->type = CARD_MMC;
            }
            else
            {
                goto unknown_card;
            }
        }
    }

    if (card->type != CARD_MMC)
    {
        if (sdTransaction(card, APP_CLR_CARD_DETECT, 0, NULL))
        {
            goto unknown_card;
        }
    }

    if (card->type != CARD_SDHC)
    {
        if (sdTransaction(card, SET_BLOCKLEN, 512, NULL) != 0x00)
        {
            goto unknown_card;
        }
    }

    card->sectors = sdGetSectorsCount(card);

    if (card->frequency > 25000000)
    {
        card->frequency = 25000000;
    }

    card->status &= ~STA_NOINIT;
    return card->status;

unknown_card:
    card->type = CARD_UNKNOWN;
    return card->status;
}

DRESULT sd_disk_read(ardu_sdcard_t *card, uint8_t *buffer, unsigned long long sector, unsigned int count)
{
    if (card->status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }
    DRESULT res = RES_OK;

    AcquireSPI lock(card);

    if (count > 1)
    {
        res = sdReadSectors(card, (char *)buffer, sector, count) ? RES_OK : RES_ERROR;
    }
    else
    {
        res = sdReadSector(card, (char *)buffer, sector) ? RES_OK : RES_ERROR;
    }
    return res;
}

DRESULT sd_disk_write(ardu_sdcard_t *card, const uint8_t *buffer, unsigned long long sector, unsigned int count)
{
    if (card->status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (card->status & STA_PROTECT)
    {
        return RES_WRPRT;
    }
    DRESULT res = RES_OK;

    AcquireSPI lock(card);

    if (count > 1)
    {
        res = sdWriteSectors(card, (const char *)buffer, sector, count) ? RES_OK : RES_ERROR;
    }
    else
    {
        res = sdWriteSector(card, (const char *)buffer, sector) ? RES_OK : RES_ERROR;
    }
    return res;
}

DRESULT sd_disk_ioctl(ardu_sdcard_t *card, uint8_t cmd, unsigned long *buff)
{
    switch (cmd)
    {
	case 1: // INIT-SD
		DSTATUS sd_stat;
		sd_stat=sd_disk_initialize(card);
		*buff=sd_stat;
		return RES_OK;
		break;
	case 2: // DEINIT-SD
    	if (card == NULL)
        	return RES_PARERR;
    	card->status |= STA_NOINIT;
    	card->type = CARD_NONE;
		card->sectors = 0;
    	return RES_OK;
		break;
    case 3: // CTRL-Sync
    {
        AcquireSPI lock(card);
        if (sdSelectCard(card))
        {
            sdDeselectCard(card);
            return RES_OK;
        }
    }
        return RES_ERROR;
		break;
    case 4: // GET_SECTOR_COUNT
        *buff = card->sectors;
        return RES_OK;
		break;
    case 5: // GET_SECTOR_SIZE:
        *buff = 512;
        return RES_OK;
		break;
    }
    return RES_PARERR;
}
