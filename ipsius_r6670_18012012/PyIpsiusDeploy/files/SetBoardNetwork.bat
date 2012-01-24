rem Setup network settings on board
rem %1 -> board address
rem %2 -> UseDHCP
rem %3 -> IP
rem %4 -> Gateway
rem %5 -> NetMask
rem %6 -> Password

.\bin\ProjIpsius.exe -n Ipsius -la 56001 -v -i ../isc -sc SetNetwork.isc -x %1 %2 %3 %4 %5 %6

pause
