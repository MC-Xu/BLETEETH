/**
 *******************************************************************************
 * @FileName  : fifo.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-02-18
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "defs_types.h"

#define FIFO_NEW_EN               0

/*! FIFO error code */
#define FIFO_SUCCESS              0x00
#define FIFO_ERR_INVALID_PARAM    0x01
#define FIFO_ERR_INVALID_LENGTH   0x02
#define FIFO_ERR_INVALID_NUM      0x03

typedef uint8_t buf_num_t;

/*! FIFO type */
typedef struct{
	uint8_t       *pBuf;
	uint32_t       size;
	buf_num_t      r;
	buf_num_t      w;

#if FIFO_NEW_EN
	buf_num_t      num;
#else
	buf_num_t      numMask;
#endif
}fifo_t;

/**
 * !!!Note: num must is 2 power.
 */
#if FIFO_NEW_EN
	#define FIFO_DEFINE(fifoName, size, num) \
	static uint8_t (fifoName##Buf)[(size) * (num)];\
	fifo_t fifoName = { \
	   (fifoName##Buf),\
	   (size),\
	   0,\
	   0,\
	   (num),\
	}
#else
	#define FIFO_DEFINE(fifoName, size, num) \
	static uint8_t (fifoName##Buf)[(size) * (num)];\
	fifo_t fifoName = { \
	   (fifoName##Buf),\
	   (size),\
	   0,\
	   0,\
	   (num)-1,\
	}
#endif

/**
 * @brief: FIFO initialization.
 * @param: pFifo     pointer to FIFO
 * @param: pBuf      pointer to memory
 * @param: byteSize  the size of memory
 * @param: num       number of buffer;
 * @return:
 */
uint8_t FIFO_Init(fifo_t *pFifo, uint8_t *pBuf, uint32_t byteSize, uint32_t num);

/**
 * @brief:  Get the number of used buffer
 * @param:  pFifo     pointer point to FIFO
 * @return:
 */
static inline buf_num_t FIFO_Length(fifo_t *pFifo)
{
#if FIFO_NEW_EN
	if(pFifo->w < pFifo->r){
		return (buf_num_t)(pFifo->num + pFifo->w - pFifo->r);
	}
	return (buf_num_t)(pFifo->w - pFifo->r);
#else
	return (buf_num_t)(pFifo->w - pFifo->r);
#endif
}

/**
 * @brief:  Get FIFO total number
 * @param:  pFifo     pointer point to FIFO
 * @return:
 */
static inline buf_num_t FIFO_GetBufTotalNum(fifo_t *pFifo)
{
#if FIFO_NEW_EN
	return pFifo->num;
#else
	return pFifo->numMask + 1;
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline bool_t FIFO_IsFull(fifo_t *pFifo)
{
#if FIFO_NEW_EN
	return FIFO_Length(pFifo) >= pFifo->num ? true : false;
#else
	return FIFO_Length(pFifo) >= (pFifo->numMask + 1);
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline uint8_t *FIFO_GetWriteBuf(fifo_t *pFifo)
{
	if(pFifo == NULL){
		return NULL;
	}
#if FIFO_NEW_EN
	return pFifo->pBuf + pFifo->size * (pFifo->w % pFifo->num);
#else
	return pFifo->pBuf + pFifo->size * (pFifo->w & pFifo->numMask);
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline uint8_t *FIFO_GetNextWriteBuf(fifo_t *pFifo)
{
	if(pFifo == NULL){
		return NULL;
	}

#if FIFO_NEW_EN
	pFifo->w = (++pFifo->w) % pFifo->num;
	return pFifo->pBuf + pFifo->size * (pFifo->w % pFifo->num);
#else
	pFifo->w++;
	return pFifo->pBuf + pFifo->size * (pFifo->w & pFifo->numMask);
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline uint8_t *FIFO_GetPreWriteBuf(fifo_t *pFifo)
{
	if(pFifo == NULL){
		return NULL;
	}

#if FIFO_NEW_EN
	pFifo->w = (--pFifo->w) % pFifo->num;
	return pFifo->pBuf + pFifo->size * (pFifo->w % pFifo->num);
#else
	pFifo->w--;
	return pFifo->pBuf + pFifo->size * (pFifo->w & pFifo->numMask);
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline void FIFO_MoveToNextWriteBuf(fifo_t *pFifo)
{
	if(pFifo == NULL)
		return;

#if FIFO_NEW_EN
	pFifo->w = (++pFifo->w) % pFifo->num;
#else
	pFifo->w++;
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline uint8_t *FIFO_GetReadBuf(fifo_t *pFifo)
{
	if(pFifo == NULL){
		return NULL;
	}

#if FIFO_NEW_EN
	return pFifo->pBuf + pFifo->size * (pFifo->r % pFifo->num);
#else
	return pFifo->pBuf + pFifo->size * (pFifo->r & pFifo->numMask);
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline void FIFO_MoveToNextReadBuf(fifo_t *pFifo)
{
	if(pFifo == NULL)
		return;

#if FIFO_NEW_EN
	pFifo->r = (++pFifo->r) % pFifo->num;
#else
	pFifo->r++;
#endif
}

/**
 * @brief:
 * @param:
 * @return:
 */
static inline uint32_t FIFO_Flush(fifo_t *pFifo)
{
	if(pFifo == NULL){
		return FIFO_ERR_INVALID_PARAM;
	}

	pFifo->r = pFifo->w;//=0
	return FIFO_SUCCESS;
}


#endif /* FIFO_H_ */
