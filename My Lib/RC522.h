#ifndef _RC522_H_
#define _RC522_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>

/* ============================================================
 * Timeout
 * ============================================================ */
#define RC522_TIMEOUT_MS 1000

/* ============================================================
 * Register Address RC522
 * ============================================================ */

/* Page 0: Command and Status */
#define RC522_REG_COMMAND 0x01
#define RC522_REG_COM_IEN 0x02
#define RC522_REG_DIV_IEN 0x03
#define RC522_REG_COM_IRQ 0x04
#define RC522_REG_DIV_IRQ 0x05
#define RC522_REG_ERROR 0x06
#define RC522_REG_STATUS1 0x07
#define RC522_REG_STATUS2 0x08
#define RC522_REG_FIFO_DATA 0x09
#define RC522_REG_FIFO_LEVEL 0x0A
#define RC522_REG_WATER_LEVEL 0x0B
#define RC522_REG_CONTROL 0x0C
#define RC522_REG_BIT_FRAMING 0x0D
#define RC522_REG_COLL 0x0E

/* Page 1: Command */
#define RC522_REG_MODE 0x11
#define RC522_REG_TX_MODE 0x12
#define RC522_REG_RX_MODE 0x13
#define RC522_REG_TX_CONTROL 0x14
#define RC522_REG_TX_ASK 0x15
#define RC522_REG_TX_SEL 0x16
#define RC522_REG_RX_SEL 0x17
#define RC522_REG_RX_THRESHOLD 0x18
#define RC522_REG_DEMOD 0x19
#define RC522_REG_MIFARE 0x1C
#define RC522_REG_MANUAL_RCV 0x1D
#define RC522_REG_TYPED_STANDARD 0x1F

/* Page 2: Configuration */
#define RC522_REG_CRC_RESULT_M 0x21
#define RC522_REG_CRC_RESULT_L 0x22
#define RC522_REG_MOD_WIDTH 0x24
#define RC522_REG_RF_CFG 0x26
#define RC522_REG_GS_N 0x27
#define RC522_REG_CW_GS_P 0x28
#define RC522_REG_MOD_GS_P 0x29
#define RC522_REG_T_MODE 0x2A
#define RC522_REG_T_PRESCALER 0x2B
#define RC522_REG_T_RELOAD_H 0x2C
#define RC522_REG_T_RELOAD_L 0x2D
#define RC522_REG_T_COUNTER_H 0x2E
#define RC522_REG_T_COUNTER_L 0x2F

/* Page 3: Test */
#define RC522_REG_TEST_SEL1 0x31
#define RC522_REG_TEST_SEL2 0x32
#define RC522_REG_TEST_PIN_EN 0x33
#define RC522_REG_TEST_PIN_VALUE 0x34
#define RC522_REG_TEST_BUS 0x35
#define RC522_REG_AUTO_TEST 0x36
#define RC522_REG_VERSION 0x37
#define RC522_REG_ANALOG_TEST 0x38
#define RC522_REG_TEST_ADC1 0x39
#define RC522_REG_TEST_ADC2 0x3A
#define RC522_REG_TEST_ADC0 0x3B

/* ============================================================
 * RC522 Internal Commands
 * ============================================================ */
#define RC522_CMD_IDLE 0x00
#define RC522_CMD_MEM 0x01
#define RC522_CMD_GEN_RANDOM_ID 0x02
#define RC522_CMD_CALC_CRC 0x03
#define RC522_CMD_TRANSMIT 0x04
#define RC522_CMD_NO_CMD_CHANGE 0x07
#define RC522_CMD_RECEIVE 0x08
#define RC522_CMD_TRANSCEIVE 0x0C
#define RC522_CMD_MF_AUTHENT 0x0E
#define RC522_CMD_SOFT_RESET 0x0F

/* ============================================================
 * PICC Commands (Card-side)
 * ============================================================ */
