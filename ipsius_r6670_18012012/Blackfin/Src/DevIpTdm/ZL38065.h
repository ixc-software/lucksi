#ifndef __ZL38065__
#define __ZL38065__

#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"

namespace DevIpTdm
{

	enum
	{
	        CPackLen = 3,
	        CRead = true,
	        CWrite = false,
	        CRefAddr = 0x400,
	        CRefVal = 0x47,
	        CEchoCancellerCount = 32,
	        CMaxReadAttempts = 3,
	        CMaxWriteAttempts = 3
	};

		
    using namespace Platform;

    class IZl38065Transport : public Utils::IBasicInterface
    {
    public:
        virtual bool WriteRead(byte valWrite, byte &valRead) = 0;
        virtual void ClearRxBuff() = 0;        
    };

    // ----------------------------------------------------------
    
    // Control for ZL38065 thru Acex/UART
    class ZL38065 : boost::noncopyable
    {
        IZl38065Transport &m_transport;
        bool m_writeVerify;
               
		//Packet coding
        static void CodePackToBuff(dword &pack, word addr, byte data, bool readOp);
		//Packet sending/recieving        
		static dword SendPack(dword pack, IZl38065Transport &transport, std::string &extErrorInfo);
		//Packet compare
		static bool isRequested(dword pack, word addr, bool readOp, std::string &extErrorInfo);
		//Get data from packet		
		static byte GetData(dword pack);		
		//Pack Pause
		static void PackPause();
		
		
		//Register Writing
		bool WriteReg(word addr, byte val, bool verify, std::string &extErrorInfo);
			
		static const char* BoolToOper(bool val)
		{
			return val ? "Read" : "Write";
		}    
   		
        

    public:

        ESS_TYPEDEF(DeviceError);

        ZL38065(IZl38065Transport &transport);

        // pair 0 .. 15 -- channel pair
        // SetMode(4, true, ...)              -- enable 128 ms echo suppress on 8th channel
        // SetMode(1, false, false, false)    -- disable echo on 2th and 3th channels
        // SetMode(1, false, false, true)     -- enable echo on 3th channel
        // return false on fail
        bool SetMode(int pair, bool enableExtended, bool enableLoCh, bool enableHiCh, 
            /* out */ std::string &extErrorInfo);

        // return true if ZL38065 detected
        static bool TryDetect(IZl38065Transport &transport);
        
    };
    
}  // namespace DevIpTdm

#endif
