#ifndef __DS2155_BERT__
#define __DS2155_BERT__

#include "Platform\platformtypes.h"
#include "ds2155_reg.h"

namespace Ds2155
{
    using namespace Platform;    
    
	class IMembersToBoard;
    /*
        BERT unit
        Класс тестирования качества передачи данных через 
        интерфейсы Е1/T2 и TDM, посредством ПСП (2е11-1, Rx inv, Tx inv)
    */
        
	class Bert
	{
	public:
	    
		Bert(IMembersToBoard &board);
		~Bert();		
		void Start(bool direction);		// запуск тестирования
		void Stop();					// остановка тестирования
		bool isTesting() const;			// состояние тестирования
        void SetChannelToTest(byte ch);	// подключить канал тестированию		
        void ClearAllChannels();		// отключить все каналы от тестироавния
        void EnableAIS();				// включить генерацию аварии AIS в поток Е1/T1
        void DisableAIS();              // выключить генерацию аварии AIS в поток Е1/T1  
        bool isAISGen() const;	        // состояние генерации аварии AIS в поток Е1/T1
		void OneErrInsert();			// вставка единичной ошибки в тестируемый поток
		dword UpdateErrBitCount();		// получить количество ошибок в тестируемом потоке
		void ClearErrBitCount();		// очистить счетчик ошибок в тестируемом потоке		
		
                
		enum
		{
			TDM	=	true,				// направления тестирования
			LINE =	false,
		};				    
		
	private:
	
		void SetOptions();				// установка опций тестирования
		void SendInv(bool inv);			// установка инверсии потока передачи
		void RecvInv(bool inv);			// установка инверсии потока приема
		void StartRxBertTest();			// запуск приема потока тестирования
		void StopRxBertTest();			// остановка приема потока тестирования
		void StartTxBertTest();			// запуск передачи потока тестирования
		void StopTxBertTest();			// остановка передачи потока тестирования
		void SetChannelsMask();			// установка маски каналов для потока тестирования
		void ClearChannelsMask();		// очистка маски каналов для потока тестирования
		void ErrCounterUpd();			// Обновить счетчик ошибок
	
	private:		
			
		IMembersToBoard &m_board;
		bool	m_isTesting;			// статус тестирования
		dword   m_lPerChannelData;		// маска каналов для потока тестирования
		bool	m_sendInv;				// инверсия потока передачи		
		bool	m_recvInv;				// инверсия потока приема
		bool	m_direction;			// направление генерации потока
		bool	m_aIS;					// состояние генерации аварии AIS
		byte	m_pcpr;					// настроечный регистр
		dword	m_bertErrors;			// счетсик ошибок в потоке тестирования				
		
	};
};

#endif







