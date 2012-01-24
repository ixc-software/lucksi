#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"
#include "Utils/IntToString.h"
#include "Utils/QtHelpers.h"

#include "MngLwip.h"


namespace
{
    enum 
    {
        CEmulMode = false, 
    };

	E1App::Stack *GPStack = 0;

    QString LocalIP()
    {
        QList<QHostAddress> addrs = QNetworkInterface::allAddresses();

        for(int i = 0; i < addrs.size(); ++i)
        {
            QString s = addrs.at(i).toString();
            if (s.startsWith("192.168")) return s;
        }

        return (addrs.size() > 0) ? (addrs.at(0).toString()) : "127.0.0.1";
    }

}


// -----------------------------------

namespace E1App
{
	
	Stack::Stack(const NetworkSettings &settings, bool doStart)
	{
        m_started = false;
        if (doStart) Start();
	}

    // -----------------------------------

    bool Stack::IsEstablished() const
    {
        return true;
    }

    // -----------------------------------

    bool Stack::Establish(int timeout)
    {
        return true;
    }

    // -----------------------------------

    std::string Stack::GetIP() const
    {
        QString ip = LocalIP();
        return Utils::QStringToString(ip);
    }
    
	// -----------------------------------------------

	bool Stack::HwEmulationMode()
	{
		return CEmulMode;
	}

    // -----------------------------------------------

    void Stack::Start()
    {        
        ESS_ASSERT(!m_started);
        m_started = true;
    }

	// -----------------------------------------------
	
	void Stack::Init(const NetworkSettings &settings, bool doStart)  // static 
	{
		ESS_ASSERT(GPStack == 0);
		GPStack = new Stack(settings, doStart);
	}


	Stack& Stack::Instance()   // static
	{
		ESS_ASSERT(GPStack != 0);		
		return *GPStack;
	}

    // -----------------------------------------------

    bool Stack::IsInited()   // static
    {
        return GPStack != 0;
    }
	
	
}  // namespace E1App
