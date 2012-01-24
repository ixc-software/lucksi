#include "stdafx.h"

#include "ChipVerify.h"

// ---------------------------------------------------

namespace
{

    enum
    {
        CMinRandomTransformIntervalMs = 1000,
        CRandomReqDivider = 20,
    };

}  // namespace


// ---------------------------------------------------

namespace ChipSecure
{
       
    ChipSecure::ChipVerify::List ChipVerify::MakeList()
    {
        List lx;

        Add(lx, 0xdc39b9c2, 0xfc38f377);
        Add(lx, 0xf7012ccf, 0xa4dc98b3);
        Add(lx, 0x54a12010, 0xbe2d54b1);
        Add(lx, 0x44366455, 0x5e322d88);
        Add(lx, 0x337c4dae, 0x88b65ebd);
        Add(lx, 0xfef3c06b, 0xc7645e87);
        Add(lx, 0x0d5a721c, 0xf71ac4e2);
        Add(lx, 0x5e832e91, 0xcbe9a9d4);

        return lx;
    }

    ChipVerify::ChipVerify( dword seed ) : m_rnd(seed), m_chip(m_twi)
    {
        m_lastRndRequestTime = Platform::GetSystemTickCount();
        m_statEvent = 0;
        m_statRandom = 0;

        // challenge list
        m_list = MakeList();

        // events list
        m_eventsList.push_back(0);
        m_eventsList.push_back(16);
        m_eventsList.push_back(100);
        m_eventsList.push_back(200);
        m_eventsList.push_back(1024);
        m_eventsList.push_back(4000);
        m_eventsList.push_back(8000);
        m_eventsList.push_back(20000);

        if ( !RunTest(0) ) ESS_THROW(Error);
    }

    void ChipVerify::Process()
    {
        dword currT = Platform::GetSystemTickCount();
        if (currT - m_lastRndRequestTime < CMinRandomTransformIntervalMs) return;

        dword div = m_rnd.Next(CRandomReqDivider);
        if (div > 0) return;

        // do random challenge
        m_lastRndRequestTime = currT;
        ++m_statRandom;

        dword dummi = ChipUtils::ChipTransform(m_chip, m_rnd.Next());
    }

}  // namespace ChipSecure
