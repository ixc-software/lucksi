#include "stdafx.h"
#include "BidirBufferTest.h"
#include "Utils/BidirBuffer.h"
#include "Utils/BidirBufferCreator.h"
#include "Utils/MemoryPool.h"
#include "Utils/VirtualInvoke.h"

namespace 
{    
    using Utils::BidirBufferCreator;
    using Utils::BidirBuffer;
    using Platform::byte;
    using Utils::VirtualInvoke;
    using Utils::IVirtualInvoke;

    void TestResizeOperations()    
    {
        const int CBuffCapacity = Platform::PlatformDataAlign( 128 );        
        Utils::FixedMemBlockPool<> buffPool(CBuffCapacity, 1);
        BidirBuffer buff(buffPool, BidirBuffer::CCenter);

        TUT_ASSERT (buff.Size() == 0);
        TUT_ASSERT (buff.getBlockSize() == CBuffCapacity);
        TUT_ASSERT (buff.GetDataOffset() == (CBuffCapacity / 2));

        // проверка абсолютного сдвига смещения
        {            
            buff.SetDataOffset(0);
            TUT_ASSERT (buff.Size() == 0);        
            TUT_ASSERT (buff.GetDataOffset() == 0);
            buff.SetDataOffset(CBuffCapacity - 1);
            TUT_ASSERT (buff.Size() == 0);        
            TUT_ASSERT (buff.GetDataOffset() == CBuffCapacity - 1);
        }        

        // resize от начала блока данных
        {            
            const int stepSize = 10;            
            const int startOffset = (CBuffCapacity / 2) - 1;
            buff.SetDataOffset(startOffset);
            ESS_ASSERT(startOffset >= 2 * stepSize);
            // step 1:
            buff.SetDataOffset(buff.GetDataOffset() - stepSize, buff.Size() + stepSize);
            TUT_ASSERT (buff.Size() == stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset - stepSize);
            // step 2:
            buff.AddSpaceFront(stepSize);
            TUT_ASSERT (buff.Size() == 2 * stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset - 2 * stepSize);
            // step 3:
            buff.AddSpaceFront(-stepSize);
            TUT_ASSERT (buff.Size() == stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset - stepSize);
        }

        // resize от конца блока данных
        {            
            const int stepSize = 10;            
            const int startOffset = CBuffCapacity / 2;
            buff.SetDataOffset(startOffset);
            ESS_ASSERT(startOffset + 2 * stepSize <= CBuffCapacity);
            // step 1:
            buff.SetDataOffset(buff.GetDataOffset(), buff.Size() + stepSize);
            TUT_ASSERT (buff.Size() == stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset);
            // step 2:
            buff.AddSpaceBack(stepSize);
            TUT_ASSERT (buff.Size() == 2 * stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset);
            // step 3:
            buff.AddSpaceBack(-stepSize);
            TUT_ASSERT (buff.Size() == stepSize);
            TUT_ASSERT (buff.GetDataOffset() == startOffset);
        }
    }

    //---------------------------------------------------------------------------------------------

    void TestPushAndAcessOperations()
    {
        const byte CBuffCapacity = 128;//Platform::PlatformDataAlign( 128 );        
        Utils::FixedMemBlockPool<> buffPool(CBuffCapacity, 1);
        BidirBuffer buff(buffPool, 0);

        // examination PushBack(byte val), all index access
        {
            buff.SetDataOffset(0);
            byte j = 0;
            while (j < buff.getBlockSize())
            {
                buff.PushBack(j++);
            }
            TUT_ASSERT(buff.Size() == j);

            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {
                TUT_ASSERT (buff.At(i) == i);
                TUT_ASSERT (buff[i] == i);

                const BidirBuffer& constBuff = buff;
                TUT_ASSERT (constBuff.At(i) == i);
                TUT_ASSERT (constBuff[i] == i);
            }
        }

        // examination PushBack(void*,int)
        {
            buff.SetDataOffset(0);
            byte srcData[CBuffCapacity];
            for (byte i = 0; i < CBuffCapacity; ++i)
            {
                srcData[i] = i;
            }

            buff.PushBack(srcData, CBuffCapacity);

            TUT_ASSERT (buff.Size() == CBuffCapacity);
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {                
                TUT_ASSERT (buff[i] == i);
            }
        }

        // examination PushFront(byte val), all index access
        {
            buff.SetDataOffset(CBuffCapacity);

            byte j = 0;
            while (j < buff.getBlockSize())
            {
                buff.PushFront(j++);
            }
            TUT_ASSERT(buff.Size() == j);

            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {                
                TUT_ASSERT (buff[buff.getBlockSize() - 1 - i] == i);
            }
        }

        // examination PushFront(void*,int)
        {
            buff.SetDataOffset(CBuffCapacity);
            byte srcData[CBuffCapacity];
            for (byte i = 0; i < CBuffCapacity; ++i)
            {
                srcData[i] = i;
            }

            buff.PushFront(srcData, CBuffCapacity);

            TUT_ASSERT (buff.Size() == CBuffCapacity);
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {                
                TUT_ASSERT (buff[i] == i);
            }
        }
    }  

