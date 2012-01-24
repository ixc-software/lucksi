#include "stdafx.h"
#include "NObjBfTaskUpdateBooter.h"
#include "BfBootCli/TaskComposer.h"
#include "IpsiusService/BootImage.h"


namespace BfBootDRI
{    
    void NObjBfTaskUpdateBooter::SetBootImage( QString file )
    {
        m_bootImage.reset( new IpsiusService::BootImage(file) ); // catch ?
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskUpdateBooter::Run( DRI::IAsyncCmd *pAsyncCmd )
    {
        m_prof->RunTask(pAsyncCmd, *this, Name());        
    }

    // ------------------------------------------------------------------------------------

    void NObjBfTaskUpdateBooter::Run(bool& isLastTask)
    {
        isLastTask = true;
        if (!m_bootImage) ThrowRuntimeException("Set boot image before run");

        m_task = BfBootCli::TaskComposer::UpdateBootImg(
            m_prof->getMngProf(), 
            m_prof->getPwd(),
            *m_bootImage,
            m_prof->getTransport(),                
            m_prof->getTraceServer()
            );
    }

    // ------------------------------------------------------------------------------------

    void NObjBfTaskUpdateBooter::Init( QString profile )
    {
        std::string err;
        if (!m_prof.Init(profile, &err)) ThrowRuntimeException(err);        
    }
} // namespace BfBootDRI

