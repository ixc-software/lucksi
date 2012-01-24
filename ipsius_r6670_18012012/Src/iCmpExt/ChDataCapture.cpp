#include "stdafx.h"
#include "ChDataCapture.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ExeName.h"
#include "Utils/MsWave.h"
#include "iDSP/g711codec.h"

namespace iCmpExt
{
    class IStore : public Utils::IBasicInterface
    {
    public:
        ESS_TYPEDEF(Err);
        typedef std::vector<Platform::byte> Buff;
        virtual void Write(const Buff &rx, const Buff &tx, const Buff &rxFixed) = 0;
    };
}

// ------------------------------------------------------------------------------------

namespace 
{   
    using iCmpExt::IStore;

    const char* CPath = "../bin";
    QString PreparePath(QString path = CPath)
    {
        QString exeDir(Utils::ExeName::GetExeDir().c_str());        
        QDir dir((QDir::isAbsolutePath(path)) ? path : exeDir + path);            
        if (!dir.exists()) dir.mkpath(path);   
        return QDir::cleanPath( dir.path() );
    }
    
       

    class StoreBase : public IStore
    {
        bool m_writeBegin;
        QString m_path; // рабочий каталог хранилища
        QString m_id;   // префикс идентификатор
        QFile m_descrFile;
        QByteArray m_descr;

    private:

        static int ExtractId(QString baseName, QString sep)
        {
            QStringList sl = baseName.split(sep);
            if (sl.empty()) return -1;
            bool ok = false;
            int id = sl.at(0).toInt(&ok);
            return ok ? id : -1;
        }
        static QString GenId(QString path)
        {            
            QStringList exts("*.bin, *.wav, *.txt");
            exts.push_back("*.bin");
            exts.push_back("*.wav");
            exts.push_back("*.txt");

            QString sep = "_";

            QFileInfoList files = QDir(path).entryInfoList(exts, QDir::Files);        

            int lastId = -1;
            for(int i = 0; i < files.size(); ++i)
            {
                int id = ExtractId(files.at(i).baseName(), sep);                
                if (lastId < id) lastId = id;
            }

            return QString("%1%2").arg(lastId + 1).arg(sep);
        }            

    private:
        virtual void TryOpenFiles(bool rxx) = 0;
        virtual void WriteData(const Buff &rx, const Buff &tx, const Buff &rxFixed) = 0;
        
    // IStore
    private:
        void Write(const Buff &rx, const Buff &tx, const Buff &rxFixed)
        {
            if ( m_writeBegin )
            {                
                // can throw:
                TryOpenFiles(!rxFixed.empty());
                TryOpenFile(m_descrFile, "This file");
                AddInfo("Start capture time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
                m_writeBegin = false;
            }

            try{ WriteData(rx, tx, rxFixed); }
            catch(const Err&)
            {
                m_descrFile.close();
                throw;
            }
            
        }
    protected:
        StoreBase(QString path, QString generalInfo)
            : m_writeBegin(true),
            m_path(path),
            m_id( GenId(path) ),
            m_descrFile(GenerateFullFileName("descr.txt"))
        {
            AddInfo("GeneralInfo", generalInfo);
        }
        ~StoreBase()
        {
            if (m_descrFile.isOpen()) m_descrFile.write(m_descr.data(), m_descr.size());            
        }
        void AddInfo(QString par, QString val)
        {            
            m_descr.append(par).append(": ").append(val).append("\n");
        }    
        QString GenerateFullFileName(QString shortName) const
        {
            return m_path + "/" + m_id + shortName; // правило формирования имен
        }
        void TryOpenFile(QFile& file, QString description = "")
        {        
            if (!file.open(QIODevice::WriteOnly)) ThrowCantOpen(file.fileName());            

            if (description.isEmpty()) return;
            AddInfo(QFileInfo(file).fileName(), description);
        }

        void ThrowCantOpen(QString file)
        {
            std::string err = "Can`t open file ";
            err.append(file.toStdString());
            ESS_THROW_MSG(IStore::Err, err);
        }
    };
    

    // сохранение потока в сыром виде
    class BinStore : public StoreBase
    {
        QFile m_rx;
        QFile m_tx;
        QFile m_rxx; 

