#!/usr/bin/env python3
#
# Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
#
# SPDX-License-Identifier: Apache-2.0
#
import re
import argparse

key_factor = bytes([0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa])

def decrypt_key_func(key):
    step3_res = []
    step4_res = ""

    local_key = key[16:] + key[0:16]
    key_bytes = bytes.fromhex(local_key)

    for i in range(0, 16):
        step3_res.append(hex(key_factor[i] ^ key_bytes[i]))

    for i in range(0, 16):
        step4_res = step4_res + (step3_res[i][2:]).zfill(2)

    return step4_res

def encrypt_key_func(key):
    step1_res = []
    step2_res = ""
    key_bytes = bytes.fromhex(key)

    for i in range(0, 16):
        step1_res.append((hex(key_factor[i] ^ key_bytes[i])).zfill(2))

    for i in range(8, 16):
        step2_res = step2_res + (step1_res[i][2:]).zfill(2)

    for i in range(0, 8):
        step2_res = step2_res + (step1_res[i][2:]).zfill(2)

    return step2_res


def crc_cal(file_data):
    crc = sum(file_data)
    return hex(crc)


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
        "--output_bin_path",
        required=True,
        help="Directory of output encoded encrypt_info_enc.bin")

    args = parser.parse_args()
    # print(args.input_yaml_path)
    # print(args.output_bin_path)

    # open file
    with open(args.input_yaml_path+"/encrypt_info.yaml", "r") as original_file:
        with open(args.output_bin_path+"/encrypt_info_enc.bin", "wb+") as encrypt_file:
            # encode data and write to new file (binary format)
            for line in original_file:
                res = re.search("encrypt_key", line)
                if res:
                    symbol_index = line.index("'")
                    # Check if not empty string
                    if line[symbol_index+1] != "'":
                        key = line[symbol_index+1:symbol_index+33]
                        line_list = line.split("'", 2)
                        enc_key_encode = encrypt_key_func(key)
                        line = line_list[0] + "'" + enc_key_encode + "'" + line_list[2]

                res = re.search("debug_key", line)
                if res:
                    symbol_index = line.index("'")
                    # Check if not empty string
                    if line[symbol_index+1] != "'":
                        key = line[symbol_index+1:symbol_index+17]
                        line_list = line.split("'", 2)
                        dbg_key_encode = encrypt_key_func(key + key)
                        line = line_list[0] + "'" + dbg_key_encode + "'" + line_list[2]

                write_data =line.encode('utf-8')
                encrypt_file.write(write_data)

            # write crc to enc file
            encrypt_file.flush()
            encrypt_file.seek(0, 0)
            file_data = encrypt_file.read()
            crc = crc_cal(file_data)
            write_data = "check_info:\r\n".encode('utf-8')
            encrypt_file.write(write_data)
            crc_str = "  crc: '" + crc + "'\r\n"
            write_data = crc_str.encode('utf-8')
            encrypt_file.write(write_data)

    print("Encoded encrypt info file (encrypt_info_enc.bin) generated done")

if __name__ == '__main__':
    main()
