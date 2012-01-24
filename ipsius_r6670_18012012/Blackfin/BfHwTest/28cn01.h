#ifndef __DS28CN01__
#define __DS28CN01__

#include "macros.h"

namespace DevIpTDM
{	
    
	enum
	{
        CSecureChipAddress = 0x0050,	    
        CChallengeLen = 7,
        CMacLen = 20,
        CSecretLen = 8,        
        CUINLen = 6,        
        CUROMLen = 8,
        CPageNum = 4,
        CPageCapacity = 32
	};
    
    /*
        DS28CN01 class
        .....
    */                
               
	class DS28CN01
	{		
			
	public:

		DS28CN01(BfDev::BfTwi &twi, word chipAddress);
		~DS28CN01();					 
		
		//���������� ����� UIN, 48-������ ���������� ����� ����, ���� 0, �� ������
		byte GetUIN(byte *UINbuff);
						
		//�� �������� ������ ������ 7 ���� ������������ ��� Challenge ���������,
		//����� ��������� �������� � ������ ��������� 20 ���� ����
		//���������� ����� ��������� ����, ���� 0, �� ������
		byte GetHash(byte *hashBuff);				
		
		//���������� ����� �������� ������ (CPageCapacity), ���� 0, �� ������
		byte ReadPage(byte pageNum, byte *dataBuff);
		
		//���������� ������, ������ = CSecretLen
		bool WriteSecret(byte *secretBuff);		
		
		//��������� ��������� ���� � ������������ ��������� ������(�� CRC8)
		bool CheckSignature();				
		       		
    private:    
    
        word            m_chipAddress;
        BfDev::BfTwi    &m_twi;        
    
		//���������� ����� UROM, 48-������ ���������� ����� ���� + 8��� ��������� + 8 ��� CRC8,
		// ���� 0, �� ������
	    byte GetUROM(byte *UINbuff);
        
        //��������� ��������� ���� � I2C
		bool SetI2CMode();
		
		
    	
	};

} //namespace BfDev

#endif

