#!/usr/bin/env python3
#
# Copyright (c) 2020-2022 Shanghai Panchip Microelectronics Co.,Ltd.
#
# SPDX-License-Identifier: Apache-2.0
#

"""
PAN1080 Image Encrypt Tool

This script is used for encrypting the PAN1080 Image by AES ECB algorithm,
which is indispensable for SoC that has enabled the Hardware Secure Flow.

@author: limi@panchip.com
         lihui@panchip.com

@change-history:
  v1.0.0: * (Apr/14/2020) Initial version
  v1.1.0: * (Apr/13/2022) Add several enhancements for easier use
"""

import sys
import os
import binascii
from Crypto.Cipher import AES
from binascii import b2a_hex
from intelhex import IntelHex
import yaml

#Get from input hex
image_start_addr = None
image_size = None

# Get from input yaml
encrypt_flash_offset = None
encrypt_key = None
expected_image_start_addr = None

usage = '''
Usage:

CMD Syntax:
   python %s <input_hex_image_file> <encrypt_info_yaml_file>

Notes:
1. Currently the tool only support HEX format file as input image.
2. The tool currently gets "encrypt_flash_offset", "encrypt_key"
   and "expected_start_addr" from the <encrypt_info_yaml_file> for
   checking and encrypting use.
3. Once encryption done, the tool generate 2 output files in the
   same folder: a hex file and a bin file, and each file has a
   '_enc' name suffix.

e.g.
Provided we have a <input_hex_image_file> named "path/to/input_file.hex",
and a <encrypt_info_yaml_file> named "path2/to/encrypt_info.yaml", and
now we open a cmd prompt and type:

   python %s path/to/input_file.hex path2/to/encrypt_info.yaml

Soon we can find 2 output files in the input image file folder:
  1. "path/to/input_file_enc.hex"
  2. "path/to/input_file_enc.bin"
''' % (os.path.basename(__file__), os.path.basename(__file__))

# Check input parameters
if len(sys.argv) != 3:
    print(usage)
    sys.exit(1)

# Input image file with Intel HEX format
input_hex_image_file = sys.argv[1]
# Encrypt information file with YAML format
encrypt_info_yaml_file = sys.argv[2]

print('Image Encryption start..')
print("Input image file: '{:s}'".format(input_hex_image_file))
print("Encrypt information file: '{:s}'".format(encrypt_info_yaml_file))

#------------------ Hex file processing ------------------
ihi = IntelHex(input_hex_image_file)
segments = ihi.segments()
segment_num = len(segments)

# Currently we don't support to parse hex file with multiple segments
if segment_num != 1:
    print("Error: Input file has multiple segments!")
    sys.exit(1)

print("Hex Image Entry Address Info:", ihi.start_addr)

for s in segments:
    image_start_addr = s[0]
    image_size = s[1]-s[0]
    print("Hex Segment: {{ start: 0x{:08X}, end: 0x{:08X}, length: 0x{:08X} ({:d}) }}"
        .format(s[0], s[1]-1, s[1]-s[0], s[1]-s[0]))

# Check if the image start address is 256-bytes align
if image_start_addr % 256 != 0:
    print('Error: The start address in hex file is not 256-bytes align!')
    sys.exit(1)

#------------------ Yaml file processing ------------------
with open(encrypt_info_yaml_file, encoding='utf-8') as f:
    enc_info_list = yaml.safe_load(f)
    # print(enc_info_list)
    encrypt_flash_offset = enc_info_list['encrypt_info']['encrypt_flash_offset']
    encrypt_key = enc_info_list['encrypt_info']['encrypt_key']
    expected_image_start_addr = enc_info_list['image_info']['expected_start_addr']
    print('encrypt flash offset: 0x{:X} ({:d})'.format(encrypt_flash_offset, encrypt_flash_offset))
    print('encrypt key:', encrypt_key)
    print('expected image start addr: 0x{:X}'.format(expected_image_start_addr))

# Check if the image start address read from the input HEX file is
# same with expected start address read from the input YAML file.
if expected_image_start_addr != image_start_addr:
    print('Error: Start address in input HEX file is not same as expected in encrypt info YAML file!')
    sys.exit(1)

# Check if the encrypt addr is larger than image start address
if (encrypt_flash_offset * 256 < image_start_addr or
        encrypt_flash_offset * 256 + 256 > image_start_addr + image_size):
    print('Error: Encrypt flash offset is out of image range!')
    sys.exit(1)

