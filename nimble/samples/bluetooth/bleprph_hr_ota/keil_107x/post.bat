@echo off

%5 --bin --output %1 %3
%5 --text -a -c --output %2 %3
python ..\..\..\..\scripts\signed_image.py .\Images\ndk_app.bin .\Images\ndk_app.signed.bin
echo %1
srec_cat %4.bin  -binary -offset 0xA000 -o %4.hex -intel

echo "Proccess Over!"