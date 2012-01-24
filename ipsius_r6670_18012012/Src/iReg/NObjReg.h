#pragma once

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "Utils/ExeName.h"

#include "ISysReg.h"
#include "RegDirection.h"

namespace iReg
{
    using boost::scoped_ptr;
    
    class NObjReg : 
        public Domain::NamedObject, 
        public DRI::INonCreatable,
        public ISysReg
    {
        Q_OBJECT;

        class Wrapper : public IDriSessionLog
        {
            RegDirection m_dir;

            // stats
            int m_writes;
            int m_totalChars;

        // IDriSessionLog impl
        private:

            void Add(const QString &data)
            {
                ++m_writes;
                m_totalChars += data.length();

                m_dir.AddRec(data, true);
            }

        public:

            Wrapper(const Settings &s) : 
              m_dir(s), m_writes(0), m_totalChars(0)
            {
            }

            ~Wrapper()
            {
                // nothing 
            }
        };

        enum
        {
            CMaxChunks          = 8,
            CMaxChunkSizeKb     = 256,
        };

        QString m_dir;

        scoped_ptr<RegDirection> m_calls;
        scoped_ptr<RegDirection> m_events;
        bool m_regTelnet;

        void CheckDir()
        {
            QDir d(m_dir);

            if ( d.exists() ) return;

            if ( !d.mkpath( d.absolutePath() ) )
            {
                std::string path = d.absolutePath().toStdString();
                ESS_THROW_MSG(ESS::Exception, "Can't create dir " + path);
            }
        }

        void SetDir(QString val)
        {
            // TODO -- check all regs must be disabled - ?!
            // ... 

            m_dir = val;
        }

        void FillSettings(Settings &s)
        {
            s.MaxChunkSizeKb = CMaxChunkSizeKb;
            s.MaxChunks = CMaxChunks;
            s.FilenameAddDatetime = true;
            s.Codec = "utf-8";
            s.SplitOnEveryDay = true;
            s.TimestampForRecords = true;
        }

        bool GetRegCalls() const { return (m_calls != 0); }

        void SetRegCalls(bool val)
        {
            if ( val == GetRegCalls() ) return;

            if (!val) 
            {
                m_calls.reset();
                return;
            }

            CheckDir();

            Settings prof(m_dir, "calls");
            FillSettings(prof);

            m_calls.reset( new RegDirection(prof) );
        }

        bool GetRegEvents() const { return (m_events != 0); }

        void SetRegEvents(bool val)
        {
            if ( val == GetRegEvents() ) return;

            if (!val) 
            {
                m_events.reset();
                return;
            }

            CheckDir();

            Settings prof(m_dir, "events");
            FillSettings(prof);

            m_events.reset( new RegDirection(prof) );
        }

        void SetRegTelnet(bool val)
        {
            m_regTelnet = val;

            if (m_regTelnet) CheckDir();
        }

        void TelnetTestLoop()
        {
            SetRegTelnet(true);

            const int CLoops = CMaxChunks + 4;
            const int CLargeIter = CLoops - 2;
            const int CLargeWrites = (CMaxChunkSizeKb * 1024) / 10;

            for(int i = 0; i < CLoops; ++i)
            {
                scoped_ptr<IDriSessionLog> log( CreateDriSessionLog( QString("login%1").arg(i) ) );
                ESS_ASSERT(log);

                int writes = (i == CLargeIter) ? CLargeWrites : 4;

                for(int j = 0; j < writes; ++j)
                {
                    log->Add( QString("This is test record number %1; iter %2 ").arg(j).arg(i) );
                }

            }
        }


    // ISysReg impl
    private:

        void CallInfo(const Domain::ObjectName &sender, 
            const CallRecord &rec)
        {
            if ( !GetRegCalls() ) return;

            m_calls->AddRec( QString("%1: %2").arg(sender.Name(), rec.ToString()) , true );
        }

        void EventInfo(const Domain::ObjectName &sender, const QString &msg)
        {
            if ( !GetRegEvents() ) return;

            m_events->AddRec( QString("%1: %2").arg(sender.Name(), msg), true );
        }

        IDriSessionLog* CreateDriSessionLog(const QString &login)
        {
            if (!m_regTelnet) return 0;

            Settings prof(m_dir, "telnet");
            FillSettings(prof);

            prof.TimestampForRecords = true;  // ?
            prof.FilenameExtraInfo = login;

            return new Wrapper(prof);
        }

    public:

        NObjReg(Domain::NamedObject *pParent) : 
          Domain::NamedObject(&pParent->getDomain(), "Reg", pParent),
		  m_regTelnet(false)
        {
            m_dir = (Utils::ExeName::GetExeDir() + "reg").c_str();
        }

        Q_PROPERTY(QString Dir READ m_dir WRITE SetDir);

        Q_PROPERTY(bool RegCalls  READ GetRegCalls  WRITE SetRegCalls);
        Q_PROPERTY(bool RegEvents READ GetRegEvents WRITE SetRegEvents);
        Q_PROPERTY(bool RegTelnet READ m_regTelnet  WRITE SetRegTelnet); 

        Q_INVOKABLE void RunSelfTest()
        {
            bool currTelnet = m_regTelnet;

            TelnetTestLoop();

            SetRegTelnet(currTelnet);
        }

    };
    
    
}  // namespace iReg