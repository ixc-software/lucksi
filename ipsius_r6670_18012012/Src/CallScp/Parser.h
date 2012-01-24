#pragma once

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringList.h"
#include "Utils/FullEnumResolver.h"
#include "Utils/ComparableT.h"
#include "Utils/ManagedList.h"

#include "Domain/Identificator.h"
#include "DRI/DRICommandStreamParser.h"
#include "dri/drifileload.h"


namespace CallScp
{
    using boost::shared_ptr;
    using Utils::ManagedList;

    class BasicCommand
    {
        QString m_cmd; 
        QStringList m_args; 
        QString m_locationInfo;

    public:

        BasicCommand(QString cmd, QStringList args, QString locationInfo) :
          m_cmd(cmd), m_args(args), m_locationInfo(locationInfo)
        {
        }

        QString Info() const
        {
            return m_cmd + " " + m_args.join(" ");
        }

    };

    // ----------------------------------------------------------

    template<class TCmd>
    class CmdStream
    {
        QString m_className, m_eventName;
        shared_ptr< ManagedList<TCmd> > m_list;

    public:

        CmdStream() { }

        CmdStream(const QString &className, const QString &eventName, shared_ptr< ManagedList<TCmd> > list) : 
            m_className(className), m_eventName(eventName), m_list(list) 
        { 
            ESS_ASSERT(m_className.length() && m_eventName.length());
            ESS_ASSERT( !list->IsEmpty() );           
        }
        
        CmdStream(const CmdStream<TCmd> &src, int insertPos, const CmdStream<TCmd> &insertCmd) : 
            m_className(src.m_className), m_eventName(src.m_eventName)
        {
            m_list.reset( new ManagedList<TCmd>() );

            // src[0 .. insertPos)
            for(int i = 0; i < insertPos; ++i)
            {
                m_list->Add( new TCmd(src[i]) );
            }

            // insertCmd[..]
            for(int i = 0; i < insertCmd.Size(); ++i)
            {
                m_list->Add( new TCmd(insertCmd[i]) );
            }

            // src[insertPos .. ]
            for(int i = insertPos; i < src.Size(); ++i)
            {
                m_list->Add( new TCmd(src[i]) );
            }
        } 

        bool Equal(const QString &className, const QString &eventName) const
        {
            return (m_className == className) && (m_eventName == eventName);
        }

        const QString& ClassName() const { return m_className; }
        const QString& EventName() const { return m_eventName; }

        bool Empty() const { return Size() == 0; }

        int Size() const
        {
            return m_list ? m_list->Size() : 0;
        }

        const TCmd& operator[](int index) const
        {
            ESS_ASSERT(m_list);

            return *((*m_list)[index]);
        }

        QString StreamHeader() const
        {
            return m_className + " " + m_eventName;
        }

        QStringList Info() const
        {
            QStringList res;

            if (m_list == 0) 
            {
                res.append( "(null)" );
                return res;
            }

            res.append(StreamHeader() + ":");

            for(int i = 0; i < Size(); ++i)
            {
                res.append( "    " + (*this)[i].Info() );
            }

            return res;
        }
    };

    // ----------------------------------------------------------

    template<class TCmd>
    class IScpTree : public Utils::IBasicInterface
    {
    public:
        virtual QStringList EnumerateClasses() const = 0;
        virtual QStringList EnumerateEvents(const QString &className) const = 0;
        virtual CmdStream<TCmd> FindCmdStream(const QString &className, const QString &eventName) const = 0;
        virtual QString Info() const = 0;
    };

    // ----------------------------------------------------------

    template<class TCmd>
    class ICmdMaker : public Utils::IBasicInterface
    {
    public:
        virtual bool VerifyEventName(const QString &className, const QString &eventName, /* out */ QString &error) = 0; 
        virtual TCmd* CreateCmd(const QString &cmd, const QStringList &args, const QString &locationInfo) = 0;   // can throw or return 0
    };

    // ----------------------------------------------------------

    template<class TCmd>
    class Parser
    {
        enum { CTypeCheck = boost::is_base_of<BasicCommand, TCmd>::value };
        BOOST_STATIC_ASSERT( CTypeCheck );

        typedef QList< CmdStream<TCmd> > CmdStreamList;
        typedef DRI::DRIFileLoad::RawCommand RawCmd;

        static int FindInList(const CmdStreamList &list, const QString &className, const QString &eventName)
        {
            for(int i = 0; i < list.size(); ++i)
            {
                if ( list.at(i).Equal(className, eventName) ) return i;
            }

            return -1;
        }

        // IScpTree impl for Map
        class TreeStore : public IScpTree<TCmd>
        {
            typedef TreeStore T;

            CmdStreamList m_list;

