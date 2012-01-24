call "%VS80COMNTOOLS%vsvars32.bat"

devenv .\ULVWin32.sln /build Release 

if ERRORLEVEL 1 goto exit

:exit
pause
