#include "stdafx.h"

#include "Psb2170.h"
#include "PsbCofidecInit.h"
#include "Utils/IntToString.h"
#include "Utils/StringParser.h"
#include "psb2170reg.h"

namespace
{
	PsbCofidec::Psb2170 *Psb2170Obj = 0;
}

namespace PsbCofidec
{

	struct ExtRegiterSet
	{
		string Name;
		AceRegAddress Address;
		word	Value;

		ExtRegiterSet(string name, AceRegAddress address, word value):
			Name(name),
			Address(address),
			Value(value){}
	};

	enum 
	{
		CChipSignature = 0x3000,
		CPowerDown = 0x100,
		CMaxAttemptsRDYWait = 200,
		CBfChannelSource = 5,
		CAFEChannelSource = 3,
		CACEAcousticSource = 11,
		CACELineSource = 12,
		CLineEchoSource = 15,
		CSdConfVal = 0x1f01,	//32timeSlots, enable IOM-2 interface
		CSdConfCh1 = 0x0180,	//A-law, TS = 0, enable
		CSdConfCh2 = 0x018a,	//A-law, TS = 0, enable
	};

	enum 
	{
		HWCONFIG0 = 0,
		HWCONFIG1 = 1,
		HWCONFIG2 = 2,
		HWCONFIG3 = 3,
	};

	
    //-------------------------------------------------------------------

	Psb2170::Psb2170(const BfDev::SpiBusPoint &point, IPsbDebug *iDebug) :
        m_point(point),
        m_debug(iDebug),
    	m_spi(m_point.Spi()),
		m_verify(true),
        m_regWritesCount(0),
        m_regVerErrorCount(0)
    {
		ESS_ASSERT(Psb2170Obj == 0 && "Psb2170 already created");
				
		m_RDYMaxAttempts = 0;
		m_RDYMidAttempts = 0;
		
		if (!Init()) ESS_THROW_MSG(PSB2170Error, "device is absent");
		    
        SetModeOff();		    
		Psb2170Obj = this;
    }
    
    //-------------------------------------------------------------------            
    
	Psb2170::~Psb2170()
	{
		Psb2170Obj = 0;
	}

	//-------------------------------------------------------------------            

	void Psb2170::SetModeOff()
	{
	}

	//-------------------------------------------------------------------            

	void Psb2170::SetModeHandSpeak(bool crossTest)
	{

		SendToLog(" HandSet Mode\n\r");
		ExtRegiterSet *pregSet;

		WriteRegister(IFS3, CAFEChannelSource);	// afe2 -> iom1

		pregSet = FindExtRegister("LECCTL");
		if(pregSet)
		{
			if((pregSet->Value & 0x8000) && !crossTest)
			{
				SendToLog(" LineEchoCancel enabled\n\r");
				word temp = 0x8000 | CAFEChannelSource << 5 | CBfChannelSource;
				WriteRegister(pregSet->Address, temp);
				WriteRegister(IFS2, CLineEchoSource);	// LineEcho -> afe2
				return;
			}
		}
		WriteRegister(IFS2, CBfChannelSource);	// iom1 -> afe2
	}

	//-------------------------------------------------------------------            