            static void SelectClasses(QStringList &res, const CmdStream<TCmd> &current)
            {
                QString className = current.ClassName();
                if ( res.contains(className) ) return;
                res.push_back(className);
            }

            static void SelectEvents(QStringList &res, QString className, const CmdStream<TCmd> &current)
            {
                if (current.ClassName() != className) return;
                res.push_back( current.EventName() );
            }

        // IScpTree impl
        private:

            QStringList EnumerateClasses() const
            {
                QStringList res;

                std::for_each(m_list.begin(), m_list.end(), 
                    boost::bind(&T::SelectClasses, boost::ref(res), _1) );

                return res;
            }

            QStringList EnumerateEvents(const QString &className) const 
            {
                QStringList res;

                std::for_each(m_list.begin(), m_list.end(), 
                    boost::bind(&T::SelectEvents, boost::ref(res), className, _1) );

                return res;
            }

            CmdStream<TCmd> FindCmdStream(const QString &className, const QString &eventName) const
            {
                int i = Parser<TCmd>::FindInList(m_list, className, eventName);
                
                return (i >= 0) ? m_list.at(i) : CmdStream<TCmd>();
            }

            QString Info() const
            {       
                QStringList sl;

                CmdStreamList::const_iterator i = m_list.begin();

                while( i != m_list.end() )
                {
                    sl += i->Info();

                    ++i;
                }

                return sl.join("\n");
            }

        public:

            TreeStore(const CmdStreamList &list) : m_list(list)
            {
            }

        };

        class TreeParser
        {

            ESS_TYPEDEF(Error);

            static void ThrowError(QString msg, QString location)
            {
                ESS_THROW_MSG( Error, QString("%1 at %2").arg(msg).arg(location).toStdString() );  
            }

            static void VerifyIdentificator(QString i, QString errorMsg, QString location)
            {
                if ( i.isEmpty() || !Domain::Identificator::CheckObjectName(i) )
                {
                    ThrowError(errorMsg, location);
                }
            }

            static QStringList TrimStringList(const QStringList &sl)
            {
                QStringList res;

                for(int i = 0; i < sl.size(); ++i)
                {
                    res.push_back( sl.at(i).trimmed() );
                }

                return res;
            }

            static QString TryParseClassName(const RawCmd &rawCmd)
            {
                const QString CClass = "class";

                QString cmd = rawCmd.get();

                bool ok = cmd.startsWith(CClass + " ") && cmd.endsWith(":");
                if (!ok) return "";

                cmd = cmd.mid(CClass.length(), cmd.length() - CClass.length() - 1).trimmed();  // cut 'class' + ':' at end

                VerifyIdentificator( cmd, "Bad class name " + cmd, rawCmd.Location() );

                return cmd;
            }

            static QString TryParseEventName(const RawCmd &rawCmd)
            {
                QString cmd = rawCmd.get();

                if ( !cmd.endsWith(":") ) return "";

                cmd = cmd.mid(0, cmd.length() - 1);

                VerifyIdentificator(cmd, "Bad event name " + cmd, rawCmd.Location() );

                return cmd;
            }

            //static void ParseCmd(const RawCmd &rawCmd, 
            //        /* out */ QString &cmd, QStringList &args)
            //{
            //    // TODO -- fix requered, space inside quotes don't detected 
            //    QStringList sl = rawCmd.get().split(" ", QString::SkipEmptyParts);

            //    if ( sl.isEmpty() ) ThrowError( "Empty command", rawCmd.Location() );

            //    cmd = sl.at(0).trimmed();
            //    VerifyIdentificator( cmd, "Bad command " + cmd, rawCmd.Location() );

            //    args = TrimStringList( sl.mid(1) );
            //}

            static bool DoParseCmd(const QString &commandStr, /* out */ QString &cmd, QStringList &args)
            {
                using namespace DRI;

                DRICommandStreamParser p;
                p.AddData(commandStr + "\n");

                if ( !p.HasParsedCmd() ) return false;

                boost::shared_ptr<ParsedDRICmd> c = p.GetParsedCmd();

                if ( p.HasParsedCmd() ) return false;
                if (c->getActionType() == ParsedDRICmd::PropertyWrite) return false;

                cmd  = c->getMethodOrPropertyName();
                args = c->getParams();

                return true;
            }

            // this code is unstable 'couse it's using DRICommandStreamParser
            static void ParseCmd(const RawCmd &rawCmd, 
                /* out */ QString &cmd, QStringList &args)
            {
                try
                {
                    if ( !DoParseCmd(rawCmd.get(), cmd, args) )
                    {
                        ThrowError("Can't parse cmd!", rawCmd.Location());
                    }
                }
                catch (const ESS::Exception &e)
                {
                    ThrowError( QString("Can't parse cmd! %1").arg(e.getTextMessage().c_str()), rawCmd.Location() );
                }
            }


