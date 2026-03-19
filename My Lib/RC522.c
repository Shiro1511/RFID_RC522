#include "RC522.h"

/* Debug values */
// uint8_t debug_n = 0;
// uint8_t debug_lastBits = 0;
// uint8_t debug_raw[5] = {0};

static SPI_HandleTypeDef *hspi_global;

/**
 * @brief  Transfer one byte via SPI to the MFRC522.
 *
 * This function sends one byte through SPI and simultaneously receives
 * one byte from the MFRC522 (SPI full-duplex transfer).
 *
 * @param data  Byte to be transmitted.
 *
 * @return uint8_t  Byte received from the SPI bus.
 */
static uint8_t RC522_SPI_Transfer(uchar data)
{
	uchar rx_data;
	HAL_SPI_TransmitReceive(hspi_global, &data, &rx_data, 1, 100);

	return rx_data;
}

void Write_MFRC522(uchar addr, uchar val)
{
	/* CS LOW */
	HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_RESET);

	// even though we are calling transfer frame once, we are really sending
	// two 8-bit frames smooshed together-- sending two 8 bit frames back to back
	// results in a spike in the select line which will jack with transactions
	// - top 8 bits are the address. Per the spec, we shift the address left
	//   1 bit, clear the LSb, and clear the MSb to indicate a write
	// - bottom 8 bits are the data bits being sent for that address, we send them
	RC522_SPI_Transfer((addr << 1) & 0x7E);
	RC522_SPI_Transfer(val);

	/* CS HIGH */
	HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);
}

uchar Read_MFRC522(uchar addr)
{
	uchar val;

	/* CS LOW */
	HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_RESET);

	// even though we are calling transfer frame once, we are really sending
	// two 8-bit frames smooshed together-- sending two 8 bit frames back to back
	// results in a spike in the select line which will jack with transactions
	// - top 8 bits are the address. Per the spec, we shift the address left
	//   1 bit, clear the LSb, and set the MSb to indicate a read
	// - bottom 8 bits are all 0s on a read per 8.1.2.1 Table 6
	RC522_SPI_Transfer(((addr << 1) & 0x7E) | 0x80);
	val = RC522_SPI_Transfer(0x00);

	/* CS HIGH */
	HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);

	return val;
}

void SetBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp | mask); // set bit mask
}

void ClearBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp & (~mask)); /* Clear bit mask */
}

void AntennaOn(void)
{
	Read_MFRC522(TX_CONTROL_REG);
	SetBitMask(TX_CONTROL_REG, 0x03);
}

void AntennaOff(void)
{
	ClearBitMask(TX_CONTROL_REG, 0x03);
}

void MFRC522_Reset(void)
{
	Write_MFRC522(COMMAND_REG, PCD_RESETPHASE);
}

void MFRC522_Init(RC522_HandleTypeDef *hrfid, SPI_HandleTypeDef *hspi)
{
	hrfid->hspi = hspi;
	hrfid->cs_port = MFRC522_CS_PORT;
	hrfid->cs_pin = MFRC522_CS_PIN;
	hrfid->rst_port = MFRC522_RST_PORT;
	hrfid->rst_pin = MFRC522_RST_PIN;
	hspi_global = hspi;

	HAL_GPIO_WritePin(hrfid->cs_port, hrfid->cs_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(hrfid->rst_port, hrfid->rst_pin, GPIO_PIN_SET);
	HAL_Delay(50);

	MFRC522_Reset();
	HAL_Delay(10);

	/* Timer: TPrescaler * TreloadVal / 6.78 MHz = 24ms */
	Write_MFRC522(T_MODE_REG, 0x8D);	  /* Tauto = 1; f(timer) = 6.78Mhz / Tprescaler */
	Write_MFRC522(T_PRESCALER_REG, 0x3E); /* TModeReg[3..0] + TprescalerReg */
	Write_MFRC522(T_RELOAD_REG_H, 0x03);
	Write_MFRC522(T_RELOAD_REG_L, 0xE8);

	Write_MFRC522(TX_AUTO_REG, 0x40); /* force 100% ASK modulation */
	Write_MFRC522(MODE_REG, 0x3D);	  /* CRC Initial value 0x6363 */

	Write_MFRC522(RF_CFG_REG, 0x70);

	AntennaOn();
	HAL_Delay(10);

	//	Write_MFRC522(RX_THRESHOLD_REG, 0x84);
	//	Write_MFRC522(DEMOD_REG, 0x6D);
	//	Write_MFRC522(RF_CFG_REG, 0x70);
}

MFRC522_Status MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
	MFRC522_Status status = MI_ERR;
	uchar irqEn = 0x00;
	uchar waitIRq = 0x00;
	uchar lastBits;
	uchar n;
	uint16_t i;

	switch (command)
	{
	case PCD_AUTHENT: /* Certification cards close */
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	case PCD_TRANSCEIVE: /* Transmit FIFO data */
		irqEn = 0x77;
		waitIRq = 0x30;
		break;

	default:
		break;
	}

	Write_MFRC522(COMM_IEN_REG, irqEn | 0x80); /* Interrupt request */
	ClearBitMask(COMM_IRQ_REG, 0x80);		   /* Clear all interrupt request bit */
	SetBitMask(FIFO_LEVEL_REG, 0x80);		   /* FlushBuffer = 1, FIFO Intialization */

	Write_MFRC522(COMMAND_REG, PCD_IDLE); /* No action; Cancel the current command */

	/* Writing data to the FIFO */
	for (i = 0; i < sendLen; i++)
	{
		Write_MFRC522(FIFO_DATA_REG, sendData[i]);
	}

	/* Execute the command */
	Write_MFRC522(COMMAND_REG, command);
	if (command == PCD_TRANSCEIVE)
	{
		SetBitMask(BIT_FRAMING_REG, 0x80); /* StartSend = 1, tranmission of data starts */
	}

	/* Waiting to receive data to complete */
	i = 2000; /* i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms */
	do
	{
		/* CommIrqReg[7..0] */
		/* Set1 Tx IRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq */
		n = Read_MFRC522(COMM_IRQ_REG);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	ClearBitMask(BIT_FRAMING_REG, 0x80); /* StartSend = 0*/

	if (i != 0)
	{
		if (!(Read_MFRC522(ERROR_REG) & 0x1B)) /* BufferOvfl Collerr CRCErr ProtecolErr */
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{
				status = MI_NOTAGERR;
			}
			if (command == PCD_TRANSCEIVE)
			{
				n = Read_MFRC522(FIFO_LEVEL_REG);
				lastBits = Read_MFRC522(CONTROL_REG) & 0x07;
				//				debug_n = n;
				//				debug_lastBits = lastBits;
				if (lastBits)
				{
					*backLen = (n - 1) * 8 + lastBits;
				}
				else
				{
					*backLen = n * 8;
				}

				if (n == 0)
				{
					n = 1;
				}
				if (n > MAX_LEN)
				{
					n = MAX_LEN;
				}

				// Reading the received data in FIFO
				for (i = 0; i < n; i++)
				{
					backData[i] = Read_MFRC522(FIFO_DATA_REG);
					//					if(i < 5)
					//					{
					//						debug_raw[i] = backData[i];
					//					}
				}
			}
		}
		else
		{
			status = MI_ERR;
		}
	}

	return status;
}