#define PICC_CMD_REQA 0x26
#define PICC_CMD_WUPA 0x52
#define PICC_CMD_CT 0x88
#define PICC_CMD_SEL_CL1 0x93
#define PICC_CMD_SEL_CL2 0x95
#define PICC_CMD_SEL_CL3 0x97
#define PICC_CMD_HLTA 0x50
#define PICC_CMD_MF_AUTH_KEY_A 0x60
#define PICC_CMD_MF_AUTH_KEY_B 0x61
#define PICC_CMD_MF_READ 0x30
#define PICC_CMD_MF_WRITE 0xA0
#define PICC_CMD_MF_DECREMENT 0xC0
#define PICC_CMD_MF_INCREMENT 0xC1
#define PICC_CMD_MF_RESTORE 0xC2
#define PICC_CMD_MF_TRANSFER 0xB0
#define PICC_CMD_UL_WRITE 0xA2

/* ============================================================
 * Enumerations
 * ============================================================ */

typedef enum
{
    RC522_OK = 0,
    RC522_ERR = 1,
    RC522_COLLISION = 2,
    RC522_TIMEOUT = 3,
    RC522_NO_ROOM = 4,
    RC522_INTERNAL_ERR = 5,
    RC522_INVALID = 6,
    RC522_CRC_WRONG = 7,
    RC522_MIFARE_NACK = 8,
} RC522_Status;

typedef enum
{
    PICC_TYPE_UNKNOWN = 0,
    PICC_TYPE_ISO_14443_4 = 1,
    PICC_TYPE_ISO_18092 = 2,
    PICC_TYPE_MIFARE_MINI = 3,
    PICC_TYPE_MIFARE_1K = 4,
    PICC_TYPE_MIFARE_4K = 5,
    PICC_TYPE_MIFARE_UL = 6,
    PICC_TYPE_MIFARE_PLUS = 7,
    PICC_TYPE_TNP3XXX = 8,
    PICC_TYPE_NOT_COMPLETE = 0xFF,
} PICC_Type;

/* ============================================================
 * Data Structures
 * ============================================================ */

typedef struct
{
    uint8_t size;        /* Number of bytes in UID (4, 7 or 10) */
    uint8_t uidByte[10]; /* UID bytes                           */
    uint8_t sak;         /* Select Acknowledge byte             */
} RC522_UID;

typedef struct
{
    uint8_t keyByte[6];
} MIFARE_Key;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;
} RC522_HandleTypeDef;

/* ============================================================
 * Public API
 * ============================================================ */

/* Initialisation */
RC522_Status RC522_Init(RC522_HandleTypeDef *hrc522,
                        SPI_HandleTypeDef *hspi,
                        GPIO_TypeDef *cs_port,
                        uint16_t cs_pin,
                        GPIO_TypeDef *rst_port,
                        uint16_t rst_pin);
void RC522_Reset(RC522_HandleTypeDef *hrc522);
uint8_t RC522_GetVersion(RC522_HandleTypeDef *hrc522);

/* Card detection */
RC522_Status RC522_IsCardPresent(RC522_HandleTypeDef *hrc522);
RC522_Status RC522_ReadCardUID(RC522_HandleTypeDef *hrc522, RC522_UID *uid);
RC522_Status RC522_HaltCard(RC522_HandleTypeDef *hrc522);

/* Mifare Classic */
RC522_Status RC522_Authenticate(RC522_HandleTypeDef *hrc522,
                                uint8_t cmd,
                                uint8_t blockAddr,
                                MIFARE_Key *key,
                                RC522_UID *uid);
RC522_Status RC522_ReadBlock(RC522_HandleTypeDef *hrc522,
                             uint8_t blockAddr,
                             uint8_t *data,
                             uint8_t *dataLen);
RC522_Status RC522_WriteBlock(RC522_HandleTypeDef *hrc522,
                              uint8_t blockAddr,
                              uint8_t *data);
void RC522_StopCrypto(RC522_HandleTypeDef *hrc522);

/* Register access */
uint8_t RC522_ReadRegister(RC522_HandleTypeDef *hrc522, uint8_t reg);
void RC522_WriteRegister(RC522_HandleTypeDef *hrc522, uint8_t reg, uint8_t value);

/* Utility */
PICC_Type RC522_GetPICCType(uint8_t sak);
const char *RC522_GetPICCTypeName(PICC_Type type);
void RC522_UIDtoString(RC522_UID *uid, char *buf, uint8_t bufLen);

extern const MIFARE_Key MIFARE_DEFAULT_KEY;

#endif /* _RC522_H_ */