#ifndef _RC522_H_
#define _RC522_H_

#include "stm32f1xx_hal.h"

#define uchar unsigned char
#define uint unsigned int

// Maximum length of the array
#define MAX_LEN 16

#define MFRC522_CS_PORT GPIOA
#define MFRC522_CS_PIN GPIO_PIN_4
#define MFRC522_RST_PORT GPIOA
#define MFRC522_RST_PIN GPIO_PIN_3

// MFRC522 commands. Described in chapter 10 of the datasheet.
#define PCD_IDLE 0x00       // no action, cancels current command execution
#define PCD_AUTHENT 0x0E    // performs the MIFARE standard authentication as a reader
#define PCD_RECEIVE 0x08    // activates the receiver circuits
#define PCD_TRANSMIT 0x04   // transmits data from the FIFO buffer
#define PCD_TRANSCEIVE 0x0C // transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
#define PCD_RESETPHASE 0x0F // resets the MFRC522
#define PCD_CALCCRC 0x03    // activates the CRC coprocessor or performs a self-test

// Commands sent to the PICC.
#define PICC_REQIDL 0x26    // Request command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define PICC_REQALL 0x52    // Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define PICC_ANTICOLL 0x93  // Anti collision/Select, Cascade Level 1
#define PICC_SElECTTAG 0x95 // Anti collision/Select, Cascade Level 2
#define PICC_AUTHENT1A 0x60 // Perform authentication with Key A
#define PICC_AUTHENT1B 0x61 // Perform authentication with Key B
#define PICC_READ 0x30      // Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define PICC_WRITE 0xA0     // Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define PICC_DECREMENT 0xC0 // Decrements the contents of a block and stores the result in the internal data register.
#define PICC_INCREMENT 0xC1 // Increments the contents of a block and stores the result in the internal data register
#define PICC_RESTORE 0xC2   // Reads the contents of a block into the internal data register.
#define PICC_TRANSFER 0xB0  // Writes the contents of the internal data register to a block.
#define PICC_HALT 0x50      // HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.

// MFRC522 registers. Described in chapter 9 of the datasheet.

/* -------------------------------------------------------------------------- */
/* Page 0: Command and Status Registers                                       */
/* -------------------------------------------------------------------------- */

#define RESERVED_00 0x00
#define COMMAND_REG 0x01     /**< Starts and stops command execution */
#define COMM_IEN_REG 0x02    /**< Communication interrupt enable */
#define DIV_IEN_REG 0x03     /**< Divider interrupt enable */
#define COMM_IRQ_REG 0x04    /**< Communication interrupt request */
#define DIV_IRQ_REG 0x05     /**< Divider interrupt request */
#define ERROR_REG 0x06       /**< Error status register */
#define STATUS1_REG 0x07     /**< Communication status register */
#define STATUS2_REG 0x08     /**< Receiver and transmitter status */
#define FIFO_DATA_REG 0x09   /**< FIFO data input/output */
#define FIFO_LEVEL_REG 0x0A  /**< Number of bytes stored in FIFO */
#define WATER_LEVEL_REG 0x0B /**< FIFO level warning threshold */
#define CONTROL_REG 0x0C     /**< Miscellaneous control register */
#define BIT_FRAMING_REG 0x0D /**< Bit framing adjustments */
#define COLL_REG 0x0E        /**< Collision detection register */
#define RESERVED_01 0x0F

/* -------------------------------------------------------------------------- */
/* Page 1: Command Configuration Registers                                    */
/* -------------------------------------------------------------------------- */

#define RESERVED_10 0x10
#define MODE_REG 0x11         /**< Defines general transmission modes */
#define TX_MODE_REG 0x12      /**< Transmission data rate and framing */
#define RX_MODE_REG 0x13      /**< Reception data rate and framing */
#define TX_CONTROL_REG 0x14   /**< Controls the antenna driver */
#define TX_AUTO_REG 0x15      /**< Automatic transmission settings */
#define TX_SEL_REG 0x16       /**< Internal transmitter source selection */
#define RX_SEL_REG 0x17       /**< Internal receiver settings */
#define RX_THRESHOLD_REG 0x18 /**< Receiver threshold configuration */
#define DEMOD_REG 0x19        /**< Demodulator settings */
#define RESERVED_11 0x1A
#define RESERVED_12 0x1B
#define MIFARE_REG 0x1C /**< MIFARE protocol configuration */
#define RESERVED_13 0x1D
#define RESERVED_14 0x1E
#define SERIAL_SPEED_REG 0x1F /**< UART communication speed */

/* -------------------------------------------------------------------------- */
/* Page 2: Configuration Registers                                            */
/* -------------------------------------------------------------------------- */

