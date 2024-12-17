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

#ifndef H_BLEHOG_SENSOR_
#define H_BLEHOG_SENSOR_

#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "host/ble_hs.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BT_UUID_HIDS 0x1812

/** @def BT_UUID_HIDS_BOOT_MOUSE_IN_REPORT_VAL
 *  @brief HID Boot Mouse Input Report Characteristic UUID value
 */
#define BT_UUID_HIDS_BOOT_MOUSE_IN_REPORT 0x2a33

/** @def BT_UUID_HIDS_INFO_VAL
 *  @brief HID Information Characteristic UUID value
 */
#define BT_UUID_HIDS_INFO 0x2a4a

/** @def BT_UUID_HIDS_REPORT_MAP_VAL
 *  @brief HID Report Map Characteristic UUID value
 */
#define BT_UUID_HIDS_REPORT_MAP 0x2a4b

/** @def BT_UUID_HIDS_CTRL_POINT_VAL
 *  @brief HID Control Point Characteristic UUID value
 */
#define BT_UUID_HIDS_CTRL_POINT 0x2a4c

/** @def BT_UUID_HIDS_REPORT_VAL
 *  @brief HID Report Characteristic UUID value
 */
#define BT_UUID_HIDS_REPORT 0x2a4d

#define BT_UUID_HIDS_REPORT_REF 0x2908

/** @def BT_UUID_HIDS_PROTOCOL_MODE_VAL
 *  @brief HID Protocol Mode Characteristic UUID value
 */
#define BT_UUID_HIDS_PROTOCOL_MODE 0x2a4e

/** @def BT_UUID_RSC_MEASUREMENT_VAL
 *  @brief RSC Measurement Characteristic UUID value
 */
#define BT_UUID_RSC_MEASUREMENT 0x2a53

extern uint16_t hid_input_handle;
extern uint16_t hid_consumer_input_handle;

int gatt_svr_hid_init(void);
#ifdef __cplusplus
}
#endif

#endif