#----------------------------- Encrypt params ---------------------------
# offset in binary file
offset = encrypt_flash_offset - image_start_addr / 256
offset = int(offset)
# AES key for the following encrypt flow use
key    = encrypt_key

#------------------------- Processing binary data ----------------------
text_aes = ihi.tobinarray()

text_aes_128_1  = b2a_hex(text_aes[(  0+int(offset)*256):( 16+int(offset)*256)])
text_aes_128_2  = b2a_hex(text_aes[( 16+int(offset)*256):( 32+int(offset)*256)])
text_aes_128_3  = b2a_hex(text_aes[( 32+int(offset)*256):( 48+int(offset)*256)])
text_aes_128_4  = b2a_hex(text_aes[( 48+int(offset)*256):( 64+int(offset)*256)])
text_aes_128_5  = b2a_hex(text_aes[( 64+int(offset)*256):( 80+int(offset)*256)])
text_aes_128_6  = b2a_hex(text_aes[( 80+int(offset)*256):( 96+int(offset)*256)])
text_aes_128_7  = b2a_hex(text_aes[( 96+int(offset)*256):(112+int(offset)*256)])
text_aes_128_8  = b2a_hex(text_aes[(112+int(offset)*256):(128+int(offset)*256)])
text_aes_128_9  = b2a_hex(text_aes[(128+int(offset)*256):(144+int(offset)*256)])
text_aes_128_10 = b2a_hex(text_aes[(144+int(offset)*256):(160+int(offset)*256)])
text_aes_128_11 = b2a_hex(text_aes[(160+int(offset)*256):(176+int(offset)*256)])
text_aes_128_12 = b2a_hex(text_aes[(176+int(offset)*256):(192+int(offset)*256)])
text_aes_128_13 = b2a_hex(text_aes[(192+int(offset)*256):(208+int(offset)*256)])
text_aes_128_14 = b2a_hex(text_aes[(208+int(offset)*256):(224+int(offset)*256)])
text_aes_128_15 = b2a_hex(text_aes[(224+int(offset)*256):(240+int(offset)*256)])
text_aes_128_16 = b2a_hex(text_aes[(240+int(offset)*256):(256+int(offset)*256)])

def big_2_small_endian(data):
    return binascii.hexlify(binascii.unhexlify(data)[::-1])

text_aes_128_1 = big_2_small_endian(text_aes_128_1)
text_aes_128_2 = big_2_small_endian(text_aes_128_2)
text_aes_128_3 = big_2_small_endian(text_aes_128_3)
text_aes_128_4 = big_2_small_endian(text_aes_128_4)
text_aes_128_5 = big_2_small_endian(text_aes_128_5)
text_aes_128_6 = big_2_small_endian(text_aes_128_6)
text_aes_128_7 = big_2_small_endian(text_aes_128_7)
text_aes_128_8 = big_2_small_endian(text_aes_128_8)
text_aes_128_9 = big_2_small_endian(text_aes_128_9)
text_aes_128_10 = big_2_small_endian(text_aes_128_10)
text_aes_128_11 = big_2_small_endian(text_aes_128_11)
text_aes_128_12 = big_2_small_endian(text_aes_128_12)
text_aes_128_13 = big_2_small_endian(text_aes_128_13)
text_aes_128_14 = big_2_small_endian(text_aes_128_14)
text_aes_128_15 = big_2_small_endian(text_aes_128_15)
text_aes_128_16 = big_2_small_endian(text_aes_128_16)

def str_2_ascii(string):
    string_hex = string.zfill(32)
#    print('string_hex value is:',string_hex)
    string_ascii = binascii.a2b_hex(string_hex)
#    print('string_ascii value is: ',string_ascii)
    return string_ascii

def encrypt(key_ascii,text_ascii):
#    print('key_ascii value is :',key_ascii)
#    print('text_ascii value is:',text_ascii)
    mode     = AES.MODE_ECB
    aes      = AES.new(key_ascii,mode)
    en_text = aes.encrypt(text_ascii)
    # print('en_text value is:',en_text)
    # encrypt_data = b2a_hex(en_text)
    # print('encrypt value data is:',encrypt_data)
    return en_text

#------------------------- Encryption Flow ----------------------

