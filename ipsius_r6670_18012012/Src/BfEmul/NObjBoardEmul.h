#ifndef NOBJBOARDEMUL_H
#define NOBJBOARDEMUL_H

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Utils/IExitTaskObserver.h"
#include "Utils/threadtaskrunner.h"
#include "NObjBoardSettings.h"

#include "E1App/RtpPortRange.h"
#include "Domain/NObjLogSettings.h"


namespace BfEmul
{   
       
    class NObjBoardEmul 
        : public Domain::NamedObject,
        Utils::IExitTaskObserver,
        DRI::INonCreatable
    {
        Q_OBJECT;
    
    public:
        NObjBoardEmul(Domain::NamedObject *pParent, const Domain::ObjectName &name,
            Platform::dword boardId, E1App::IRtpRangeCreator &rtp, bool setToDefaultCfg = false);

        int BoardNumber() const
        {
            return m_boardId;
        }           

    // DRI interface                    
        Q_PROPERTY(bool UseBooter READ m_useBooter WRITE m_useBooter);
        Q_PROPERTY(int MinRtpPort READ MinRtpPort);        
        Q_PROPERTY(int MaxRtpPort READ MaxRtpPort);               
        Q_PROPERTY(bool TraceInd READ m_traceE1App WRITE m_traceE1App);         
        Q_PROPERTY (int SoftRev READ m_softVer WRITE m_softVer);             

        Q_PROPERTY(bool Enabled READ m_enabled);
        bool Enabled() const 
        {
            return m_enabled;
        }  

    // owner interface
    public:

        void Start(Utils::ThreadTaskRunner &, const iLogW::LogSettings &);
        void Stop();

        std::string StringId() const
        {
            QString name(Name().Name());
            name.replace(".", "_");
            return name.toStdString();
        }

        
    // Utils::IExitTaskObserver impl
    private:
        bool IsTaskOver() const;

    private:                         

        int MinRtpPort() const
        {
            return (m_rtpRange == 0) ? 0 : m_rtpRange->MinPort();
        }        

        int MaxRtpPort() const
        {
            return (m_rtpRange == 0) ? 0 : m_rtpRange->MaxPort();
        }
        
        void InitDefault()
        {            
            m_enabled.Set(false);                 
            m_useBooter = true;
            m_traceE1App = false;            
            m_softVer = -1; 
        }      

        Utils::AtomicBool m_enabled;
        NObjBoardSettings* m_pSettings;
        
        bool m_useBooter;              
        int m_softVer;

        bool m_traceE1App;
        boost::shared_ptr<E1App::RtpPortRange> m_rtpRange;        
        int m_boardId;
    };
} // namespace BfEmul

#endif
