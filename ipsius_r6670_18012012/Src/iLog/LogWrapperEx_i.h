#ifndef __LOGWRAPPEREX_I__
#define __LOGWRAPPEREX_I__

namespace iLogW
{

    inline LogSession& operator<<(LogSession &session, const QString &s)
    {
        session << s.toStdWString();

        return session;
    }
    
}  // namespace iLogW

#endif
