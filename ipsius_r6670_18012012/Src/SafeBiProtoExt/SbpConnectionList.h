#ifndef _SBP_CONNECTION_LIST_H_
#define _SBP_CONNECTION_LIST_H_

#include "Utils/ManagedList.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

namespace SBProto	
{	
	class ISbpConnection; 
	class ISbpConnectionEvents; 
};

namespace SBProtoExt
{
	class SbpConnectionList : boost::noncopyable,
		public iCore::MsgObject
	{
		typedef SbpConnectionList T;
	public:
		SbpConnectionList(iCore::MsgThread &thread, Utils::SafeRef<ISbpConnectionEvents> user);
		void Process();
		void Push(boost::shared_ptr<ISbpConnection>);
	private:
		void Add(ISbpConnection *item);
		void Delete(ISbpConnection *item);
		void onDelete(ISbpConnection *item);
	private:
		class Item;
		Utils::SafeRef<ISbpConnectionEvents> m_user;
		Utils::ManagedList<ISbpConnection> m_list;
	};
};

#endif
