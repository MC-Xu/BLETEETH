#!/usr/bin/env python3
#
# Copyright (c) 2024 Shanghai Panchip Microelectronics Co.,Ltd.
#
# SPDX-License-Identifier: Apache-2.0
#

"""
PAN10XX JLink Script Generating Tool

This script is used for generating JLink Script File to unlock SWD authority
for SoC that enabled SWD (Debug) Protection Feature.

@change-history:
  v1.0.0: * (Sep/24/2024) Initial version
"""

import sys
import os
import argparse
import yaml

jlink_script_template = """
/*
 * JLink Script (Hook Functions) for PAN107X SoC downloading/debugging
 */

/*
 * PAN107X RCC Registers Map:
 *
 * #define CLK_BASE					(0x40040000UL)
 *
 * #define CLK_RSTSTS				(0x40040000UL)
 * #define CLK_IPRST0				(0x40040004UL)
 * #define CLK_IPRST1				(0x40040008UL)
 * #define CLK_BODCTL_3V			(0x4004000CUL)
 * #define CLK_BLDBCTL_3V			(0x40040010UL)
 * #define CLK_CLK_TOP_CTRL_3V		(0x40040014UL)
 * #define CLK_RCL_CTRL_3V			(0x40040018UL)
 * #define CLK_RCH_CTRL				(0x4004001CUL)
 * #define CLK_XTL_CTRL_3V			(0x40040020UL)
 * #define CLK_XTH_CTRL				(0x40040024UL)
 * #define CLK_DPLL_CTRL			(0x40040028UL)
 * #define CLK_AHB_CLK_CTRL			(0x4004002CUL)
 * #define CLK_APB1_CLK_CTRL		(0x40040030UL)
 * #define CLK_APB2_CLK_CTRL		(0x40040034UL)
 * #define CLK_MEAS_CLK_CTRL		(0x40040038UL)
 */

/*
 * PAN107X ANA Registers Map:
 *
 * #define ANA_BASE					(0x40070000UL)
 *
 * #define ANA_LP_REG_SYNC			(0x40070000UL)
 * #define ANA_LP_FL_CTRL_3V		(0x40070004UL)
 * #define ANA_CPU_ADDR_REMAP_CTRL	(0x4007005CUL)
 */

/*********************************************************************
*
*       Constants, fixed
*
**********************************************************************
*/

//
// Registers (PAN107X)
//
__constant U32 ANA_CPU_ADDR_REMAP_CTRL_REG_ADDR     = 0x4007005C; // SoC CPU Remap Ctrl Register
__constant U32 ANA_LP_FL_CTRL_3V_REG_ADDR           = 0x40070004; // SoC Low Power Ctrl Register

//
// Registers (ARM)
//
__constant U32 _AIRCR_ADDR         = 0xE000ED0C; // Application Interrupt and Reset Control Register
__constant U32 _DHCSR_ADDR         = 0xE000EDF0; // Debug Halting Control and Status Register
__constant U32 _DEMCR_ADDR         = 0xE000EDFC; // Debug Exception and Monitor Control Register

//
// Bits & Shifts (ARM)
//
__constant U32 _DP_CTRL_STAT_BIT_STICKYERR = (1 <<  5);

__constant U32 _DHCSR_DBGKEY               = (0xA05F << 16);
__constant U32 _DHCSR_C_DEBUGEN            = (1 <<  0);
__constant U32 _DHCSR_C_HALT               = (1 <<  1);
__constant U32 _DHCSR_S_HALT               = (1 << 17);
__constant U32 _DHCSR_S_RESET_ST           = (1 << 25);

__constant U32 _DEMCR_VC_CORERESET         = (1 <<  0);
__constant U32 _DEMCR_TRCENA               = (1 << 24);

__constant U32 _AIRCR_VECTKEY              = (0x05FA << 16);
__constant U32 _AIRCR_SYSRESETREQ          = (1 <<  2);

/*********************************************************************
*
*       Constants, configurable
*
**********************************************************************
*/

__constant U32 _DEF_POST_RESET_DELAY        = 50;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*
 * Do Panchip customized Cortex-M system reset flow
 * NOTE: __probe attribute specifies that this function is executed in the J-Link firmware
 *       rather than on the PC side, so changes to global variables may only be temporarily
 *       valid inside this function (not guaranteed to be valid later)
 */
__probe int DoSysReset(void)
{
    int r;
    U32 v;
    int t;

    JLINK_SYS_Report("Reset: Halt core after reset via DEMCR.VC_CORERESET.");
    // Halt the CPU
    JLINK_MEM_WriteU32(_DHCSR_ADDR, (_DHCSR_DBGKEY | _DHCSR_C_HALT | _DHCSR_C_DEBUGEN));
    // Set vector catch on reset (to halt the CPU immediately after reset)
    JLINK_MEM_WriteU32(_DEMCR_ADDR, (_DEMCR_VC_CORERESET | _DEMCR_TRCENA));
    // Make sure that no sticky error bits are set on the DP
    JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_ABORT, 0x1E);

    JLINK_SYS_Report("Reset: Clear CPU Remap Ctrl Register.");
    // Clear CPU Remap ctrl reg before reset triggering
    JLINK_MEM_WriteU32(ANA_CPU_ADDR_REMAP_CTRL_REG_ADDR, 0x0);

    // Reset SoC Low Power ctrl reg (Clear BIT6 to indicate SysReset)
    JLINK_MEM_WriteU32(ANA_LP_FL_CTRL_3V_REG_ADDR, 0x00000008);

    // Make sure that no sticky error bits are set on the DP
    JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_ABORT, 0x1E);

    JLINK_SYS_Report("Reset: Reset device via AIRCR.SYSRESETREQ.");
    // Issue reset request via AIRCR.SYSRESETREQ
    JLINK_MEM_WriteU32(_AIRCR_ADDR, (_AIRCR_VECTKEY | _AIRCR_SYSRESETREQ));
    // The spec does not guarantee that the reset takes place immediately, so we give the target some time.
    JLINK_SYS_Sleep(_DEF_POST_RESET_DELAY);

    // Now the SoC is expected to be reset successfully and CPU core is expected to be halted
    // at the 1st instruction to be executed (in Reset_Handler).
    // NOTE:
    //   For PAN107X SoC, sys_reset would cause swd connection lost, and here we re-init DAP module
    //   before exiting this ResetTarget() hook function.

    //
    // Output DAP init sequence
    //
    r  = JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_ABORT, 0x1E);                       // Make sure that no sticky error flags are set
    r |= JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_CTRL_STAT, (1 << 30) | (1 << 28));  // Set DBGPWRUPREQ and SYSPWRUPREQ to power up the complete DAP. Usually, no need to wait for the DAP to power up.
    r |= JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_SELECT, (0 << 24) | (0 << 4));      // Select AP[0] (AHB-AP), bank 0
    if (r < 0) {
      JLINK_CORESIGHT_WriteDP(JLINK_CORESIGHT_DP_REG_ABORT, 0x1E);                          // Make sure that no sticky error flags are set
      JLINK_SYS_Report("Reset: SWD sticky error occurred.");
      return 1;
    }

    //
    // Here we should clear the vector catch we have set before
    //
    JLINK_MEM_WriteU32(_DEMCR_ADDR, (0x0 | _DEMCR_TRCENA));

    return 0;
}

/*********************************************************************
*
*  ResetTarget()
*
*  Function description
*    Replaces reset strategies of DLL. No matter what reset type is selected in the DLL, if this function is present,
*    it will be called instead of the DLL internal reset.
*
*  Notes
*    (1) DLL expects target CPU to be halted / in debug mode, when leaving this function
*    (2) May use MEM_ API functions
*/
int ResetTarget(void)
{
    int r;

    // Do Cortex-M sys reset
    if (DoSysReset()) {
        return 1;
    }

    return 0;
}

void ApplyDebugKey(void)
{
    // Write debug key to KEY Registers in eFuse module
    JLINK_MEM_WriteU32(0x40080068, DEBUG_UNLOCK_KEY1); // EF_VERIFY_DEBUG1
    JLINK_MEM_WriteU32(0x4008006C, DEBUG_UNLOCK_KEY2); // EF_VERIFY_DEBUG2
    JLINK_MEM_WriteU32(0x40080070, DEBUG_UNLOCK_KEY3); // EF_VERIFY_DEBUG3
    JLINK_MEM_WriteU32(0x40080074, DEBUG_UNLOCK_KEY4); // EF_VERIFY_DEBUG4
}

int AfterResetTarget(void)
{
    JLINK_SYS_Report("AfterResetTarget()");

    ApplyDebugKey();

    return 0;
}
"""

