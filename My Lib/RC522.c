#include "RC522.h"

/* ============================================================
 * Constant
 * ============================================================ */
const MIFARE_Key MIFARE_DEFAULT_KEY = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

/* ============================================================
 * Private macros
 * ============================================================ */
#define _CS_LOW(h) HAL_GPIO_WritePin((h)->cs_port, (h)->cs_pin, GPIO_PIN_RESET)
#define _CS_HIGH(h) HAL_GPIO_WritePin((h)->cs_port, (h)->cs_pin, GPIO_PIN_SET)
#define _RST_LOW(h) HAL_GPIO_WritePin((h)->rst_port, (h)->rst_pin, GPIO_PIN_RESET)
#define _RST_HIGH(h) HAL_GPIO_WritePin((h)->rst_port, (h)->rst_pin, GPIO_PIN_SET)

/* ============================================================
 * Public: Register access
 * ============================================================ */

void RC522_WriteRegister(RC522_HandleTypeDef *hrc522, uint8_t reg, uint8_t value)
{
    uint8_t tx[2];
    tx[0] = (reg << 1) & 0x7E; /* bit7=0 write, bit0=0 reserved */
    tx[1] = value;

    _CS_LOW(hrc522);
    HAL_SPI_Transmit(hrc522->hspi, tx, 2, RC522_TIMEOUT_MS);
    _CS_HIGH(hrc522);
}

uint8_t RC522_ReadRegister(RC522_HandleTypeDef *hrc522, uint8_t reg)
{
    uint8_t tx = ((reg << 1) & 0x7E) | 0x80; /* bit7=1 read */
    uint8_t rx = 0;

    _CS_LOW(hrc522);
    HAL_SPI_Transmit(hrc522->hspi, &tx, 1, RC522_TIMEOUT_MS);
    HAL_SPI_Receive(hrc522->hspi, &rx, 1, RC522_TIMEOUT_MS);
    _CS_HIGH(hrc522);

    return rx;
}

/* ============================================================
 * Private: register bit helpers
 * ============================================================ */

static void _SetBits(RC522_HandleTypeDef *hrc522, uint8_t reg, uint8_t mask)
{
    RC522_WriteRegister(hrc522, reg, RC522_ReadRegister(hrc522, reg) | mask);
}

static void _ClearBits(RC522_HandleTypeDef *hrc522, uint8_t reg, uint8_t mask)
{
    RC522_WriteRegister(hrc522, reg, RC522_ReadRegister(hrc522, reg) & ~mask);
}

/* ============================================================
 * Private: bulk FIFO read / write
 * ============================================================ */

static void _WriteRegisterBurst(RC522_HandleTypeDef *hrc522,
                                uint8_t reg, uint8_t count, uint8_t *values)
{
    uint8_t addr = (reg << 1) & 0x7E;

    _CS_LOW(hrc522);
    HAL_SPI_Transmit(hrc522->hspi, &addr, 1, RC522_TIMEOUT_MS);
    HAL_SPI_Transmit(hrc522->hspi, values, count, RC522_TIMEOUT_MS);
    _CS_HIGH(hrc522);
}

static void _ReadRegisterBurst(RC522_HandleTypeDef *hrc522,
                               uint8_t reg, uint8_t count,
                               uint8_t *values, uint8_t rxAlign)
{
    if (count == 0)
        return;

    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;

    _CS_LOW(hrc522);
    HAL_SPI_Transmit(hrc522->hspi, &addr, 1, RC522_TIMEOUT_MS);

    for (uint8_t i = 0; i < count; i++)
    {
        if (i == 0 && rxAlign)
        {
            uint8_t mask = 0x00;
            for (uint8_t b = rxAlign; b <= 7; b++)
                mask |= (1u << b);
            uint8_t raw = 0;
            HAL_SPI_Receive(hrc522->hspi, &raw, 1, RC522_TIMEOUT_MS);
            values[0] = (values[0] & ~mask) | (raw & mask);
        }
        else
        {
            HAL_SPI_Receive(hrc522->hspi, &values[i], 1, RC522_TIMEOUT_MS);
        }
    }
    _CS_HIGH(hrc522);
}

