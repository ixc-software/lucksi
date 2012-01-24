#include "stdafx.h"

#include "Utils/Random.h"
#include "TestFw/TestFwGroup.h"
#include "iLog/LogManager.h"

#include "MixerCore.h"
#include "MixPointImpl.h"
#include "ConferenceMng.h"

#include "MixerTest.h"

// --------------------------------------------

namespace
{
    using namespace SndMix;

    class TestUtils
    {

        static void VecAdd(std::vector<int> &v, int i)
        {
            if (i >= 0) v.push_back(i);
        }

    public:

        static std::vector<int> MakeVec(int i0 = -1, int i1 = -1, 
            int i2 = -1, int i3 = -1,
            int i4 = -1, int i5 = -1)
        {
            std::vector<int> res;

            VecAdd(res, i0);
            VecAdd(res, i1);
            VecAdd(res, i2);
            VecAdd(res, i3);
            VecAdd(res, i4);
            VecAdd(res, i5);

            return res;
        }

        static void AddMode(MixMode &mode, int outCh, const std::vector<int> &inCh)
        {
            std::vector<ChInput> input;
            for(int i = 0; i < inCh.size(); ++i) input.push_back( inCh.at(i) );

            ChOutputDesc desc(outCh);
            desc.InputChannels = input;

            mode.ChOutputMode(desc);
        }

        static void AddMode(MixMode &mode, int outCh, 
            int inCh0 = -1, int inCh1 = -1, int inCh2 = -1)
        {
            AddMode( mode, outCh, MakeVec(inCh0, inCh1, inCh2) );
        }

    };

}  // namespace

// --------------------------------------------

namespace
{
    using namespace SndMix;

    class MixModeTest
    {

        static void DoRun(bool useRawAdd)
        {
            MixMode mode;

            if (useRawAdd)
            {
                TestUtils::AddMode(mode, 0, 1, 2);
                TestUtils::AddMode(mode, 1, 0, 2);

                TUT_ASSERT( TestUtils::MakeVec(0, 1)    == mode.AllOutputChs() );
                TUT_ASSERT( TestUtils::MakeVec(0, 1, 2) == mode.AllInputChs() );

                TestUtils::AddMode(mode, 4, 0, 3);
                TestUtils::AddMode(mode, 3, 1);
                TestUtils::AddMode(mode, 6, 1);
            }
            else
            {
                std::string s = "0 = 1,2; 1 = 0, 2 ; 4 = 0, 3  ; 3 = 1; 6 = 1";
                mode = MixMode(s);
            }

            TUT_ASSERT( TestUtils::MakeVec(0, 1, 3, 4, 6)    == mode.AllOutputChs() );
            TUT_ASSERT( TestUtils::MakeVec(0, 1, 2, 3)       == mode.AllInputChs() );
        }


    public:

        static void Run()
        {
            DoRun(true);
            DoRun(false);
        }

    };

}  // namespace


// --------------------------------------------

namespace
{
    using namespace SndMix;

    class RndStream
    {
        Utils::Random m_rnd;

    public:

        RndStream(int chNumber) : m_rnd(chNumber * 3 + 17)
        {
        }

        int Next()
        {
            return m_rnd.Next(9) - 18;
        }
    };

    // ---------------------------------------

    class GndMixStream
    {
        std::vector<RndStream> m_list;

    public:

        GndMixStream(const std::vector<int> &chIn)
        {
            for(int i = 0; i < chIn.size(); ++i)
            {
                m_list.push_back( RndStream( chIn.at(i) ) );
            }
        }

        int Next()
        {
            int sum = 0;

            for(int i = 0; i < m_list.size(); ++i)
            {
                sum += m_list.at(i).Next();
            }

            return sum;
        }
    };

    // ---------------------------------------

    class Point : public MixPointImpl, public ICyclicBuffDataReady
    {
        const int m_chNum;
        const int m_blockToMix, 
                  m_blockFromMix;
        const std::vector<int> m_chIn;

        RndStream m_rnd;
        std::vector<int16> m_dataToMix;

        GndMixStream m_readSrc;
        int m_outputSamples;

        /*
        static MixPointImplProfile GetProfile(int blockToMix, int blockFromMix)
        {
            MixPointImplProfile p;
            p.OutBuffMinDataForNotify = blockFromMix;
            return p;
        } */

