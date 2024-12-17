/**
 *******************************************************************************
 * @FileName  : defs_types.h
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

#ifndef DEFS_TYPES_H
#define DEFS_TYPES_H

#include <stdint.h>

typedef uint8_t bool_t;

/**
 * GCC code optimization.
 *
 * - Complier cmd line parameter: -ffunction-sections -fdata-sections
 * - Linker cmd line parameter: --gc-sections
 */

#ifndef EXTERN
#define EXTERN            extern
#endif

#ifndef NOINLINE
#define NOINLINE          __attribute__((noinline))
#endif

#define WEAK              __attribute__((weak))
#define ALIGN(x)          __attribute__((aligned(x)))
#define SECTION(x)        __attribute__((section(x)))
#define PACKED            __attribute__((packed))

#ifndef RAM_CODE
#define RAM_CODE          __attribute__((section(".ramfunc")))
#endif

#ifndef RAM_CODE0 // save 800B
#define RAM_CODE0
#endif

#ifndef RAM_CODE1 //save 1.5K
#define RAM_CODE1
#endif

#ifndef RAM_CODE2
#define RAM_CODE2
#endif

#ifndef RAM_CODE3
#define RAM_CODE3
#endif

#ifndef NULL
#define NULL      ((void *)0)
#endif

#ifndef false
#define false     0
#endif

#ifndef FALSE
#define FALSE     0
#endif

#ifndef true
#define true      1
#endif
#ifndef TRUE
#define TRUE      1
#endif

#ifndef SUCCESS
#define SUCCESS   0
#endif

#ifndef FAILED
#define FAILED    1
#endif


#define UNUSED(x)   (void)(x)

#define MAX(a,b)   ((a) > (b) ? (a) : (b))
#define MIN(a,b)   ((a) < (b) ? (a) : (b))

#ifndef min
#define min           MIN
#endif

#ifndef max
#define max           MAX
#endif

#define BOUND(min, val, max)  MAX(min, MIN(max,val))

#define COUNTOF(x)    (sizeof(x)/sizeof((x)[0]))

#define HI_U16(x)     (((x)>>8) & 0xFF)
#define LO_U16(x)     ((x) & 0xff)

#define IS_POWER_OF_TWO(A)   ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )

#define OFFSETOF(s, m)   ((unsigned int)&(( ((s)*)0 )->(m)))

#ifndef BIT
#define BIT(x)      (1<<(x))
#endif

#define UINT64(x)     (uint64_t)(x)

/**
 * @brief : Macro for performing rounded integer division (as opposed to truncating the result).
 * @param :  A   Numerator.
 * @param :  B   Denominator.
 * @return:  Rounded (integer) result of dividing A by B.
 */
#define ROUNDED_DIV(A, B) (((A) + ((B) / 2)) / (B))

#define MEM_ALIGNED_SIZE(SIZE, ALIGN)    ( ((SIZE) + ((ALIGN) - 1)) & ~((ALIGN)-1) )
#define MEM_ALIGN_ADDR(addr, MEM_ALIGN)  ((void *)(((uint32_t)(addr) + MEM_ALIGN - 1) & ~(uint32_t)(MEM_ALIGN - 1)))

#define MEM_ALIGNED4(size)  MEM_ALIGNED_SIZE(size, 4 )
#define MEM_ALIGNED8(size)  MEM_ALIGNED_SIZE(size, 8 )
#define MEM_ALIGNED16(size) MEM_ALIGNED_SIZE(size, 16)

#define REG_32(addr)    *((volatile uint32_t *)(addr))

#define REG_WRITE(addr, value)                  REG_32(addr) = value
#define REG_WR_BITS(addr, pos, mask, value)		REG_32(addr) = ((REG_32(addr) & ~(mask)) | ((value)<<(pos)))

#define REG_READ(addr)                   REG_32(addr)
#define REG_READ_BITS(addr, pos, mask)   ((REG_READ(addr) >> pos) & mask)



#define DECLARE_SECTION_INFO(s)				   \
extern char __##s##_start[], __##s##_end[], __##s##_lma[]; \
extern unsigned __##s##_size

#define LMA(s) __##s##_lma
#define VMA(s) __##s##_start
#define SSIZE(s) (__##s##_end - __##s##_start)

DECLARE_SECTION_INFO(ram_code);
DECLARE_SECTION_INFO(data);
DECLARE_SECTION_INFO(bss);
DECLARE_SECTION_INFO(cache);

#endif