/* ============================================================
 * Private: CRC co-processor
 * ============================================================ */

static RC522_Status _CalculateCRC(RC522_HandleTypeDef *hrc522,
                                  uint8_t *data, uint8_t length,
                                  uint8_t *result)
{
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
    RC522_WriteRegister(hrc522, RC522_REG_DIV_IRQ, 0x04);    /* clear CRCIRq */
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_LEVEL, 0x80); /* flush FIFO   */
    _WriteRegisterBurst(hrc522, RC522_REG_FIFO_DATA, length, data);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_CALC_CRC);

    uint32_t deadline = HAL_GetTick() + 100;
    while (HAL_GetTick() < deadline)
    {
        if (RC522_ReadRegister(hrc522, RC522_REG_DIV_IRQ) & 0x04)
        {
            RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
            result[0] = RC522_ReadRegister(hrc522, RC522_REG_CRC_RESULT_L);
            result[1] = RC522_ReadRegister(hrc522, RC522_REG_CRC_RESULT_M);
            return RC522_OK;
        }
    }
    return RC522_TIMEOUT;
}

/* ============================================================
 * Private: Transceive  (dùng cho MF_READ, MF_WRITE, HLTA)
 * ============================================================ */

static RC522_Status _Transceive(RC522_HandleTypeDef *hrc522,
                                uint8_t *sendData, uint8_t sendLen,
                                uint8_t *backData, uint8_t *backLen,
                                uint8_t *validBits,
                                uint8_t rxAlign,
                                uint8_t checkCRC)
{
    uint8_t txLastBits = validBits ? *validBits : 0;
    uint8_t bitFraming = (uint8_t)((rxAlign << 4) | txLastBits);

    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_LEVEL, 0x80); /* flush FIFO    */
    RC522_WriteRegister(hrc522, RC522_REG_COM_IRQ, 0x7F);    /* clear all IRQ */
    _WriteRegisterBurst(hrc522, RC522_REG_FIFO_DATA, sendLen, sendData);
    RC522_WriteRegister(hrc522, RC522_REG_BIT_FRAMING, bitFraming);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_TRANSCEIVE);
    _SetBits(hrc522, RC522_REG_BIT_FRAMING, 0x80); /* StartSend = 1 */

    uint32_t deadline = HAL_GetTick() + 200;
    uint8_t irq = 0;
    while (HAL_GetTick() < deadline)
    {
        irq = RC522_ReadRegister(hrc522, RC522_REG_COM_IRQ);
        if (irq & 0x30)
            break;
        if (irq & 0x01)
        {
            _ClearBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);
            RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
            return RC522_TIMEOUT;
        }
    }

    _ClearBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);

    if (!(irq & 0x30))
        return RC522_TIMEOUT;

    uint8_t errReg = RC522_ReadRegister(hrc522, RC522_REG_ERROR);
    if (errReg & 0x93)
        return RC522_ERR; /* BufferOvfl | ParityErr | ProtocolErr */

    uint8_t _validBits = 0;

    if (backData && backLen)
    {
        uint8_t n = RC522_ReadRegister(hrc522, RC522_REG_FIFO_LEVEL);
        if (n > *backLen)
            return RC522_NO_ROOM;
        *backLen = n;
        _ReadRegisterBurst(hrc522, RC522_REG_FIFO_DATA, n, backData, rxAlign);
        _validBits = RC522_ReadRegister(hrc522, RC522_REG_CONTROL) & 0x07;
        if (validBits)
            *validBits = _validBits;
    }

    if (errReg & 0x08)
        return RC522_COLLISION;

    if (checkCRC && backData && backLen)
    {
        if (*backLen < 2 || _validBits != 0)
            return RC522_CRC_WRONG;
        uint8_t crc[2];
        RC522_Status s = _CalculateCRC(hrc522, backData, *backLen - 2, crc);
        if (s != RC522_OK)
            return s;
        if ((backData[*backLen - 2] != crc[0]) ||
            (backData[*backLen - 1] != crc[1]))
            return RC522_CRC_WRONG;
    }

    return RC522_OK;
}

