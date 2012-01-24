#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "Utils/TimerTicks.h"
#include "ISbpConnection.h"
#include "SafeBiProto/SbpError.h"
#include "SbpConnectionList.h"

namespace {
	const int CWaitFirstEvent = 1000;
	const int CWaitBeforeDelete = 1000;
};

namespace SBProtoExt
{
	using namespace SBProto;

	class SbpConnectionList::Item : boost::noncopyable,
		public iCore::MsgObject,
		public ISbpConnectionEvents,
		public ISbpConnection
	{
		typedef Item T;
	public:
		Item(SbpConnectionList &owner, 
			 Utils::SafeRef<ISbpConnectionEvents> user,
			 boost::shared_ptr<ISbpConnection> connection) :
			iCore::MsgObject(owner.getMsgThread()),
			m_owner(owner),
            m_connection(connection),
            m_user(user)
		{
			m_firstEventTimer.Set(CWaitFirstEvent, false);
			m_timeBeforeDelete = CWaitBeforeDelete;
			m_connection->BindUser(this);
			m_owner.Add(this);
		}
		~Item()
		{
		
		}
	// SBProto::ISbpConnection
	private:			
		void Process()
		{
			if(m_deleteTimer.TimeOut())
			{
				Delete();
				return;
			}
			if(m_firstEventTimer.TimeOut() 	&& !m_user.IsEmpty())
			{
				boost::shared_ptr<SbpError> err(new SbpError("Timeout."));
				m_user->ProtocolError(this, err);
			}
            if(m_connection != 0) m_connection->Process();
		}

		void BindUser(Utils::SafeRef<ISbpConnectionEvents> user)
		{
			ESS_ASSERT(m_user.IsEmpty());

			ESS_HALT("Unexpected");
		}

		void UnbindUser()
		{
			if(!m_user.IsEmpty()) m_user.Clear();

			if(m_connection == 0 || m_timeBeforeDelete == 0)
			{
				Delete();
				return;
			}
			m_firstEventTimer.Stop();
			m_deleteTimer.Set(m_timeBeforeDelete, false);			
		}

		void ActivateConnection()
		{
			ESS_ASSERT(!m_user.IsEmpty());

			m_connection->ActivateConnection();
		}

		void DeactivateConnection()
		{
			ESS_ASSERT(!m_user.IsEmpty());

			m_connection->DeactivateConnection();
		}

		SBProto::ISafeBiProtoForSendPack &Proto()
		{
			return m_connection->Proto();
		}

		std::string TransportInfo() const
		{
			return m_connection->TransportInfo();
		}

		std::string Name() const
		{
			return m_connection->Name();
		}
	// SBProto::ISbpConnectionEvents	
	private:
		void CommandReceived(Utils::SafeRef<ISbpConnection> src,
			boost::shared_ptr<SbpRecvPack> data)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));

			if(m_user.IsEmpty()) return;

			m_firstEventTimer.Stop();
			m_user->CommandReceived(this, data);

		}
		void ResponseReceived(Utils::SafeRef<ISbpConnection> src,
			boost::shared_ptr<SbpRecvPack> data)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));
			
			if(m_user.IsEmpty()) return;

			m_firstEventTimer.Stop();
			m_user->ResponseReceived(this, data);
		}
		void InfoReceived(Utils::SafeRef<ISbpConnection> src,
			boost::shared_ptr<SbpRecvPack> data)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));

			if(m_user.IsEmpty()) return;

			m_firstEventTimer.Stop();
			m_user->InfoReceived(this, data);
						
		}

		void ProtocolError(Utils::SafeRef<ISbpConnection> src,
			boost::shared_ptr<SbpError> err)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));

			if(m_user.IsEmpty()) return;
			
			m_firstEventTimer.Stop();
			m_user->ProtocolError(this, err);
		}

		void ConnectionActivated(Utils::SafeRef<ISbpConnection> src)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));
		}

		void ConnectionDeactivated(Utils::SafeRef<ISbpConnection> src,
			const std::string &err)
		{
			ESS_ASSERT(src.IsEqualIntf(m_connection.get()));

			if(!m_user.IsEmpty())
            {
			    m_firstEventTimer.Stop();
			    m_user->ConnectionDeactivated(this, err);
            }

			DeleteConnection();
		}
	private:
		void DeleteConnection()
		{
			if(m_connection == 0) return;
			
			m_connection->UnbindUser();
			PutMsg(this, &T::onDeleteConnection, m_connection);
			m_connection.reset();
		}

		void Delete()
		{
			DeleteConnection();
			m_owner.Delete(this);
		}

		void onDeleteConnection(boost::shared_ptr<ISbpConnection> connection)
		{}
	private:
		SbpConnectionList &m_owner;
		boost::shared_ptr<ISbpConnection> m_connection;
		Utils::SafeRef<ISbpConnectionEvents> m_user;
		int m_timeBeforeDelete;
		Utils::TimerTicks m_firstEventTimer;
		Utils::TimerTicks m_deleteTimer;
	};

	// ----------------------------------------------------------------------------

	SbpConnectionList::SbpConnectionList(iCore::MsgThread &thread, Utils::SafeRef<ISbpConnectionEvents> user) : 
		iCore::MsgObject(thread),		  
		m_user(user)
	{}

	// ----------------------------------------------------------------------------

	void SbpConnectionList::Process()
	{
		for (int i = 0; i < m_list.Size(); ++i)
		{
			m_list[i]->Process();
		}
	}

	// ----------------------------------------------------------------------------

	void SbpConnectionList::Push(boost::shared_ptr<ISbpConnection> connection)
	{
		Item *item = new Item(*this, m_user, connection);
	}

	// ----------------------------------------------------------------------------

	void SbpConnectionList::Add(ISbpConnection *item)
	{
		m_list.Add(item);
	}

	// ----------------------------------------------------------------------------

	void SbpConnectionList::Delete(ISbpConnection *item)
	{
		int i = m_list.Find(item);
		ESS_ASSERT(i != -1);
		m_list.Detach(i);
		PutMsg(this, &T::onDelete, item);
	}

	// ----------------------------------------------------------------------------

	void SbpConnectionList::onDelete(ISbpConnection *item)
	{
	}

};
