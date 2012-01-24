
#include "stdafx.h"

#include <sys/exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>

// #include <math.h>
// #include <cplbtab.h> 

#include "BfDev/SysProperties.h"
#include "tdm.h"

namespace
{
    
    class TdmList  : public boost::noncopyable
    {    	    
    	    
    public:    	                
    
    	TdmList():
    		m_tdmpList(BfTdm::TdmRegMap::GetSportsCount())
    	{			
	   		for (int i=0; i < m_tdmpList.size(); ++i)
	   			m_tdmpList.at(i) = 0;
    	}
        	
    	void Register(Platform::word sportNum, BfTdm::Tdm *tdmobj)
    	{

   			ESS_ASSERT(sportNum < BfTdm::TdmRegMap::GetSportsCount() && "Requested SPORT don't exist.");
   		    ESS_ASSERT(m_tdmpList.at(sportNum) == 0 && "m_tdmpList.at(sportNum) already exist.");
	   		m_tdmpList.at(sportNum) = tdmobj;
    	}
    	    	
    	void Unregister(BfTdm::Tdm *tdmobj)
    	{
    		       	
			List::iterator i = m_tdmpList.begin();
			for(;i != m_tdmpList.end() && (*i) != tdmobj; i++);
		
			ESS_ASSERT(i != m_tdmpList.end() && "Requested Tdm(sportNum) wasn't finded in TdmList");        	   			
			(*i) = 0;					
    	}
    	    	
		BfTdm::Tdm &Instance(Platform::word sportNum)
		{
			ESS_ASSERT(m_tdmpList.at(sportNum) != 0 && "Tdm doesn't exist");
			return *m_tdmpList.at(sportNum);
		}
    	                    
    private:        
    
		typedef std::vector<BfTdm::Tdm*>  List;
		List m_tdmpList;            
    };                			
    
    TdmList TdmListObj;


    /*
    void FlushCache()
    {
        flush_data_cache();

        dcache_invalidate(CPLB_INVALIDATE_A);
        dcache_invalidate(CPLB_INVALIDATE_B);
    } */
		    
}    
    
namespace BfTdm
{
    using namespace Platform;
    
    //---------------------------------------------------------------------
    
    Tdm::Tdm(ITdmObserver &observer, word sportNum, const TdmProfile &profile):
		m_profile(profile),
        m_observer(observer),    
        m_regMap(sportNum),
        m_enabled(false),
        m_started(false),
        m_Tx(m_regMap.m_pDMATx_IRQ_Status),        
        m_Rx(m_regMap.m_pDMARx_IRQ_Status)
    {
   		
   		TdmListObj.Register(sportNum, this);

/*   		   		     	
   		std::cout << "Sport Num " << (int)sportNum << "\n";
   		std::cout << "GetDMABlocksCount() =" << (int)m_profile.GetDMABlocksCount() << "\n";
   		std::cout << "GetDMABuffSize() =" << (int)m_profile.GetDMABuffSize() << "\n";				
   		std::cout << "isExtSync() =" << (int)m_profile.isExtSync() << "\n";				   						
   		std::cout << "GetDMABlockCapacity() =" << (int)m_profile.GetDMABlockCapacity() << "\n";

   		std::cout << std::endl;   		   		   		
*/   		
        AllocBlocks();   		

        InitSport();
        InitSPORTDMA();        
    }    
    
    //---------------------------------------------------------------------
    
    Tdm::~Tdm()
    {

//		m_regMap.DisableDMAInt();    	

		TdmListObj.Unregister(this);   		
		
    }    
        
    //-----------------------------------------------------------            
                	
	Tdm &Tdm::Instance(word sportNum)
	{
		return TdmListObj.Instance(sportNum);
	}
	
	//-------------------------------------------------------------	        
            
    void Tdm::BlockProcess()
    {                
        m_Rx.ClearNewBlockFlag();
        m_Tx.ClearNewBlockFlag();        

        // READY blocks
        UserBlock &Ch0 = m_Ch0.GetCursorBlock();                
        UserBlock &Ch1 = m_Ch1.GetCursorBlock();
              
        m_Ch0.IncCursor();
        m_Ch1.IncCursor();        
    
        if(!m_enabled) return;  // don't send blocks to user if disabled

        // for READY blocks set user using (only if block with not collision)
        Ch0.SetUserUsing();
        Ch1.SetUserUsing();                

        // stats update
        m_statistic.usedBlocks = m_Ch0.GetUsedUsersBlocksCount();
        m_statistic.CheckMaxUsedBlocks();	        

        // detect collision for CURRENT blocks
        m_Ch0.GetCursorBlock().DetectCollision();
        m_Ch1.GetCursorBlock().DetectCollision();

        // reset collision for READY block
        bool collision = Ch0.ResetCollision();
        collision      = Ch1.ResetCollision() || collision;

        if (collision)
        { 
            m_statistic.collisionCount++;
        }	    	    
        
        m_observer.NewBlockProcess(m_regMap.GetSportNum(), Ch0, Ch1, collision);                
    }
                        