/* ============================================================
 * Private: REQA helper  (viết tay, không dùng _Transceive)
 * ============================================================ */

static RC522_Status _RequestA_or_WakeupA(RC522_HandleTypeDef *hrc522,
                                         uint8_t cmd,
                                         uint8_t *bufATQA,
                                         uint8_t *bufSize)
{
    if (!bufATQA || *bufSize < 2)
        return RC522_NO_ROOM;

    /* Đảm bảo antenna bật */
    uint8_t tx_ctrl = RC522_ReadRegister(hrc522, RC522_REG_TX_CONTROL);
    if (!(tx_ctrl & 0x03))
    {
        RC522_WriteRegister(hrc522, RC522_REG_TX_CONTROL, tx_ctrl | 0x03);
        HAL_Delay(1);
    }

    _ClearBits(hrc522, RC522_REG_COLL, 0x80); /* ValuesAfterColl = 0 */
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_LEVEL, 0x80); /* flush FIFO          */
    RC522_WriteRegister(hrc522, RC522_REG_COM_IRQ, 0x7F);    /* clear all IRQ       */
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_DATA, cmd);
    RC522_WriteRegister(hrc522, RC522_REG_BIT_FRAMING, 0x07); /* 7-bit short frame  */
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_TRANSCEIVE);
    _SetBits(hrc522, RC522_REG_BIT_FRAMING, 0x80); /* StartSend = 1       */

    uint32_t deadline = HAL_GetTick() + 100;
    uint8_t irq = 0;
    while (HAL_GetTick() < deadline)
    {
        irq = RC522_ReadRegister(hrc522, RC522_REG_COM_IRQ);
        if (irq & 0x30)
            break;
        if (irq & 0x01)
        {
            _ClearBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);
            RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
            return RC522_TIMEOUT;
        }
    }

    _ClearBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);

    if (!(irq & 0x30))
        return RC522_TIMEOUT;

    uint8_t errReg = RC522_ReadRegister(hrc522, RC522_REG_ERROR);
    if (errReg & 0x13)
        return RC522_ERR;

    uint8_t n = RC522_ReadRegister(hrc522, RC522_REG_FIFO_LEVEL);
    if (n == 0)
        return RC522_ERR;

    uint8_t toRead = (n < *bufSize) ? n : *bufSize;
    *bufSize = toRead;
    _ReadRegisterBurst(hrc522, RC522_REG_FIFO_DATA, toRead, bufATQA, 0);

    uint8_t validBitsRx = RC522_ReadRegister(hrc522, RC522_REG_CONTROL) & 0x07;
    if (*bufSize == 2 && validBitsRx == 0)
        return RC522_OK;

    return RC522_ERR;
}

/* ============================================================
 * Public: Initialisation
 * ============================================================ */