    private:
        static void WriteToFile(QFile& file, const std::vector<Platform::byte> &data)
        {
            if (data.size() == 0) return;
            const char* pData = reinterpret_cast<const char*>(&data.at(0));
            if (file.write(pData, data.size()) != data.size()) ESS_THROW_MSG(IStore::Err, "WriteError"); 
        }
  
    // Base impl:
    private:
        void TryOpenFiles(bool rxx)
        {
            TryOpenFile(m_rx, "Receive data");
            TryOpenFile(m_tx, "Transmit data");                                       
            if (rxx) TryOpenFile(m_rxx, "Echo suppress result");   
        }
        void WriteData(const Buff &rx, const Buff &tx, const Buff &rxFixed)
        {
            WriteToFile(m_rx, rx);
            WriteToFile(m_tx, tx);
            if (!rxFixed.empty()) WriteToFile(m_rxx, rxFixed);     
        }

    public:
        BinStore(QString path, QString info) : StoreBase(path, info)
        {
            m_rx.setFileName(GenerateFullFileName("rx.bin"));
            m_tx.setFileName(GenerateFullFileName("tx.bin"));
            m_rxx.setFileName(GenerateFullFileName("rxx.bin"));
            AddInfo("File format", "binary");
        }
    };

    class WavStore : public StoreBase
    {
        //QFile m_wavFile;   
        QString m_fileName;
        boost::scoped_ptr<Utils::MsWaveWriter> m_wavFile;
        bool m_rxFixedUsed;

    // Base impl:
    private:
        void TryOpenFiles(bool rxx)
        {            
            //TryOpenFile(m_wavFile);                        
            try
            {
                m_wavFile.reset(new Utils::MsWaveWriter(m_fileName, Utils::MsWaveBase::Pcm, 2, 8000));
                QString descr = QString("2 ch wave in Pcm, 1ch - tx, 2ch - %2").arg(rxx ? "rxFized." : "rx.");
                AddInfo(QFileInfo(m_fileName).fileName(), descr);
                m_rxFixedUsed = rxx;
            }
            catch(const Utils::MsWaveWriter::CantOpenFileForWrite& err)
            {
                ThrowCantOpen( m_fileName );
            }
            
        }
        void WriteData(const Buff &rx, const Buff &tx, const Buff &rxFixed)
        {
            if (!m_wavFile) return;           
            const Buff& rxBuff = m_rxFixedUsed ? rxFixed : rx;
            ESS_ASSERT(rxBuff.size() == tx.size());

            iDSP::CodecAlow codec;
            for (int i = 0; i < rxBuff.size(); ++i)
            {
                m_wavFile->Add(codec.Decode(tx.at(i)), codec.Decode(rxBuff.at(i)));
            }
        }

    public:
        WavStore(QString path, QString info) : StoreBase(path, info)
        {
            m_fileName = GenerateFullFileName(".wav");
            m_rxFixedUsed = false;
        }
    };


} // namespace 

// ------------------------------------------------------------------------------------

namespace iCmpExt
{
    
	ChDataCapture::ChDataCapture(const std::string &generalInfo, 
		ChDataCaptureMode::Value mode,
		Utils::WeakRef<IChDataCaptureSender&> sender,
		int channel) :
		m_sender(sender),
		m_channel(channel)
	{	        
		m_sender.Value().SendDataCapture(m_channel, true);

        QString path = PreparePath();
        
        if (mode == ChDataCaptureMode::Bin)
        {
            m_store.reset( new BinStore(path, generalInfo.c_str()) );
        }
        else
        if (mode == ChDataCaptureMode::Wav)
        {
            m_store.reset( new WavStore(path, generalInfo.c_str()) );
        }   
        else
            ESS_HALT("Invalid mode");
    }

	// -------------------------------------------------------------------------------		

	ChDataCapture::~ChDataCapture()
	{                
		m_sender.Value().SendDataCapture(m_channel, false);
    }

	// -------------------------------------------------------------------------------		

	int  ChDataCapture::ChannelNumber() const
	{
		return m_channel;
	}

	// -------------------------------------------------------------------------------		

	void ChDataCapture::Event(const std::vector<Platform::byte> &rx, 
		const std::vector<Platform::byte> &tx, 
		const std::vector<Platform::byte> &rxFixed)
	{       
        if (!m_store) return;

        try{ m_store->Write(rx, tx, rxFixed); }
        catch(const IStore::Err& e)
        {
            //Todo: Log( e.getTextMessage() );
            m_store.reset();
        }
        
	}
}