    void TestPopOperation()
    {
        const byte CBuffCapacity = Platform::PlatformDataAlign( 128 );        
        Utils::FixedMemBlockPool<> buffPool(CBuffCapacity, 1);
        BidirBuffer buff(buffPool, 0);

        // test PopBack
        {
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {
                buff.PushBack(i);
            }            
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {                
                TUT_ASSERT (buff.PopBack() == (buff.getBlockSize() - i - 1));
            }
            TUT_ASSERT (buff.Size() == 0);
        }

        // test PopFront
        {
            buff.SetDataOffset(CBuffCapacity);
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {
                buff.PushFront(i);
            }            
            for (byte i = 0; i < buff.getBlockSize(); ++i)
            {                
                TUT_ASSERT (buff.PopFront() == (buff.getBlockSize() - i - 1) );
            }
            TUT_ASSERT (buff.Size() == 0);
        }
    }

    //---------------------------------------------------------------------------------------------


    struct AssertTestingFunctions
    {
        enum ErrorType
        {
            // типы ошибок индексного доступа                
            NegaiveInde_At,
            OutOfRange_At,
            NegaiveInde_opIndex,
            OutOfRange_opIndex,
            // типы ошибок при установке смещения
            NegativeOffset_setFn,
            NegativeSize_setFn,
            OutOfRange_setFn,
            // типы ошибок при вызове Back
            NegativeOffset_Back,
            CallWithEmpty_Back,
            OutOfRange_Back,
            // типы ошибок при вызове Front
            CallWithEmpty_Front,
        };

        //-------------------------------------------

        ESS_TYPEDEF_FULL(BreakAbort, ESS::HookRethrow); // can't be defined inside of function
        
        static void DoError(ErrorType errorType)
        {
            // ESS_TYPEDEF_FULL(BreakAbort, ESS::HookRethrow);
            ESS::ExceptionHookRethrow<BreakAbort> hook;

            const int CBuffCapacity = Platform::PlatformDataAlign( 128 );        
            Utils::FixedMemBlockPool<> buffPool(CBuffCapacity, 1);
            BidirBuffer buff(buffPool, 0);  

            bool wasException = false;
            try
            {
                switch (errorType)
                {
                case(NegaiveInde_At):
                    buff.At(-1);            
                case(OutOfRange_At):
                    buff.At(buff.Size());
                case(NegaiveInde_opIndex):
                    buff[-1];            
                case(OutOfRange_opIndex):
                    buff[buff.Size()];
                case(NegativeOffset_setFn):
                    buff.SetDataOffset(-1);
                case(NegativeSize_setFn):
                    buff.SetDataOffset(0, -1);
                case(OutOfRange_setFn):            
                    buff.SetDataOffset(CBuffCapacity, 1);
                case(CallWithEmpty_Back):
                    buff.Back(10);
                case(NegativeOffset_Back):
                    buff.SetDataOffset(0, 10);
                    buff.Back(-1);
                case(OutOfRange_Back):
                    buff.SetDataOffset(0, 10);
                    buff.Back(10);
                case(CallWithEmpty_Front):
                    buff.SetDataOffset(0, 0);
                    buff.Front();
                default:
                    ESS_HALT("Wrong err type");
                }
            }
            catch (/*const*/ BreakAbort &e)
            {
                wasException = true;
            }            
            TUT_ASSERT (wasException);            
        }

        //-------------------------------------------

    public:

        static void TestAssertions()
        {                    
            // examination At
            DoError(NegaiveInde_At);
            DoError(OutOfRange_At);
            // examination operator[]
            DoError(NegaiveInde_opIndex);
            DoError(NegaiveInde_opIndex);
            // examination Set SetDataOffset
            DoError(NegativeOffset_setFn);
            DoError(NegativeSize_setFn);
            DoError(OutOfRange_setFn);
            // examination Back
            DoError(NegativeOffset_Back);
            DoError(CallWithEmpty_Back);
            DoError(OutOfRange_Back);
            // examination Front
            DoError(CallWithEmpty_Front);
        }
    };
    

    

} // namespace 

namespace UtilsTests
{
    void BidirBufferTest()
    {
        TestResizeOperations();
        TestPushAndAcessOperations();     
        TestPopOperation();
        AssertTestingFunctions::TestAssertions();
    }
} // namespace UtilsTests

