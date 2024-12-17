@echo off

rem echo %1
rem echo %2
rem echo %3
rem echo %4
rem echo %5

%5 --bin --output %1\ndk_app.bin %3
%5 --text -a -c --output %1\ndk_app.asm %3

copy %4 %1\ndk_app.hex
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

python ..\..\..\..\scripts\encode_yaml.py -i .\ -o %1
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred in encode_yaml.py, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

python ..\..\..\..\scripts\encrypt_tool.py %1\ndk_app.hex .\encrypt_info.yaml
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred in encrypt_tool.py, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

python ..\..\..\..\scripts\signed_image.py %1\ndk_app_enc.bin %1\ndk_app_enc.signed.bin
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred in signed_image.py, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

echo Post Build Complete!
