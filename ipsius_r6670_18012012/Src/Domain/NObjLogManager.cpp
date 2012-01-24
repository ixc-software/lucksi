#include "stdafx.h"

#include "Utils/ExeName.h"
#include "Utils/StringList.h"

#include "NObjLogManager.h"

namespace Domain
{

	NObjLogManager::NObjLogManager(Domain::NamedObject *pParent, 
                                   const Domain::ObjectName &name) : 
        Domain::NamedObject(&pParent->getDomain(), name, pParent),
		m_log(getDomain().Log())
	{ 
    }
	
	// ---------------------------------------------------------------------

    void NObjLogManager::TraceToCout(bool traceInd)
	{
		Output().TraceToCout(traceInd);
	}

	// ---------------------------------------------------------------------

	bool NObjLogManager::TraceToCout() const
	{
		return Output().Settings().File().TraceInd;
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::TraceToFile(bool traceInd)
	{
		if (traceInd && Output().Settings().File().NameLogFile.size() == 0) ThrowRuntimeException("File name is empty");

		Output().TraceToFile(traceInd);
	}

	// ---------------------------------------------------------------------

	bool NObjLogManager::TraceToFile() const
	{
		return Output().Settings().File().TraceInd;
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::FileName(const QString &fileName)
	{
		if(TraceToFile()) ThrowRuntimeException("Trace to file active.");

		Output().NameLogFile(fileName.toStdString());
	}

	// ---------------------------------------------------------------------

	QString NObjLogManager::FileName() const
	{
		return Output().Settings().File().NameLogFile.c_str();
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::DirectSaveToFile(bool directSaveToFile)
	{
		if(TraceToFile()) ThrowRuntimeException("Trace to file active.");

		Output().DirectSaveToFile(directSaveToFile);
	}

	// ---------------------------------------------------------------------

	bool NObjLogManager::DirectSaveToFile() const
	{
		return Output().Settings().File().DirectSaveToFile;
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::TraceToUdp(bool traceInd)
	{
		if(traceInd && Output().Settings().Udp().DstHost.Empty()) ThrowRuntimeException("Address of log destination is empty");		
		
		Output().TraceToUdp(traceInd);
	}

	// ---------------------------------------------------------------------

	bool NObjLogManager::TraceToUdp() const
	{
		return Output().Settings().Udp().TraceInd;
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::DstUdpTrace(const QString &ip)
	{
		if(TraceToUdp()) ThrowRuntimeException("Trace to udp is activated.");
		
		Output().DstUdpTrace(Utils::HostInf::FromString(ip.toStdString()));
	}

	// ---------------------------------------------------------------------

	QString NObjLogManager::DstUdpTrace() const
	{
		return Output().Settings().Udp().DstHost.ToString().c_str();
	}

	// ---------------------------------------------------------------------

	bool NObjLogManager::TimestampInd() const
	{
		return m_log.TimestampInd();
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::TimestampInd(bool val)
	{
		m_log.TimestampInd(val);
	}

	// ---------------------------------------------------------------------

	int NObjLogManager::CountToSync() const
	{
		return m_log.CountToSync();
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::CountToSync(int countToSync)
	{
		m_log.CountToSync(countToSync);
	}

	// ---------------------------------------------------------------------

	int NObjLogManager::TimeoutToSync() const
	{
		return m_log.TimeoutToSync();
	}

	// ---------------------------------------------------------------------

	void NObjLogManager::TimeoutToSync(int timeoutToSync)
	{
		m_log.TimeoutToSync(timeoutToSync);
	}

    // ---------------------------------------------------------------------

    void NObjLogManager::DumpSessionsToFile(QString fileName)
    {
        // get dump
        std::vector<std::string> dump;
        m_log.DumpSessionNames(dump);

        // dump to StringList
        Utils::StringList sl;
        for(int i = 0; i < dump.size(); ++i) sl.push_back( dump.at(i).c_str() );

        // make file name
        if (fileName.isEmpty()) fileName = "LogSessionsDump.txt";
        fileName = Utils::ExeName::GetExeDir().c_str() + fileName;

        // save
        sl.SaveToFile(fileName);
    }

} // Domain