    //-----------------------------------------------------------    
        
    void Tdm::InitSport()
    {	
                
    	*m_regMap.m_pSPORT_MTCS0 = 0xffffffff; // включим 0-31 каналы на передачу
    	*m_regMap.m_pSPORT_MTCS1 = 0x00000000;
    	*m_regMap.m_pSPORT_MTCS2 = 0x00000000;
    	*m_regMap.m_pSPORT_MTCS3 = 0x00000000;
    	*m_regMap.m_pSPORT_MRCS0 = 0xffffffff;	// включим 0-31 каналы на прием
    	*m_regMap.m_pSPORT_MRCS1 = 0x00000000;
    	*m_regMap.m_pSPORT_MRCS2 = 0x00000000;
    	*m_regMap.m_pSPORT_MRCS3 = 0x00000000;


    	*m_regMap.m_pSPORT_TCR1  = TFSR;    	
    	*m_regMap.m_pSPORT_RCR1  = RFSR;
    	   	
    	if (m_profile.GetCompandingLaw() == CAlawCompanding)
    	{

        	*m_regMap.m_pSPORT_TCR1  |= DTYPE_ALAW;
        	*m_regMap.m_pSPORT_RCR1  |= DTYPE_ALAW;
    	        	        	    
    	}    	    

    	if (m_profile.GetCompandingLaw() == CUlawCompanding)
    	{

        	*m_regMap.m_pSPORT_TCR1  |= DTYPE_ULAW;
        	*m_regMap.m_pSPORT_RCR1  |= DTYPE_ULAW;
    	        	        	    
    	}    	        	    	
    	
    
    	if (!m_profile.isExtSync())
    	{

        	*m_regMap.m_pSPORT_TCR1  |= DITFS | ITCLK;
        	*m_regMap.m_pSPORT_RCR1  |= IRFS | IRCLK;
    	        	        	    
    	}    	    
    	    	
    	*m_regMap.m_pSPORT_RCR2  = 0x0107;
    	*m_regMap.m_pSPORT_TCR2  = 0x0107;    	

    	
    	*m_regMap.m_pSPORT_MCMC1 = 0x3000;
    	*m_regMap.m_pSPORT_MCMC2 = 0x001C;

    	if (m_profile.isExtSync()) return;
    	
    	enum
    	{
    	    E1FREQUENCY = 2048000
    	};
    	
    	ESS_ASSERT(BfDev::SysProperties::Instance().getFrequencySys() == 118750000 && "Not Correct FrequencySys");
        int divider = ceil((float)(BfDev::SysProperties::Instance().getFrequencySys())/E1FREQUENCY/2-1);        
        if (divider!=28)
        std::cout << "Check SPORT Frequence, divider =" << divider << std::endl;        
    	*m_regMap.m_pSPORT_RClkDiv   = divider;
    	*m_regMap.m_pSPORT_RFSDiv    = 0x00ff;    // кадровый сигнал каждые 256 бит (32*8 бит) 
    }

    //-----------------------------------------------------------            
    
    void Tdm::InitSPORTDMA()
    {
        // конфигурация DMA приемника
        *m_regMap.m_pDMARx_Config = WNR | WDSIZE_16 | DMA2D | DI_EN  | NDSIZE_4| FLOW_LARGE;
        *m_regMap.m_pDMARx_Next_Desc_Ptr = m_Rx.GetFirstDescriptorP();


        *m_regMap.m_pDMARx_X_Count = 2;	
        *m_regMap.m_pDMARx_X_Modify = m_profile.GetDMABuffSize();		
        *m_regMap.m_pDMARx_Y_Count = m_profile.GetDMABuffSize()/2;	
        *m_regMap.m_pDMARx_Y_Modify = (1 - (m_profile.GetDMABuffSize()/2)) * 2;	

        // конфигурация DMA передатчика для режима 2d автобуфер (2 канала передачи)        
        *m_regMap.m_pDMATx_Config = WDSIZE_16 | DMA2D | DI_EN  | NDSIZE_4| FLOW_LARGE;
        *m_regMap.m_pDMATx_Next_Desc_Ptr = m_Tx.GetFirstDescriptorP();
        
        *m_regMap.m_pDMATx_X_Modify = m_profile.GetDMABuffSize();
        *m_regMap.m_pDMATx_X_Count = 2;
        *m_regMap.m_pDMATx_Y_Count = m_profile.GetDMABuffSize()/2;        
        *m_regMap.m_pDMATx_Y_Modify = (1 - (m_profile.GetDMABuffSize()/2)) * 2;              
                
    }
    
    //-----------------------------------------------------------            