	void Psb2170::SetModeLoudSpeak()
	{
		bool isSPhone = false;
		bool isLecho = false;
		word temp;

		SendToLog(" HandsFreeMode\n\r");

		ExtRegiterSet *pregSet = FindExtRegister("SCTL");
		if(pregSet) if(pregSet->Value & 0xc000) isSPhone = true;

		pregSet = FindExtRegister("LECCTL");
		if(pregSet) if(pregSet->Value & 0x8000) isLecho = true;

		string log = "SpeakerPhone " + BoolToString(isSPhone) + 
			"; LineEchoCancel " + BoolToString(isLecho);
		SendToLog(log);

		if(!isLecho && !isSPhone)
		{
			WriteRegister(IFS2, CBfChannelSource);	// iom1 -> afe2
			WriteRegister(IFS3, CAFEChannelSource);	// afe2 -> iom1
			return;
		}

		if(isLecho && !isSPhone)
		{
			temp = 0x8000 | CAFEChannelSource << 5 | CBfChannelSource;
			WriteRegister(pregSet->Address, temp);
			WriteRegister(IFS2, CLineEchoSource);	// LineEcho -> afe2
			return;
		}

		if(!isLecho && isSPhone)
		{
			WriteRegister(IFS3, CACELineSource);	// aceline -> iom1 
			WriteRegister(IFS2, CACEAcousticSource);	// aceacc -> afe2

			WriteRegister(SSRC1, CAFEChannelSource);	// aceacc <- afe2
			WriteRegister(SSRC2, CBfChannelSource);	// aceline <- iom1 
			return;
		}

		if(isLecho && isSPhone)
		{
			temp = 0x8000 | CACELineSource << 5 | CBfChannelSource;
			WriteRegister(pregSet->Address, temp);

			WriteRegister(IFS3, CACELineSource);	// aceline -> iom1 
			WriteRegister(IFS2, CACEAcousticSource);	// aceacc -> afe2

			WriteRegister(SSRC1, CAFEChannelSource);	// aceacc <- afe2
			WriteRegister(SSRC2, CLineEchoSource);	// aceline <- iom1 
			return;
		}
	}

	//-------------------------------------------------------------------            

	void Psb2170::SetModeLoudRing()
	{
		SendToLog(" RingingMode\n\r");

		WriteRegister(IFS2, CBfChannelSource);	// iom1 -> afe2
		WriteRegister(IFS3, 0);	// silent -> iom1

	}

	//-------------------------------------------------------------------

