#include "stdafx.h"
#include "TestClientServerTasks.h"

#include "Domain/DomainTestRunner.h"

#include "ClientServerScenaries.h"


namespace TestBfBoot
{       
    void TestClientServerTasks()
    {                
        ScnUpdateFw::Settings prof;            
        prof.TraceClient = false;
        prof.TraceServer = false;
        prof.TraceTest = true;               

        iLogW::LogSettings settings;
        settings.CountSyncroToStore(1);
        settings.TimestampInd(true);
        settings.out().Cout().TraceInd = true;        
        settings.out().Udp().DstHost = Utils::HostInf ("192.168.0.144", 56001);
        settings.out().Udp().TraceInd = true;

        Domain::DomainTestRunner test(settings, -1);
        
        test.Run<ScnUpdateFw>(prof);
    }
} // namespace TestBfBoot
