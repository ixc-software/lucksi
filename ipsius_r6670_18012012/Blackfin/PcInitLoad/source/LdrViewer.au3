#include-once

#include "macros.au3"
;#include "BfFlashLoader.au3"

;----------------------------------------------------------------	

Func LoadBooter()	

	While ProcessExists($ldrUartProcess)
		ProcessClose($ldrUartProcess)
	WEnd

	Run($ldrUartLoader)	; ������ LdrViewer
	AnyLdrWindow();
	ControlSend("Untitled - LdrViewer", "", "[ID:59419]", "{ALT}" & "{ENTER}" & "{ENTER}")		
	WinWaitCheck("Open", "���� ������ ����� �� �������")	
;	MsgBox(0, "�������", $UartLoaderProj)		
;	ControlSend("Open", "", "[ID:1152]", $UartLoaderProj & "{ENTER}")			
	ControlFocus("Open", "", "[ID:1152]")	
	ControlSetText("Open", "", "[ID:1152]", $UartLoaderProj)
	ControlSend("Open", "", "[ID:1152]", "{ENTER}")
	WinWaitCheck($UartLoaderName & " - LdrViewer", "���� c ������ ���������� �� �������")		
	ControlSend($UartLoaderName & " - LdrViewer", "", "[ID:59419]", "{ALT}" & "{RIGHT}" & "{ENTER}" & "{ENTER}")		
	WinWaitCheck("UART Boot", "���� UART Boot �� �������")

	ControlSend("UART Boot", "", "[ID:59419]", "{ALT}" & "{RIGHT}" & "{ENTER}" & "{ENTER}")		
	
	ControlClick("UART Boot", "", "[CLASSNN:Edit1]")	
	ControlSend("UART Boot", "", "[CLASSNN:Edit1]", "{DEL}")
	
;	ControlSend("UART Boot", "", "[CLASSNN:Edit1]", $ComPort & "{TAB}")			
	ControlSetText("UART Boot", "", "[CLASSNN:Edit1]", $ComPort)
	ControlFocus("UART Boot", "", "[CLASSNN:Edit2]")	
	ControlSetText("UART Boot", "", "[CLASSNN:Edit2]", $ComSpeed)
;	ControlSend("UART Boot", "", "[CLASSNN:Edit2]", $ComSpeed & "{TAB}")				
		
	
	While (Not CheckUart())	
		if MsgBox(5, "�������", "���� �� �����") == 2 Then	MyExit()			
	WEnd
	
	MsgBox(0, "�������", "������� �� RESET ����������� �����")	
	
	While (Not CheckAutoBaud())	
		if MsgBox(5, "�������", "����� �� ��������" & @CRLF & "��������� ������������� BMODE" & @CRLF & "������� �� RESET") == 2 Then	MyExit()
	WEnd
	
	MsgBox(0, "�������", "���������� ��������" & @CRLF & "��� ����� ������ ��������� �����", 1)	
	WinActivate("UART Boot")	
	ControlClick("UART Boot", "", "[CLASSNN:Button9]")		
	
	While (Not EndOfLoad())	
	WEnd

	WinActivate("UART Boot")	
	ControlClick("UART Boot", "", "[CLASSNN:Button1]")		
	
	While ProcessExists($ldrUartProcess)
		ProcessClose($ldrUartProcess)
	WEnd		
	
EndFunc 

;----------------------------------------------------------------	

Func AnyLdrWindow()		
	$timeOut = 5
	$MainWindow = 0
	$BugWindow = 0
	While($timeOut And (not $BugWindow) And (Not $MainWindow))
		$BugWindow = WinWaitActive("LdrViewer", "", 1)
		$MainWindow = WinWaitActive("Untitled - LdrViewer", "", 1)		
		$timeOut = $timeOut -1;
	WEnd

	if $timeOut == 0 Then 		
		MsgBox(0, "�������", "���� LdrViewer �� �������")	
		MyExit()
	EndIf

	if $BugWindow Then ControlClick("LdrViewer", "", "[ID:2]")		
	
	WinWaitCheck("Untitled - LdrViewer", "�������� ���� LdrViewer �� �������")			
EndFunc 

;----------------------------------------------------------------	

Func EndOfLoad()	
	WinActivate("UART Boot")
	$Answer = ControlGetText("UART Boot", "", "[CLASSNN:Static5]")	
	return StringInStr($Answer, $LoadOk) > 0
EndFunc 

;----------------------------------------------------------------	

Func CheckAutoBaud()	
	WinActivate("UART Boot")
	ControlClick("UART Boot", "", "[CLASSNN:Button10]")	
	Sleep(200)	
	$Answer = ControlGetText("UART Boot", "", "[CLASSNN:Static5]")	
	return StringInStr($Answer, $AutobaudOk) > 0
EndFunc 

;----------------------------------------------------------------	

Func CheckUart()
	
	WinActivate("UART Boot")
	ControlClick("UART Boot", "", "[CLASSNN:Button4]")	
	Sleep(100)
	Return $ComPortOk == ControlGetText("UART Boot", "", "[CLASSNN:Static5]")	
EndFunc 

;----------------------------------------------------------------	