	void Psb2170::InitExtRegisterListDefault()
	{
		m_extRegisterSetList.clear();
		
		#define ADD(regName, defaultValue) \
		  m_extRegisterSetList.push_back( ExtRegiterSet(#regName, regName, defaultValue) );

		ADD(SCTL,  0xc000);
		ADD(SATT1, 0x2c6a);
		ADD(SATT2, 0x0aff);
		ADD(SCATTL, 0x0800);
		ADD(SLGA,  0x4040);

		ADD(SAELEN, 0x0200);
		ADD(SAEATT, 0x1400);
		ADD(SAEGS,  0x0);
		ADD(SAEPS,  0x0);
		ADD(SAEBL,  0x0);
		ADD(SAEWFL, 0x5000);
		ADD(SAEWFT, 0x4000);

		ADD(LECCTL, 0x0000);
		ADD(LECLEV, 0x64ff);
		ADD(LECATT, 0x03fc);
		ADD(LECMGN, 0x06a40);
		
		#undef ADD

		for (word i = 0; i < m_extRegisterSetList.size(); ++i)
		{
			const ExtRegiterSet &set = m_extRegisterSetList.at(i);
			WriteRegister( set.Address, set.Value );
		}
			
	}

	//-------------------------------------------------------------------

	ExtRegiterSet *Psb2170::FindExtRegister(const string &regname)
	{

		for (word i = 0; i < m_extRegisterSetList.size(); ++i)
			if(m_extRegisterSetList[i].Name == regname) return &(m_extRegisterSetList[i]);
		return 0;
	}

	//-------------------------------------------------------------------

	void Psb2170::ReloadExtRegisterListFromIniFile(const Utils::IniFileSection &iniFileSec)
	{
        ExtRegiterSet *pregSet;
		word itemsCount = iniFileSec.ItemsCount();
		word loadedToDevice = 0;

		string log = "Loaded items count = " + Utils::IntToString(itemsCount) + "\n\r";
		for(word i = 0; i < itemsCount; ++i)
		{
			pregSet = FindExtRegister( iniFileSec[i].Name() );

			if(!pregSet)
            {
                ESS_THROW_MSG( PSB2170Error, "Bad register " + iniFileSec[i].Name() );
            }

			pregSet->Value = ConvertItem( iniFileSec[i].Value() );
			WriteRegister((pregSet->Address), pregSet->Value);
			if(m_debug)
			{
				log += pregSet->Name + " = " + Utils::IntToHexString(pregSet->Value) + "\n\r";			
				loadedToDevice++;
			}
		
		}
		log += "Loaded to device items count = " + Utils::IntToString(loadedToDevice) + "\n\r";
		SendToLog(log);		
	}

    //-------------------------------------------------------------------

	void Psb2170::LoadExtRegsFromIniFile(const string &data)
	{
		Utils::ManagedList<string> toIniFileData;
		StringParser(data, "\n", toIniFileData, true); 
		
        boost::scoped_ptr<Utils::IniFile> iniFile( 
            Utils::IniFile::LoadFromManagedList(toIniFileData) 
            );

		ReloadExtRegisterListFromIniFile( (*iniFile)[0] );
	}

	//-------------------------------------------------------------------

	word Psb2170::ConvertItem(const string &val)	// static
	{
		int pos, result;
		dword hresult;

		if (val.find("0x") == string::npos) 
		{
			if ( Utils::StringToInt(val, result) ) return result;
		}
		else
		{
			if ( Utils::HexStringToInt(val, hresult, true) ) return hresult;
		}

        ESS_THROW_MSG(PSB2170Error, "Can't convert " + val);

		return 0;  // dummi
	}

    //-------------------------------------------------------------------

	bool Psb2170::Init()
	{					
        ESS_ASSERT( PsbCofidecInit::InitDone() );  // sync must started 

		{		
			BfDev::SpiBusLocker busLock(m_point);	    
			word address = 0x5000;
			m_spi.Transfer(address>>8);
			m_spi.Transfer(address);
		}

		if(ReadRegister(REV) != CChipSignature)
		{
			return false;
		}

		InitExtRegisterListDefault();

		WriteConfigRegister(HWCONFIG1, 0x60);
		WriteRegister(SDCONF, CSdConfVal);
		WriteRegister(SDCHN1, CSdConfCh1);
//		WriteRegister(IFS2, CBfChannelSource);	// iom1 -> afe2
//		WriteRegister(IFS3, CAFEChannelSource);	// afe2 -> iom1 
		WriteRegister(AFECTL, 1);
		SetAESRegs();

		return true;
	}		

    //-------------------------------------------------------------------

	void Psb2170::SetPowerDown(bool val)
	{
		if(val) WriteRegister(CCTL, CPowerDown);
		else WriteRegister(CCTL, 0);
	}

	//-------------------------------------------------------------------
	//-------------------------------------------------------------------

	word Psb2170::ReadRegister(AceRegAddress address)
	{
		ReadRegCmd(address);
		return ReadData();
	}

	//-------------------------------------------------------------------

	word Psb2170::ReadConfigRegisters(bool pairNum)
	{
		ReadyWait();
		word status, result, address;
		BfDev::SpiBusLocker busLock(m_point);	    
		address = 0x7000;
		if(pairNum) address |= 0x200;
		m_spi.Transfer(address>>8);
		m_spi.Transfer(address);
		status = m_spi.Transfer(0xFF);		
		status <<= 8;
		status += m_spi.Transfer(0xFF);
		if(!(status & 0x8000))
			ESS_THROW_MSG(PSB2170Error, "device not ready");
		result = m_spi.Transfer(0xFF);
		result <<= 8;
		result += m_spi.Transfer(0xFF);
		return result;
	}

	//-------------------------------------------------------------------

	void Psb2170::WriteConfigRegister(byte address, byte data)
	{
		ESS_ASSERT(address < 4 && "Bad PSB2170 Address");
		ReadyWait();
		BfDev::SpiBusLocker busLock(m_point);	    
		address |= 0x60;
		m_spi.Transfer(address);
		m_spi.Transfer(data);
	}

	//-------------------------------------------------------------------

	void Psb2170::WriteRegister(AceRegAddress wrAddress, word data)
	{
		{		
			ReadyWait();
			word status;
			BfDev::SpiBusLocker busLock(m_point);	    
			word address = wrAddress;
			address &= 0x0fff;
			address |= 0x4000;
			m_spi.Transfer(address>>8);
			m_spi.Transfer(address);
			status = m_spi.Transfer(0xFF);		
			status <<= 8;
			status += m_spi.Transfer(0xFF);
			if(!(status & 0x8000)) 
				ESS_THROW_MSG(PSB2170Error, "device not ready");
			m_spi.Transfer(data>>8);
			m_spi.Transfer(data);
			m_regWritesCount++;
		}
		if(!m_verify) return;
		m_spi.Transfer(0xff);
		if(ReadRegister(wrAddress) != data) m_regVerErrorCount++;
	}

	//-------------------------------------------------------------------

	void Psb2170::ReadyWait()
	{
		word attempts = 0;
		while (!(ReadStatus() & 0x8000))
		{
			attempts++;
			m_spi.Transfer(0x00);
			if(attempts > CMaxAttemptsRDYWait) 
				ESS_THROW_MSG(PSB2170Error, "RDY Timeout");
		}
		if (attempts > m_RDYMaxAttempts) m_RDYMaxAttempts = attempts;
		if (!m_RDYMidAttempts) m_RDYMidAttempts = attempts;
		m_RDYMidAttempts = (m_RDYMidAttempts + attempts) /2;
	}

	//-------------------------------------------------------------------

	void Psb2170::ReadRegCmd(word address)
	{
		ReadyWait();
		BfDev::SpiBusLocker busLock(m_point);	    
		address &= 0x0fff;
		address |= 0x5000;
		m_spi.Transfer(address>>8);
		m_spi.Transfer(address);
	}

	//-------------------------------------------------------------------

	word Psb2170::ReadData()
	{
		ReadyWait();
		BfDev::SpiBusLocker busLock(m_point);	    
		word result, status;
		m_spi.Transfer(0x30);
		m_spi.Transfer(0x00);
		status = m_spi.Transfer(0xFF);		
		status <<= 8;
		status += m_spi.Transfer(0xFF);		
		result = m_spi.Transfer(0xFF);		
		result <<= 8;
		result += m_spi.Transfer(0xFF);
		if(!(status & 0x8000)) 
			ESS_THROW_MSG(PSB2170Error, "device not ready");
		return result;
	}

	//-------------------------------------------------------------------

	word Psb2170::ReadStatus()
	{
		BfDev::SpiBusLocker busLock(m_point);	    
		word result;
		m_spi.Transfer(0x30);
		m_spi.Transfer(0x00);
		result = m_spi.Transfer(0xFF);		
		result <<= 8;
		result += m_spi.Transfer(0xFF);		
		return result;
	}

	//-------------------------------------------------------------------

	void Psb2170::SetLoopMode(CsEnum::LoopMode loopMode)
	{
		if (loopMode == CsEnum::lmAACE) 
		{
			SendToLog("mAACE Loop\n\r");
			WriteRegister(IFS2, CAFEChannelSource);	// iom1 -> iom1
			WriteRegister(IFS3, CBfChannelSource);	// afe2 -> afe2 
		}
		else
		{
			SendToLog("Loop Off\n\r");
			WriteRegister(IFS2, CBfChannelSource);	// iom1 -> afe2
			WriteRegister(IFS3, CAFEChannelSource);	// afe2 -> iom1 
		}
	}

	//-------------------------------------------------------------------            

	void Psb2170::SendToLog(const string &line)
	{
		if(!m_debug) return;
		m_debug->WriteLn("ACE -> " + line);
	}

	//-------------------------------------------------------------------

	void Psb2170::SetAESRegs()
	{
		WriteRegister(SSDX1, 0x1b3a);
		WriteRegister(SSDX2, 0x1006);
		WriteRegister(SSDX3, 0xc0c0);	// Tx Speach Detector
		WriteRegister(SSDX4, 0x9428);

		WriteRegister(SSDR1, 0x1437);	
		WriteRegister(SSDR2, 0x1006);	
		WriteRegister(SSDR3, 0xc0c0);	// Rx Speach Detector	
		WriteRegister(SSDR4, 0x9428);	

		WriteRegister(SSCAS1, 0x0800);	
		WriteRegister(SSCAS2, 0x1006);	// Acoustic side speech comparator
		WriteRegister(SSCAS3, 0x1006);	

		WriteRegister(SSCLS1, 0x0000);	
		WriteRegister(SSCLS2, 0x2002);	// Line side speech comparator
		WriteRegister(SSCLS3, 0x2002);	

		WriteRegister(SAGX1, 0x005f);	// AGX
		WriteRegister(SAGX2, 0x7fff);	
		WriteRegister(SAGX3, 0x0014);
		WriteRegister(SAGX4, 0x4020);	

		WriteRegister(SAGR1, 0x006f);	// AGR
		WriteRegister(SAGR2, 0x7fff);	
		WriteRegister(SAGR3, 0x001a);
		WriteRegister(SAGR4, 0x4020);	


		WriteRegister(SCSD1, 0x7f29);	
		WriteRegister(SCSD2, 0x1006);	
		WriteRegister(SCSD3, 0x0202);	
		WriteRegister(SCSD4, 0x9457);	

		WriteRegister(SCLPT, 0x0020);	
		WriteRegister(SCCR, 0x7c28);	
		WriteRegister(SCCRN, 0x3000);	
		WriteRegister(SCCRS, 0xfccd);	
		WriteRegister(SCCRL, 0x6ccc);	
		WriteRegister(SCDT, 0x1800);	
		WriteRegister(SCDTN, 0x2200);	
		WriteRegister(SCDTS, 0xfd9a);	
		WriteRegister(SCDTL, 0x1000);	

		WriteRegister(SCATTN, 0x3000);	
		WriteRegister(SCATTS, 0x0800);
//		WriteRegister(SCATTL, 0x3800);

		WriteRegister(SCAECL, 0x0c00);

		WriteRegister(SCSTGP, 0x1f40);
		WriteRegister(SCSTATT, 0x1a93);
		WriteRegister(SCSTNL, 0x4fbb);
		WriteRegister(SCSTS, 0x0800);
		WriteRegister(SCSTTIM, 0x3e80);
		WriteRegister(SCSTIS, 0x002a);
		WriteRegister(SCSTDS, 0x00ff);

		WriteRegister(SCLSPN, 0x442a);
		WriteRegister(SCLSPS, 0x0200);
		WriteRegister(SCLSPL, 0x0800);
		WriteRegister(SCCN1, 0x0000);
		WriteRegister(SCCN2, 0x0600);
		WriteRegister(SCCN3, 0x0800);
	}

    //-------------------------------------------------------------------            

	const string Psb2170::GetState()
	{
		string result;

		result += "m_RDYMidAttempts is " + Utils::IntToHexString(m_RDYMidAttempts) + "\n\r";
		result += "m_RDYMaxAttempts is " + Utils::IntToHexString(m_RDYMaxAttempts) + "\n\r";

		result += "m_regWritesCount is " + Utils::IntToHexString(m_regWritesCount) + "\n\r";
		result += "m_regVerErrorCount is " + Utils::IntToHexString(m_regVerErrorCount) + "\n\r";
		return result;
	}


	// -------------------------------------------------------------------------

	const string Psb2170::BoolToString(bool val)
	{
		return val ? "Enabled" : "Disabled";
	}


}	//namespace PsbCofidec
