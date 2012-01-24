#include "stdafx.h"

#include "BfDev/BfTwi.h"
#include "BfDev/BfTimerCounter.h"
#include "Ds28cn01/ds28cn01.h"
#include "Utils/Random.h"
#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "MakeSecureResult.h"
#include "ChipUtils.h"

using Utils::UpdateCRC32;

// ------------------------------------------------------------

namespace ChipSecure
{

    class SecureSingleItem
    {
        dword m_dataIn;
        dword m_dataOut;
        const dword m_crc;

    public:

        SecureSingleItem(const dword &dataIn, 
                         const dword &dataOut) :
            m_dataIn(dataIn),
            m_dataOut(dataOut),
            m_crc(dataIn ^ dataOut)
        {
        }

        std::string Dump()
        {
            using namespace Utils;

            std::ostringstream oss;

            oss << IntToHexString(m_dataIn) << " -> " << IntToHexString(m_dataOut)
                << " : CRC " << IntToHexString(m_crc);

            return oss.str();
        }

        dword CRC() const { return m_crc; }

    };

    // -------------------------------------------------------
            
    std::string SecureResults::Dump() const
    {
        using std::endl;
        std::ostringstream oss;

        oss << "Items:" << endl;

        for(int i = 0; i < m_list.Size(); ++i)
        {
            oss << m_list[i]->Dump() << endl;
        }

        oss << "CRC: " << Utils::IntToHexString( CRC() ) << endl;
        oss << "Mcs per transform: " << McsPerTransform() << endl;

        return oss.str();
    }

    // -------------------------------------------------------

    void SecureResults::Add( SecureSingleItem *item )
    {
        m_list.Add(item);
    }

    // -------------------------------------------------------

    void SecureResults::Clear()
    {
        m_list.Clear();
    }

    // -------------------------------------------------------

    Utils::dword SecureResults::CRC() const
    {
        dword crc = 0;

        for(int i = 0; i < m_list.Size(); ++i)
        {
            crc ^= m_list[i]->CRC();
        }

        return crc;
    }

    // -------------------------------------------------------

    SecureResults::SecureResults() 
    {
        m_mcsPerTransform = 0;
    }   

    SecureResults::~SecureResults() 
    {
        // nothing, for correct SecureSingleItem delete
    }

    // -------------------------------------------------------

    void MakeSecureResult(dword seed, dword count, bool verify, SecureResults &result)
    {
        using namespace BfDev;

        result.Clear();

        BfTwi twi;
        Ds28cn01::DS28CN01 chip(twi);

        ddword tSum = 0;
        BfTimerCounter t( BfTimerCounter::GetFreeTimer() );

        // generate
        Utils::Random rnd(seed);

        for(int i = 0; i < count; ++i)
        {
            dword dataIn = rnd.Next();

            t.Reset();
            dword dataOut = ChipUtils::ChipTransform(chip, dataIn);
            tSum += t.GetCounter();
            
            SecureSingleItem *item = 
                new SecureSingleItem(dataIn, dataOut);

            result.Add(item);
        }

        if (count > 0)
        {
            result.McsPerTransform( t.CounterToMcs(tSum) / count );
        }

        if (!verify) return;

        // verify
        ESS_UNIMPLEMENTED;

    }

    void MakeSecureDump()
    {
        using namespace std;

        cout << "Started..." << endl;

        SecureResults res;
        MakeSecureResult(0xff105a, 64, false, res);

        cout << res.Dump() << endl;
    } 

}  // namespace ChipSecure


