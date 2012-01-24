
#ifndef __MFPACKETFACTORY__
#define __MFPACKETFACTORY__

// MfPacketFactory.h

#include "MfPacket.h"
#include "Utils/IBinaryReader.h"
#include "Utils/ManagedList.h"

namespace MiniFlasher
{
    // MfPacket factory.
    // Register all packets in constructor
    class MfPacketFactory 
    { 
        typedef MfPacketField* (*PFnRead)(const std::string&, Utils::IBinaryReader&);

        struct FieldDesc 
        {
            std::string Name;
            PFnRead FnRead;

            FieldDesc(const std::string &name, PFnRead fnRead) :
                    Name(name), FnRead(fnRead)
            {
            }
        };

        struct PacketType
        {
            Protocol::RespCode Type;
            Utils::ManagedList<FieldDesc> Fields;

            PacketType(Protocol::RespCode type) : Type(type), Fields(true)
            {
            }
        };

        Utils::ManagedList<PacketType> m_factory;

        PacketType* FindTypeDesc(Protocol::RespCode type);

    // For type registration
    private:
        static MfPacketField* ReadDword(const std::string &fieldName, 
                                        Utils::IBinaryReader &reader);
        static MfPacketField* ReadBinary(const std::string &fieldName, 
                                         Utils::IBinaryReader &reader);
        static MfPacketField* ReadError(const std::string &fieldName, 
                                        Utils::IBinaryReader &reader);

        void RegisterType(Protocol::RespCode type, 
                          FieldDesc *pF0 = 0,
                          FieldDesc *pF1 = 0, 
                          FieldDesc *pF2 = 0);

    public: 
        ESS_TYPEDEF(BadDataSize);
        
    public:
        MfPacketFactory();

        MfPacket* Create(Protocol::RespCode type, 
                         Utils::IBinaryReader &reader); // can throw 
                                                        // MfPacketFactory::BadDataSize,
                                                        // MfPacketField::UnknownProtocolError
    };

    
} // namespace MiniFlasher

#endif
