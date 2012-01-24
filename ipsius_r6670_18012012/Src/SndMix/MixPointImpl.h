#ifndef __MIXPOINTIMPL__
#define __MIXPOINTIMPL__

#include "IMixPoint.h"
#include "MixerCodec.h"
#include "Utils/BidirBuffer.h"

namespace SndMix
{

    struct MixPointImplProfile
    {
        int BuffSize;

        // freq adaptation params - ?
        // ... 

		static const int CTypicalPacket = 160;

		MixPointImplProfile(int typicalPacketSize = CTypicalPacket, int kMul = 6)
		{
            BuffSize = typicalPacketSize * kMul;
		}

        bool Validate() const
        {
            return true;
        }
        
    };

    // ----------------------------------------------------------------

    /* Buffered implementation for IMixPoint

        m_toMixerBuff служит для стыковки блоков данных потенциально разных
        размеров -- источника данных и миксера, который внутри полагает, что
        все источники имеют блоки данных одинакового размера.
        Плюс дополнительное сглаживание (за счет буферизации) скоростей 
        поступления данных от всех источников данных. 

        m_fromMixerBuff -- аналогично, в обратном направлении. 

    */
    class MixPointImpl : 
        public IMixPoint, 
        public virtual Utils::SafeRefServer,
        boost::noncopyable
    {
        std::string m_name;

        const int m_handle;
        const PointMode m_mode;
        const MixPointImplProfile m_profile;
        SafeRef<IMixPointOwner> m_owner;

        CyclicBuff m_toMixerBuff,
                   m_fromMixerBuff;

        static SafeRef<IMixPointOwner> ValidateParams(SafeRef<IMixPointOwner> owner, 
                                                      const MixPointImplProfile &profile)
        {
            ESS_ASSERT( !owner.IsEmpty() );
            ESS_ASSERT( profile.Validate() );

            return owner;
        }

    /*

    // ICyclicBuffDataReady impl
    private:

        IDataNotify *m_pNotify;  // callback for m_profile.FromMixerCallbackDataSize 

        class IDataNotify : public Utils::IBasicInterface
        {
        public:
            virtual void OnDataFromMixerReady() = 0;
        };

        bool OnCyclicBuffDataReady(CyclicBuff *pSender)
        {                        
            
            if (pSender == &m_toMixerBuff)  // m_owner->BlockSize()
            {
                m_owner->ReadyToRead(*this);
                return true;
            }

            if (pSender == &m_fromMixerBuff)  // m_profile.FromMixerCallbackDataSize 
            {
                if (m_pNotify != 0)
                {
                    m_pNotify->OnDataFromMixerReady();
                    return true;
                }
                return false;
            }

            ESS_HALT("");
            return false;  // dummi
        } 
        
    */

    // IMixPoint impl
    private:

        int Handle() const
        {
            return m_handle; 
        }

        PointMode GetMode() const
        {
            return m_mode;
        }

        MixPointState PointState() const
        {
            return MixPointState( m_name, m_toMixerBuff.GetState(), m_fromMixerBuff.GetState() );
        }

        CyclicBuff& ReadBuff()  { return m_toMixerBuff; }
        CyclicBuff& WriteBuff() { return m_fromMixerBuff; }

    public:

        MixPointImpl(const std::string &name, int handle, PointMode mode, 
            SafeRef<IMixPointOwner> owner, 
            const MixPointImplProfile &profile) :
            m_name(name),
            m_handle(handle),
            m_mode(mode),
            m_profile(profile),
            m_owner( ValidateParams(owner, profile) ),
            m_toMixerBuff(profile.BuffSize),  
            m_fromMixerBuff(profile.BuffSize) 
        {
            // m_toMixerBuff.SetupNotifyCallback  (this, owner->BlockSize());
            // m_fromMixerBuff.SetupNotifyCallback(this, profile.FromMixerCallbackDataSize);

            m_owner->Bind(this);
        }

        ~MixPointImpl()
        {
            ESS_ASSERT( !m_owner.IsEmpty() );
            m_owner.Clear()->Unbind(*this);
        }

        CyclicBuff& ToMixerBuff()  { return m_toMixerBuff; }
        CyclicBuff& FromMixerBuff() { return m_fromMixerBuff; }

        SafeRef<IMixPointOwner> GetOwner() const
        {
            return m_owner;
        }

    };    
    
}  // namespace SndMix


#endif
