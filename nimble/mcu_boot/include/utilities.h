/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
 
#define ASSERT 			  while(1);

#define ALIGN(x)          __attribute__((aligned(x)))
#define RAM_FUNC		  __attribute__((section(".ramfunc")))
