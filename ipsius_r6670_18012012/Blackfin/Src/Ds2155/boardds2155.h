#ifndef __DS2155_BRD__
#define __DS2155_BRD__

#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/BidirBufferCreator.h"

#include "Ds2155hal.h"
#include "Liu.h"
#include "Bert.h"
#include "Hdlc.h"



namespace Ds2155
{
    using namespace Platform;
       
	ESS_TYPEDEF(DS2155Removed);        
	ESS_TYPEDEF(DS2155Absent);        		    
    
    
    /*
        DS2155 board class
    */

    //--------------------------------------------------------------------	        
    
    class IMembersToBoard : public Utils::IBasicInterface
    {
    	
    public:
    
        virtual byte Read(byte addr) =0;
        virtual void Write(byte addr, byte data) = 0;  	
	    virtual byte ReadStatus(byte addr, byte mask) =0;		
        
	   	//--------------------------------------------		    
	
		void MaskSet(byte addr, byte mask)
		{
	    	Write(addr, Read(addr) | mask);		
		}
    
	   	//--------------------------------------------    
   	
	    void MaskClear(byte addr, byte mask)
	    {
	    	Write(addr, Read(addr) & ~mask);    	
	    }
		    
	   	//--------------------------------------------        
	   	
    	virtual bool isDevicePresent() = 0;
		virtual bool isT1Mode() const = 0;    	
		virtual byte GetChannelsCount() const = 0;
            	
		virtual void Register(IBoardToHdlc *controller, int ctrlIndex) = 0;    	
		virtual void Unregister(IBoardToHdlc *controller) = 0;		
    	
    };
    
	    
    //--------------------------------------------------------------------	        
	/*        
        DS2155 board class    
        Содержит классы:
        LIU - Line Intrface Unit;
        BERT - Bit Error Rate Testing;
        HDLC
    */
    
	class BoardDs2155Base : boost::noncopyable,
		IMembersToBoard
	{
		
	public:
        // конструктор объекта BoardDs2155Base, создаваемого в static Init()
		BoardDs2155Base(
			boost::shared_ptr<IDs2155HAL> hal,
			dword baseAddress,
		 	const LiuProfile &src,
			const HdlcProfile &profile,
			ILogHdlc *log);
		~BoardDs2155Base();
				
		Hdlc *AllocHdlc(byte channel,
		 				HdlcLogProfile logProfile = HdlcLogProfile(false,0));
		void HdlcPolling();
		
		void EnableLoopBack();
		void DisableLoopBack();	

		// вывод полной информации о конфигурации объекта
	    std::string &ToString(std::string &str) const;
	           
        Liu &LIU()
        {        
        	return m_liu;
        }
        
        Bert &BERT()
        {        
        	return m_bert;
        }                
                       
        // метод включающий генерацию Exeption при потере устройства на шине
        void SetExeptionMode(bool mode)
        {
            m_exeptionsOn = mode;
        }        

	// impl IMembersToBoard        
	public:
		
	    // методы доступа к регистрам чипа
        byte Read(byte addr);
        void Write(byte addr, byte data);  	
	    byte ReadStatus(byte addr, byte mask);
        
        
    	bool isDevicePresent();        
		bool isT1Mode() const;
		byte GetChannelsCount() const;		
		
		void Register(IBoardToHdlc *controller, int ctrlIndex);
		void Unregister(IBoardToHdlc *controller);		
    	
	private:				
				
        const byte *getBaseAddress() const 
        {
        	return m_baseAddress;
        }

        byte *getBaseAddress() 
        {
        	return m_baseAddress;
        }
        
	private:	
        static int  m_countBoard;
		boost::shared_ptr<IDs2155HAL> m_hal;        
		
		bool        m_isT1;                 // operation mode
		byte        m_channelsCount;        // Per Frame channels count		
        
	    byte        *m_baseAddress;         // base address	    
        Utils::BidirBufferCreator<> m_pool;				    
		ILogHdlc *m_log;        
		bool        m_onBus;                // present on parallel Bus		
		Liu         m_liu;                  // Line Interface Unit object
		Bert		m_bert;					// Bit Error Rate Testing
		bool        m_exeptionsOn;          // exeptions ON\OFF
		int 		m_hdlcMaxPackSize;			// 		
		int 		m_poolBlocksNum;			// 				
		
		typedef std::vector<IBoardToHdlc*>  HDLCList;
		HDLCList 	m_hdlcList;            		
		
	};
	
	
    //--------------------------------------------------------------------	                
    
	class BoardDs2155 : public BoardDs2155Base	
	{
		
	public:
		BoardDs2155(
            const std::string &name, 
			dword baseAddress,
		 	const LiuProfile &liuProfile,
			const HdlcProfile &hdlcProfile,
			ILogHdlc *log = 0) :
			BoardDs2155Base(boost::shared_ptr<IDs2155HAL>(new Ds2155hal()),
			baseAddress,
		 	liuProfile,
			hdlcProfile,
			log){}
	};

	
};

#endif







