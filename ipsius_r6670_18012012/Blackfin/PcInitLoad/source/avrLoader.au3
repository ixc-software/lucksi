#include-once

;#include "BfFlashLoader.au3"
#include "macros.au3"

;----------------------------------------------------------------	

Func LoadFlash()	
	While ProcessExists($FlashUartProcess)
		ProcessClose($FlashUartProcess)
	WEnd
	
	Run($FlashUartLoader)
	SetFlashLoaderParams()		
	SetBasic()		
;	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton14]")
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
	MsgBox(0, "Отладка", "Очистка памяти займет более 2-ух минут",1)		
	WinActivate($FlashUartTitle)
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton8]")	
	SetExtFlash()
	Sleep(500)	
	WaitEndOfErase()
	MsgBox(0, "Отладка", "Память очищена", 1)
	WinActivate($FlashUartTitle)	
	MsgBox(0, "Отладка", "Запись проекта может занять до 15-ти минут",1)			
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton4]")				
	WaitEndOfLoad()
	MsgBox(0, "Отладка", "Программирование завершено успешно" & @CRLF & "Удалите перемычку J3 и нажмите Reset")	
		
EndFunc 	

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

Func GetFlashProjFile()		
	While ($FlashProjFile = "")	
		$FlashProjFile = FileOpenDialog ( "Файл проекта", $LastProjFileDir, "файлы загрузчиков(*.ldr)" , 1 )				
		if @error or $FlashProjFile == "" Then		
			if MsgBox(5, "Отладка", "ошибка открытия файла") == 2 Then	MyExit()			
			$FlashProjFile = ""
		EndIf			
	WEnd		
	$LastProjFileDir = $FlashProjFile
	$FlashProjFile = StringTrimRight($FlashProjFile, 3) & "bin"
	FileCopy ($LastProjFileDir, $FlashProjFile, 1 )
	
EndFunc 


;----------------------------------------------------------------	

Func WaitEndOfErase()	
	$i = $ChipEraseTimeoutSec / 5
	ControlClick($FlashUartTitle, "", "[CLASSNN:TButton2]")		
	While (StringInStr(ControlGetText($FlashUartTitle, "", "[CLASSNN:TMemo1]"), $EndOfErase) < 1)
		Sleep(5000)
		ControlClick($FlashUartTitle, "", "[CLASSNN:TButton2]")				
		$i = $i - 1
		if ($i == 0) Then
			MsgBox(0, "Отладка", "Превышен таймаут стирания флэш")	
			MyExit()			
		EndIf
	WEnd	
EndFunc 

;----------------------------------------------------------------	

Func SetFlashLoaderParams()
	WinWaitCheck($FlashUartTitle, "окно AVR loader не найдено")	
	SetBasic();
	
	SetComboBoxTo($FlashUartTitle, "[CLASSNN:TComboBox4]", $ComPort, $ComPort & " не существует")		
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
;	ControlSend("Открыть", "", "[CLASSNN:Edit1]", $FlashProjFile & "{ENTER}")
	ControlSetText("", "Имя файла:", "[ID:1148]", $FlashProjFile)
	ControlSend("", "Имя файла:", "[CLASSNN:Edit1]", "{ENTER}")	
		
EndFunc 	

;----------------------------------------------------------------	

Func SetBasic()	
	ControlSend($FlashUartTitle, "", "[CLASSNN:TPageControl1]", "{LEFT}")			
EndFunc 

;----------------------------------------------------------------	

Func SetExtFlash()	
	ControlSend($FlashUartTitle, "", "[CLASSNN:TPageControl1]", "{RIGHT}")			
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