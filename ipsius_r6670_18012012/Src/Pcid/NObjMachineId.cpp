#include "stdafx.h"
#include "NObjMachineId.h"
#include "Utils/ExeName.h"
#include "DmiDecodeWrap.h"
#include "ProtSerial.h"

namespace 
{
	const char *CPcidDmiDecodeFileName = "pcid_dmiDecode.txt";

	QString FileName(const QString &arg)
	{
		QFileInfo fileInfo(arg);
		if(!fileInfo.isAbsolute())
		{ 
			fileInfo = Utils::ExeName::GetExeDir().c_str() + arg;
		}
		return fileInfo.absoluteFilePath();
	}

	const char *CGenPassword = "ipsius-odessa";
};

namespace Pcid
{

	NObjMachineId::NObjMachineId(Domain::IDomain *pDomain, const Domain::ObjectName &name) :
		Domain::NamedObject(pDomain, name)
	{}

	// -------------------------------------------------------------------------------

	void NObjMachineId::GenPcid(DRI::ICmdOutput *pOutput)
	{
		std::string info;
		if(!GetMachineID(info)) ThrowRuntimeException("Generation error");
		
		pOutput->Add(QString("Pcid: '%1'").arg(info.c_str()));
		SaveResultFile(ProtSerial::CPcidFileName(), info.c_str());
		pOutput->Add(QString("Result was saved to file '%1'").arg(ProtSerial::CPcidFileName()));
	}

	// -------------------------------------------------------------------------------

	void NObjMachineId::GenPcidDmiDecode(DRI::ICmdOutput *pOutput, const QString &fileName)
	{

		std::string info;
		{
			// read dmiDecode file
			QFile file(FileName(fileName));
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				ThrowRuntimeException("File not found");
			}
            QTextStream stream(&file);
            bool ok = DmiDecodeWrap::GetMachineID(stream, info);
			if(!ok) ThrowRuntimeException("Generation error");
		}		
		// result output 
		pOutput->Add(QString("Pcid: '%1'").arg(info.c_str()));
		SaveResultFile(CPcidDmiDecodeFileName, info.c_str());
		pOutput->Add(QString("Result was saved to file '%1'").arg(CPcidDmiDecodeFileName));
	}

	// -------------------------------------------------------------------------------

	void NObjMachineId::TestSerialFile(DRI::ICmdOutput *pOutput)
	{
		if (ProtSerial::CheckSerialFile()) 
			pOutput->Add("Correct serial number!");
		else
		{
			pOutput->Add("Wrong serial number!");
			GenPcid(pOutput);
		}
	}

	// -------------------------------------------------------------------------------

	void NObjMachineId::TestPcid()
	{
		ProtSerial::Test();
	}

	// -------------------------------------------------------------------------------

	void NObjMachineId::GenSerial(DRI::ICmdOutput *pOutput, const QString &password)
	{
		if (password != CGenPassword) ESS_HALT("");

		QFile file(QString(Utils::ExeName::GetExeDir().c_str()) + ProtSerial::CPcidFileName());
		if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
		{
			ThrowRuntimeException("File 'pcid.txt' not found");
		}
		std::string info = ProtSerial::MakeSerial(file.readAll().constData());
		pOutput->Add(QString("Serial: '%1'").arg(info.c_str()));
		SaveResultFile(ProtSerial::CSerialFileName(), info.c_str());	
		pOutput->Add(QString("Result was saved to file '%1'").arg(ProtSerial::CSerialFileName()));
	}

	// -------------------------------------------------------------------------------

	void NObjMachineId::SaveResultFile(const QString &fileName, const QString &info)
	{
		QFile file(Utils::ExeName::GetExeDir().c_str() + fileName);
		if (!file.open(QIODevice::WriteOnly)) ThrowRuntimeException("File not opened");

		QTextStream out(&file);
		out << info;
	}
}; // namespace Pcid



