@echo off

%5 --bin --output %1 %3
%5 --text -a -c --output %2 %3
start python ..\..\..\..\scripts\signed_image.py
echo "Proccess Over!"