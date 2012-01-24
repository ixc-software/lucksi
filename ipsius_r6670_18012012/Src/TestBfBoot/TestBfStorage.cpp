#include "stdafx.h"
#include "TestBfStorage.h"

#include "BfBootSrv/BfStorage.h"
#include "BfBootSrv/FieldList.h"
#include "BfBootSrv/FlashIOStream.h"

#include "FlashEmul.h"

namespace 
{
    namespace Srv = BfBootSrv;
    namespace Test = TestBfBoot;
    using Platform::byte;
    using Platform::dword;

    void FillVector(std::vector<byte>& out)
    {
        for (int i = 0; i < out.size(); ++i)
        {
            out[i] = i;
        }
    }

    // простая проверка нормальной работы
    void TestStorage()
    {
        Test::BfRangeStorageDevEmul flashEmul;
        Srv::RangeIOBase& flash = flashEmul.getRangeIO();
        
        ESS_ASSERT("Need more then 3 sector for this test" && range.Count >= 3);
        //const dword CCapacityByte = flash.RangeToSize();

        Srv::Config cfg;
        Srv::BfStorage imgStorage(cfg, flash);

        // writing through sector limit, read, compare
        {            
            const int CUseSectors = 2;
            dword size = 0;
            for (int i = 0; i < CUseSectors; ++i) // size = сумма размеров екторов
            {
                 size += flash.getSectorInfInRangeByIndex(range, i).Size;
            }

            std::vector<byte> data(size);
            FillVector(data);
            
            imgStorage.Open(size);
            const int CChunkCount = 10;

            int pos = 0;
            for (int i = 0; i < CChunkCount; ++i)
            {                
                int chunkSize = (i == CChunkCount - 1) ? size - pos : size/CChunkCount;
                imgStorage.Write(&data[pos], chunkSize);
                pos += chunkSize;
            }
            const std::string CImgName("Image_probe");
            Srv::ImgDescriptor descr = imgStorage.Close(CImgName);
            /*end writing*/

            TUT_ASSERT( cfg.AppImgTable.Value().size() == 1 );
            TUT_ASSERT( cfg.AppImgTable.Value().at(0).Size == size );
            TUT_ASSERT( cfg.AppImgTable.Value().at(0).Name == CImgName );

            std::vector<byte> rxData(size);
            for (int i = 0; i < CChunkCount; ++i)
            {
                int pos = 0;
                int chunkSize = (i == CChunkCount - 1) ? size - pos : size/CChunkCount;

                imgStorage.Read(descr, pos, &rxData[pos], chunkSize);
                pos += chunkSize;
            }
            TUT_ASSERT(rxData == data);

            
            //TUT_ASSERT(!imgStorage.Open(1));
            
        }        

        // OutOfRange (requested size)
        {
            imgStorage.DeleteAll();
            dword maxSize = flash.RangeToSize(range);
            try
            {
                imgStorage.Open(maxSize + 1);
                TUT_ASSERT(0 && "Expected CantOpen error");
            }
            catch(const Srv::BfStorage::CantOpen&)
            {}                        

            imgStorage.Open(maxSize);
            std::vector<byte> data(maxSize + 1);
            try
            {
                imgStorage.Write(data);
                TUT_ASSERT(0 && "Expected OutOfRange error");
            }
            catch (const Srv::BfStorage::OutOfRange& err)
            {
            }
        }
    }
} // namespace 

namespace TestBfBoot
{
    void TestBfStorage()
    {
        TestStorage();
    }
} // namespace TestBfBoot
