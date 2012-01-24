#pragma once 

#include "Utils/ErrorsSubsystem.h"

namespace iReg
{

    struct Settings
    {
        QString Dir;            
        int MaxChunkSizeKb;
        int MaxChunks;
        QString FilenameTypePrefix;     // name: [prefix]_[chunk number]_[datetime]_[extra info].txt
        QString FilenameExtraInfo;
        bool FilenameAddDatetime;
        QString Codec;
        bool SplitOnEveryDay;
        bool TimestampForRecords;

        Settings(const QString &dir, const QString &filenameTypePrefix) :
            Dir(dir), FilenameTypePrefix(filenameTypePrefix)
        {
            ESS_ASSERT( Dir.size() && FilenameTypePrefix.size() );

            MaxChunkSizeKb = -1;
            MaxChunks = -1;
            FilenameAddDatetime = true;
            SplitOnEveryDay = true;
            TimestampForRecords = true;
        }
    };

    ESS_TYPEDEF(Exception);

    // -------------------------------------------------------------
    
    class RegDirection : boost::noncopyable
    {

        class FileList
        {
            typedef std::pair<QString, int> Pair;  // filename, chunkNumber

            const Settings &m_settings;

            QList<Pair> m_files;

            static QString GetDatetime()
            {                
                return QDateTime::currentDateTime().toString("yyMMdd hhmm");
            }

        public:

            FileList(const Settings &settings) : m_settings(settings)
            {
                QDir d(m_settings.Dir);

                if ( !d.exists() ) 
                {
                    ESS_THROW_MSG( Exception, "Dir not exists " + m_settings.Dir.toStdString() );
                }

                QStringList names;
                names << (m_settings.FilenameTypePrefix + "_*.txt");
                QStringList fl = d.entryList(names, QDir::Files, QDir::Name);

                for(int i = 0; i < fl.size(); ++i)
                {
                    QString s = fl.at(i);
                    int n = s.indexOf("_");
                    if (n < 0) continue;
                    s = s.mid(n + 1);

                    n = s.indexOf("_");
                    if (n < 0) continue;
                    s = s.mid(0, n);

                    bool ok;
                    int val = s.toUInt(&ok);
                    if (!ok || (val < 0)) continue;

                    m_files.push_back( Pair( fl.at(i), val ) );
                }

            }

            void Cleanup()
            {
                if (m_settings.MaxChunks <= 0) return;

                // m_files are ordered, so we should delete first N items
                while(m_files.size() > m_settings.MaxChunks)
                {
                    Pair p = m_files.at(0);
                    m_files.pop_front();

                    QString name = QDir(m_settings.Dir).absoluteFilePath( p.first );                    
                    bool ok = QFile::remove(name);  // don't check result
                }
            }

            QString GetName()
            {
                const int CChunkDigits = 5;

                int chunkNum = 0;
                if ( m_files.size() > 0 ) chunkNum = m_files.back().second + 1;

                QString dt;
                if (m_settings.FilenameAddDatetime) 
                {
                    dt = "_" + GetDatetime();
                }

                QString postfix;
                if (m_settings.FilenameExtraInfo.size()) 
                {
                    postfix = "_" + m_settings.FilenameExtraInfo;
                }

                QString name = QString("%1_%2%3%4.txt")
                    .arg(m_settings.FilenameTypePrefix)
                    .arg(chunkNum, CChunkDigits, 10, QChar('0'))
                    .arg(dt)
                    .arg(postfix);

                return QDir(m_settings.Dir).absoluteFilePath(name);
            }

        };

        
        class Stream
        {
            QDateTime m_started;
            QFile m_file;
            QTextStream m_stream;

        public:

            Stream(QString fileName, QString codec) : 
              m_started( QDateTime::currentDateTime() ),
              m_file(fileName)
            {
                if ( !m_file.open(QIODevice::Truncate | QIODevice::WriteOnly) )
                {
                    ESS_THROW_MSG( Exception, "Can't create file " + fileName.toStdString() );
                }

                m_stream.setDevice(&m_file);

                if ( !codec.isEmpty() )
                {
                    QTextCodec *pC = QTextCodec::codecForName( codec.toStdString().c_str() );
                    if (pC == 0) 
                    {
                        ESS_THROW_MSG( Exception, "Can't find codec " + codec.toStdString() );
                    }

                    m_stream.setCodec(pC);
                }

            }

            void Write(const QString &data) 
            {
                m_stream << data;
            }

            int Pos() const { return m_stream.pos(); }
            QDateTime Started() const { return m_started; }

        };

        const Settings m_settings;
        boost::scoped_ptr<Stream> m_file;

        void StartNewChunk()
        {
            m_file.reset(); // close previous file

            FileList fl(m_settings);

            fl.Cleanup();

            QString fileName = fl.GetName();            
            m_file.reset( new Stream(fileName, m_settings.Codec) );
        }

    public:
        
        RegDirection(const Settings &settings) : m_settings(settings)
        {
            StartNewChunk();
        }

        void AddRec(const QString &data, bool lf = true)
        {
            QDateTime now = QDateTime::currentDateTime();

            // write record
            if (m_settings.TimestampForRecords)
            {
                m_file->Write( now.toString("[hh:mm:ss] ") );
            }

            m_file->Write(data);

            if (lf) m_file->Write("\n");

            // split check
            bool splitReq = false;

            if ( m_settings.SplitOnEveryDay && 
                now.date().day() != m_file->Started().date().day() )
            {
                splitReq = true;
            }

            if ( (m_settings.MaxChunkSizeKb > 0) && 
                (m_file->Pos() >= m_settings.MaxChunkSizeKb * 1024) )
            {
                splitReq = true;
            }

            if (splitReq) StartNewChunk();
        }


    };
    
    
    
}  // namespace iReg