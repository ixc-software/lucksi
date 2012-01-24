#ifndef __DS2155_BERT__
#define __DS2155_BERT__

#include "ds2155_reg.h"

enum
{
	TDM	=	true,				// ����������� ������������
	LINE =	false,
};				    


void BertStart(bool direction);		// ������ ������������
void BertStop();					// ��������� ������������
bool BertisTesting();				// ��������� ������������
void BertSetChannelToTest(byte ch);	// ���������� ����� ������������		
void BertClearAllChannels();		// ��������� ��� ������ �� ������������
void BertEnableAIS();				// �������� ��������� ������ AIS � ����� �1
void BertDisableAIS();              // ��������� ��������� ������ AIS � ����� �1  
bool BertisAISGen();	        	// ��������� ��������� ������ AIS � ����� �1
void BertOneErrInsert();			// ������� ��������� ������ � ����������� �����
dword BertUpdateErrBitCount();		// �������� ���������� ������ � ����������� ������
void BertClearErrBitCount();		// �������� ������� ������ � ����������� ������				
bool BertLSS();						// �������� ��������� ������������� �� ���
        			
#endif







