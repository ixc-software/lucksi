
#ifndef __MFPACKET__
#define __MFPACKET__

// MfPacket.h

#include "MfProtocol.h"
#include "Utils/BinaryReader.h"
#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"
#include "Utils/ManagedList.h"
#include "Utils/IntToString.h"

namespace MiniFlasher
{
    using Platform::dword;
    
    // Packet data field.
    class MfPacketField
    {        
    public:
        enum FieldType
        {
            FDword = 0,
            FBinary,
            FError,
        };

    public:
        ESS_TYPEDEF(UnknownProtocolError);
        
    public:
        MfPacketField(const std::string &name, FieldType type, 
                      void *pData, int size)                 // can throw
            : m_name(name), m_type(type)
        {
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(size != 0);
            
            if ((type == FDword) || (type == FError)) ESS_ASSERT(size == sizeof(dword));

            m_stream.Write(pData, size);
            
            if ((type == FError) && (!Protocol::IsValidError(ReadDword())))
            {
                ESS_THROW_MSG(UnknownProtocolError, 
                              Utils::IntToHexString(ReadDword()));
            }
        }

        dword AsDword() const
        {
            ESS_ASSERT(m_type == FDword);
            
            return ReadDword();
        }

        const void* AsBinary() const
        {
            ESS_ASSERT(m_type == FBinary);

            return m_stream.GetBuffer();
        }

        Protocol::Error AsError() const
        {
            ESS_ASSERT(m_type == FError);
            
            return static_cast<Protocol::Error>(ReadDword());
        }

        int Size() const { return m_stream.BufferSize(); }
        FieldType Type() const { return m_type; }
        const std::string& Name() const { return m_name; }

    private:

        dword ReadDword() const
        {
            ESS_ASSERT(Size() == sizeof(dword));

            Utils::MemReaderStream st(m_stream.GetBuffer(), m_stream.BufferSize());
            Utils::BinaryReader<Utils::MemReaderStream> reader(st);
            
            return reader.ReadDword();
        }
        
        std::string m_name;
        FieldType m_type;
        Utils::MemWriterDynStream m_stream;
    };

    // ----------------------------------------------------

    // Created from raw data that was received by MfClient.
    // To create packet of needed type use MfPacketFactory
    class MfPacket
    {
        Protocol::RespCode m_type;
        Utils::ManagedList<MfPacketField> m_fields;

        MfPacketField* Find(const std::string &name) const
        {
            for (int i = 0; i < Count(); ++i)
            {
                if (m_fields[i]->Name() == name) return m_fields[i];
            }
            return 0;
        }

    public:
        MfPacket(Protocol::RespCode type) : m_type(type), m_fields(true)
        {
        }

        void Add(MfPacketField *pField)
        {
            ESS_ASSERT(pField != 0);
            m_fields.Add(pField);
        }

        // name - CFieldX constant using on packet registration
        const MfPacketField& Get(const std::string &name) const
        {
            MfPacketField *pRes = Find(name);
            if (pRes == 0)
            {
                QString msg = QString("Invalid packet field name: %1").arg(name.c_str());
                ESS_HALT(msg.toStdString());
            }

            return *pRes;
        }

        const MfPacketField& operator[](const std::string &name) const { return Get(name); }

        Protocol::RespCode Type() const { return m_type; }
        size_t Count() const { return m_fields.Size(); }
    };

    // ----------------------------------------------------

    /* Example:

    void Process(const MfPacket &pack)
    {
        if (pack.Type() = Protocol::RespFlashTestProgress)
        {
            dword percent = pack[CFieldPercent].AsDword();
            dword errors = pack[CFieldErorrs].AsDword();
        }

        // ...
    }
    */
    
} // namespace MiniFlasher

#endif
