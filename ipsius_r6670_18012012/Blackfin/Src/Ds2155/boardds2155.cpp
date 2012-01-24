#include "stdafx.h"

#include <cdefBF537.h>
#include "BoardDs2155.h"

namespace 
{
	typedef std::vector<Ds2155::BoardDs2155Base*>  BoardsList;
	BoardsList boardsList;
};


namespace Ds2155
{
	enum
    {
   		INFO4 = 0x2d,
    };        

	
	
    int BoardDs2155Base::m_countBoard = 0;
		
	//----------------------------------------------------------------------------------------------	
	BoardDs2155Base::BoardDs2155Base(
		boost::shared_ptr<IDs2155HAL> hal,
		dword baseAddress, 
		const LiuProfile &liuProfile,
		const HdlcProfile &hdlcProfile,
		ILogHdlc *log) : 
        m_isT1(liuProfile.LineCode() == Enums::lcB8zs),		
        m_channelsCount(m_isT1 ? 24 : 32),
		m_hal(hal),
	    m_baseAddress((byte*)baseAddress),
		m_pool(hdlcProfile.GetMemPoolBlockSize(), 
			   hdlcProfile.GetBlocksNum(),
			   hdlcProfile.GetBidirOffset()),
		m_hdlcMaxPackSize(hdlcProfile.GetMaxPackSize()),
		m_poolBlocksNum(hdlcProfile.GetBlocksNum()),
	    m_hdlcList(HdlcRegMap::GetControllersCount()),
	    m_log(log),
	    m_onBus(false),
	    m_liu(*this, liuProfile),
		m_bert(*this),
	    m_exeptionsOn(false)
    {        
        BoardsList::iterator i = boardsList.begin();
		for(;i != boardsList.end() && 
			(*i)->getBaseAddress() != m_baseAddress; ++i);
					
		if (!isDevicePresent()) ESS_THROW(DS2155Absent);
			
		ESS_ASSERT(hdlcProfile.GetMaxPackSize() <= hdlcProfile.GetMemPoolBlockSize() &&
			"Memory pool block size smaller then HDLC maxPackSize");			
			
		ESS_ASSERT(i == boardsList.end() && "Base address or board name already used.");
		
		boardsList.push_back(this);
				     	
		m_countBoard++;

		for (int i=0; i < m_hdlcList.size(); ++i)
			m_hdlcList.at(i) = 0;		    	    	
			
    	if (m_liu.Init())  m_onBus = true;
    }
    
    //--------------------------------------------------------------------
        
	BoardDs2155Base::~BoardDs2155Base()
	{
		BoardsList::iterator i = boardsList.begin();
		for(;i != boardsList.end() && 
			(*i) != this; ++i);
		
		ESS_ASSERT(i != boardsList.end() && "Board not registered.");

		for (int i = 0; i < m_hdlcList.size(); ++i)
			ESS_ASSERT(!m_hdlcList.at(i) && "Kill HDLC controllers first");			
        
	    boardsList.erase(i);	 
	}

    //--------------------------------------------------------------------	
	
	void BoardDs2155Base::Register(IBoardToHdlc *controller, int ctrlIndex)
	{		

		ESS_ASSERT(m_hdlcList.at(ctrlIndex) == 0 && "This controller is already registered");
		m_hdlcList.at(ctrlIndex) = controller;		
		
	}	
	
    //--------------------------------------------------------------------	
	
	void BoardDs2155Base::Unregister(IBoardToHdlc *controller)
	{		
		HDLCList::iterator i = 
			std::find(m_hdlcList.begin(), m_hdlcList.end(), controller);
		
	
		ESS_ASSERT(i != m_hdlcList.end() && "Controller is already unregistered");
		*i = 0;
	}
	
    //--------------------------------------------------------------------
		    
