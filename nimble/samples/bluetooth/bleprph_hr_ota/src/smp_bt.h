#ifndef SMP_BT_H
#define SMP_BT_H

#define BT_UUID_SMP_SERVICE_VAL_ARG              0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d
#define BT_UUID_SMP_CHARAC_VAL_ARG               0x48, 0x7c, 0x99, 0x74, 0x11, 0x26, 0x9e, 0xae, 0x01, 0x4e, 0xce, 0xfb, 0x28, 0x78, 0x2e, 0xda


/* SMP service.
 * {8D53DC1D-1DB7-4CD3-868B-8A527460AA84}
 */
#define BT_UUID_SMP_SERVICE_VAL                  (BLE_UUID128_DECLARE(BT_UUID_SMP_SERVICE_VAL_ARG))


/* SMP characteristic; used for both requests and responses.
 * {DA2E7828-FBCE-4E01-AE9E-261174997C48}
 */
#define BT_UUID_SMP_CHAR_VAL                     (BLE_UUID128_DECLARE(BT_UUID_SMP_CHARAC_VAL_ARG))


void ble_svc_smp_init(void);

#endif