RC522_Status RC522_Init(RC522_HandleTypeDef *hrc522,
                        SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *cs_port,
                        uint16_t cs_pin,
                        GPIO_TypeDef *rst_port,
                        uint16_t rst_pin)
{
    if (!hrc522 || !hspi || !cs_port || !rst_port)
        return RC522_INVALID;

    hrc522->hspi = hspi;
    hrc522->cs_port = cs_port;
    hrc522->cs_pin = cs_pin;
    hrc522->rst_port = rst_port;
    hrc522->rst_pin = rst_pin;

    _CS_HIGH(hrc522);
    _RST_HIGH(hrc522);
    HAL_Delay(10);

    RC522_Reset(hrc522);

    /* Timer: TAuto=1, TGated=11 (mux timer), prescaler → ~30 kHz, reload=30 */
    RC522_WriteRegister(hrc522, RC522_REG_T_MODE, 0x8D);
    RC522_WriteRegister(hrc522, RC522_REG_T_PRESCALER, 0x3E);
    RC522_WriteRegister(hrc522, RC522_REG_T_RELOAD_H, 0x00);
    RC522_WriteRegister(hrc522, RC522_REG_T_RELOAD_L, 0x1E);

    /* 100% ASK modulation */
    RC522_WriteRegister(hrc522, RC522_REG_TX_ASK, 0x40);

    /* CRC preset = 0x6363 (ISO 14443-3 §6.2.4) */
    RC522_WriteRegister(hrc522, RC522_REG_MODE, 0x3D);

    /* RF gain: max (48 dB) */
    RC522_WriteRegister(hrc522, RC522_REG_RF_CFG, 0x7F);

    /* Enable antenna TX1 + TX2 */
    _SetBits(hrc522, RC522_REG_TX_CONTROL, 0x03);

    uint8_t ver = RC522_GetVersion(hrc522);
    if (ver == 0x00 || ver == 0xFF)
        return RC522_ERR;

    return RC522_OK;
}

void RC522_Reset(RC522_HandleTypeDef *hrc522)
{
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_SOFT_RESET);
    HAL_Delay(50);
    uint8_t retries = 3;
    while ((RC522_ReadRegister(hrc522, RC522_REG_COMMAND) & (1 << 4)) && retries--)
        HAL_Delay(50);
}

uint8_t RC522_GetVersion(RC522_HandleTypeDef *hrc522)
{
    return RC522_ReadRegister(hrc522, RC522_REG_VERSION);
}

/* ============================================================
 * Public: Card detection
 * ============================================================ */

RC522_Status RC522_IsCardPresent(RC522_HandleTypeDef *hrc522)
{
    uint8_t atqa[2];
    uint8_t atqaSize = sizeof(atqa);
    return _RequestA_or_WakeupA(hrc522, PICC_CMD_REQA, atqa, &atqaSize);
}

/**
 * @brief Đọc UID 4 byte (Cascade Level 1) từ thẻ Mifare Classic.
 *
 * Gửi lệnh anticollision 0x93 0x20, đọc 5 byte từ FIFO
 * (4 byte UID + 1 byte BCC checksum), kiểm tra BCC rồi lưu UID.
 * uid->sak được đặt = 0 — nếu cần SAK, dùng SELECT đầy đủ.
 */
RC522_Status RC522_ReadCardUID(RC522_HandleTypeDef *hrc522, RC522_UID *uid)
{
    if (!hrc522 || !uid)
        return RC522_INVALID;

    _ClearBits(hrc522, RC522_REG_COLL, 0x80);
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_LEVEL, 0x80);
    RC522_WriteRegister(hrc522, RC522_REG_COM_IRQ, 0x7F);

    /* Anticollision CL1: SEL=0x93, NVB=0x20 (2 bytes, 0 known bits) */
    uint8_t cmd[2] = {PICC_CMD_SEL_CL1, 0x20};
    RC522_WriteRegister(hrc522, RC522_REG_BIT_FRAMING, 0x00);
    _WriteRegisterBurst(hrc522, RC522_REG_FIFO_DATA, 2, cmd);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_TRANSCEIVE);
    _SetBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);

    uint32_t deadline = HAL_GetTick() + 100;
    uint8_t irq = 0;
    while (HAL_GetTick() < deadline)
    {
        irq = RC522_ReadRegister(hrc522, RC522_REG_COM_IRQ);
        if (irq & 0x30)
            break;
    }

    _ClearBits(hrc522, RC522_REG_BIT_FRAMING, 0x80);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);

    if (!(irq & 0x30))
        return RC522_TIMEOUT;

    uint8_t errReg = RC522_ReadRegister(hrc522, RC522_REG_ERROR);
    if (errReg & 0x13)
        return RC522_ERR;

    uint8_t fifoLevel = RC522_ReadRegister(hrc522, RC522_REG_FIFO_LEVEL);
    if (fifoLevel != 5)
        return RC522_ERR; /* 4 byte UID + 1 byte BCC */

    uint8_t buf[5];
    _ReadRegisterBurst(hrc522, RC522_REG_FIFO_DATA, 5, buf, 0);

    /* Kiểm tra BCC: XOR của 4 byte UID phải bằng byte thứ 5 */
    if ((buf[0] ^ buf[1] ^ buf[2] ^ buf[3]) != buf[4])
        return RC522_ERR;

    uid->size = 4;
    uid->uidByte[0] = buf[0];
    uid->uidByte[1] = buf[1];
    uid->uidByte[2] = buf[2];
    uid->uidByte[3] = buf[3];
    uid->sak = 0x00; /* SAK chưa được đọc qua SELECT */

    return RC522_OK;
}

