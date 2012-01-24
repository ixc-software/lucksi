#ifndef __DS28CN01__
#define __DS28CN01__

#include "Platform/Platform.h"
#include "BfDev/BfTwi.h"

namespace Ds28cn01
{	
    
    using namespace Platform;
                	
    template <int buffSize> 
    struct Buff
    {
        Buff()
        {
            Clear();
        }
        
        byte& operator[](int i)
        {
            ESS_ASSERT( i < CSize);
            return DataBuff[i];
        }                                     
        
        const byte& operator[](int i) const
        {
            ESS_ASSERT( i < CSize);
            return DataBuff[i];
        }           

        void Fill(byte val)
        {
            for(int i = 0; i < CSize; i++) DataBuff[i] = val;
        }

        void Clear()
        {
            Fill(0x00);
        }
                    
        static const int CSize = buffSize;
        
        private:
            
            byte DataBuff[buffSize];                    
            
    };	
    
    typedef Buff<20> MACBuffStruct;
    typedef Buff<7> ChallendgeBuffStruct;      
    typedef Buff<8> SecretBuffStruct;        	

    /*
        DS28CN01 class
        
        Все методы являются блокирующими.
    */                
               
	class DS28CN01 : boost::noncopyable
	{		
			
	public:
	
        ESS_TYPEDEF(Absent);    
        ESS_TYPEDEF(Error);                                	

		DS28CN01(BfDev::BfTwi &twi, word chipAddress = 0x0050);
		~DS28CN01();					 
		
		//получить UIN, 48-битный уникальный номер чипа
		ddword GetUIN();
						
		MACBuffStruct GetHash(const ChallendgeBuffStruct &challendge);
		
		//возвращает колво принятых данных (CPageCapacity), если 0, то ошибка
		byte ReadPage(byte pageNum, byte *dataBuff);
		
		//записывает секрет
		void WriteSecret(const SecretBuffStruct &secret);
		
		//проверяет сигнатуру чипа и корректность серийного номера(по CRC8)
		bool CheckSignature();
		
		//проверяет отсутствие прошитого секрета		
        bool isSecretClear();
       
        		       		
    private:    
    
        word            m_chipAddress;
        BfDev::BfTwi    &m_twi;        
    

		//возвращает UROM, 48-битный уникальный номер чипа + 8бит сигнатура + 8 бит CRC8,
	    ddword GetUROM();        
                
        //переводит интерфейс чипа в I2C
		void SetI2CMode();
		
		
    	
	};

} //namespace BfDev

#endif

