#ifndef _MEMBER_SOCKET_THREAD_H_
#define _MEMBER_SOCKET_THREAD_H_

class QObject;

namespace iNet
{
    // ѕоток, обслуживающий сетевую подсистему
    class MemberSocketThread : boost::noncopyable
    {
    public:
        MemberSocketThread( QObject &newObject );
        ~MemberSocketThread();
		static void StartNetThreadMon(int timeout);  // throw
		static void StopNetThreadMon();
    };

}  // namespace iCore

#endif

