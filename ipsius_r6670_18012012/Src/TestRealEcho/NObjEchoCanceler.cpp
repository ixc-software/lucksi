#include "stdafx.h"
#include "NObjEchoCanceler.h"

#include "iDSP/EchoCancel.h"
#include "Utils/MsWave.h"

namespace 
{
    using Utils::MsWaveWriter;
    using Utils::MsWaveBase;
    void WriteWav(QString fileName, const std::vector<int>&data)
    {
        const int CSampleRate = 8000;

        QByteArray wavOut = MsWaveWriter::MakeWavStreamOneCh(MsWaveBase::Pcm, CSampleRate, data);

        QFile f(fileName);
        ESS_ASSERT( f.open(QIODevice::WriteOnly | QIODevice::Truncate) );
        f.write(wavOut);
    }

} // namespace 

namespace TestRealEcho
{

    using boost::scoped_ptr;
    
    Q_INVOKABLE void NObjEchoCanceler::Process( QString txFileName, QString rxFileName, int echoTaps, QString out )
    {        
        // Open input
        QFile txFile(txFileName);
        QFile rxFile(rxFileName);
        scoped_ptr<QTextStream> tsTx(CreateStream(txFile));
        scoped_ptr<QTextStream> tsRx(CreateStream(rxFile));

        // Create canceler
        scoped_ptr<iDSP::EchoCancel> canceler;(echoTaps);
        if (echoTaps > 0) canceler.reset(new iDSP::EchoCancel(echoTaps));

        std::vector<int> resultData;
        
        
        int rx;
        int tx;
        while (!tsRx->atEnd() && !tsTx->atEnd())
        {
            *tsTx >> tx;
            *tsRx >> rx;
            if (canceler) resultData.push_back( canceler->Process(tx, rx) );
            else          resultData.push_back( rx );            

        }

        WriteWav(out, resultData);
    }

    QTextStream* NObjEchoCanceler::CreateStream(QFile& f)
    {                
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) ThrowRuntimeException("Cant in open file.");

        QTextStream* result =new QTextStream(&f);
        result->setCodec("Windows-1251"); // ?
        return result;
    }
} // namespace TestRealEcho
