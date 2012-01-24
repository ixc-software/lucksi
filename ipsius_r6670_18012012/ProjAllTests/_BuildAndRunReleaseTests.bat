call ..\_build_setup.bat

devenv .\ProjAllTests.sln /build Release /Project ProjAllTests

if ERRORLEVEL 1 goto exit

:run
.\Release\ProjAllTests.exe all

:exit
pause
