#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_icon=F:\Program Files\Analog Devices\VisualDSP 5.0\System\SystemBuilder\ProjectTypes\VDK\vdk.ico
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
#include-once

#include "macros.au3"
#include "LdrViewer.au3"
#include "avrLoader.au3"

;----------------------------------------------------------------	
;	HotKeySet("{ESC}", "MyExit")

	AutoItSetOption ( "SendKeyDelay", 1) 
	AutoItSetOption ( "SendKeyDownDelay", 1 ) 
	
	MsgBox(0, "�������", "��������� ������������� ���" & @CRLF & "�������� ����� ���������� *.ldr" & @CRLF & "�� ������� Flash ������ ����� IPTDMv2."  & @CRLF &  "��� �������� �������� ����������:" & @CRLF & "��������� ����� � �� �� UART"  & @CRLF & "������ ������� �� �����"  & @CRLF & "���������� ��������� J3"  & @CRLF & "��������� ���������� ���������")	

	LoadIni()	
	GetFlashProjFile()
	LoadBooter()	
	LoadFlash()		
	MyExit()	

;----------------------------------------------------------------	

Func MyExit()

	SaveIni()		
	
	While ProcessExists($ldrUartProcess)
		ProcessClose($ldrUartProcess)
	WEnd
	While ProcessExists($FlashUartProcess)
		ProcessClose($FlashUartProcess)
	WEnd
	
	
    Exit 
EndFunc 

