#ifndef ISDNINFR_H
#define ISDNINFR_H
 
#include "stdafx.h"
#include "IIsdnLog.h"
#include "IIsdnAlloc.h"
#include "IIsdnClock.h"
#include "PacketCreate.h"

namespace Utils
{
    class Random;
} // namespace Utils

namespace ISDN
{		
    using Platform::dword;
    using Platform::byte;

    // �������� �������������� - ����������� ����������� ����������, ������, ������������� ������� 2������.
    class IsdnInfra: public IIsdnAlloc, public IIsdnLog, public IIsdnClock, public PacketCreate
	{
    public:

        IsdnInfra() : PacketCreate(this)
        {}

        virtual dword GetSeed() = 0;
    };
	

    // ���������� ����������� ����������
    class IsdnAllocDefault
	{
	public:

		IsdnAllocDefault():alloc_count(0)   {}		
        ~IsdnAllocDefault();
		
        void* alloc(size_t size);
		void free(void* p);

	private:

		dword alloc_count;
	};


	// ���������� ���������� ��������������
	class IsdnInfraDefault: public IsdnInfra
	{
	public:
        IsdnInfraDefault (Utils::Random* pRnd, 
            iLogW::ILogSessionToLogStore& storeIntf, 
            iLogW::LogSessionProfile& logSessionProf);

    // IIsdnAlloc implementation
    private:
		void* alloc(size_t size);		
        void free(void* ptr);

    // IIsdnClock implementation
    //private:
    public:		
		dword GetTick() const;

    // IsdnInfra implementation
    private:		
        dword GetSeed();

    // IIsdnLog implementation
    private:
        iLogW::ILogSessionToLogStore& getLogStoreIntf();        
        iLogW::LogSessionProfile& getLogSessionProfile();

	private:
		
        IsdnAllocDefault allocator;		
        Utils::Random* m_pRnd;
        iLogW::ILogSessionToLogStore& m_storeIntf;
        // TODO �������� ����� ����� ��������� � ������������ ���������� ��������
        iLogW::LogSessionProfile& m_logSessionProf;
	};

} // namespace ISDN


#endif


