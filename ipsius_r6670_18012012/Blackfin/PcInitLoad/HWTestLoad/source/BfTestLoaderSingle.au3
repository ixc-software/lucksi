#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_icon=F:\Program Files\Analog Devices\VisualDSP 5.0\System\SystemBuilder\ProjectTypes\VDK\vdk.ico
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
#include-once

	$FlashUartProcess = "avr_loader.exe" 	
	$FlashUartTitle = "AVR loader"	

	$FlashProjFileMain = "HWTestSingle.bin"



	$AvrLoaderConnectTimeout = 2000
	$AvrLoaderConnected = "Set speed"	
	$EndOfErase = "Flash present:"
	$EndOfLoad = "Done"	
	$ChipEraseTimeoutSec = 400	
	$LoadProjTimeoutSec = 15 * 60		
	
	

	$IniFile = @ScriptDir & "\" & "BfTestLoader.ini"
	$ComPort = IniRead( $IniFile, "ComPort", "ComPort", "1")		
	$ComSpeed = IniRead( $IniFile, "ComPort", "Speed", "115200")			


;----------------------------------------------------------------	

Func WaitEndOfLoad()	
	$i = $LoadProjTimeoutSec
	While (StringInStr(ControlGetText($FlashUartTitle, "", "[CLASSNN:TMemo1]"), $EndOfLoad) < 1)
		Sleep(1000)
		$i = $i - 1
		if ($i == 0) Then
			MsgBox(0, "Отладка", "Превышен таймаут записи файла")	
			MyExit()			
		EndIf
	WEnd	
EndFunc 


;----------------------------------------------------------------	

Func WaitEndOfErase()	
	$i = $ChipEraseTimeoutSec / 5
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton2]")		
	While (StringInStr(ControlGetText($FlashUartTitle, "", "[CLASSNN:TMemo1]"), $EndOfErase) < 1)
		Sleep(500)
		ControlClick($FlashUartTitle, "", "[CLASSNN:TButton2]")				
		$i = $i - 1
		if ($i == 0) Then
			MsgBox(0, "Отладка", "Превышен таймаут стирания флэш")	
			MyExit()			
		EndIf
	WEnd	
EndFunc 

;----------------------------------------------------------------	

Func SetComboBoxTo($Title, $class, $value, $ErrorMessage)	
	ControlCommand ( $Title, "", $class, "SelectString", $value) 
	if @error Then 
		MsgBox(0, "Отладка", $ErrorMessage)	
		MyExit()
	EndIf
EndFunc 

;----------------------------------------------------------------	

Func WinWaitCheck($Title, $ErrorMessage)
	
	if Not WinWaitActive($Title, "", 5) Then 		
		MsgBox(0, "Отладка", $ErrorMessage)	
		MyExit()
	EndIf
EndFunc 

;----------------------------------------------------------------	

Func SetFlashLoaderParams()
	WinWaitCheck($FlashUartTitle, "окно AVR loader не найдено")	
	SetBasic();
	
	SetComboBoxTo($FlashUartTitle, "[CLASSNN:TComboBox4]", "COM" & $ComPort, $ComPort & " не существует")		
	SetComboBoxTo($FlashUartTitle, "[CLASSNN:TComboBox3]", $ComSpeed, $ComSpeed & " не существует")
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox4]", "Check", "" )
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox1]", "UnCheck", "" )
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox2]", "UnCheck", "" )
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox3]", "UnCheck", "" )
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox5]", "UnCheck", "" )
	SetExtFlash()
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox1]", "UnCheck", "" )
	ControlCommand ($FlashUartTitle, "", "[CLASSNN:TCheckBox2]", "UnCheck", "" )
;	ControlSend($FlashUartTitle, "", "[CLASSNN:TEdit3]", "0")				
	ControlSetText($FlashUartTitle, "", "[CLASSNN:TEdit3]", "0")
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton5]")					
	WinWaitCheck("Открыть", "окно открытия файла не найдено")
;	ControlSend("Открыть", "", "[CLASSNN:Edit1]", @ScriptDir & "\" & "UsedExe" & "\" & $FlashProjFileMain & "{ENTER}")
	ControlSetText("Открыть", "", "[CLASSNN:Edit1]", @ScriptDir & "\" & "UsedExe" & "\" & $FlashProjFileMain)
	ControlSend("Открыть", "", "[CLASSNN:Edit1]", "{ENTER}")
		
EndFunc 	

;----------------------------------------------------------------	

Func SetBasic()	
	ControlSend($FlashUartTitle, "", "[CLASSNN:TPageControl1]", "{LEFT}")			
EndFunc 

;----------------------------------------------------------------	

Func SetExtFlash()	
	ControlSend($FlashUartTitle, "", "[CLASSNN:TPageControl1]", "{RIGHT}")			
EndFunc 


;#include "macros.au3"
;#include "avrLoader.au3"

;----------------------------------------------------------------	

	HotKeySet("{ESC}", "MyExit")

	AutoItSetOption ( "SendKeyDelay", 1) 
	AutoItSetOption ( "SendKeyDownDelay", 1 ) 	
	
;----------------------------------------------------------------	


;	MsgBox(0, "Отладка", "Нажмите на RESET прошиваемой платы")
	$runstring = @ScriptDir & "\" & "UsedExe" & "\" & "ProjBfFlashWriter.exe " & $ComPort & " " & $ComSpeed & " " & "bf537loader0sector.ldr"
;	MsgBox(0, "Отладка", $runstring)	
	Run($runstring);
	Sleep(2000);
	Run(@ScriptDir & "\" & "UsedExe" & "\" & "avr_loader.exe");
	SetFlashLoaderParams()		
	SetBasic()		
	ControlSend($FlashUartTitle, "", "[CLASSNN:TPageControl1]", "{F5}")			
	
	$i = $AvrLoaderConnectTimeout / 100
	While (StringInStr(ControlGetText($FlashUartTitle, "", "[CLASSNN:TMemo1]"), $AvrLoaderConnected) < 1)
		Sleep(100)
		$i = $i - 1
		if ($i == 0) Then
			MsgBox(0, "Отладка", "Плата не отвечает"  & @CRLF & "Проверьте файл загрузчика")	
			MyExit()			
		EndIf
	WEnd
;	MsgBox(0, "Отладка", "Очистка памяти займет более 2-ух минут",1)			
	WinActivate($FlashUartTitle)
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton8]")	
	SetExtFlash()
	Sleep(500)	
	WaitEndOfErase()
;	MsgBox(0, "Отладка", "Память очищена", 1)
	WinActivate($FlashUartTitle)	
;	MsgBox(0, "Отладка", "Запись проекта может занять до 15-ти минут",1)			
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton4]")				
	WaitEndOfLoad()
	
;	MsgBox(0, "Отладка", "Программирование завершено успешно" & @CRLF & "Удалите перемычку J3 и нажмите Reset")	
	

;	LoadFlash()		
	MyExit()	

;----------------------------------------------------------------	

Func MyExit()

	IniWrite( $IniFile, "ComPort", "ComPort", $ComPort)		
	IniWrite( $IniFile, "ComPort", "Speed", $ComSpeed)					


	While ProcessExists($FlashUartProcess)
		ProcessClose($FlashUartProcess)
	WEnd
	
	
    Exit 
EndFunc 

