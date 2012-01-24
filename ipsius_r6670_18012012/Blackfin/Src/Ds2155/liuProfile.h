#ifndef __DS2155_LIU_PROFILE__
#define __DS2155_LIU_PROFILE__

#include "Platform/PlatformTypes.h"
#include "SafeBiProto/ISerialazable.h"

namespace Ds2155
{
    using namespace Platform;

    // ----------------------------------------------------------

    // shared meta enums 
    class Enums : public QObject
    {
        Q_OBJECT;

    public:

        enum LineCodeType
        {
            lcAmi,
            lcHdb3,
            lcB8zs,
        };

        enum CycleType
        {
            ctPCM30,
            ctPCM31,
            ctNoStruct,
            ctD4,
            ctEsf,
        };

        Q_ENUMS(CycleType LineCodeType);

        // replace with meta enum resolve - ?? 
        // don't work on Blackfin
        static const char* LineCodeToString(LineCodeType val)
        {
            if (val == lcAmi) return "AMI";
            if (val == lcHdb3) return "HDB3";
            if (val == lcB8zs) return "B8ZS";            
            return "";
        }

        static const char* CycleTypeToString(CycleType val)
        {
            if (val == ctPCM30) return "PCM30";
            if (val == ctPCM31) return "PCM31";
            if (val == ctNoStruct) return "NoStruct";
            if (val == ctD4) return "D4";
            if (val == ctEsf) return "ESF";            
            return "";
        }

    };


    // ----------------------------------------------------------

    /* 
       Liu profile
     Liu supports two operation modes: E1(Default)  & T1.
          For using Liu in T1 mode call method SetAsT1(), 
          profile parametres CRC4, NFAS, MFAS, FAS are ignored in this case    
          For using Liu in E1 mode call any of methods SetAsPCM30() or SetAsPCM31().
    !!! Check profile before using by calling Verify() method, 
        else possible ESS_ASSERT from DS2155 constructor. 
    */
    class LiuProfile : public SBProto::ISerialazable
	{

        enum
        {
            DEF_FAS = 0x1B,
            DEF_NFAS= 0x40,
            DEF_MFAS= 0x0B
        };

	public:
	    
        LiuProfile()
        {
            SetAsPcm31();
        }
	
        std::string ToString() const
        {
            using namespace std;

            std::ostringstream out;
            
            if (m_lineCode == Enums::lcB8zs)
            {
                out << "Operation mode " << "T1"
                    << "; Code " << Enums::LineCodeToString(m_lineCode)
                    << "; Cycle " << Enums::CycleTypeToString(m_cycle);                
                return out.str();                 
            }

            out << "Operation mode " << "E1"            
                << "; Code " << Enums::LineCodeToString(m_lineCode)
                << "; Cycle " << Enums::CycleTypeToString(m_cycle)
                << "; CRC4 " << m_enableCrc4
                << hex
                << "; MFAS " << (int)m_MFAS
                << "; FAS "  << (int)m_FAS
                << "; NFAS " << (int)m_NFAS;

            return out.str();
        }

        void SetDefaultFAS()
        {
            m_MFAS = DEF_MFAS;
            m_FAS  = DEF_FAS;
            m_NFAS = DEF_NFAS;
        }

        void SetAsPcm30()
        {
            m_lineCode   = Enums::lcHdb3;
            m_cycle      = Enums::ctPCM30;
            m_enableCrc4 = false;
            m_syncMaster = false;
            SetDefaultFAS();
        }

        void SetAsPcm31()
        {
            m_lineCode   = Enums::lcHdb3;
            m_cycle      = Enums::ctPCM31;
            m_enableCrc4 = false;
            m_syncMaster = false;            
            SetDefaultFAS();
        }
        
        void SetAsT1()
        {
            m_lineCode   = Enums::lcB8zs;
            m_cycle      = Enums::ctD4;
            m_syncMaster = false;
        }
        
        bool Verify() const
        {
            if(m_lineCode == Enums::lcB8zs) return m_cycle == Enums::ctD4 || m_cycle == Enums::ctEsf;

			return m_cycle == Enums::ctPCM31 || m_cycle == Enums::ctPCM30 || m_cycle == Enums::ctNoStruct;
        }    

        bool IsT1Mode() const
        {
            return m_lineCode == Enums::lcB8zs;
        }

        Enums::LineCodeType LineCode() const { return m_lineCode; }
        void LineCode(Enums::LineCodeType val) { m_lineCode = val; }

        Enums::CycleType Cycle() const { return m_cycle; }
        void Cycle(Enums::CycleType val) { m_cycle = val; }

        bool Crc4() const { return m_enableCrc4; }
        void Crc4(bool val) { m_enableCrc4 = val; }
        
        bool SyncMaster() const { return m_syncMaster; }
        void SyncMaster(bool val) { m_syncMaster = val; }
        

        byte MFAS() const { return m_MFAS; }
        void MFAS(byte val) { m_MFAS = val; }

        byte FAS() const { return m_FAS; }
        void FAS(byte val) { m_FAS = val; }

        byte NFAS() const { return m_NFAS; }
        void NFAS(byte val) { m_NFAS = val; }

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.m_lineCode
                << data.m_cycle
                << data.m_enableCrc4
                << data.m_syncMaster
                << data.m_MFAS
                << data.m_FAS
                << data.m_NFAS;
        }
			        				
	protected: 

        Enums::LineCodeType m_lineCode;    
        Enums::CycleType m_cycle;       
        bool m_enableCrc4;        
        bool m_syncMaster;
        byte m_MFAS;
        byte m_FAS;
        byte m_NFAS;

	};

};

#endif