#define RESERVED_20 0x20
#define CRC_RESULT_REG_H 0x21 /**< CRC calculation result MSB */
#define CRC_RESULT_REG_L 0x22 /**< CRC calculation result LSB */
#define RESERVED_21 0x23
#define MOD_WIDTH_REG 0x24 /**< Modulation width control */
#define RESERVED_22 0x25
#define RF_CFG_REG 0x26            /**< RF receiver gain configuration */
#define GS_N_REG 0x27              /**< Conductance settings */
#define CW_GS_P_REG 0x28           /**< CW conductance settings */
#define MOD_GS_P_REG 0x29          /**< Modulation conductance settings */
#define T_MODE_REG 0x2A            /**< Timer mode settings */
#define T_PRESCALER_REG 0x2B       /**< Timer prescaler value */
#define T_RELOAD_REG_H 0x2C        /**< Timer reload value high byte */
#define T_RELOAD_REG_L 0x2D        /**< Timer reload value low byte */
#define T_COUNTER_VALUE_REG_H 0x2E /**< Timer counter value high byte */
#define T_COUNTER_VALUE_REG_L 0x2F /**< Timer counter value low byte */

/* -------------------------------------------------------------------------- */
/* Page 3: Test Registers                                                     */
/* -------------------------------------------------------------------------- */

#define RESERVED_30 0x30
#define TEST_SEL1_REG 0x31
#define TEST_SEL2_REG 0x32
#define TEST_PIN_EN_REG 0x33
#define TEST_PIN_VALUE_REG 0x34
#define TEST_BUS_REG 0x35
#define AUTO_TEST_REG 0x36
#define VERSION_REG 0x37 /**< MFRC522 firmware version */
#define ANALOG_TEST_REG 0x38
#define TEST_DAC1_REG 0x39
#define TEST_DAC2_REG 0x3A
#define TEST_ADC_REG 0x3B
#define RESERVED_31 0x3C
#define RESERVED_32 0x3D
#define RESERVED_33 0x3E
#define RESERVED_34 0x3F

typedef enum
{
    MI_OK,
    MI_NOTAGERR,
    MI_ERR
} MFRC522_Status;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;
} RC522_HandleTypeDef;

/**
 * @brief  Initialize the MFRC522 RFID module.
 *
 * This function sets up and initializes the MFRC522 chip using SPI communication.
 * It assigns the SPI handle to the RC522_HandleTypeDef structure and performs
 * the necessary configuration steps for the RFID module to operate correctly.
 *
 * @param hrfid  Pointer to the RC522_HandleTypeDef structure.
 * @param hspi   Pointer to the SPI_HandleTypeDef structure used for SPI
 *               communication with the MFRC522.
 *
 * @retval None
 */
void MFRC522_Init(RC522_HandleTypeDef *hrfid, SPI_HandleTypeDef *hspi);

/**
 * @brief  Request a card from the RF field.
 * This function searches for RFID cards within the antenna field and
 * returns the detected card type.
 * @param reqMode  Card detection mode.
 *                      - 0x26 : Request idle cards (REQA)
 *                      - 0x52 : Request all cards (WUPA)
 * @param TagType  Pointer to a 2-byte buffer that receives the card type:
 *                      - 0x4400 : Mifare Ultralight
 *                      - 0x0400 : Mifare One (S50)
 *                      - 0x0200 : Mifare One (S70)
 *                      - 0x0800 : Mifare Pro (X)
 *                      - 0x4403 : Mifare DESFire
 * @retval MFRC522_Status
 *         - MI_OK       : Card detected successfully
 *         - MI_ERR      : Error occurred
 *         - MI_NOTAGERR : No card found
 */
MFRC522_Status MFRC522_Request(uchar reqMode, uchar *TagType);

/**
 * @brief  Perform anti-collision detection and retrieve the card UID.
 *
 * This function executes the ISO14443 anti-collision procedure to obtain
 * the UID (Unique Identifier) of the RFID card currently in the RF field.
 *
 * @param serNum  Pointer to a buffer that receives the card UID.
 *                The buffer must be at least 5 bytes:
 *                - Byte 0..3 : UID
 *                - Byte 4    : BCC (Block Check Character)
 *
 * @retval MFRC522_Status
 *         - MI_OK  : UID successfully received and verified
 *         - MI_ERR : UID checksum error or communication failure
 */
MFRC522_Status MFRC522_Anticoll(uchar *serNum);

/**
 * @brief  Select an RFID card and retrieve its SAK (Select Acknowledge).
 *
 * This function selects a card using its UID and receives the SAK response
 * from the card. The SAK value can be used to determine the card type
 * and memory capacity.
 *
 * @param serNum  Pointer to the card UID buffer (5 bytes):
 *                    - Byte 0..3 : UID
 *                    - Byte 4    : BCC (checksum)
 *
 * @return uchar
 *         - SAK value (card type / capacity information) if successful
 *         - 0 if selection failed
 */
uchar MFRC522_SelectTag(uchar *serNum);

/**
 * @brief  Write 16 bytes of data to a MIFARE card block.
 *
 * This function writes a full 16-byte data block to the specified block
 * address of a MIFARE card. The operation is performed according to the
 * ISO14443 protocol using the MFRC522 transceiver.
 *
 * @param blockAddr  Block address to write (0–63 for MIFARE Classic 1K).
 * @param writeData  Pointer to a 16-byte buffer containing the data to write.
 *
 * @retval MFRC522_Status
 *         - MI_OK  : Write operation successful
 *         - MI_ERR : Write operation failed or card did not acknowledge
 */
