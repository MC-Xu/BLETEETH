#include "os/panchip_mempool.h"
#include <os/os_mempool.h>
#include "nimble_syscfg.h"
#include "pan_ble_stack.h"
#include <nimble/ble.h>
#include <nimble/hci_common.h>
#include "os/os.h"
#include "mem/mem.h"

#ifdef IP_101x

#define LL_DATA_FREE_SIZE          	   4096

#define PANCHIP_MEM_ALIGN_4(addr)        (((addr) + 3) & (~((uint32_t)0x03))) 	/*4 bytes aligned*/
#define PANCHIP_MEM_ALIGN_8(addr)        (((addr) + 7) & (~((uint32_t)0x07))) 	/*8 bytes aligned*/

typedef struct {
	uint8_t *p_addr;
	uint8_t *end_addr;
} panchip_mem_t;

panchip_mem_t m_panchip_mem;


void *panchip_mem_get(uint32_t size, uint8_t align)
{
	uint8_t *re_addr;
	if (m_panchip_mem.p_addr + align + size > m_panchip_mem.end_addr)
		return NULL;
	
	if (align == 4) {
		re_addr = (uint8_t *)PANCHIP_MEM_ALIGN_4((uint32_t)m_panchip_mem.p_addr);
	} else if (align == 8) {
		re_addr = (uint8_t *)PANCHIP_MEM_ALIGN_8((uint32_t)m_panchip_mem.p_addr);
	}
	
	m_panchip_mem.p_addr = re_addr + size;
	return re_addr;
	/*
	uint32_t tmp = m_panchip_mem.index;
	m_panchip_mem.index += size;
	
	if (m_panchip_mem.index < LL_DATA_FREE_SIZE) {
		return (m_panchip_mem.p_addr + tmp);
	} else {
		m_panchip_mem.index -= size;
		return NULL;
	} */
	
}

void *panchip_mem_init(void)
{
	m_panchip_mem.p_addr = pan_misc_malloc_ll_data_ram(LL_DATA_FREE_SIZE);
	m_panchip_mem.end_addr = m_panchip_mem.p_addr + LL_DATA_FREE_SIZE;
		
	return m_panchip_mem.p_addr;
}

#endif
