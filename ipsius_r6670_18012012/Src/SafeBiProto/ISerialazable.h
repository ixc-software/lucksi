#ifndef __ISBPSERIALAZABLE__
#define __ISBPSERIALAZABLE__

#include "Utils/IBasicInterface.h"

namespace SBProto
{
    
    class ISerialazable : public Utils::IBasicInterface
    {
    public:

        /* 
            Class must implement

            template<class Data, class TStream>
            static void Serialize(Data &data, TStream &s)

        */
    };

    // -----------------------------------------------------
    
    class PacketRead;
    class PacketWrite;
    
    template<class T>
    class SerialazableT
    {

    protected:

        SerialazableT()
        {
            const static bool CBase = boost::is_base_of<SerialazableT<T>, T>::value;
            BOOST_STATIC_ASSERT(CBase);
        }

    public:

        PacketRead& operator << (PacketRead &s)
        {
            T::Serialize(*this, s);
            return s;
        }
        
        PacketWrite& operator << (PacketWrite &s) const 
        {
            T::Serialize(*this, s);
            return s;            
        }

    };
        
    
}  // namespace SBProto

#endif