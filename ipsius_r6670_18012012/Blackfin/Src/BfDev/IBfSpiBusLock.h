#ifndef _I_BF_SPI_LOCK_H_
#define _I_BF_SPI_LOCK_H_

#include "BfPio.h"

namespace BfDev
{
    
    // ----------------------------------------
    
    class IBfSpiBusLock : public Utils::IBasicInterface
    {
    public:
		virtual void ChipEnable() = 0;
		virtual void ChipDisable() = 0;		
    };

    // ----------------------------------------

    class BfSpiBusLock : public IBfSpiBusLock
    {
        BfPioInfo m_pio;
        Platform::word m_mask;

    // IBfSpiBusLock impl
    public:

        void ChipEnable()
        {
            *m_pio.Clear = m_mask;
        }

        void ChipDisable()
        {
            *m_pio.Set = m_mask;
        }

    public:

        BfSpiBusLock(PioPort pio, Platform::byte pin) : 
          m_pio( BfPio::Get(pio) ),
          m_mask(1 << pin)
        {
            ESS_ASSERT(pin < 16);

            *m_pio.Dir |= m_mask;
            ChipDisable();
        }

    };

    // ----------------------------------------
    
    class SpiBusLocker : public boost::noncopyable
    {
    public:
        SpiBusLocker(IBfSpiBusLock &deviceBusLock):
            m_deviceBusLock(deviceBusLock)
        {
            m_deviceBusLock.ChipEnable();
        };
        
        ~SpiBusLocker()
        {
            m_deviceBusLock.ChipDisable();
        };                
    
    private:
        
        IBfSpiBusLock &m_deviceBusLock;
            
    };    
    
};
#endif
