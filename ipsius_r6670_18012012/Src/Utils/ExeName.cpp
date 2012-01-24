#include "stdafx.h"
#include "ExeName.h"
#include "ErrorsSubsystem.h"

using std::string;

// ----------------------------------------------------

namespace
{

    /* 
        Данный класс создается в единственном экземпляре в ExeName::Init
        Деструктор для него никогда не вызывается для того, чтобы гарантировать
        доступность его данных из деструкторов других глобальных классов
    */
    class ExeNameInner
    {
        string m_exeName;   // full exe name (absolute)
        string m_exeDir;    // exe directory (absolute)

    public:

        ExeNameInner(const char *pExeName)
        {
            QFileInfo fn(QString(pExeName).trimmed());

            if (!fn.isAbsolute())
            {
                // fn = QFileInfo(QDir::current(), pExeName);
                fn.makeAbsolute();
            }

            m_exeName = fn.canonicalFilePath().toStdString();
            m_exeDir = fn.canonicalPath().toStdString() + "/";   

            ESS_ASSERT(!m_exeName.empty());
        }


        const string& GetExeName() const
        {
            return m_exeName;
        }

        const string& GetExeDir() const
        {
            return m_exeDir;
        }
    };


    ExeNameInner* PExeName = 0;

}  // namespace

// ----------------------------------------------------

namespace Utils
{
	
	void ExeName::Init(const char *pExeName)
	{
        ESS_ASSERT((PExeName == 0) && "Trying re-init"); 
	
	    PExeName = new ExeNameInner(pExeName);
	}
	
	// ----------------------------------------------------
	
	const string& ExeName::GetExeName()
	{
		ESS_ASSERT(PExeName != 0);
	
	    return PExeName->GetExeName();
	}
	
	// ----------------------------------------------------
	
	const string& ExeName::GetExeDir()
	{
		ESS_ASSERT(PExeName != 0);
	
	    return PExeName->GetExeDir();
	}
	
		
}  // namespace Utils



