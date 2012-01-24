
#include "stdafx.h"
#include "MfPacketFactory.h"
#include "MfPacketFieldNames.h"
#include "MfProtocol.h"

namespace MiniFlasher
{
    // -----------------------------------------------------------
    // MfPacketFactory impl
    
    MfPacketFactory::PacketType* MfPacketFactory::FindTypeDesc(Protocol::RespCode type)
    {
        for (int i = 0; i < m_factory.Size(); ++i)
        {
            if (m_factory[i]->Type == type) return m_factory[i];
        }

        return 0;
    }

    // -----------------------------------------------------------
    
    MfPacketField* MfPacketFactory::ReadDword(const std::string &fieldName, 
                                              Utils::IBinaryReader &reader)
    {
        dword val = reader.ReadDword();
        return new MfPacketField(fieldName, MfPacketField::FDword, &val, sizeof(val));
    }

    // -----------------------------------------------------------

    MfPacketField* MfPacketFactory::ReadError(const std::string &fieldName, 
                                              Utils::IBinaryReader &reader)
    {
        dword val = reader.ReadDword();
        return new MfPacketField(fieldName, MfPacketField::FError, &val, sizeof(val));
    }

    // -----------------------------------------------------------

    MfPacketField* MfPacketFactory::ReadBinary(const std::string &fieldName, 
                                               Utils::IBinaryReader &reader)
    {
        const int CDataOffs = 1 + (4);  // byte cmd + dword size

        dword size = reader.ReadDword();
        if (size > Protocol::CMaxPacketBodySize)
        {
            QString msg = QString("Too big packet size: %1").arg(size);
            ESS_THROW_MSG(BadDataSize, msg.toStdString());
        }
        
        std::vector<Platform::byte> buff(size, 0);
        reader.ReadData(&buff.at(0), buff.size());

        return new MfPacketField(fieldName, MfPacketField::FBinary, 
                                 &buff.at(0), buff.size());
    }

    // -----------------------------------------------------------

    void MfPacketFactory::RegisterType(Protocol::RespCode type, 
                                       FieldDesc *pF0,
                                       FieldDesc *pF1, 
                                       FieldDesc *pF2)
    {
        PacketType *p= new PacketType(type);
        if (pF0 != 0) p->Fields.Add(pF0);
        if (pF1 != 0) p->Fields.Add(pF1);
        if (pF2 != 0) p->Fields.Add(pF2);

        m_factory.Add(p);
    }

    // -----------------------------------------------------------

    MfPacket* MfPacketFactory::Create(Protocol::RespCode type, 
                                      Utils::IBinaryReader &reader)
    {
        PacketType *pType = FindTypeDesc(type);
        ESS_ASSERT(pType != 0);

        MfPacket *pRes = new MfPacket(type);
        for (int i = 0; i < pType->Fields.Size(); ++i)
        {
            pRes->Add(pType->Fields[i]->FnRead(pType->Fields[i]->Name, reader));
        }

        return pRes;
    }

    // -----------------------------------------------------------
    // Register packets here ---v
    
    MfPacketFactory::MfPacketFactory()
    {
        typedef MfPacketFactory T;
        using namespace MfPacketFieldNames;
        
        RegisterType(Protocol::RespErased);
        RegisterType(Protocol::RespWrited);
        RegisterType(Protocol::RespRunned);

        RegisterType(Protocol::RespError,              
                     new FieldDesc(FCode, &T::ReadError));

        RegisterType(Protocol::RespConnected,          
                     new FieldDesc(FVersion, &T::ReadDword));
        
        RegisterType(Protocol::RespEraseProgress,      
                     new FieldDesc(FSize,    &T::ReadDword));
        
        RegisterType(Protocol::RespVerifyProgress,     
                     new FieldDesc(FSize,    &T::ReadDword));
        
        RegisterType(Protocol::RespVerifyDone,         
                     new FieldDesc(FCRC32,   &T::ReadDword));

        RegisterType(Protocol::RespReaded,             
                     // new FieldDesc(FSize,    &T::ReadDword),
                     new FieldDesc(FData,    &T::ReadBinary));

        RegisterType(Protocol::RespFlashTestCompleted, 
                     new FieldDesc(FErrors,  &T::ReadDword));
        
        RegisterType(Protocol::RespFlashTestProgress,  
                     new FieldDesc(FPercent, &T::ReadDword),
                     new FieldDesc(FErrors,  &T::ReadDword));

        // ...
    }
    
} // namespace MiniFlasher