        private:

            enum State 
            { 
                stWaitClass, 
                stWaitEventOrClass, 
                stWaitCmdOrEventOrClass,
            };

            CmdStreamList m_list;
            State m_state;

            QString m_currClass, m_currEvent;
            shared_ptr< ManagedList<TCmd> > m_currList;

            void SetClass(QString className)
            {
                ESS_ASSERT( !className.isEmpty() );

                TryCloseCurrList();

                m_currClass = className;
                m_currEvent = "";
                m_state = stWaitEventOrClass;
            }

            void SetEvent(QString eventName, ICmdMaker<TCmd> &maker, QString location)
            {
                ESS_ASSERT( !eventName.isEmpty() );
                ESS_ASSERT( !m_currClass.isEmpty() );

                TryCloseCurrList();

                QString error;
                if ( !maker.VerifyEventName(m_currClass, eventName, error) )
                {
                    ThrowError( 
                        QString("Bad event name %1 for class %2 (%3)").arg(eventName).arg(m_currClass).arg(error), 
                        location );
                }
            
                m_currEvent = eventName;
                m_state = stWaitCmdOrEventOrClass;
            }

            void AddCommand(TCmd *p, QString location)
            {
                ESS_ASSERT( !m_currClass.isEmpty() && !m_currEvent.isEmpty() );

                if (m_currList == 0) 
                {
                    m_currList.reset( new ManagedList<TCmd>() );
                }

                m_currList->Add(p);
            }

            void TryCloseCurrList()
            {
                if (m_currList == 0) return;

                ESS_ASSERT( !m_currClass.isEmpty() && !m_currEvent.isEmpty() );
                ESS_ASSERT(m_currList->Size() > 0);

                int i = Parser<TCmd>::FindInList(m_list, m_currClass, m_currEvent);
                if (i >= 0) 
                {
                    ThrowError(
                        QString("Dublicate class %1 event %2").arg(m_currClass).arg(m_currEvent), 
                        "(undefined)" );
                }

                m_list.append( CmdStream<TCmd>(m_currClass, m_currEvent, m_currList) );

                m_currList.reset();
            }

        public:

            TreeParser(const std::vector<RawCmd> &cmds, ICmdMaker<TCmd> &maker)
            {
                m_state = stWaitClass;

                for(int i = 0; i < cmds.size(); ++i)
                {
                    RawCmd c = cmds.at(i);

                    // try parse class name (any state)
                    {
                        QString className = TryParseClassName(c);

                        if ( !className.isEmpty() )                            
                        {
                            SetClass(className);
                            continue;
                        }
                    }

                    if (m_state == stWaitClass) 
                        ThrowError( "Wait class", c.Location() );

                    // try parse event in (stWaitEventOrClass, stWaitCmdOrEventOrClass)
                    {
                        QString eventName = TryParseEventName(c);
                        
                        if ( !eventName.isEmpty() )
                        {
                            SetEvent( eventName, maker, c.Location() );
                            continue;
                        }
                    }

                    if (m_state == stWaitEventOrClass) 
                        ThrowError( "Wait class or event", c.Location() );

                    // try parse cmd in stWaitCmdOrEventOrClass
                    ESS_ASSERT(m_state = stWaitCmdOrEventOrClass);

                    {
                        QString cmd;
                        QStringList args;
                        ParseCmd(c, cmd, args);

                        TCmd *pCmd = 0;
                        QString extErrorMsg;
                        try
                        {
                            pCmd = maker.CreateCmd( cmd, args, c.Location() );
                        }
                        catch(const ESS::Exception &e)
                        {
                            extErrorMsg = e.getTextMessage().c_str();
                        }

                        if (pCmd == 0) 
                        {
                            QString msg = QString("Can't create cmd '%1' args [%2]; reason (%3)")
                                .arg(cmd).arg(args.join(", ")).arg(extErrorMsg);
                            ThrowError( msg, c.Location() );
                        }

                        AddCommand( pCmd, c.Location() );
                    }
                }           // for_each(cmds)

                TryCloseCurrList();
            }

            CmdStreamList Result() const { return m_list; }

        };

    public:
        
        static IScpTree<TCmd>* CreateTree(const Utils::StringList &scp, ICmdMaker<TCmd> &maker)  // can throw
        {
            using namespace DRI;

            std::vector<RawCmd> cmds;
            DRIFileLoad::Parse(scp, DriIncludeDirList(), QStringList(), QString(), cmds);

            TreeParser p(cmds, maker);
            
            return new TreeStore( p.Result() );
        }

    };
    
        
}  // namespace CallScp