    bool BoardDs2155Base::isDevicePresent() 
    {
		if (m_onBus && ((Read(L_IDR) & 0xf0 ) != 0xb0))
		{
        	m_onBus = false;			
			if (m_exeptionsOn) ESS_THROW(DS2155Removed);			
        }
        else m_onBus = ((Read(L_IDR) & 0xf0 ) == 0xb0);                
        
    	return m_onBus;
    }
    
    //--------------------------------------------------------------------
		    
    bool BoardDs2155Base::isT1Mode() const
    {        
    	return m_isT1;
    }    

    //--------------------------------------------------------------------
		    
    byte BoardDs2155Base::GetChannelsCount() const
    {        
    	return m_channelsCount;
    }                
    
    //--------------------------------------------------------------------        
            
	Hdlc *BoardDs2155Base::AllocHdlc(byte channel, HdlcLogProfile logProfile)
	{
		
		int ctrlIndex = 0; 
						
		while(ctrlIndex < m_hdlcList.size())
		{
			if (!m_hdlcList.at(ctrlIndex)) break;
			++ctrlIndex;
		}
		
		ESS_ASSERT(ctrlIndex != m_hdlcList.size() && "No Free HDLC Controllers");		
		
		for (int i = 0; i < m_hdlcList.size(); ++i)
		{
			IBoardToHdlc *hdlc = m_hdlcList.at(i);
			
	  		if (hdlc)
				ESS_ASSERT(hdlc->GetChannelLink() != channel && "Requested channel already used");	  		

		}
		
		ESS_ASSERT(!(!m_log & logProfile.isAnyFlag()) && "Undefined Log object");

		Hdlc *hdlc = new Hdlc(*this, ctrlIndex, m_hdlcMaxPackSize, m_pool, m_poolBlocksNum, channel, m_log, logProfile);
		return hdlc;

	}        
	
    //--------------------------------------------------------------------    	
    /*
		“.к. у чипа DS2155 имеетс€ регистр очищаемый при чтении, хран€щий
	 	данные дл€ обоих контроллеров, необходимо в процессе поллинга
	 	считывать его содержимое и передавать параметром в методы поллинга
	 	каждого контроллера
	*/	
			
	void BoardDs2155Base::HdlcPolling()
	{		
		Write(INFO4, 0x0f);
		byte info4 = Read(INFO4);		
		
		bool txOvrun = (info4 & 0x02);

		if (m_hdlcList.at(0))
			m_hdlcList.at(0)->Polling(txOvrun);
								
		txOvrun = (info4 & 0x08);
		
		if (m_hdlcList.at(1)) 
			m_hdlcList.at(1)->Polling(txOvrun);


	}
	
    //--------------------------------------------------------------------
	//‘ункции доступа к регистрам чипа		

    byte BoardDs2155Base::Read( byte addr)
	{
        return m_hal->Read(m_baseAddress, addr);
	}
	
   	//--------------------------------------------
 		
    void BoardDs2155Base::Write(byte addr, byte data)
	{
		m_hal->Write(m_baseAddress, addr, data);
	}
           
   	//--------------------------------------------    
	
    byte BoardDs2155Base::ReadStatus(byte addr, byte mask)
    {
		return m_hal->ReadStatus(m_baseAddress, addr, mask);    	
    }	    
	
   	//--------------------------------------------   	
   		
    std::string &BoardDs2155Base::ToString(std::string &str) const
	{
		using std::cout;
		using std::endl;
		{		
			std::ostringstream out;

			out << "  Base Address = " << (int *)getBaseAddress() << std::endl;
			out << "  Online       = " << (int)m_onBus << std::endl;
//			out << "  Liu address  = " << &m_liu << std::endl;
			str += out.str();
		}
		
		m_liu.ToString(str);
		return str;
	}	
    
    //--------------------------------------------------------------------        
    
	void BoardDs2155Base::EnableLoopBack()
	{	
		Write(L_LBCR, 0x01);
	}
		
    //--------------------------------------------------------------------    
				
	void BoardDs2155Base::DisableLoopBack()
	{	
		Write(L_LBCR, 0x00);		
	}
				
    //--------------------------------------------------------------------    
};
