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
	
	MsgBox(0, "Отладка", "Программа предназначена для" & @CRLF & "загрузки файла загрузчика *.ldr" & @CRLF & "во внешнюю Flash память платы IPTDMv2."  & @CRLF &  "Для успешной загрузки необходимо:" & @CRLF & "Соединить плату и ПК по UART"  & @CRLF & "Подать питание на плату"  & @CRLF & "Установить перемычку J3"  & @CRLF & "Исполнять инструкции программы")	

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

