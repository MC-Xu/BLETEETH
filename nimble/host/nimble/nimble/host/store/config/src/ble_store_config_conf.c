/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "nimble_syscfg.h"

#if MYNEWT_VAL(BLE_STORE_CONFIG_PERSIST)

#include <inttypes.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "host/ble_hs.h"
#include "mtb_kvstore.h"
#include "base64/base64.h"
#include "store/config/ble_store_config.h"
#include "ble_store_config_priv.h"

#include "mtb_kvstore.h"

#define STORE_CONFIG_DBG    1

//static int
//ble_store_config_conf_set(int argc, char **argv, char *val);
//static int
//ble_store_config_conf_export(void (*func)(char *name, char *val),
//                             enum conf_export_tgt tgt);

//static struct conf_handler ble_store_config_conf_handler = {
//    .ch_name = "ble_hs",
//    .ch_get = NULL,
//    .ch_set = ble_store_config_conf_set,
//    .ch_commit = NULL,
//    .ch_export = ble_store_config_conf_export
//};

#define BLE_STORE_CONFIG_SEC_ENCODE_SZ      \
    BASE64_ENCODE_SIZE(sizeof (struct ble_store_value_sec))

//#define BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ  \
//    (MYNEWT_VAL(BLE_STORE_MAX_BONDS) * BLE_STORE_CONFIG_SEC_ENCODE_SZ + 1)
	
#define BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ  \
    (1 * BLE_STORE_CONFIG_SEC_ENCODE_SZ + 1)

#define BLE_STORE_CONFIG_CCCD_ENCODE_SZ     \
    BASE64_ENCODE_SIZE(sizeof (struct ble_store_value_cccd))

//#define BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ \
//    (MYNEWT_VAL(BLE_STORE_MAX_CCCDS) * BLE_STORE_CONFIG_CCCD_ENCODE_SZ + 1)

#define BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ \
    (1 * BLE_STORE_CONFIG_CCCD_ENCODE_SZ + 1)

static void
ble_store_config_serialize_arr(const void *arr, int obj_sz, int num_objs,
                               char *out_buf, int buf_sz)
{
    int arr_size;

    arr_size = obj_sz * num_objs;
    assert(arr_size <= buf_sz);

    base64_encode(arr, arr_size, out_buf, 1);
}

static int
ble_store_config_deserialize_arr(const char *enc,
                                 void *out_arr,
                                 int obj_sz,
                                 int *out_num_objs)
{
    int len;

    len = base64_decode(enc, out_arr);
    if (len < 0) {
        return OS_EINVAL;
    }

    *out_num_objs = len / obj_sz;
    return 0;
}

//static int
//ble_store_config_conf_set(int argc, char **argv, char *val)
//{
//    int rc;

//    if (argc == 1) {
//        if (strcmp(argv[0], "our_sec") == 0) {
//            rc = ble_store_config_deserialize_arr(
//                    val,
//                    ble_store_config_our_secs,
//                    sizeof *ble_store_config_our_secs,
//                    &ble_store_config_num_our_secs);
//            return rc;
//        } else if (strcmp(argv[0], "peer_sec") == 0) {
//            rc = ble_store_config_deserialize_arr(
//                    val,
//                    ble_store_config_peer_secs,
//                    sizeof *ble_store_config_peer_secs,
//                    &ble_store_config_num_peer_secs);
//            return rc;
//        } else if (strcmp(argv[0], "cccd") == 0) {
//            rc = ble_store_config_deserialize_arr(
//                    val,
//                    ble_store_config_cccds,
//                    sizeof *ble_store_config_cccds,
//                    &ble_store_config_num_cccds);
//            return rc;
//        }
//    }
//    return OS_ENOENT;
//}

int
ble_store_config_conf_load(void)
{
    int rc,i,out_num;
    uint32_t len;
    union {
        char sec[BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ];
        char cccd[BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ];
    } buf;

	len = 4;
	rc = app_kv_read("ble_hs/num_our", (uint8_t*)&ble_store_config_num_our_secs, &len);
	rc = app_kv_read("ble_hs/num_peer", (uint8_t*)&ble_store_config_num_peer_secs, &len);
	rc = app_kv_read("ble_hs/num_cccd", (uint8_t*)&ble_store_config_num_cccds, &len);
	out_num = 0;
	for(i = 0; i < ble_store_config_num_our_secs; i++) {
		len = BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ;
		
		char str[20] = {0};
		sprintf(str, "ble_hs/our_sec%d", i);
		rc = app_kv_read(str, (uint8_t*)buf.sec, &len);
	  
		rc = ble_store_config_deserialize_arr(
						buf.sec,
						ble_store_config_our_secs + i,
						sizeof (struct ble_store_value_sec),
						&out_num);
	}
    
    #if STORE_CONFIG_DBG
        printf("ble_store_config_num_our_secs:%d,%d\n",ble_store_config_num_our_secs, out_num);
    #endif
                
	for(i = 0; i < ble_store_config_num_peer_secs; i++) {
		len = BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ;
		
		char str[20] = {0};
		sprintf(str, "ble_hs/peer_sec%d", i);
		rc = app_kv_read(str, (uint8_t*)buf.sec, &len);
	  
		rc = ble_store_config_deserialize_arr(
						buf.sec,
						ble_store_config_peer_secs + i,
						sizeof (struct ble_store_value_sec),
						&out_num);
	}
 
    #if STORE_CONFIG_DBG
        printf("ble_store_config_num_peer_secs:%d, %d\n",ble_store_config_num_peer_secs, out_num);
    #endif

	for(i = 0; i < ble_store_config_num_cccds; i++) {
		len = BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ;
		
		char str[20] = {0};
		sprintf(str, "ble_hs/cccd%d", i);
		rc = app_kv_read(str, (uint8_t*)buf.cccd, &len);
	  
		rc = ble_store_config_deserialize_arr(
						buf.cccd,
						ble_store_config_cccds + i,
						sizeof (struct ble_store_value_cccd),
						&out_num);
	}
   
    #if STORE_CONFIG_DBG
        printf("ble_store_config_num_cccds:%d, %d\n",ble_store_config_num_cccds, out_num);
    #endif
                    
    return rc;
}

