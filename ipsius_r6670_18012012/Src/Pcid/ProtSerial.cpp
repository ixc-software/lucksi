#include "stdafx.h"
#include "ProtSerial.h"
#include "Utils/ExeName.h"

namespace Pcid
{
	std::string ProtSerial::MakeSerial(const std::string &machineID)
	{
		ESS_ASSERT(machineID.size() > 0);
		Platform::dword crc = Utils::UpdateCRC32(&machineID[0], machineID.size());
		return Utils::IntToHexString(crc ^ CProtMagic, false);
	}

	// -------------------------------------------------------------------------------

	bool ProtSerial::CheckSerialFile()
	{
		QFile file(QString(Utils::ExeName::GetExeDir().c_str()) + CSerialFileName());
		return file.open(QIODevice::ReadOnly | QIODevice::Text) &&
			VerifySerial::Verify(file.readAll().constData());
	}

	// -------------------------------------------------------------------------------

	void ProtSerial::Test()
	{
		std::string id;
		ESS_ASSERT( GetMachineID(id) );

		std::string serial = MakeSerial(id);

		TUT_ASSERT( VerifySerial::Verify(serial) );
	}

}  // namespace Pcid
