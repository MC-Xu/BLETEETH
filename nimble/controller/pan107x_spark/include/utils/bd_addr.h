/**
 *******************************************************************************
 * @FileName  : bd_Addr.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-05-18
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

#ifndef BD_ADDR_H_
#define BD_ADDR_H_

#include "defs_types.h"

#define BD_ADDR_LEN    6

typedef uint8_t bdAddr_t[BD_ADDR_LEN];

/**
 * @brief  : Copy bluetooch device address.
 * @param  : pTo       Pointer point to BD_ADDR buffer.
 * @param  : pFrom     Pointer point to BD_ADDR buffer.
 * @return : None.
 */
void BdaCpy(uint8_t *pTo, uint8_t *pFrom);

/**
 * @brief  : Compare bluetooch device address.
 * @param  : pDst1       Pointer point to BD_ADDR1
 * @param  : pDst2       Pointer point to BD_ADDR2
 * @return : TRUE if BD_ADDR1 == BD_ADDR2 or FALSE.
 */
bool_t BdaCmp(uint8_t* pDst1, uint8_t *pDst2);

/**
 * @brief  : Convert bstream to BDA64.
 * @param  : p       Bstream pointer.
 * @return : Resulting BDA64 number.
 */
static inline uint64_t BstreamToBda64(const uint8_t *p)
{
  return (uint64_t)p[0] <<  0 |
         (uint64_t)p[1] <<  8 |
         (uint64_t)p[2] << 16 |
         (uint64_t)p[3] << 24 |
         (uint64_t)p[4] << 32 |
         (uint64_t)p[5] << 40;
}

/**
 * @brief  : Convert BDA64 to bstream.
 * @param  : p       Bstream pointer.
 * @param  : bda     uint64_t_t BDA.
 * @return : None.
 */
static inline void Bda64ToBstream(uint8_t *p, uint64_t bda)
{
	p[0] = (uint8_t)(bda >>  0);
	p[1] = (uint8_t)(bda >>  8);
	p[2] = (uint8_t)(bda >> 16);
	p[3] = (uint8_t)(bda >> 24);
	p[4] = (uint8_t)(bda >> 32);
	p[5] = (uint8_t)(bda >> 40);
}

int BdaToString(char *buf, int bufLen, uint8_t *bda);


#endif /* BD_ADDR_H_ */
