#ifndef __TRANSACTIONDRI__
#define __TRANSACTIONDRI__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "iCore/MsgObject.h"
#include "Domain/IDomain.h"

#include "CommandDRI.h"
#include "ITransToSession.h"

namespace DRI
{
    using boost::shared_ptr;

    class TransactionDRI;

    // --------------------------------------------------------------

    // транзакция -- неделимый набор DRI команд
    class TransactionDRI : 
        public iCore::MsgObject,
        boost::noncopyable
    {
        typedef std::queue< shared_ptr<CommandDRI> > CmdList;

        Domain::IDomain &m_domain;
        ITransToSession &m_owner;

        CmdList m_commands;
        shared_ptr<CommandDRI> m_currCmd;  // current command
        bool m_continueOnError;

        void NextCmd()
        {
            if (m_commands.size() != 0)
            {
                m_currCmd = m_commands.front();
                m_commands.pop();

                m_owner.ExecuteCommand(m_currCmd);
            }
            else
            {
                m_owner.TransactionCompleted();
            }
        }

        void AbortTransaction()
        {
            while(!m_commands.empty())
            {
                m_owner.TransactionAbortedCommand(m_commands.front());
                m_commands.pop();
            }

            m_owner.TransactionCompleted();
        }
        
    public:        
        
        TransactionDRI(Domain::IDomain &domain, ITransToSession &owner);

        ~TransactionDRI();

        void Add(shared_ptr<CommandDRI> cmd)
        {
            m_commands.push(cmd);
        }

        void Execute()
        {
            NextCmd();
        }

        shared_ptr<CommandDRI> StartedCommand()
        {
            ESS_ASSERT(m_currCmd != 0);
            return m_currCmd;
        }

        shared_ptr<CommandDRI> CommandCompleted(bool abort)
        {
            // clear current command
            ESS_ASSERT(m_currCmd != 0);
            shared_ptr<CommandDRI> cmd = m_currCmd;
            m_currCmd.reset();

            // command result
            ESS_ASSERT(cmd->Completed());
            if ((cmd->Result()->OK() || m_continueOnError) && !abort)
            {
                // async peek next command
                PutMsg(this, &TransactionDRI::NextCmd);
            }
            else
            {
                PutMsg(this, &TransactionDRI::AbortTransaction);
            }

            // return completed command
            return cmd;
        }

        void OutputToCurrentCommand(const QString &msg)
        {
            ESS_ASSERT(m_currCmd != 0);

            DRI::ICmdOutput *p = m_currCmd.get();
            p->Add(msg, true);
        }

        void ContinueOnError(bool val) { m_continueOnError = val; }

        QString Info() const
        {
            if (m_currCmd == 0) return "";
            QString s = m_currCmd->CommandToString(); 

            int duration = m_currCmd->DurationMin();
            if (duration >= 0)
            {
                s += QString(" [%1 min]").arg(duration);
            }

            return s;
        }

        void TryFinalize(bool haltOnFail)
        {
            if (m_currCmd == 0) return;
            m_currCmd->TryComplete(haltOnFail);
        }

    };
    
}  // namespace DRI

#endif
