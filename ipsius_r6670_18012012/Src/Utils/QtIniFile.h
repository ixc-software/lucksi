#ifndef __UTILSQTINIFILE__
#define __UTILSQTINIFILE__

#include "IniFile.h"

namespace Utils
{
    
    class QtIniFile : public IniFile
    {

        class ConvertFunctor
        {
            const QTextCodec *m_pCodec;

        public:

            ConvertFunctor(const char *pEncoding)
            {
                ESS_ASSERT(pEncoding != 0);
                m_pCodec = QTextCodec::codecForName(pEncoding);
                ESS_ASSERT(m_pCodec != 0);                
            }

            std::string operator() (const QString &s) const
            {
                ESS_ASSERT(m_pCodec != 0);

                QByteArray arr = m_pCodec->fromUnicode(s);

                return std::string(arr.data());
            }

        };

    public:
        
        QtIniFile(const QStringList &sl, const IniFileConfig &cfg = IniFileConfig(), 
            const char *pEncoding = "Windows-1251") : 
          IniFile(sl, ConvertFunctor(pEncoding), cfg)
        {
            // ...
        }
            
    };    
    
}   // namespace Utils

#endif