int
ble_store_config_conf_save_all(void)
{
    int rc,i;
    uint32_t len;
    union {
        char sec[BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ];
        char cccd[BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ];
    } buf;

	printf("save sec size %d\n",BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ);
	
	for(i = 0; i < ble_store_config_num_our_secs; i++) {
		ble_store_config_serialize_arr(ble_store_config_our_secs + i,
							   sizeof (struct ble_store_value_sec),
							   1,
							   buf.sec,
							   sizeof buf.sec);
		len = strlen(buf.sec);
							   
		char str[20] = {0};
		sprintf(str, "ble_hs/our_sec%d", i);
		rc = app_kv_write(str, (const uint8_t*)buf.sec, len);
	}
                
	for(i = 0; i < ble_store_config_num_peer_secs; i++) {
		ble_store_config_serialize_arr(ble_store_config_peer_secs + i,
									   sizeof (struct ble_store_value_sec),
									   1,
									   buf.sec,
									   sizeof buf.sec);
		len = strlen(buf.sec);
									   
		char str[20] = {0};
		sprintf(str, "ble_hs/peer_sec%d", i);
		rc = app_kv_write(str, (const uint8_t*)buf.sec, len);
	}
                
	for(i = 0; i < ble_store_config_num_cccds; i++) {
		ble_store_config_serialize_arr(ble_store_config_cccds + i,
									   sizeof (struct ble_store_value_cccd),
									   1,
									   buf.cccd,
									   sizeof buf.cccd);
		len = strlen(buf.cccd);
									   
		char str[20] = {0};
		sprintf(str, "ble_hs/cccd%d", i);
		rc = app_kv_write(str, (const uint8_t*)buf.cccd, len);
	}
	
	rc = app_kv_write("ble_hs/num_our", (const uint8_t*)&ble_store_config_num_our_secs, 4);
	rc = app_kv_write("ble_hs/num_peer", (const uint8_t*)&ble_store_config_num_peer_secs, 4);
	rc = app_kv_write("ble_hs/num_cccd", (const uint8_t*)&ble_store_config_num_cccds, 4);
                                   
    return rc;
}

static int
ble_store_config_persist_sec_set(const char *setting_name,
                                 const struct ble_store_value_sec *secs,
                                 int num_secs)
{
    char buf[BLE_STORE_CONFIG_SEC_SET_ENCODE_SZ];
    int rc;
    uint32_t len;
    
    ble_store_config_serialize_arr(secs, sizeof *secs, num_secs,
                                   buf, sizeof buf);    
    len = strlen(buf);
 
    rc = app_kv_write(setting_name, (const uint8_t*)buf, len);
    
    if (rc != 0) {
        return BLE_HS_ESTORE_FAIL;
    }

    return 0;
}

int
ble_store_config_persist_our_secs(void)
{
    int rc,i;

	for(i = 0; i < ble_store_config_num_our_secs; i++) {
		char str[20] = {0};

		sprintf(str, "ble_hs/our_sec%d", i);
		rc = ble_store_config_persist_sec_set(str,
											  ble_store_config_our_secs + i,
											  1);
		if (rc != 0) {
			return rc;
		}
	}
	
	printf("ble_save_our_secs:%d\n",ble_store_config_num_our_secs);
	rc = app_kv_write("ble_hs/num_our", (const uint8_t*)&ble_store_config_num_our_secs, 4);

    return 0;
}

int
ble_store_config_persist_peer_secs(void)
{
    int rc,i;

	for(i = 0; i < ble_store_config_num_peer_secs; i++) {
		char str[20] = {0};

		sprintf(str, "ble_hs/peer_sec%d", i);
		rc = ble_store_config_persist_sec_set(str,
											  ble_store_config_peer_secs + i,
											  1);
		if (rc != 0) {
			return rc;
		}
	}
	rc = app_kv_write("ble_hs/num_peer", (const uint8_t*)&ble_store_config_num_peer_secs, 4);

    return 0;
}

int
ble_store_config_persist_cccds(void)
{
    char buf[BLE_STORE_CONFIG_CCCD_SET_ENCODE_SZ];
    int rc,i;
    uint32_t len;

	for(i = 0; i < ble_store_config_num_cccds; i++) {
		ble_store_config_serialize_arr(ble_store_config_cccds + i,
									   sizeof (struct ble_store_value_cccd),
									   1,
									   buf,
									   sizeof buf);                                  
		len = strlen(buf);

		char str[20] = {0};
		sprintf(str, "ble_hs/cccd%d", i);
		rc = app_kv_write(str, (const uint8_t*)buf, len);

		if (rc != 0) {
			return BLE_HS_ESTORE_FAIL;
		}
	}
	rc = app_kv_write("ble_hs/num_cccd", (const uint8_t*)&ble_store_config_num_cccds, 4);

    return 0;
}

void ble_store_init_database(void)
{
    ble_store_config_conf_load();
}

void
ble_store_config_conf_init(void)
{
    ble_store_init_database();
}

#endif /* MYNEWT_VAL(BLE_STORE_CONFIG_PERSIST) */