    void Tdm::EnableDMASport()
    {        	    	
    	// enable DMAs
    	*m_regMap.m_pDMARx_Config |= DMAEN;
    	*m_regMap.m_pDMATx_Config |= DMAEN;

    	ssync();	
	
    	// enable Sport0 TX and RX
    	*m_regMap.m_pSPORT_TCR1 	|= TSPEN;
    	*m_regMap.m_pSPORT_RCR1 	|= RSPEN;
    	ssync();	
    }

    //-----------------------------------------------------------            

    void Tdm::DisableDMASport()
    {        	    	        
    	*m_regMap.m_pDMARx_Config &= ~DMAEN;
    	*m_regMap.m_pDMATx_Config &= ~DMAEN;

    	ssync();	
	
    	*m_regMap.m_pSPORT_TCR1 	&= ~TSPEN;
    	*m_regMap.m_pSPORT_RCR1 	&= ~RSPEN;
    }    
    
    //-----------------------------------------------------------                    
        
	void Tdm::Enable()
	{
 	    ESS_ASSERT(m_enabled == false && "TDM already enabled");				
      	ESS_ASSERT( m_Ch0.CheckUsersBlocksFree() );
    	ESS_ASSERT( m_Ch1.CheckUsersBlocksFree() );    	
    	
//    	if (m_started==false) 
    	{
    		m_started = true;
        	m_regMap.InitDMAInt();    			        
			EnableDMASport();            		
    	}
		m_enabled = true;	    	
        
	}
	
    //-----------------------------------------------------------                	
		
	void Tdm::Disable()
	{
		iVDK::CriticalRegion DisInt;
		
 	    ESS_ASSERT(m_enabled == true && "TDM already disabled");				 	     	    
		m_enabled = false;

		m_regMap.DisableDMAInt();    	
		
		DisableDMASport();		
		    					
	}    
            
    // -----------------------------------------------------------                		
	
	bool Tdm::isEnabled()
	{
		return m_enabled;
	}
	
    // -----------------------------------------------------------                

    void Tdm::DMAInterrupt(word sportNum, InterruptDirection dir)
    {
        // BfDev::DisableDataCache cacheDisabler;    // <-- wrong!!
        // flush_data_cache();  // ???

        // FlushCache();

        Instance(sportNum).DMAInterrupt(dir);        
    }
    
    // -----------------------------------------------------------                
    
    void Tdm::DMAInterrupt(InterruptDirection dir)
    {        
        ITdmProfiler *p = m_profile.Profiler();

        if (dir == TxInterrupt) m_Tx.DMAIntProcess();
        if (dir == RxInterrupt) m_Rx.DMAIntProcess();                                        

        if (m_Rx.isNewBlockFlag() && m_Tx.isNewBlockFlag())
        {
            if (p != 0) p->OnProcessingBegin();

            BlockProcess();        

            if (p != 0) p->OnProcessingEnd();
        }

    }
    
    // -----------------------------------------------------------                                
    
    void Tdm::AllocBlocks()
    {
        DMABlock *p_blockTx = 0;       
        DMABlock *p_blockRx = 0;
                                                                        
        for (int i=0; i< m_profile.GetDMABlocksCount(); ++i)        
        {
            DMABlock *p_newTx = new DMABlock(m_profile.GetDMABuffSize(), p_blockTx);
            DMABlock *p_newRx = new DMABlock(m_profile.GetDMABuffSize(), p_blockRx);            
            
            m_Tx.push_back(p_newTx);
            m_Rx.push_back(p_newRx);            
            
            p_blockTx = p_newTx;                        
            p_blockRx = p_newRx;                                    
            
            m_Ch0.AllocUserBlock(m_profile.GetDMABlockCapacity(), 
            	TdmRegMap::GetSportTimeSlotsCount(),
                p_blockTx->GetBufferCh0(),
                p_blockRx->GetBufferCh0(),
                m_profile.isT1Mode(),
                m_profile.GetCompandingLaw() == CNoCompanding);

            m_Ch1.AllocUserBlock(m_profile.GetDMABlockCapacity(), 
            	TdmRegMap::GetSportTimeSlotsCount(),
                p_blockTx->GetBufferCh1(),
                p_blockRx->GetBufferCh1(),
                m_profile.isT1Mode(),
                m_profile.GetCompandingLaw() == CNoCompanding);
                                        
        }
        p_blockTx->SetNext(m_Tx.front());            
        p_blockRx->SetNext(m_Rx.front());                    
    }
    
	//------------------------------------------------		
			
	const TdmStatistic &Tdm::PeekStatistic() const
	{
		return m_statistic;
	}        
    
    //-----------------------------------------------------------                

    Platform::word Tdm::GetSportNum() const
    {
        return m_regMap.GetSportNum();
    }
        

} //namespace BFTdm