MFRC522_Status MFRC522_Write(uchar blockAddr, uchar *writeData);

/**
 * @brief  Authenticate a MIFARE card block using Key A or Key B.
 *
 * This function performs authentication for a specific block of a MIFARE
 * card using either Key A or Key B. Authentication must succeed before
 * reading or writing protected blocks.
 *
 * @param authMode   Authentication mode:
 *                   - 0x60 : Key A authentication
 *                   - 0x61 : Key B authentication
 *
 * @param BlockAddr  Block address to authenticate.
 *
 * @param Sectorkey  Pointer to a 6-byte sector key (Key A or Key B).
 *
 * @param serNum     Pointer to the first 4 bytes of the card UID.
 *
 * @retval MFRC522_Status
 *         - MI_OK  : Authentication successful
 *         - MI_ERR : Authentication failed
 */
MFRC522_Status MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum);

/**
 * @brief  Read a 16-byte data block from a MIFARE card.
 *
 * This function reads a full data block from the specified block address
 * of a MIFARE card using the MFRC522 transceiver.
 *
 * @param  blockAddr  Block address to read.
 * @param  recvData   Pointer to a buffer that receives the block data.
 *                    The buffer must be at least 18 bytes:
 *                    - Byte 0..15 : Data
 *                    - Byte 16..17: CRC_A
 *
 * @retval MFRC522_Status
 *         - MI_OK  : Read operation successful
 *         - MI_ERR : Read operation failed
 */
MFRC522_Status MFRC522_Read(uchar blockAddr, uchar *recvData);

/**
 * @brief  Put the RFID card into HALT state.
 *
 * This function sends the HALT command to the card, causing it to enter
 * the HALT state. In this state, the card stops responding to further
 * commands until it is woken up by a REQA or WUPA request.
 *
 * @param None
 *
 * @retval None
 */
void MFRC522_Halt(void);

/**
 * @brief  Communicate with a card via the RC522 using ISO14443 protocol.
 *
 * This function sends a command and data to the RFID card through the
 * MFRC522 reader and receives the response from the card.
 *
 * @param  command   MFRC522 command (e.g., PCD_TRANSCEIVE, PCD_AUTHENT).
 * @param  sendData  Pointer to the data buffer to be transmitted to the card.
 * @param  sendLen   Number of bytes to send.
 * @param  backData  Pointer to the buffer that receives data returned from the card.
 * @param  backLen   Pointer to variable that stores the received data length (in bits).
 *
 * @retval MFRC522_Status
 *         - MI_OK       : Communication successful
 *         - MI_ERR      : Communication error
 *         - MI_NOTAGERR : No card detected
 */
MFRC522_Status MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen);

/**
 * @brief  Read a byte from an MFRC522 register.
 *
 * @param  addr   Register address.
 * @retval uchar  Value read from the specified register.
 */
uchar Read_MFRC522(uchar addr);

/**
 * @brief  Write a byte to an MFRC522 register.
 *
 * @param addr  Register address.
 * @param val   Data value to be written to the register.
 *
 * @retval None
 */
void Write_MFRC522(uchar addr, uchar val);

/**
 * @brief Open antennas, each time you start or shut down the natural
 barrier between the transmitter should be at least 1ms interval
 * @param None
 * @retval None
 */
void AntennaOn(void);

/**
 * @brief Close antennas, each time you start or shut down the natural
 barrier between the transmitter should be at least 1ms interval
 * @param None
 * @retval None
 */
void AntennaOff(void);

/**
 * @brief Set RC522 register bit
 * @param reg   register address
 * @param mask  set value
 * @retval None
 */
void SetBitMask(uchar reg, uchar mask);

/**
 * @brief Clear RC522 register bit
 * @param reg   register address
 * @param mask  clear bit value
 * @retval None
 */
void ClearBitMask(uchar reg, uchar mask);

/**
 * @brief  Calculate CRC using the MFRC522 CRC coprocessor.
 *
 * This function uses the internal CRC unit of the MFRC522 to calculate
 * the CRC_A checksum required by the ISO14443 protocol.
 *
 * @param  pIndata   Pointer to the input data buffer.
 * @param  len       Length of the input data (in bytes).
 * @param  pOutData  Pointer to a 2-byte buffer that receives the CRC result
 *                       (CRC_LSB and CRC_MSB).
 *
 * @retval None
 */
void CalculateCRC(uchar *pIndata, uchar len, uchar *pOutData);

/**
 * @brief Reset RC522
 * @param None
 * @retval None
 */
void MFRC522_Reset(void);

/* Debug value */
// extern uint8_t debug_n;
// extern uint8_t debug_lastBits;
// extern uint8_t debug_raw[5];

#endif /* _RC522_H_ */