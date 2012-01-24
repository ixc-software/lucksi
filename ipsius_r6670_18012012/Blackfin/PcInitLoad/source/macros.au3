#include-once

;----------------------------------------------------------------	
	$IniFile = @ScriptDir & "\" & "bf537loader.ini"

	$ComPort = ""
	$ComSpeed = ""

;----------------------------------------------------------------	

	$ldrUartProcess = "LdrViewer.exe" 	
	$ldrUartLoader = @ScriptDir & "\" & "UsedExe" & "\" & $ldrUartProcess 
	$UartLoaderName = "bf537loader.ldr" 			
	$UartLoaderProj = @ScriptDir & "\" & $UartLoaderName 	
	
	$ComPortOk = "Port ready for use. Reset hardware before autobaud."
	$AutobaudOk = "Blackfin replied correctly."	
	$LoadOk = ".. done"			

;----------------------------------------------------------------			

	$FlashUartProcess = "avr_loader.exe" 	
	$FlashUartLoader = @ScriptDir & "\" & "UsedExe" & "\" & $FlashUartProcess 
	$FlashUartTitle = "AVR loader"	
	

	$AvrLoaderConnectTimeout = 2000
	$ChipEraseTimeoutSec = 400	
	$LoadProjTimeoutSec = 30 * 60		
	$AvrLoaderConnected = "Set speed"	
	$EndOfErase = "Flash present:"
	$EndOfLoad = "Done"	
	
;----------------------------------------------------------------			

	$FlashProjFile = ""
	$LastProjFileDir = ""

;----------------------------------------------------------------	

Func LoadIni()
	$ComPort = IniRead( $IniFile, "ComPort", "ComPort", "COM1")		
	$ComSpeed = IniRead( $IniFile, "ComPort", "Speed", "115200")			
	$LastProjFileDir = IniRead( $IniFile, "LastProjDir", "Dir", @ScriptDir)			
EndFunc 	

;----------------------------------------------------------------	

Func SaveIni()
	IniWrite( $IniFile, "ComPort", "ComPort", $ComPort)		
	IniWrite( $IniFile, "ComPort", "Speed", $ComSpeed)					
	IniWrite( $IniFile, "LastProjDir", "Dir", $LastProjFileDir)						
EndFunc 	

;----------------------------------------------------------------	

Func WinWaitCheck($Title, $ErrorMessage)
	
	if Not WinWaitActive($Title, "", 5) Then 		
		MsgBox(0, "Отладка", $ErrorMessage)	
		MyExit()
	EndIf
EndFunc 