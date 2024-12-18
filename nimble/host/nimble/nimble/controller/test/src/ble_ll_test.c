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

#include "sysinit/sysinit.h"
#include "nimble_syscfg.h"
#include "controller/ble_ll_test.h"
#include "os/os.h"
#include "testutil/testutil.h"
#include "ble_ll_csa2_test.h"

#if MYNEWT_VAL(SELFTEST)

int
main(int argc, char **argv)
{
    ble_ll_csa2_test_suite();
    return tu_any_failed;
}

#endif