text = text_aes_128_1
text_ascii      = str_2_ascii(text)
#print('text_aes_1 text_ascii value is ',text_ascii)
#print('text_aes_1 hex value is',b2a_hex(text_ascii))
key_ascii       = str_2_ascii(key)
encrypt_text_1  = encrypt(key_ascii,text_ascii)

text = text_aes_128_2
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_2  = encrypt(key_ascii,text_ascii)

text = text_aes_128_3
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_3  = encrypt(key_ascii,text_ascii)

text = text_aes_128_4
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_4  = encrypt(key_ascii,text_ascii)

text = text_aes_128_5
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_5  = encrypt(key_ascii,text_ascii)

text = text_aes_128_6
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_6  = encrypt(key_ascii,text_ascii)

text = text_aes_128_7
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_7  = encrypt(key_ascii,text_ascii)

text = text_aes_128_8
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_8  = encrypt(key_ascii,text_ascii)

text = text_aes_128_9
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_9  = encrypt(key_ascii,text_ascii)

text = text_aes_128_10
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_10 = encrypt(key_ascii,text_ascii)

text = text_aes_128_11
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_11 = encrypt(key_ascii,text_ascii)

text = text_aes_128_12
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_12 = encrypt(key_ascii,text_ascii)

text = text_aes_128_13
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_13 = encrypt(key_ascii,text_ascii)

text = text_aes_128_14
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_14 = encrypt(key_ascii,text_ascii)

text = text_aes_128_15
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_15 = encrypt(key_ascii,text_ascii)

text = text_aes_128_16
text_ascii      = str_2_ascii(text)
#text_ascii     = text
key_ascii       = str_2_ascii(key)
encrypt_text_16 = encrypt(key_ascii,text_ascii)

#---------------------------- Re-combine data -----------------------------
encrypt_text_1 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_1))))
encrypt_text_2 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_2))))
encrypt_text_3 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_3))))
encrypt_text_4 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_4))))
encrypt_text_5 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_5))))
encrypt_text_6 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_6))))
encrypt_text_7 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_7))))
encrypt_text_8 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_8))))
encrypt_text_9 = str_2_ascii((big_2_small_endian(b2a_hex(encrypt_text_9))))
encrypt_text_10 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_10))))
encrypt_text_11 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_11))))
encrypt_text_12 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_12))))
encrypt_text_13 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_13))))
encrypt_text_14 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_14))))
encrypt_text_15 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_15))))
encrypt_text_16 = str_2_ascii(big_2_small_endian((b2a_hex(encrypt_text_16))))

text_aes_new = text_aes[0:(0+int(offset)*256)].tobytes()    \
                + encrypt_text_1    \
                + encrypt_text_2    \
                + encrypt_text_3    \
                + encrypt_text_4    \
                + encrypt_text_5    \
                + encrypt_text_6    \
                + encrypt_text_7    \
                + encrypt_text_8    \
                + encrypt_text_9    \
                + encrypt_text_10   \
                + encrypt_text_11   \
                + encrypt_text_12   \
                + encrypt_text_13   \
                + encrypt_text_14   \
                + encrypt_text_15   \
                + encrypt_text_16   \
                + text_aes[(256+int(offset)*256):].tobytes()

#---------------------------- Generate output files -----------------------------
iho = IntelHex()
iho.frombytes(text_aes_new, image_start_addr) # keep start address (offset) unchanged

# Output files name
output_bin_file = os.path.splitext(input_hex_image_file)[0] + '_enc.bin'
output_hex_file = os.path.splitext(input_hex_image_file)[0] + '_enc.hex'

# Generate binrary output file
try:
    iho.tobinfile(output_bin_file)
except IOError:
    e = sys.exc_info()[1]     # current exception
    print("ERROR: Could not write to file: %s: %s" % (output_bin_file, str(e)))
    sys.exit(1)

# Generate Intel Hex output file
try:
    # iho.start_addr = ihi.start_addr # keep start address (entry) unchanged
    iho.write_hex_file(output_hex_file)
except IOError:
    e = sys.exc_info()[1]     # current exception
    print("ERROR: Could not write to file: %s: %s" % (output_hex_file, str(e)))
    sys.exit(1)

print("Output bin file: '{:s}'".format(output_bin_file))
print("Output hex file: '{:s}'".format(output_hex_file))

print('Image encryption finish!')
