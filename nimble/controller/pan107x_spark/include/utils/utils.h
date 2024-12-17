/**
 *******************************************************************************
 * @FileName  : utils.h
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

#ifndef UTILS_H_
#define UTILS_H_

#include "defs_types.h"

void *xmemcpy(void *dst, const void *src, uint32_t n);
int   xmemcmp(const void *m1, const void *m2, uint32_t n);
void *xmemset(void *buf, int c, uint32_t n);

/**
 * @brief  :  Copy the the buffer 'srx' to the buffer 'dest'.
 *
 * @param :  dest      Pointer to destination buffer
 * @param :  destLen   length of destination buffer
 * @param :  src       Pointer to origin buffer
 * @param :  srclen    length of origin buffer
 *
 * @return: returns   0 if srclen > destLen.
 */
uint32_t x_copy(uint8_t *dest, uint32_t destLen, const uint8_t *src, uint32_t srclen);

/**
 * @brief : Set the value 'val' into the buffer 'to', 'len' times.
 *
 * @param : to    Pointer to destination buffer
 * @param : val   value to be set in 'to'
 * @param : len   number of times the value will be copied
 *
 * @return: none.
 */
void x_set(void *to, uint8_t val, uint32_t len);

/**
 * @brief : Compare equal.
 * @param : a     Pointer point to data memory.
 * @param : b     Pointer point to data memory.
 * @param : size  length of data.
 * @return: Returns 0 if equal, and non-zero otherwise
 *
 * Note: This function can only compare for equality.
 */
int x_compare(const uint8_t *a, const uint8_t *b, uint32_t size);

/**
 * @brief : Reverse data.
 * @param : pdata    Pointer point to data memory.
 * @param : len      length of data.
 * @return: None.
 */
void _reverse(uint8_t *pdata, uint32_t len);

/**
 * @brief : Reverse data.
 * @param : from     Pointer point to input data buffer.
 * @param : to       Pointer point to output data buffer.
 * @param : len      length of data.
 * @return: None.
 */
void _swap(uint8_t *from, uint8_t *to, uint32_t len);

//XXX:64M system clock
static inline void sleep_us(uint32_t us)
{
	for(int i=0; i<us; i++){
		for(volatile int i=0; i<7; i++){};
	}
}

#endif /* UTILS_H_ */
