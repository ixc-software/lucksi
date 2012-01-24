set PYTHONPATH=%~dp0\PyLib;%~dp0\Python\PyQt4

rem %~dp0\Python\python.exe 

ECHO %~dp0

"%~dp0\Python\python.exe"  "%~dp0\PyIpsiusQConfig\src\Main\main.py" /e "%~dp0bin\ProjIpsius.exe" /c "%~dp0PyIpsiusQConfig\UserCfg"

pause