RC522_Status RC522_HaltCard(RC522_HandleTypeDef *hrc522)
{
    uint8_t buf[4];
    buf[0] = PICC_CMD_HLTA;
    buf[1] = 0;
    RC522_Status s = _CalculateCRC(hrc522, buf, 2, &buf[2]);
    if (s != RC522_OK)
        return s;

    uint8_t dummyLen = 0;
    s = _Transceive(hrc522, buf, 4, NULL, &dummyLen, NULL, 0, 0);
    return (s == RC522_TIMEOUT) ? RC522_OK : RC522_ERR;
}

/* ============================================================
 * Public: Mifare Classic
 * ============================================================ */

RC522_Status RC522_Authenticate(RC522_HandleTypeDef *hrc522,
                                uint8_t cmd,
                                uint8_t blockAddr,
                                MIFARE_Key *key,
                                RC522_UID *uid)
{
    if (!hrc522 || !key || !uid)
        return RC522_INVALID;

    uint8_t send[12];
    send[0] = cmd;
    send[1] = blockAddr;
    for (uint8_t i = 0; i < 6; i++)
        send[2 + i] = key->keyByte[i];

    uint8_t uidOffset = uid->size - 4;
    for (uint8_t i = 0; i < 4; i++)
        send[8 + i] = uid->uidByte[uidOffset + i];

    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_IDLE);
    RC522_WriteRegister(hrc522, RC522_REG_COM_IRQ, 0x7F);
    RC522_WriteRegister(hrc522, RC522_REG_FIFO_LEVEL, 0x80);
    _WriteRegisterBurst(hrc522, RC522_REG_FIFO_DATA, 12, send);
    RC522_WriteRegister(hrc522, RC522_REG_COMMAND, RC522_CMD_MF_AUTHENT);

    uint32_t deadline = HAL_GetTick() + 200;
    uint8_t irq = 0;
    while (HAL_GetTick() < deadline)
    {
        irq = RC522_ReadRegister(hrc522, RC522_REG_COM_IRQ);
        if (irq & 0x10)
            break;
        if (irq & 0x01)
            return RC522_TIMEOUT;
    }
    if (!(irq & 0x10))
        return RC522_TIMEOUT;

    if (RC522_ReadRegister(hrc522, RC522_REG_ERROR) & 0x08)
        return RC522_ERR;
    if (!(RC522_ReadRegister(hrc522, RC522_REG_STATUS2) & 0x08))
        return RC522_ERR;

    return RC522_OK;
}

RC522_Status RC522_ReadBlock(RC522_HandleTypeDef *hrc522,
                             uint8_t blockAddr,
                             uint8_t *data,
                             uint8_t *dataLen)
{
    if (!data || !dataLen || *dataLen < 18)
        return RC522_NO_ROOM;

    data[0] = PICC_CMD_MF_READ;
    data[1] = blockAddr;
    RC522_Status s = _CalculateCRC(hrc522, data, 2, &data[2]);
    if (s != RC522_OK)
        return s;

    return _Transceive(hrc522, data, 4, data, dataLen, NULL, 0, 1);
}

