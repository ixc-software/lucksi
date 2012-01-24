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
		
		//возвращает длину UIN, 48-битный уникальный номер чипа, если 0, то ошибка
		byte GetUIN(byte *UINbuff);
						
		//из входного буфера первые 7 байт используются как Challenge сообщение,
		//после окончания подсчета в буфере останется 20 байт хэша
		//возвращает длину принятого хэша, если 0, то ошибка
		byte GetHash(byte *hashBuff);				
		
		//возвращает колво принятых данных (CPageCapacity), если 0, то ошибка
		byte ReadPage(byte pageNum, byte *dataBuff);
		
		//записывает секрет, длиниа = CSecretLen
		bool WriteSecret(byte *secretBuff);		
		
		//проверяет сигнатуру чипа и корректность серийного номера(по CRC8)
		bool CheckSignature();				
		       		
    private:    
    
        word            m_chipAddress;
        BfDev::BfTwi    &m_twi;        
    
		//возвращает длину UROM, 48-битный уникальный номер чипа + 8бит сигнатура + 8 бит CRC8,
		// если 0, то ошибка
	    byte GetUROM(byte *UINbuff);
        
        //переводит интерфейс чипа в I2C
		bool SetI2CMode();
		
		
    	
	};

} //namespace BfDev

#endif

