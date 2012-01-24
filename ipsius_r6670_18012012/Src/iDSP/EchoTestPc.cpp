#include "stdafx.h"

#include "Utils/MsWave.h"
#include "Utils/StringList.h"

#include "iDSP/EchoTestPc.h"
#include "iDSP/EchoCancel.h"

// -------------------------------------------------

namespace
{
    using namespace iDSP;
    using Utils::MsWaveReader;
    using Utils::MsWaveWriter;
    using Utils::MsWaveBase;

    void ReadWav(QString fileName, std::vector<int> &res)
    {
        MsWaveReader wave(fileName);

        MsWaveReader::Iterator i(wave, 0);

        res.clear();

        while(!i.Eof())
        {
            res.push_back( i.Get() );

            i.Next();
        }
    }

    // -----------------------------------------------

    void WriteWav(QString fileName, std::vector<int> &data)
    {
        const int CSampleRate = 8000;

        QByteArray wavOut = MsWaveWriter::MakeWavStreamOneCh(MsWaveBase::Pcm, CSampleRate, data);

        QFile f(fileName);
        ESS_ASSERT( f.open(QIODevice::WriteOnly | QIODevice::Truncate) );
        f.write(wavOut);
    }

    // -----------------------------------------------

    QString VectorToQString(std::vector<int> &v)
    {
        QString res;

        for(int i = 0; i < v.size(); ++i)
        {
            res += QString::number( v.at(i) );
            res += "\t";
        }

        return res;
    }

    // -----------------------------------------------

    void AddCoef(EchoCancel &echo, int step, Utils::StringList &sl)
    {
        std::vector<int> foreground;
        std::vector<int> background;

        echo.GetFirCoeffs(foreground, background);

        sl.append(QString("Step %1").arg(step));
        sl.append( VectorToQString(foreground) );
        sl.append( VectorToQString(background) );
        sl.append("");
    }

    // -----------------------------------------------

    void RunEchoLoop(std::vector<int> &dataTx, int tapsSize, QString fileName, bool useCout, 
        int echoShift, int echoDiv, QString coefFileName = "", QString adaptFileName = "")
    {
        const int CCoefDropStep = 100;
        const int CCoefMaxStep  = 8000;

        Utils::StringList sl;

        EchoCancel echo( tapsSize, (adaptFileName.size() > 0) );

        std::vector<int> cleanRx;
        cleanRx.reserve( dataTx.size() );

        int beginT = Platform::GetSystemTickCount();

        bool dropCoef = (coefFileName.length() > 0);

        for(int i = 0; i < dataTx.size(); ++i)
        {
            int tx = dataTx.at(i);

            int rx = 0;
            int rxIndx = i - echoShift;
            if ((rxIndx) >= 0 && (rxIndx < dataTx.size()))
            {
                rx = dataTx.at(rxIndx) / echoDiv;
            }

            int res = echo.Process(tx, rx);

            cleanRx.push_back( res );

            if (dropCoef)
            {
                if (((i % CCoefDropStep) == 0) && (i < CCoefMaxStep)) AddCoef(echo, i, sl);
            }
        }

        int runTime = Platform::GetSystemTickCount() - beginT;
        int samplesPerSec = (dataTx.size() * 1000) / runTime;

        if (useCout)
        {
            std::cout << "Taps " << tapsSize << "; Samples/sec " << samplesPerSec <<
                "; Channels " << (samplesPerSec / 8000) << std::endl;
        }

        // save
        if (fileName.length() > 0) WriteWav(fileName, cleanRx);

        // save coef
        if (dropCoef) sl.SaveToFile(coefFileName);

        // save adapt debug
        if (adaptFileName.size() > 0)
        {
            std::string s;
            echo.AdoptStatToString(s);

            Utils::StringList sl;
            sl.push_back( QString(s.c_str()) );
            sl.SaveToFile(adaptFileName);
        }
    }

    // -----------------------------------------------

    bool VerifyFiles(QString wav0, QString wav1)
    {
        MsWaveReader w0(wav0);
        MsWaveReader::Iterator i0(w0, 0);

        MsWaveReader w1(wav1);        
        MsWaveReader::Iterator i1(w1, 0);

        if (i0.Size() != i1.Size()) return false;

        while(!i0.Eof())
        {
            if (i0.Get() != i1.Get()) return false;

            i0.Next();
            i1.Next();
        }

        return true;
    }


    // -----------------------------------------------

    const QString CPath("C:\\temp2\\echo\\");

    /*
    void MakeRef()
    {
        std::vector<int> sampleTx;

        // load
        ReadWav(CPath + "sample_ref.wav", sampleTx);

        // process
        const int CEchoShift = 50;
        const int CEchoDiv   = 4;

        RunEchoLoop(sampleTx, 32,  CPath + "ref_echo_32.wav",  true, CEchoShift, CEchoDiv);
        RunEchoLoop(sampleTx, 64,  CPath + "ref_echo_64.wav",  true, CEchoShift, CEchoDiv);
        RunEchoLoop(sampleTx, 128, CPath + "ref_echo_128.wav", true, CEchoShift, CEchoDiv);
    } */

    void MakeRef()
    {
        std::vector<int> sampleTx;

        // load
        ReadWav(CPath + "sample_ref.wav", sampleTx);

        // process
        const int CEchoShift = 50;
        const int CEchoDiv   = 4;

        // RunEchoLoop(sampleTx, 32,  CPath + "ref_echo_32.wav",  true, CEchoShift, CEchoDiv);
        // RunEchoLoop(sampleTx, 64,  CPath + "ref_echo_64.wav",  true, CEchoShift, CEchoDiv);
        RunEchoLoop(sampleTx, 128, CPath + "ref_echo_128.wav", true, CEchoShift, CEchoDiv, CPath + "coef128.txt");
    }

    // -----------------------------------------------

    void ProfileRun()
    {
        std::vector<int> sampleTx;

        // load
        ReadWav(CPath + "sample_ref.wav", sampleTx);
        sampleTx.resize(4000);  // cut size

        // process
        const int CEchoShift = 50;
        const int CEchoDiv   = 4;

        RunEchoLoop(sampleTx, 128, CPath + "profile.wav", true, CEchoShift, CEchoDiv, 
            CPath + "profile_coef.txt", CPath + "profile_adopt.txt");
    }

    // -----------------------------------------------

    void DoVerify()
    {
        std::vector<int> sampleTx;
        ReadWav(CPath + "sample_ref.wav", sampleTx);

        // process
        const int CEchoShift = 50;
        const int CEchoDiv   = 4;
        
        QString fileName(CPath + "curr_echo.wav");
        RunEchoLoop(sampleTx, 128, fileName, true, CEchoShift, CEchoDiv, "");

        TUT_ASSERT( VerifyFiles(fileName, CPath + "ref_echo_128.wav") );
    }
    
}  // namespace

// -------------------------------------------------

namespace Echo
{
    
    void RunEchoTestPc()
    {
        // MakeRef();
    
        ProfileRun();
    
        DoVerify();        
    }
    
}  // namespace Echo


