#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "nimble/nimble_port.h"
#include "nimble_syscfg.h"
#include "nimble/ble.h"

#include "nimble/pan107x/nimble_glue_spark.h"
#include "nimble/hci_common.h"
#include "pan_ble_stack.h"
#include "pan_svc_call.h"
#include "comm_prf.h"

#include "ble_config.h"

#if (configUSE_TICKLESS_IDLE == 1)
extern void UpdateTickAndSch(void);
#endif

static TaskHandle_t host_task_h;
static uint8_t ll_enable = false;

#define HCI_BUF_SIZE_ALIGN4(payload) MEM_ALIGNED4(4 + payload)

#ifdef IP_107x
#define PAN_BLE_CTLR_BUFFER_SIZE    MEM_ALIGNED4(2148 +                                                                  \
												(HCI_BUF_SIZE_ALIGN4(MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE) << 3) +            \
												(HCI_BUF_SIZE_ALIGN4(65) << 3) +                                           \
												(500+256) * (CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL) +   \
												80 * CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM +                               \
												36 * (2 + CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL)  \
												) 
												

#elif defined(IP_101x)										
#define PAN_BLE_CTLR_BUFFER_SIZE    MEM_ALIGNED4(1024 +                                                                   \
												(HCI_BUF_SIZE_ALIGN4(251)*1) +                                            \
												(HCI_BUF_SIZE_ALIGN4(65) *1) +                                            \
												(500 + 256) * (CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL) +  \
												80 * CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM +                              \
												36 * (2 + CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL) \
												)  
#endif
				 
static uint32_t mem_buffer[PAN_BLE_CTLR_BUFFER_SIZE / 4];
static uint32_t mem_pos = 0;

static void *mem_alloc(uint32_t size)
{
	void *mem_ret = NULL;
	char *p_mem = (char *)mem_buffer;

	if (PAN_BLE_CTLR_BUFFER_SIZE - mem_pos >= size) {
		mem_ret = &p_mem[mem_pos];
		mem_pos += size;
	} else {
	#if CONFIG_UART_LOG_ENABLE
		printf("[E] BLE Heap buffer allocated Failed, need:%d B(%d)\n",size,PAN_BLE_CTLR_BUFFER_SIZE);
	#endif
		configASSERT(0);
	}
	
#if CONFIG_UART_LOG_ENABLE
	printf("BLE Heap size:%d B, remain:%d B\n", PAN_BLE_CTLR_BUFFER_SIZE, PAN_BLE_CTLR_BUFFER_SIZE - mem_pos);
#endif
	return mem_ret;
}


/**@brief BLE Configuration */
pan_ble_cfg ble_cfg = {
	.sleep_clock_source   = CONFIG_LOW_SPEED_CLOCK_SRC,       
	.sleep_clock_accuracy = CONFIG_BT_CTLR_SCA,
	.max_num_of_states    = 0,                                 /*unused param*/
	.tx_power             = CONFIG_BT_CTLR_TX_POWER_DFT,

	.pf_mem_init          = mem_alloc,
	.link_layer_debug     = (CONFIG_BT_CTLR_LINK_LAYER_DEBUG ? true:false),
	.agc_cfg_mode         = 0,

	/* BLE Controller Configuration Parameter. */
   #if CONFIG_PM
	.pmEnable    = true,
   #endif

	.mstMargin   = CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN,
	.wlNum       = CONFIG_BLE_CONTROLLER_WIHTELIST_NUM,
	.rlNum       = CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM,
	
	.maxMst      = CONFIG_BT_MAX_NUM_OF_CENTRAL,
	.maxSlv      = CONFIG_BT_MAX_NUM_OF_PERIPHERAL,
	
	.maxAclLen   = MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE,
	.numRxBufs   = CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM,
	.numTxBufs   = CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM,
	.numMoreData = CONFIG_BLE_CONTROLLER_MORE_DATA_NUM,
	.llEncTime   = CONFIG_BLE_CONTROLLER_LL_ENC_TIME,
};


CONFIG_RAM_CODE int ll_semphr_cback(void)
{
	NVIC_EnableIRQ(BLE_EVENT_PROC_IRQn);
	NVIC_SetPendingIRQ(BLE_EVENT_PROC_IRQn);

	return 0;
}

void ble_hs_thread_entry(void *parameter)
{
	nimble_port_run();
}

void hs_thread_init(void)
{
	xTaskCreate(ble_hs_thread_entry, "host", MYNEWT_VAL(BLE_HOST_THREAD_STACK_SIZE),
		    NULL, configMAX_PRIORITIES - os_priority_low, &host_task_h);
}

void vApplicationUserHook(void)
{
	if(ll_enable){
		pan_update_stimer();
	}
}

int host_copydata(void *from, void *dst, uint16_t len)
{
	struct os_mbuf *om = from;

	return os_mbuf_copydata(om, 0, OS_MBUF_PKTLEN(om), dst);
}

void ble_hci_evt_ll_to_host_cbk(uint8_t *p_evt, uint16_t evt_len)
{
	void *buf;

	if (p_evt[2] == BLE_HCI_LE_SUBEV_ADV_RPT) {
		buf = ble_transport_alloc_evt(1);
	} else {
		buf = ble_transport_alloc_evt(0);
	}

	if (buf) {
		memcpy(buf, p_evt, evt_len);
		ble_transport_to_hs_evt(buf);
	}
}

void ble_hci_acl_ll_to_host_cbk(uint8_t *p_acl, uint16_t acl_len)
{
	int ret;
	struct os_mbuf *om;

	om = ble_transport_alloc_acl_from_ll();

	if (om) {
		ret = os_mbuf_append(om, p_acl, acl_len);
		if (ret) {
			os_mbuf_free_chain(om);
		} else {
			ble_transport_to_hs_acl(om);
		}
	}
	else{
		APP_TRACK_ERR("Host transport alloc ACL buffer failed\n");
	}
}

void ll_init(void)
{
	//printf("LL Spark Controller Version:%07x\n", pan_ble_get_version()->commit_id);

	pan_ll_register_hostcopy_cb(host_copydata);
	pan_ll_register_semphr_cback(ll_semphr_cback);
	pan_ble_hci_init(ble_hci_evt_ll_to_host_cbk, ble_hci_acl_ll_to_host_cbk);
	pan_ble_init(&ble_cfg);
	
	ll_enable = true;
}

int ble_transport_to_ll_acl_impl(struct os_mbuf *om)
{
	int rc = 0;

	rc = pan_ble_hci_acl_nimble_handle((void *)om, OS_MBUF_PKTLEN(om));

	os_mbuf_free_chain(om);

	return (rc != 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

int ble_transport_to_ll_cmd_impl(void *buf)
{
	int rc;
	struct ble_hci_cmd *cmd = buf;

	pan_ble_hci_cmd_handle(buf, sizeof(struct ble_hci_cmd) + cmd->length, 0, 0);

	ble_transport_free(cmd);

	return (rc < 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

void TRIM_IRQHandler(void)
{
    printf("trim\n");
}

CONFIG_RAM_CODE void BLE_EVENT_PROC_IRQ(void)
{
	pan_ble_handle();

#if (configUSE_TICKLESS_IDLE == 1)
	UpdateTickAndSch();
#endif
}

CONFIG_RAM_CODE void LL_IRQHandler(void)
{
#ifdef PRF_BLE_DUAL_MODE
	if (panchip_prf_ble_handler()) {
		return;
	}
#endif
	pan_ble_irq();
}

uint32_t db_set_bd_address(uint8_t *bd_addr)
{
	pan_misc_set_bd_addr(bd_addr);
	return 0;
}