def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument(
        "-i",
        "--input_yaml_path",
        required=True,
        help="Directory of encrypt_info.yaml")
    parser.add_argument(
        "-o",
        "--output_jlinkscript_path",
        required=True,
        help="Directory of output JLinkSettings.JLinkScript")

    args = parser.parse_args()
    # print(args.input_yaml_path)
    # print(args.output_jlinkscript_path)

    with open(args.input_yaml_path+"/encrypt_info.yaml", encoding='utf-8') as f:
        enc_info_list = yaml.safe_load(f)
        # print(enc_info_list)
        debug_key = enc_info_list['encrypt_info']['debug_key']
        print('debug key:', debug_key)

    debug_key_split_list = [int(debug_key[i:i+2], 16) for i in range(0, len(debug_key), 2)]
    # print('debug key split: ', [hex(num)[2:] for num in debug_key_split_list])

    debug_unlock_key1 = 0x0
    debug_unlock_key2 = 0x0
    debug_unlock_key3 = 0x0
    debug_unlock_key4 = 0x0
    debug_unlock_key1 |= debug_key_split_list[0]
    debug_unlock_key1 |= debug_key_split_list[1] << 24
    debug_unlock_key2 |= (~debug_key_split_list[2] & 0xFF) << 8
    debug_unlock_key3 |= debug_key_split_list[4]
    debug_unlock_key3 |= debug_key_split_list[5] << 8
    debug_unlock_key3 |= (~debug_key_split_list[6] & 0xFF) << 16
    debug_unlock_key4 |= debug_key_split_list[7] << 16
    print(f"Inferred Debug Unlock Key: 0x{debug_unlock_key1:08x} 0x{debug_unlock_key2:08x} 0x{debug_unlock_key3:08x} 0x{debug_unlock_key4:08x}")

    jlink_script_final = jlink_script_template
    jlink_script_final = jlink_script_final.replace("DEBUG_UNLOCK_KEY1", f"0x{debug_unlock_key1:08x}")
    jlink_script_final = jlink_script_final.replace("DEBUG_UNLOCK_KEY2", f"0x{debug_unlock_key2:08x}")
    jlink_script_final = jlink_script_final.replace("DEBUG_UNLOCK_KEY3", f"0x{debug_unlock_key3:08x}")
    jlink_script_final = jlink_script_final.replace("DEBUG_UNLOCK_KEY4", f"0x{debug_unlock_key4:08x}")

    with open(args.output_jlinkscript_path+"/JLinkSettings.JLinkScript", 'w', encoding='utf-8') as file:
        file.write(jlink_script_final)

    print("JLink Script File (JLinkSettings.JLinkScript) generated done.")

if __name__ == '__main__':
    main()
