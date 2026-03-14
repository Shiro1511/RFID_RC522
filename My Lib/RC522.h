#ifndef _RC522_H_
#define _RC522_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal_def.h"
#include <stdint.h>
#include <string.h>

/* Hardware Configuration */

#define RC522_CS_PORT GPIOA
#define RC522_CS_PIN GPIO_PIN_4
#define RC522_RST_PORT GPIOA
#define RC522_RST_PIN GPIO_PIN_3
#define RC522_TIMEOUT_MS 1000

/* Register Address RC522 */

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

/* RC522 Internal Commands */

#define RC522_CMD_IDLE 0x00          /* Do nothing                      */
#define RC522_CMD_MEM 0x01           /* Save 25 bytes into internal buf */
#define RC522_CMD_GEN_RANDOM_ID 0x02 /* Generate random ID              */
#define RC522_CMD_CALC_CRC 0x03      /* Calculate CRC                   */
#define RC522_CMD_TRANSMIT 0x04      /* Transmit data from FIFO         */
#define RC522_CMD_NO_CMD_CHANGE 0x07 /* No change command               */
#define RC522_CMD_RECEIVE 0x08       /* Receive data to FIFO            */
#define RC522_CMD_TRANSCEIVE 0x0C    /* Transmit + Receive              */
#define RC522_CMD_MF_AUTHENT 0x0E    /* Authenticate Mifare             */
#define RC522_CMD_SOFT_RESET 0x0F    /* Reset software                  */

/* PICC Commands (Card-side) */

#define PICC_CMD_REQA 0x26          /* Request Type A              */
#define PICC_CMD_WUPA 0x52          /* Wake-up Type A              */
#define PICC_CMD_CT 0x88            /* Cascade Tag                 */
#define PICC_CMD_SEL_CL1 0x93       /* Anti-collision/Select CL1   */
#define PICC_CMD_SEL_CL2 0x95       /* Anti-collision/Select CL2   */
#define PICC_CMD_SEL_CL3 0x97       /* Anti-collision/Select CL3   */
#define PICC_CMD_HLTA 0x50          /* Halt Type A                 */
#define PICC_CMD_MF_AUTH_KEY_A 0x60 /* Mifare Key A authentication */
#define PICC_CMD_MF_AUTH_KEY_B 0x61 /* Mifare Key B authentication */
#define PICC_CMD_MF_READ 0x30       /* Read Mifare block           */
#define PICC_CMD_MF_WRITE 0xA0      /* Write Mifare block          */
#define PICC_CMD_MF_DECREMENT 0xC0  /* Decrement                   */
#define PICC_CMD_MF_INCREMENT 0xC1  /* Increment                   */
#define PICC_CMD_MF_RESTORE 0xC2    /* Restore                     */
#define PICC_CMD_MF_TRANSFER 0xB0   /* Transfer                    */
#define PICC_CMD_UL_WRITE 0xA2      /* Write Mifare Ultralight     */

/* Enumerations */

typedef enum
{
    RC522_OK = 0,           /* Success             */
    RC522_ERR = 1,          /* General error       */
    RC522_COLLISION = 2,    /* Conflict detected   */
    RC522_TIMEOUT = 3,      /* Timeout             */
    RC522_NO_ROOM = 4,      /* Insufficient buffer */
    RC522_INTERNAL_ERR = 5, /* Internal error      */
    RC522_INVALID = 6,      /* Invalid parameter   */
    RC522_CRC_WRONG = 7,    /* Incorrect CRC       */
    RC522_MIFARE_NACK = 8,  /* Card sent NACK      */
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

/* Data Structures */

/** Card UID — up to 10 bytes (single / double / triple size) */
typedef struct
{
    uint8_t size;        /* Number of bytes in UID (4, 7 or 10) */
    uint8_t uidByte[10]; /* UID bytes                           */
    uint8_t sak;         /* Select Acknowledge byte             */
} RC522_UID;

/** Mifare authentication key (6 bytes) */
typedef struct
{
    uint8_t keyByte[6];
} MIFARE_Key;

/**
 * RC522 handle — pass to every API function.
 *
 * Example init:
 *   RC522_HandleTypeDef hrc522 = {
 *       .hspi     = &hspi1,
 *       .cs_port  = GPIOA, .cs_pin  = GPIO_PIN_4,
 *       .rst_port = GPIOA, .rst_pin = GPIO_PIN_3,
 *   };
 *   RC522_Init(&hrc522);
 */
typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;
} RC522_HandleTypeDef;

/* Public API */

/**
 * @brief Fill *hrc522, reset the module and configure it for operation.
 *
 * @param hrc522    Uninitialised handle — all fields are written here.
 * @param hspi      Pointer to the HAL SPI handle (already initialised).
 * @param cs_port   GPIO port of the CS pin.
 * @param cs_pin    GPIO pin mask of the CS pin.
 * @param rst_port  GPIO port of the RST pin.
 * @param rst_pin   GPIO pin mask of the RST pin.
 * @return RC522_INVALID if any pointer is NULL.
 *         RC522_ERR     if the firmware-version register returns 0x00 or 0xFF
 *                       (module absent or wiring problem).
 *         RC522_OK      on success.
 */
