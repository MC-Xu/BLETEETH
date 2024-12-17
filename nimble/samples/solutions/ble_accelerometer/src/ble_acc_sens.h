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

#ifndef H_BLEACC_SENSOR_
#define H_BLEACC_SENSOR_

#include "nimble/ble.h"
#include "modlog/modlog.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Service configuration */


extern uint16_t c2003_handle;/* Custom Service Variables */

// Form A custom Bluetooth profile for the BBC micro:bit
// Refer to:
// - https://lancaster-university.github.io/microbit-docs/ble/accelerometer-service/
// - https://lancaster-university.github.io/microbit-docs/resources/bluetooth/bluetooth_profile.html
#define ACCELEROMETERSERVICE_SERVICE_UUID           0xE9, 0x5D, 0x07, 0x53, 0x25, 0x1D, 0x47, 0x0A, 0xA0, 0x62, 0xFA, 0x19, 0x22, 0xDF, 0xA9, 0xA8
#define ACCELEROMETERDATA_CHARACTERISTIC_UUID       0xE9, 0x5D, 0xCA, 0x4B, 0x25, 0x1D, 0x47, 0x0A, 0xA0, 0x62, 0xFA, 0x19, 0x22, 0xDF, 0xA9, 0xA8
#define ACCELEROMETERPERIOD_CHARACTERISTIC_UUID     0xE9, 0x5D, 0xFB, 0x24, 0x25, 0x1D, 0x47, 0x0A, 0xA0, 0x62, 0xFA, 0x19, 0x22, 0xDF, 0xA9, 0xA8

int gatt_svr_init(void);

#ifdef __cplusplus
}
#endif

#endif // H_BLEACC_SENSOR_
