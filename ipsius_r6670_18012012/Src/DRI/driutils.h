#ifndef __DRIUTILS__
#define __DRIUTILS__

#include "Platform/Platform.h"

namespace DRI
{
    
    class DriUtils
    {

    public:

        static std::string RemoveNamespaces(const std::string &s)
        {
            size_t pos = s.rfind("::");
            if (pos == std::string::npos) return s;

            return s.substr(pos + 2);
        }

        static std::string FormatedTypename(const char *pTypeName, bool removeNamespace)
        {
            std::string name( Platform::FormatTypeidName(pTypeName) );

            if (removeNamespace) name = RemoveNamespaces(name);

            return name; 
        }

        
        template<class T>
        static std::string FormatedTypename(bool removeNamespace)
        {
            return FormatedTypename(typeid(T).name(), removeNamespace);
        }

        // must be sync'ed with CommandDRI::ProcessSpecialFirstParam
        // static bool IsSpecialTypeParam(QString typeName);

        template<class T>
        static QString QtTypename(T p = 0)
        {
            // return FormatedTypename(typeid(T).name(), false).c_str();
            
            std::string res = Platform::FormatTypeidName( typeid(T).name() );
            return QString( QMetaObject::normalizedType( res.c_str() ) );
        }

        static std::string MethodName(const QMetaMethod &method)
        {
            QString name( method.signature() );
            int j = name.indexOf("(");
            ESS_ASSERT(j >= 0);
            return name.left(j).trimmed().toStdString();
        }

        /*static QString CommandResultToLine(const QString &text, 
                                           int maxCharLength, 
                                           const QString &addEnding = " ...")
        {
            QString res(text.simplified());
            if (res.size() <= maxCharLength) return res;

            int leftSize = maxCharLength - addEnding.size();
            if (leftSize > 0) res = res.left(leftSize);
            
            res += addEnding;
            
            return res;
        }*/

        // Extract line (with LF consideration), append addEnding to the end
        static QString CommandResultToLine(const QString &text, 
                                           int maxCharLength, 
                                           bool isFullLine,
                                           const QString &addEnding = " ...")
        {
            QString res(text.simplified());
            if (isFullLine && (res.size() <= maxCharLength)) return res;
            if (!isFullLine && ((res.size() + addEnding.size()) < maxCharLength)) return res;

            int leftSize = maxCharLength - addEnding.size();
            if (leftSize > 0) res = res.left(leftSize);
            
            res += addEnding;
            
            return res;
        }

        static QString UnquoteString(const QString &s, const QChar &strChar)
        {
            int size = s.size();
    
            if (size >= 2)
            {
                if ( (s.at(0) == strChar) && (s.at(size - 1) == strChar))
                {
                    return s.mid(1, size - 2);
                }
            }
    
            return s;
        }

        static void AddToStringList(QStringList &sl, const QString &str, bool lineFeed)
        {
            if (lineFeed || (sl.isEmpty()))
            {
                sl.append(str);
            }
            else
            {
                int indx = sl.size() - 1;
                sl[indx] = sl[indx] + str;
            }
        }

        static QString ExceptionToString(const std::exception &e, const QString &addInfo = "")
        {            
            // cast to ESS::BaseException for stack trace info cut
            const ESS::BaseException *pBaseConst = dynamic_cast<const ESS::BaseException*>(&e);
            if (pBaseConst != 0) 
            {
                ESS::BaseException *pBase = const_cast<ESS::BaseException*>(pBaseConst);
                ESS_ASSERT(pBase != 0);
                pBase->NoStackTraceInMsg();
            }
            
            QString res(e.what());
            if (!addInfo.isEmpty()) res += addInfo;
            
            return res;
        }

        static bool CorrectEnumKey(const QMetaEnum &e, const char *pKey)
        {
            ESS_ASSERT(pKey != 0);

            for(int i = 0; i < e.keyCount(); ++i)
            {
                const char *pCurrKey = e.key(i);
                ESS_ASSERT(pCurrKey != 0);
                if (std::strcmp(pKey, pCurrKey) == 0) return true;
            }

            return false;
        }

    };

    static const char *C_LF = "\n"; // "\x0d\x0a";
    
}  // namespace DRI

#endif