MFRC522_Status MFRC522_Request(uchar reqMode, uchar *TagType)
{
	MFRC522_Status status;
	uint backBits; // The received data bits

	Write_MFRC522(BIT_FRAMING_REG, 0x07); // TxLastBists = BitFramingReg[2..0]

	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10))
	{
		status = MI_ERR;
	}

	return status;
}

MFRC522_Status MFRC522_Anticoll(uchar *serNum)
{
	MFRC522_Status status;
	uchar i;
	uchar serNumCheck = 0;
	uint unLen;

	Write_MFRC522(BIT_FRAMING_REG, 0x00); // TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK)
	{
		// Check card serial number
		for (i = 0; i < 4; i++)
		{
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i])
		{
			status = MI_ERR;
		}
	}

	return status;
}

void CalculateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
	uchar i, n;

	ClearBitMask(DIV_IRQ_REG, 0x04);  // CRCIrq = 0
	SetBitMask(FIFO_LEVEL_REG, 0x80); // Clear the FIFO pointer

	// Writing data to the FIFO
	for (i = 0; i < len; i++)
	{
		Write_MFRC522(FIFO_DATA_REG, *(pIndata + i));
	}
	Write_MFRC522(COMMAND_REG, PCD_CALCCRC);

	// Wait CRC calculation is complete
	i = 0xFF;
	do
	{
		n = Read_MFRC522(DIV_IRQ_REG);
		i--;
	} while ((i != 0) && !(n & 0x04)); // CRCIrq = 1

	// Read CRC calculation result
	pOutData[0] = Read_MFRC522(CRC_RESULT_REG_L);
	pOutData[1] = Read_MFRC522(CRC_RESULT_REG_H);
}

uchar MFRC522_SelectTag(uchar *serNum)
{
	uchar i;
	MFRC522_Status status;
	uchar size;
	uint recvBits;
	uchar buffer[9];

	// ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++)
	{
		buffer[i + 2] = *(serNum + i);
	}
	CalculateCRC(buffer, 7, &buffer[7]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18))
	{
		size = buffer[0];
	}
	else
	{
		size = 0;
	}

	return size;
}

MFRC522_Status MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
	MFRC522_Status status;
	uint recvBits;
	uchar i;
	uchar buff[12];

	// Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++)
	{
		buff[i + 2] = *(Sectorkey + i);
	}
	for (i = 0; i < 4; i++)
	{
		buff[i + 8] = *(serNum + i);
	}
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(Read_MFRC522(STATUS2_REG) & 0x08)))
	{
		status = MI_ERR;
	}

	return status;
}

/*
 * @brief: Read block data
 * @param: blockAddr - block address; recvData - read block data
 * @retval: the successful return MI_OK
 */
MFRC522_Status MFRC522_Read(uchar blockAddr, uchar *recvData)
{
	MFRC522_Status status;
	uint unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	CalculateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90))
	{
		status = MI_ERR;
	}

	return status;
}

MFRC522_Status MFRC522_Write(uchar blockAddr, uchar *writeData)
{
	MFRC522_Status status;
	uint recvBits;
	uchar i;
	uchar buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	CalculateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
	{
		status = MI_ERR;
	}

	if (status == MI_OK)
	{
		for (i = 0; i < 16; i++) // Data to the FIFO write 16Byte
		{
			buff[i] = *(writeData + i);
		}
		CalculateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		{
			status = MI_ERR;
		}
	}

	return status;
}

void MFRC522_Halt(void)
{
	uint unLen;
	uchar buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	CalculateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}