    // ICyclicBuffDataReady impl
    private:

        bool OnCyclicBuffDataReady(CyclicBuff *pSender)
        {
            std::vector<int16> data;
            FromMixerBuff().ReadBlock(data, m_blockFromMix);

            // verify
            for(int i = 0; i < data.size(); ++i)
            {
                int refData = m_readSrc.Next();
                if (data.at(i) != refData) 
                {
                    TUT_ASSERT(0 && "Fail!");
                }
            }

            // stats update
            m_outputSamples += data.size();

            return true;
        }

    public:

        Point(MixerCore &core, MixMode &mode, int chNum, 
            const std::vector<int> &chIn, int blockToMix, int blockFromMix) :
            MixPointImpl( "Test", chNum, SndMix::PmSendRecv, &core, MixPointImplProfile() ),
            m_chNum(chNum),
            m_blockToMix(blockToMix),
            m_blockFromMix(blockFromMix),
            m_chIn(chIn),
            m_rnd(chNum),
            m_readSrc(chIn),
            m_outputSamples(0)
        {
            FromMixerBuff().SetupNotifyCallback(this, m_blockFromMix);

            TestUtils::AddMode(mode, chNum, chIn);
        }

        int OutputSamples() const { return m_outputSamples; }

        void Next()
        {
            m_dataToMix.push_back( m_rnd.Next() );

            // don't work for freq driven mixer; m_blockToMix is ignored 
            // if (m_dataToMix.size() < m_blockToMix) return;

            ToMixerBuff().WriteBlock(m_dataToMix);
            m_dataToMix.clear();
        }
    };

    // ---------------------------------------

    void RunMixerCoreTest()
    {
        const int CBlockSize = 160;
        const int CDataCount = 32 * 1024;

        // core
        iLogW::LogSettings logSettings;
        iLogW::LogManager log(logSettings);
        MixerCore core(log, false, 0, CBlockSize);  
        MixMode mode;

        // points list
        Utils::ManagedList<Point> m_list;

        m_list.Add( new Point(core, mode, 0, TestUtils::MakeVec(1, 2),  33, 201) );
        m_list.Add( new Point(core, mode, 1, TestUtils::MakeVec(0),    160, 160) );
        m_list.Add( new Point(core, mode, 2, TestUtils::MakeVec(0, 1), 194, 95)  );

        core.SetMode(mode);

        // loop
        int mixCounter = 0;
        for(int i = 0; i < CDataCount; ++i)
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                m_list[i]->Next();
            }

            // do mix
            ++mixCounter;
            if (mixCounter == CBlockSize)
            {
                core.DoMixCycle();
                mixCounter = 0;
            }
        }

        // verify output data size
        {
            int totalOutput = 0;

            for(int i = 0; i < m_list.Size(); ++i)
            {
                totalOutput += m_list[i]->OutputSamples();
            }

            int avgPerPoint = totalOutput / m_list.Size();
            TUT_ASSERT( avgPerPoint + 1000 > CDataCount );
        }

        // clear mode
        mode.Clear();
        core.SetMode(mode);

    }

}  // namespace


// --------------------------------------------

namespace 
{

    class SndMixTestGroup : public TestFw::TestGroup<SndMixTestGroup>
    {

        SndMixTestGroup(TestFw::ILauncher &launcher) 
            : TestFw::TestGroup<SndMixTestGroup>(launcher, "SndMix")             
        {
            Add("MixMode",          &MixModeTest::Run);
            Add("MixerCore",        &RunMixerCoreTest);
            Add("CycBuff",          &CycBuffTestNormal);
            Add("CycBuffErrRead",   &CycBuffReadOverrunTest);
            Add("CycBuffErrWrite",  &CycBuffWriteOverrunTest);
        }

    public:

        static void AddToLauncher(TestFw::ILauncher &launcher)
        {
            new SndMixTestGroup(launcher);  // free at launcher
        }
    };

} // namespace 



// --------------------------------------------

namespace SndMix
{

    void RunMixerTest()
    {
        MixModeTest::Run();
        RunMixerCoreTest();
    }

    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        SndMixTestGroup::AddToLauncher(launcher);
    }

}  // namespace SndMix
