#ifndef __PSB2170__
#define __PSB2170__

#include "Platform/Platform.h"
#include "DevIpPult/DevIpPultSpiMng.h"
#include "Utils/IniFile.h"
#include "iPult/CofidecState.h"

#include "IPsbDebug.h"


namespace PsbCofidec
{

    using namespace Platform;
    using namespace std;
    using iPult::CofidecState;
	struct ExtRegiterSet;
	enum AceRegAddress;
    
	ESS_TYPEDEF(PSB2170Error);
	               
	class Psb2170 : boost::noncopyable
	{				
	public:

        Psb2170(const BfDev::SpiBusPoint &point, IPsbDebug *iDebug);
		~Psb2170();

		void SetLoopMode(CsEnum::LoopMode loopMode);

		void SetModeOff();
		void SetModeHandSpeak(bool crossTest = false);
		void SetModeLoudSpeak();
		void SetModeLoudRing();

		// register profile load
		void LoadExtRegsFromIniFile(const string &data);

		const string GetState();

    private:

		bool Init();
		//register profile creating & set to dafault
		void InitExtRegisterListDefault();
		//load register profile from ini file
		void ReloadExtRegisterListFromIniFile(const Utils::IniFileSection &iniFileSec);
		ExtRegiterSet *FindExtRegister(const string &regname);
		// void AddRecord(const string &name, AceRegAddress addr, word value);

		void SetPowerDown(bool val);

		const string BoolToString(bool val);

		void ReadyWait();
		word ReadStatus();
		void ReadRegCmd(word address);
		word ReadData();
		word ReadRegister(AceRegAddress address);
		void WriteRegister(AceRegAddress address, word data);
		word ReadConfigRegisters(bool pairNum);
		void WriteConfigRegister(byte address, byte data);

		void SetAESRegs();
		static word ConvertItem(const string &val);

        void SendToLog(const string &line);

    private:
    	
        BfDev::SpiBusPoint m_point;
		IPsbDebug *m_debug;
		BfDev::BfSpi	&m_spi;
		vector<ExtRegiterSet> m_extRegisterSetList;

		bool	m_verify;
		word	m_RDYMaxAttempts;
		word	m_RDYMidAttempts;

		word	m_regWritesCount;
		word	m_regVerErrorCount;

	};

} //namespace Cofidec

#endif

