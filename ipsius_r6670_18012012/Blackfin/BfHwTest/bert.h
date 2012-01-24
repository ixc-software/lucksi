#ifndef __DS2155_BERT__
#define __DS2155_BERT__

#include "ds2155_reg.h"

enum
{
	TDM	=	true,				// направления тестирования
	LINE =	false,
};				    


void BertStart(bool direction);		// запуск тестирования
void BertStop();					// остановка тестирования
bool BertisTesting();				// состояние тестирования
void BertSetChannelToTest(byte ch);	// подключить канал тестированию		
void BertClearAllChannels();		// отключить все каналы от тестироавния
void BertEnableAIS();				// включить генерацию аварии AIS в поток Е1
void BertDisableAIS();              // выключить генерацию аварии AIS в поток Е1  
bool BertisAISGen();	        	// состояние генерации аварии AIS в поток Е1
void BertOneErrInsert();			// вставка единичной ошибки в тестируемый поток
dword BertUpdateErrBitCount();		// получить количество ошибок в тестируемом потоке
void BertClearErrBitCount();		// очистить счетчик ошибок в тестируемом потоке				
bool BertLSS();						// получить состояние синхронизации по ПСП
        			
#endif