RC522_Status RC522_WriteBlock(RC522_HandleTypeDef *hrc522,
                              uint8_t blockAddr,
                              uint8_t *data)
{
    if (!data)
        return RC522_INVALID;

    uint8_t cmd[4];
    cmd[0] = PICC_CMD_MF_WRITE;
    cmd[1] = blockAddr;
    RC522_Status s = _CalculateCRC(hrc522, cmd, 2, &cmd[2]);
    if (s != RC522_OK)
        return s;

    uint8_t resp[1];
    uint8_t respLen = sizeof(resp);
    s = _Transceive(hrc522, cmd, 4, resp, &respLen, NULL, 0, 0);
    if (s != RC522_OK)
        return s;
    if (respLen != 1 || resp[0] != 0x0A)
        return RC522_MIFARE_NACK;

    uint8_t payload[18];
    memcpy(payload, data, 16);
    s = _CalculateCRC(hrc522, payload, 16, &payload[16]);
    if (s != RC522_OK)
        return s;

    respLen = sizeof(resp);
    s = _Transceive(hrc522, payload, 18, resp, &respLen, NULL, 0, 0);
    if (s != RC522_OK)
        return s;
    if (respLen != 1 || resp[0] != 0x0A)
        return RC522_MIFARE_NACK;

    return RC522_OK;
}

void RC522_StopCrypto(RC522_HandleTypeDef *hrc522)
{
    _ClearBits(hrc522, RC522_REG_STATUS2, 0x08); /* MFCrypto1On = 0 */
}

/* ============================================================
 * Public: Utility
 * ============================================================ */

PICC_Type RC522_GetPICCType(uint8_t sak)
{
    sak &= 0x7F;
    switch (sak)
    {
    case 0x04:
        return PICC_TYPE_NOT_COMPLETE;
    case 0x09:
        return PICC_TYPE_MIFARE_MINI;
    case 0x08:
        return PICC_TYPE_MIFARE_1K;
    case 0x18:
        return PICC_TYPE_MIFARE_4K;
    case 0x00:
        return PICC_TYPE_MIFARE_UL;
    case 0x10:
    case 0x11:
        return PICC_TYPE_MIFARE_PLUS;
    case 0x01:
        return PICC_TYPE_TNP3XXX;
    case 0x20:
        return PICC_TYPE_ISO_14443_4;
    case 0x40:
        return PICC_TYPE_ISO_18092;
    default:
        return PICC_TYPE_UNKNOWN;
    }
}

const char *RC522_GetPICCTypeName(PICC_Type type)
{
    switch (type)
    {
    case PICC_TYPE_ISO_14443_4:
        return "ISO/IEC 14443-4";
    case PICC_TYPE_ISO_18092:
        return "ISO/IEC 18092 (NFC)";
    case PICC_TYPE_MIFARE_MINI:
        return "MIFARE Mini (320 B)";
    case PICC_TYPE_MIFARE_1K:
        return "MIFARE Classic 1 KB";
    case PICC_TYPE_MIFARE_4K:
        return "MIFARE Classic 4 KB";
    case PICC_TYPE_MIFARE_UL:
        return "MIFARE Ultralight";
    case PICC_TYPE_MIFARE_PLUS:
        return "MIFARE Plus";
    case PICC_TYPE_TNP3XXX:
        return "MIFARE TNP3XXX";
    case PICC_TYPE_NOT_COMPLETE:
        return "UID not complete";
    default:
        return "Unknown";
    }
}

void RC522_UIDtoString(RC522_UID *uid, char *buf, uint8_t bufLen)
{
    if (!uid || !buf || bufLen < (uint8_t)(uid->size * 3))
        return;

    static const char hex[] = "0123456789ABCDEF";
    uint8_t pos = 0;

    for (uint8_t i = 0; i < uid->size; i++)
    {
        if (i > 0)
            buf[pos++] = ':';
        buf[pos++] = hex[(uid->uidByte[i] >> 4) & 0x0F];
        buf[pos++] = hex[uid->uidByte[i] & 0x0F];
    }
    buf[pos] = '\0';
}