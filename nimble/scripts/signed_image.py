from struct import *
import os
import sys
import numpy as np

image_size_offset = 12
image_check_sum_offset = 32
image_tail = 0x0769280010002000
usage = '''
Usage:

CMD Syntax:
   python %s <input_bin_image_file> <signed_bin_image_file>
''' % (os.path.basename(__file__))

# Check input parameters
if len(sys.argv) != 3:
    print(usage)
    sys.exit(1)

# Input image file with binary format
input_bin_image_file = sys.argv[1]
# Signed image file with binary format
signed_bin_image_file = sys.argv[2]

if __name__ == '__main__':
    try:
        file_size = os.path.getsize(input_bin_image_file)
        check_sum = np.uint32(0)
        file = open(input_bin_image_file, "rb+")
        file.seek(0, 0) # position for 0
        header = file.read(512)
        file_size = file_size - 512
        image = file.read(file_size)
        for value in image:
            check_sum = np.add(check_sum, np.uint32(value))
        file.close()

        # create a signed bin
        file = open(signed_bin_image_file, "wb+")
        file.write(header[0:image_size_offset])
        file.seek(image_size_offset, 0) # position for image size in image header
        file.write(pack("<L", file_size))
        file.write(header[image_size_offset+4:image_check_sum_offset-4])
        file.seek(image_check_sum_offset, 0) # position for checksum in image header
        file.write(pack("<L", check_sum))
        file.write(header[image_check_sum_offset+4:])
        file.write(image)
        file.write(pack(">Q", image_tail))
        file.write(pack("<L", check_sum))
        for i in range(28):
             file.write(pack("<B", i))
        file.close()

        print("Image signed done, bin size: %d, checksum: 0x%x" % (file_size, check_sum))

    except FileNotFoundError:
        print("ndk_app.bin does not existed")
