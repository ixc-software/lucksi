
#ifndef IDSNPACK_H
#define IDSNPACK_H

#include "stdafx.h"

#include "Platform/PlatformTypes.h"
#include "isdninfr.h"
#include "IIsdnAlloc.h"
#include "PacketCreate.h"

#include "Utils/DeleteIfPointer.h"
#include "Utils/IActiveMarkObject.h"
#include "Utils/ContainerConverter.h"


namespace ISDN
{
		

    class PacketQueue;

    class IsdnPacket 
        : public Utils::ISpecialDestructable, public Utils::IActiveMarkObject
    {
        void ProcessSpecialDelete()
        {            
            Delete();
        }

    public:

		IsdnPacket(IIsdnAlloc* allocator)
            : m_pAllocator(allocator),
            m_pIsInQueue(0)            
        {} 

        void Fill(QVector<byte> src)
        {
            Utils::Converter<>::DoConvert(src, m_Data);
        }

        QVector<byte> getAsQVector()
        {
            QVector<byte> rez;
            if (!m_Data.empty()) Utils::Converter<>::DoConvert(m_Data, rez);
            return rez;
        }

        void CopyToVector(std::vector<byte>& out)
        {
            if (m_Data.empty()) return;

            std::copy(m_Data.begin(), m_Data.end(),
                std::inserter(out, out.begin()));
        }

        dword GetLenData()  // возвращает размер деки
		{
            return m_Data.size();
        }

        /*bool CorrectSmallestDssLen()
        {
            return (m_Data.size() >= 3);
        }*/

		void Delete();
        

		byte GetByIndex(size_t i)
		{
            AssertActive();
            return m_Data.at(i);
        }

        bool IsInQueue()    // проверка - помещен ли данный экземпляр в очередь//возможно private
		{
            AssertActive();
            return m_pIsInQueue ? true : false;
        }
		
		void SetQueue(PacketQueue *pQueue)  // эта функция будет вызыватся из PQ//??может ли один пакеттоказатся в нескольких очередях одновременно????
	    {
            AssertActive();
            m_pIsInQueue = pQueue;
        }
 
        virtual IsdnPacket* Clone() // создает полную копию пакета, IsInQueue=0
		{
            AssertActive();
			IsdnPacket* pRet=new(m_pAllocator) IsdnPacket(*this);
			return pRet;
		}
		
        bool AddFront(std::vector<byte> const &data);   // memcpy(skb_push(skb, i), header, i);
		
        bool AddBack(std::vector<byte> const &data);    // memcpy(skb_put(skb, i), header, i);
		
        void ClearFront(int len);   //skb_pull- убрать len-байт от начала

		static void* operator new(size_t size, IIsdnAlloc *infra)
		{
			return infra->alloc(size);
		}

		static void operator delete(void* ptr, IIsdnAlloc *infra)
		{
			IsdnPacket* pBase = static_cast<IsdnPacket*>(ptr);
            pBase->~IsdnPacket();
            infra->free(ptr);
		}
        
        static void operator delete(void* ptr, size_t, IIsdnAlloc *infra) // используется при сбое в конструкторе
        {
            infra->free(ptr);
        }

        static void operator delete(void* ptr)
        {
            ESS_ASSERT(0 && "This operator delete() must be never called!");
        }
        
        std::deque<byte> const & ReadFullData() const 
        {
            AssertActive();
            return m_Data;
        } 

	protected:

		//Конструктор клонирования и запрет копирования
		explicit IsdnPacket(const IsdnPacket& Pack)
            : Utils::ISpecialDestructable(), Utils::IActiveMarkObject(),
              m_Data(Pack.m_Data.begin(),Pack.m_Data.end()),
			  m_pAllocator(Pack.m_pAllocator),
		      m_pIsInQueue(0)              
        {}

		//запрет стековых объектов
        virtual ~IsdnPacket();
		
		std::deque<byte> m_Data;
		IIsdnAlloc* m_pAllocator;
	
	private:

		//запрет стековых объектов
		void operator = (IsdnPacket const&);

        PacketQueue* m_pIsInQueue;        
    };

	//-----------------------------

	class IPacket : public IsdnPacket
	{
	public:

		IPacket(IIsdnAlloc* allocator) : IsdnPacket(allocator){}
		
        IPacket* Clone()  // override
        {
            AssertActive();
            IPacket* pRet=new(m_pAllocator) IPacket(*this);
            return pRet;
        }
		
	private:

		IPacket(const IPacket& src):IsdnPacket(src){}

        ~IPacket(){} // override

	
	};
//---------------------------------
	class UPacket : public IsdnPacket
	{
	public:

		UPacket(IIsdnAlloc* allocator) : IsdnPacket(allocator){}
		
        UPacket* Clone() // override
        {
            AssertActive();
            UPacket* pRet=new(m_pAllocator) UPacket(*this);
            return pRet;
        }

    private:

        UPacket(const IPacket& src):IsdnPacket(src){}

        ~UPacket(){} // override
	};

//typedef IsdnPacket UPacket;
//typedef IsdnPacket IPacket;

    
}//namespace ISDN

#endif

