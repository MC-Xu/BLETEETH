/**************************************************************************//**
* @file     pan_hal_spi.h
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series SPI HAL (Hardware Abstraction Layer) header file.
*
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _PAN_HAL_SPI_H_
#define _PAN_HAL_SPI_H_

#include "pan_hal_def.h"

/**
 * @brief SPI HAL Interface
 * @defgroup spi_hal_interface Spi HAL Interface
 * @{
 */

// Config SPI software buffer size
#define SPI_RX_BUF_SIZE         256
#define SPI_TX_BUF_SIZE         256

/**
 * @brief  Structure to configure SPI interrupt mode.
 */
typedef struct
{
    SPI_IrqDef Mode; /*!< SPI interrupt mode. */
} SPI_InterruptOptDef;

/**
 * @brief  Enumeration defining SPI baud rates.
 */
typedef enum {
    BAUDRATE_100K   = 100000,   /*!< Baud rate: 100,000 bps. */
    BAUDRATE_500K   = 500000,   /*!< Baud rate: 500,000 bps. */
    BAUDRATE_1M     = 1000000,  /*!< Baud rate: 1,000,000 bps. */
    BAUDRATE_2M     = 2000000,  /*!< Baud rate: 2,000,000 bps. */
    BAUDRATE_5M     = 5000000,  /*!< Baud rate: 5,000,000 bps. */
    BAUDRATE_10M    = 10000000  /*!< Baud rate: 10,000,000 bps. */
} SPI_BaudRate;

/**
 * @brief  SPI Init Structure definition
 */
typedef struct
{
  SPI_RoleDef role;              /*!< Specifies the SPI operating mode */
  SPI_DataFrameSizeDef dataSize; /*!< Specifies the SPI data size */
  SPI_ClockPolDef clkPolarity;   /*!< Specifies the serial clock steady state */
  SPI_ClockPhaseDef clkPhase;    /*!< Specifies the clock active edge for the bit capture */
  // SPI_BaudRate Baudrate;      /*!< Specifies the Baud Rate Prescaler used to configure the transmit and receive SCK clock. */
  SPI_BaudRateDivDef baudrateDiv; /*!< Specifies the Baud Rate Prescaler used to configure the transmit and receive SCK clock. */
  SPI_FrameFormatDef format;      /*!< Specifies the SPI format */
} SPI_InitOptDef;


typedef enum{
    SPI_STAT_NULL,
    SPI_STAT_RX= 0x10,
    SPI_STAT_TX = 0x01,
    SPI_STAT_RXTX = 0x11,
}SPI_XferStatOpt;

/**
 * @brief  SPI handle Structure definition
 */
typedef struct
{
  SPI_T *pSpix; /*!< SPI registers base address        */

  SPI_InitOptDef initObj;           /*!< SPI communication parameters      */
  SPI_XferStatOpt xferStat;    /*!< SPI transfer status               */
  SPI_InterruptOptDef interruptObj; /*!< SPI interrupt options           */

  uint16_t *pTxBuffPtr; /*!< Pointer to SPI Tx transfer buffer */
  uint16_t txXferSize; /*!< SPI Tx transfer size              */
  __IO uint16_t txXferCount; /*!< SPI Tx transfer counter           */

  uint16_t *pRxBuffPtr; /*!< Pointer to SPI Rx transfer buffer */
  uint16_t rxXferSize; /*!< SPI Rx transfer size              */
  __IO uint16_t rxXferCount; /*!< SPI Rx transfer counter           */

  IRQn_Type IRQn; /*!< SPI IRQ number                    */
  SPI_CallbackFunc rxIntCallback; /*!< Callback function for Rx interrupt */
  SPI_CallbackFunc txIntCallback; /*!< Callback function for Tx interrupt */

  uint32_t dmaSrc;         /*!< DMA source address for SPI data transfer */
  uint32_t dmaDst;         /*!< DMA destination address for SPI data transfer */

  __IO uint32_t errorCode; /*!< SPI Error code                    */

} SPI_HandleTypeDef;

/**
 * @brief Array of SPI handle structures for managing multiple SPI interfaces.
 *
 * You can access specific SPI handles using SPI0_OBJ and SPI1_OBJ aliases.
 */
extern SPI_HandleTypeDef spiHandleArray[2];

/**
 * @brief SPI handle for the first SPI interface (SPI0).
 *
 * You can use this handle to configure and manage SPI0.
 */
#define SPI0_OBJ spiHandleArray[0]

/**
 * @brief SPI handle for the second SPI interface (SPI1).
 *
 * You can use this handle to configure and manage SPI1.
 */
#define SPI1_OBJ spiHandleArray[1]

/**
 * @brief Initialize the SPI module.
 * @param spi Pointer to the SPI handle structure.
 */
void HAL_SPI_Init(SPI_HandleTypeDef *pSpi);