RC522_Status RC522_Init(RC522_HandleTypeDef *hrc522, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                        uint16_t cs_pin,
                        GPIO_TypeDef *rst_port,
                        uint16_t rst_pin);

/**
 * @brief Perform a software reset and wait until the module is ready.
 */
void RC522_Reset(RC522_HandleTypeDef *hrc522);

/**
 * @brief Read the firmware version register.
 * @return 0x91 = v1.0, 0x92 = v2.0
 */
uint8_t RC522_GetVersion(RC522_HandleTypeDef *hrc522);

/* Card detection */

/**
 * @brief Check whether a PICC is present in the RF field.
 * @return RC522_OK if a card responds to REQA.
 */
RC522_Status RC522_IsCardPresent(RC522_HandleTypeDef *hrc522);

/**
 * @brief Read the UID of the PICC currently in the RF field.
 *
 * Implements the full ISO 14443-3 anti-collision / select loop
 * supporting cascade levels 1, 2 and 3 (4-, 7- and 10-byte UIDs).
 *
 * @param uid  Output: uid->size, uid->uidByte[], uid->sak are written.
 * @return RC522_OK on success.
 */
RC522_Status RC522_ReadCardUID(RC522_HandleTypeDef *hrc522, RC522_UID *uid);

/**
 * @brief Send HLTA to put the card into HALT state.
 *
 * Per ISO 14443-3 a halted card sends no response, so a timeout
 * from _Transceive is the expected (success) outcome.
 */
RC522_Status RC522_HaltCard(RC522_HandleTypeDef *hrc522);

/* Mifare Classic authentication & block I/O */

/**
 * @brief Authenticate with a Mifare Classic sector.
 *
 * Must be called after RC522_ReadCardUID() and before any
 * RC522_ReadBlock() / RC522_WriteBlock() in the same sector.
 * Call RC522_StopCrypto() when the session is finished.
 *
 * @param cmd        PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
 * @param blockAddr  Any block address within the target sector
 * @param key        6-byte authentication key
 * @param uid        UID obtained from RC522_ReadCardUID()
 */
RC522_Status RC522_Authenticate(RC522_HandleTypeDef *hrc522,
                                uint8_t cmd,
                                uint8_t blockAddr,
                                MIFARE_Key *key,
                                RC522_UID *uid);

/**
 * @brief Read 16 bytes from a Mifare Classic block.
 *
 * Requires a prior successful RC522_Authenticate() for the sector
 * containing blockAddr.
 *
 * @param blockAddr  Block number (0-63 for 1K, 0-255 for 4K)
 * @param data       Caller buffer of at least 18 bytes
 *                   (16 payload + 2 CRC appended by the card)
 * @param dataLen    [in]  buffer capacity (≥18)
 *                   [out] bytes received (normally 18)
 */
RC522_Status RC522_ReadBlock(RC522_HandleTypeDef *hrc522,
                             uint8_t blockAddr,
                             uint8_t *data,
                             uint8_t *dataLen);

/**
 * @brief Write 16 bytes to a Mifare Classic block.
 *
 * @param blockAddr  Block number (avoid block 0 and sector trailers
 *                   unless you know exactly what you are doing)
 * @param data       Pointer to exactly 16 bytes of payload
 */
RC522_Status RC522_WriteBlock(RC522_HandleTypeDef *hrc522,
                              uint8_t blockAddr,
                              uint8_t *data);

/**
 * @brief Disable Mifare crypto after a read/write session.
 *
 * Clears MFCrypto1On so the RC522 accepts plain ISO 14443-3 frames again.
 */
void RC522_StopCrypto(RC522_HandleTypeDef *hrc522);

/* Register access (debug / advanced) */

/**
 * @brief Read one byte from an RC522 register.
 *
 *   Byte 0 (TX): address byte — bit7 = 1 (read), bits[6:1] = reg addr, bit0 = 0
 *   Byte 1 (RX): data returned by the device
 */
uint8_t RC522_ReadRegister(RC522_HandleTypeDef *hrc522, uint8_t reg);

/**
 * @brief Write one byte to an RC522 register.
 *
 * SPI frame format (RC522 datasheet §8.1.2):
 *   Byte 0: address byte  — bit7 = 0 (write), bits[6:1] = reg addr, bit0 = 0
 *   Byte 1: data byte
 */
void RC522_WriteRegister(RC522_HandleTypeDef *hrc522,
                         uint8_t reg, uint8_t value);

/* Utility */

/**
 * @brief Identify the PICC type from the SAK byte.
 */
PICC_Type RC522_GetPICCType(uint8_t sak);

/**
 * @brief Return a human-readable string for a PICC_Type value.
 */
const char *RC522_GetPICCTypeName(PICC_Type type);

/**
 * @brief Format a UID as a colon-separated hex string.
 *
 * Example output for a 4-byte UID: "A3:B2:C1:D0"
 *
 * @param uid     Source UID
 * @param buf     Output buffer — must be at least (uid->size * 3) bytes
 * @param bufLen  Size of buf
 */
void RC522_UIDtoString(RC522_UID *uid, char *buf, uint8_t bufLen);

/** Default Mifare key: FF FF FF FF FF FF */
extern const MIFARE_Key MIFARE_DEFAULT_KEY;

#endif /* _RC522_H_ */