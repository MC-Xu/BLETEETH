#ifndef PANCHIP_MEMPOOL__H
#define PANCHIP_MEMPOOL__H
#include <stdint.h>

void *panchip_mem_init(void);

void *panchip_mem_get(uint32_t size, uint8_t align);

#endif