/**
 * @brief DeInitialize the SPI module.
 * @param spi Pointer to the SPI handle structure.
 */
void HAL_SPI_DeInit(SPI_HandleTypeDef *pSpi);

/**
 * @brief Initialize SPI interrupts.
 * @param spi Pointer to the SPI handle structure.
 */
void HAL_SPI_Init_INT(SPI_HandleTypeDef *pSpi);

/**
 * @brief Initialize SPI interrupts.
 * @param spi Pointer to the SPI handle structure.
 */
void HAL_SPI_DeInit_INT(SPI_HandleTypeDef *pSpi);

/**
 * @brief Send data over SPI.
 * @param spi Pointer to the SPI handle structure.
 * @param Buf Pointer to the data buffer to send.
 * @param Size Size of the data to send.
 * @param Timeout Timeout for the send operation.
 */
void HAL_SPI_SendData(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, uint32_t timeout); 

/**
 * @brief Receive data from SPI.
 * @param spi Pointer to the SPI handle structure.
 * @param Buf Pointer to the buffer to store received data.
 * @param Size Size of the data to receive.
 * @param Timeout Timeout for the receive operation.
 */
void HAL_SPI_ReceiveData(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, uint32_t timeout); 

/**
 * @brief Send and receive data over SPI.
 * @param spi Pointer to the SPI handle structure.
 * @param SendBuf Pointer to the data buffer to send.
 * @param SendBufSize Size of the data to send.
 * @param RecvBuf Pointer to the buffer to store received data.
 * @param RecvBufSize Size of the data to receive.
 */
void HAL_SPI_SendReceiveData (SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize, uint16_t* pRecvBuf, size_t recvBufSize);
/**
 * @brief Send data over SPI with interrupt support.
 * @param spi Pointer to the SPI handle structure.
 * @param pBuf Pointer to the data buffer to send.
 * @param size Size of the data to send.
 * @param callback Callback function to handle the transmit interrupt.
 */
void HAL_SPI_SendData_INT(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, SPI_CallbackFunc callback);
/**
 * @brief Receive data over SPI with interrupt support.
 * @param spi Pointer to the SPI handle structure.
 * @param pBuf Pointer to the data buffer to store received data.
 * @param size Size of the data to receive.
 * @param callback Callback function to handle the receive interrupt.
 */
void HAL_SPI_ReceiveData_INT(SPI_HandleTypeDef *pSpi, uint16_t* pBuf, size_t size,SPI_CallbackFunc callback);

/**
 * @brief Send and receive data over SPI with interrupt support.
 * @param spi Pointer to the SPI handle structure.
 * @param SendBuf Pointer to the data buffer to send.
 * @param SendBufSize Size of the data to send.
 * @param SendCallback Callback function to handle the transmit interrupt.
 * @param RecvBuf Pointer to the buffer to store received data.
 * @param RecvBufSize Size of the data to receive.
 * @param RecvCallback Callback function to handle the receive interrupt.
 */
void HAL_SPI_SendReceiveData_INT(SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize,SPI_CallbackFunc sendCallback, uint16_t* pRecvBuf, size_t recvBufSize,SPI_CallbackFunc recvCallback);

/**
 * @brief Send data over SPI using DMA for improved performance.
 * @param spi Pointer to the SPI handle structure.
 * @param Buf Pointer to the data buffer to send.
 * @param Size Size of the data to send.
 * @param Callback Callback function to handle the DMA transfer completion.
 * @return DMA channel number used for the transfer.
 */
uint32_t HAL_SPI_SendData_DMA(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size,SPI_CallbackFunc callback);

/**
 * @brief Receive data over SPI using DMA for improved performance.
 * @param spi Pointer to the SPI handle structure.
 * @param Buf Pointer to the data buffer to send.
 * @param Size Size of the data to send.
 * @param Callback Callback function to handle the DMA transfer completion.
 * @return DMA channel number used for the transfer.
 */
uint32_t HAL_SPI_ReceiveData_DMA(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, SPI_CallbackFunc callback);
/**
 * @brief Send and receive data over SPI using DMA for improved performance.
 * @param spi Pointer to the SPI handle structure.
 * @param SendBuf Pointer to the data buffer to send.
 * @param SendBufSize Size of the data to send.
 * @param SendCallback Callback function to handle the DMA transmit completion.
 * @param RecvBuf Pointer to the buffer to store received data.
 * @param RecvBufSize Size of the data to receive.
 * @param RecvCallback Callback function to handle the DMA receive completion.
 */
void HAL_SPI_SendReceiveData_DMA(SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize,SPI_CallbackFunc sendCallback, uint16_t* pRecvBuf, size_t recvBufSize,SPI_CallbackFunc recvCallback);
/** @} */ // end of group
#endif 
