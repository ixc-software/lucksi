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
        ����� ������������ �������� �������� ������ ����� 
        ���������� �1/T2 � TDM, ����������� ��� (2�11-1, Rx inv, Tx inv)
    */
        
	class Bert
	{
	public:
	    
		Bert(IMembersToBoard &board);
		~Bert();		
		void Start(bool direction);		// ������ ������������
		void Stop();					// ��������� ������������
		bool isTesting() const;			// ��������� ������������
        void SetChannelToTest(byte ch);	// ���������� ����� ������������		
        void ClearAllChannels();		// ��������� ��� ������ �� ������������
        void EnableAIS();				// �������� ��������� ������ AIS � ����� �1/T1
        void DisableAIS();              // ��������� ��������� ������ AIS � ����� �1/T1  
        bool isAISGen() const;	        // ��������� ��������� ������ AIS � ����� �1/T1
		void OneErrInsert();			// ������� ��������� ������ � ����������� �����
		dword UpdateErrBitCount();		// �������� ���������� ������ � ����������� ������
		void ClearErrBitCount();		// �������� ������� ������ � ����������� ������		
		
                
		enum
		{
			TDM	=	true,				// ����������� ������������
			LINE =	false,
		};				    
		
	private:
	
		void SetOptions();				// ��������� ����� ������������
		void SendInv(bool inv);			// ��������� �������� ������ ��������
		void RecvInv(bool inv);			// ��������� �������� ������ ������
		void StartRxBertTest();			// ������ ������ ������ ������������
		void StopRxBertTest();			// ��������� ������ ������ ������������
		void StartTxBertTest();			// ������ �������� ������ ������������
		void StopTxBertTest();			// ��������� �������� ������ ������������
		void SetChannelsMask();			// ��������� ����� ������� ��� ������ ������������
		void ClearChannelsMask();		// ������� ����� ������� ��� ������ ������������
		void ErrCounterUpd();			// �������� ������� ������
	
	private:		
			
		IMembersToBoard &m_board;
		bool	m_isTesting;			// ������ ������������
		dword   m_lPerChannelData;		// ����� ������� ��� ������ ������������
		bool	m_sendInv;				// �������� ������ ��������		
		bool	m_recvInv;				// �������� ������ ������
		bool	m_direction;			// ����������� ��������� ������
		bool	m_aIS;					// ��������� ��������� ������ AIS
		byte	m_pcpr;					// ����������� �������
		dword	m_bertErrors;			// ������� ������ � ������ ������������				
		
	};
};

#endif







