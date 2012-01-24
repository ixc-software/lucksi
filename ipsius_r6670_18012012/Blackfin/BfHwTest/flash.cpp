#include "flash.h"

#define SPIBAUD (SYS_CLK / 2 / SPI_SPEED)

    //-------------------------------------------------------------------		
		
	void FlashSelect()	
	{
		*pPORTFIO_CLEAR = 0x0400;
	}	

    //-------------------------------------------------------------------		
	
	void FlashDeselect()
	{
		*pPORTFIO_SET = 0x0400;		
	}

    //-------------------------------------------------------------------			
	
	void LoaderSelect()	
	{
		*pPORTGIO_CLEAR = 0x0001;
		*pSPI_CTL |= CPHA;
		ssync();						
	}	
	
    //-------------------------------------------------------------------			

	void LoaderDeselect()
	{
		*pPORTGIO_SET = 0x0001;		
		*pSPI_CTL &= ~CPHA;
		ssync();			
	}


    //-------------------------------------------------------------------

	void SpiInit()
	{
		int temp;	

		*pPORTGIO_DIR |= 0x0001;	//CS of EchoProcessor							
		LoaderDeselect();		
		
		*pPORTFIO_DIR |= 0x0400;	//CS of ExtFlash				
		*pPORTFIO_DIR &= ~0x0002;	//CS of ExtFlash						
		*pPORTGIO_INEN		|= 0x0002;  //Slave Busy Pin		
		FlashDeselect();
		
		temp = *pPORTF_FER;
		ssync();
		temp |= 0x3800;
		*pPORTF_FER = temp;
		ssync();
		*pPORTF_FER = temp;
		ssync();
							
		*pSPI_CTL 	&=  (~SPE);					// disable SPI
		*pSPI_CTL = 0x01 | MSTR;
		*pSPI_BAUD  =  SPIBAUD;						
		*pSPI_CTL 	|=  SPE;					// enable SPI
		ssync();						
		
	}
			
    //-------------------------------------------------------------------	
	
	byte SpiTransfer(byte data)
	{				
		while (*pSPI_STAT & TXS);		
		*pSPI_TDBR = data;								// send data
		while (*pSPI_STAT & TXS);
		while (!(*pSPI_STAT & RXS));
		return *pSPI_RDBR;					// get input								
	}
	
    //-------------------------------------------------------------------	
	     
	void FlashWriteEnable()
	{
		FlashSelect();
		SpiTransfer(0x06);
		FlashDeselect();	
	}
	
    //-------------------------------------------------------------------	
		
	bool CheckM25P128Signature()
	{

		FlashSelect();
		SpiTransfer(0x9f);
		byte Manufacturer = SpiTransfer(0xFF);
		byte MemType = SpiTransfer(0xFF);
		byte MemCapacity = SpiTransfer(0xFF);
		FlashDeselect();			
		
		if (Manufacturer != 0x20) return false;
		if (MemType != 0x20) return false;		
		if (MemCapacity != 0x18) return false;
		return true;								
	}
		
    //-------------------------------------------------------------------

	bool FlashInit()
	{					
		FlashWriteEnable();	
		return CheckM25P128Signature();
	}
	
    //-------------------------------------------------------------------

	bool FlashTest()
	{							
		return CheckM25P128Signature();
	}
		

    //-------------------------------------------------------------------				

	bool isSlaveReady()
	{
		return !(*pPORTGIO & 0x0002);
	}	
	
    //-------------------------------------------------------------------			    

    void WaitSlaveReady() 
    {
        if(isSlaveReady()) return;
        while (!isSlaveReady());
    }

    //-------------------------------------------------------------------			    

    void SlaveLoadBlock(byte *data, dword count)
    {

		LoaderSelect();  
		for(dword i = 0; i < count; i++) 
		{
            WaitSlaveReady();			
            SpiTransfer(*data++);
		}			
		LoaderDeselect();            				
    }        

	//-------------------------------------------------------------------

	byte FlashStatRegRead()
	{
		byte result;
	
		FlashSelect();
		SpiTransfer(0x05);
		result = SpiTransfer(0xFF);
		FlashDeselect();
		return result;
	}
    
    
    //-------------------------------------------------------------------			    
    
	bool isFlashBusy()
	{
		return (FlashStatRegRead() & 0x01);
	}
	
    //-------------------------------------------------------------------        
    
	void FlashRead(dword offset, byte *pBuff, dword count)
	{
		while(isFlashBusy());
		FlashSelect();
		SpiTransfer(0x03);					//cmd
		SpiTransfer(offset>>16);
		SpiTransfer(offset>>8);
		SpiTransfer(offset);
		for (dword i = 0; i < count; i++)
		{
		  *pBuff = SpiTransfer(0xFF);      //read
		  pBuff++;
		}
		FlashDeselect();	
	}
	
            
    
    	
	
	
