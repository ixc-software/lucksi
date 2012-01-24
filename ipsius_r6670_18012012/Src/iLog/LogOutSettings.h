#ifndef _LOG_OUT_SETTINGS_I_H_
#define _LOG_OUT_SETTINGS_I_H_

#include "stdafx.h"
#include "Utils/HostInf.h"

// Platform depended

namespace Utils {   class MetaConfig; };

namespace iLogW
{
	struct FileStreamProfile 
	{
		bool TraceInd;
		std::string NameLogFile;  // Name of log file. 
		bool DirectSaveToFile; 

		FileStreamProfile() : TraceInd(false), DirectSaveToFile(false){}
	};

	struct UdpStreamProfile 
	{
		bool TraceInd;
		Utils::HostInf DstHost;

		UdpStreamProfile() : TraceInd(false), DstHost("127.0.0.1", 56001)
		{
		}
	};

	struct UartStreamProfile 
	{
		bool TraceInd;
		int  Port;

		UartStreamProfile () : TraceInd(false), Port(0){}
	};

	struct CoutStreamProfile
	{
		bool TraceInd;

		CoutStreamProfile() : TraceInd(false){}
	};

    class LogOutSettings
    {
    public:
		LogOutSettings() {}
		
		const CoutStreamProfile &Cout() const {	return m_coutStream; }
		CoutStreamProfile &Cout() {	return m_coutStream; }

		const FileStreamProfile &File() const {	return m_fileStream; }
		FileStreamProfile &File() {	return m_fileStream; }

		const UdpStreamProfile  &Udp() const  {	return m_udpStream; }
		UdpStreamProfile  &Udp() {	return m_udpStream; }

		const UartStreamProfile &Uart() const  {	return m_uartStream; }
		UartStreamProfile &Uart() {	return m_uartStream; }
    private:
		CoutStreamProfile m_coutStream;
		FileStreamProfile m_fileStream;
		UdpStreamProfile m_udpStream;
		UartStreamProfile m_uartStream;
    };
}; 

#endif 


