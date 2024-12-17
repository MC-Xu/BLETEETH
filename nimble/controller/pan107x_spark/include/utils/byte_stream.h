/**
 *******************************************************************************
 * @FileName  : byte_stream.h
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
#ifndef BYTE_STREAM_H_
#define BYTE_STREAM_H_

#include "defs_types.h"

/**
 * @Name: Macros for converting a little endian byte buffer to integers.
 * @{
 */
#define BYTES_TO_INT16(n, p)      {n = ((int16_t)(p)[0] + ((int16_t)(p)[1] << 8));}

#define BYTES_TO_UINT16(n, p)     {n = ((uint16_t)(p)[0] + ((uint16_t)(p)[1] << 8));}

#define BYTES_TO_UINT24(n, p)     {n = ((uint32_t)(p)[0] + ((uint32_t)(p)[1] << 8) + \
                                        ((uint32_t)(p)[2] << 16));}

#define BYTES_TO_UINT32(n, p)     {n = ((uint32_t)(p)[0] + ((uint32_t)(p)[1] << 8) + \
                                        ((uint32_t)(p)[2] << 16) + ((uint32_t)(p)[3] << 24));}

#define BYTES_TO_UINT40(n, p)     {n = ((uint64_t)(p)[0] + ((uint64_t)(p)[1] << 8) + \
                                        ((uint64_t)(p)[2] << 16) + ((uint64_t)(p)[3] << 24) + \
                                        ((uint64_t)(p)[4] << 32));}

#define BYTES_TO_UINT64(n, p)     {n = ((uint64_t)(p)[0] + ((uint64_t)(p)[1] << 8) + \
                                        ((uint64_t)(p)[2] << 16) + ((uint64_t)(p)[3] << 24) + \
                                        ((uint64_t)(p)[4] << 32) + ((uint64_t)(p)[5] << 40) + \
                                        ((uint64_t)(p)[6] << 48) + ((uint64_t)(p)[7] << 56));}
/**@}*/




/**
 * @Name: Macros for converting little endian integers to array of bytes
 */
#define UINT16_TO_BYTES(n)        ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT32_TO_BYTES(n)        ((uint8_t) (n)), ((uint8_t)((n) >> 8)), ((uint8_t)((n) >> 16)), ((uint8_t)((n) >> 24))
/**@}*/



/**
 * @Name: Macros for converting a little endian byte stream to integers, with increment.
 * @{
 */
#define BSTREAM_TO_INT8(n, p)     {n = (int8_t)(*(p)++);}
#define BSTREAM_TO_UINT8(n, p)    {n = (uint8_t)(*(p)++);}
#define BSTREAM_TO_INT16(n, p)    {BYTES_TO_INT16(n, p); (p) += 2;}
#define BSTREAM_TO_UINT16(n, p)   {BYTES_TO_UINT16(n, p); (p) += 2;}
#define BSTREAM_TO_UINT24(n, p)   {BYTES_TO_UINT24(n, p); (p) += 3;}
#define BSTREAM_TO_UINT32(n, p)   {BYTES_TO_UINT32(n, p); (p) += 4;}
#define BSTREAM_TO_UINT40(n, p)   {BYTES_TO_UINT40(n, p); (p) += 5;}
#define BSTREAM_TO_UINT64(n, p)   {n = BstreamToUint64(p); (p) += 8;}
#define BSTREAM_TO_BDA64(bda, p)  {bda = BstreamToBda64(p); (p) += BD_ADDR_LEN;}
/**@}*/

/**
 * @Name: Macros for converting integers to a little endian byte stream, with increment.
 * @{
 */
#define UINT8_TO_BSTREAM(p, n)    {*(p)++ = (uint8_t)(n);}

#define UINT16_TO_BSTREAM(p, n)   {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8);}

#define UINT24_TO_BSTREAM(p, n)   {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                  *(p)++ = (uint8_t)((n) >> 16);}

#define UINT32_TO_BSTREAM(p, n)   {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                   *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24);}

#define UINT40_TO_BSTREAM(p, n)   {*(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                   *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); \
                                   *(p)++ = (uint8_t)((n) >> 32);}

#define UINT64_TO_BSTREAM(p, n)   {Uint64ToBstream(p, n); p += sizeof(uint64_t);}

#define BDA64_TO_BSTREAM(p, bda)  {Bda64ToBstream(p, bda); p += BD_ADDR_LEN;}
/** @} */


void Uint64ToBstream(uint8_t *p, uint64_t n);
uint64_t BstreamToUint64(uint8_t *p);

#endif /* BYTE_STREAM_H_ */
