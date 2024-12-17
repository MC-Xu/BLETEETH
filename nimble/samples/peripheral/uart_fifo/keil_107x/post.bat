@echo off

%5 --bin --output %1 %3
%5 --text -a -c --output %2 %3
python ..\..\..\..\scripts\signed_image.py .\Images\ndk_app.bin .\Images\ndk_app.signed.bin
echo "Proccess Over!"