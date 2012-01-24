#ifndef __SBPSERIALIZE__
#define __SBPSERIALIZE__

#include "Platform/PlatformTypes.h"
#include "SbpRecvPack.h"
#include "SbpSendPack.h"
#include "ISerialazable.h"

namespace SBProto 
{
    namespace mpl = boost::mpl; 

    using Platform::byte;
    using Platform::int64;
    using Platform::word;
    using Platform::dword;

    class PacketRead : boost::noncopyable
    {
        const SbpRecvPack &m_pack; 
        int m_currIndx;
        
        struct ReadObject
        {
            template<class T>
            static void Read(PacketRead &self, T &obj)
            {
                T::Serialize(obj, self);
            }
        };

        struct ReadEnum
        {
            template<class TEnum>
            static void Read(PacketRead &self, TEnum &v)
            {
                BOOST_STATIC_ASSERT( boost::is_enum<TEnum>::value );
                BOOST_STATIC_ASSERT( sizeof(TEnum) == sizeof(int32) );

                int32 val;
                self << val;       // read as int32
                v = (TEnum)(val);  // write enum
            }
        };

    public:

        PacketRead(const SbpRecvPack &pack, int fromIndx) : m_pack(pack), m_currIndx(fromIndx)
        {
        }

        int Index() const { return m_currIndx; }

        PacketRead& operator<<(std::string &v)
        {
            v = m_pack[m_currIndx++].AsString();
            return *this;
        }

        /*
        PacketRead& operator<<(int &v)
        {
            v = m_pack[m_currIndx++].AsInt32();
            return *this;
        } */

        PacketRead& operator<<(int32 &v)
        {
            v = m_pack[m_currIndx++].AsInt32();
            return *this;
        }

        PacketRead& operator<<(int64 &v)
        {
            v = m_pack[m_currIndx++].AsInt64();
            return *this;
        }

        PacketRead& operator<<(dword &v)
        {
            v = m_pack[m_currIndx++].AsDword();
            return *this;
        }

        PacketRead& operator<<(word &v)
        {
            v = m_pack[m_currIndx++].AsWord();
            return *this;
        }

        PacketRead& operator<<(bool &v)
        {
            v = m_pack[m_currIndx++].AsBool();
            return *this;
        }

        PacketRead& operator<<(byte &v)
        {
            v = m_pack[m_currIndx++].AsByte();
            return *this;
        }

        /*
        template<class TEnum>
        PacketRead& operator<<(TEnum &v)
        {
            BOOST_STATIC_ASSERT( boost::is_enum<TEnum>::value );
            BOOST_STATIC_ASSERT( sizeof(TEnum) == sizeof(int32) );

            v = (TEnum)( m_pack[m_currIndx++].AsInt32() );
            return *this;
        } */

        template<class T>
        PacketRead& operator<<(T &v)
        {
            const bool isObject = boost::is_base_of<ISerialazable, T>::value;
            typedef typename mpl::if_c<isObject, ReadObject, ReadEnum>::type Tx;

            Tx::Read(*this, v);
            
            return *this;
        }


    };

    // ---------------------------------------------

    class PacketWrite : boost::noncopyable
    {
        SbpSendPack &m_pack;

        struct WriteObject
        {
            template<class T>
            static void Write(PacketWrite &self, const T &obj)
            {
                T::Serialize(obj, self);
            }
        };

        struct WriteEnum
        {
            template<class TEnum>
            static void Write(PacketWrite &self, const TEnum &v)
            {
                BOOST_STATIC_ASSERT( boost::is_enum<TEnum>::value );
                BOOST_STATIC_ASSERT( sizeof(TEnum) == sizeof(int32) );

                self.m_pack.WriteInt32(v);
            }
        };

    public:

        PacketWrite(SbpSendPack &pack) : m_pack(pack)
        {
        }

        PacketWrite& operator<<(const std::string &v)
        {
            m_pack.WriteString(v);
            return *this;
        }

        /*
        PacketWrite& operator<<(const int &v)
        {
            m_pack.WriteInt32(v);
            return *this;
        } */

        PacketWrite& operator<<(const int32 &v)
        {
            m_pack.WriteInt32(v);
            return *this;
        }


        PacketWrite& operator<<(const int64 &v)
        {
            m_pack.WriteInt64(v);
            return *this;
        }

        PacketWrite& operator<<(const dword &v)
        {
            m_pack.WriteDword(v);
            return *this;
        }

        PacketWrite& operator<<(const word &v)
        {
            m_pack.WriteWord(v);
            return *this;
        }

        PacketWrite& operator<<(const bool &v)
        {
            m_pack.WriteBool(v);
            return *this;
        }

        PacketWrite& operator<<(const byte &v)
        {
            m_pack.WriteByte(v);
            return *this;
        }

        /*
        template<class TEnum>
        PacketWrite& operator<<(const TEnum &v)
        {
            BOOST_STATIC_ASSERT( boost::is_enum<TEnum>::value );
            BOOST_STATIC_ASSERT( sizeof(TEnum) == sizeof(int32) );

            m_pack.WriteInt32(v);

            return *this;
        } */

        template<class T>
        PacketWrite& operator<<(const T &v)
        {
            const bool isObject = boost::is_base_of<ISerialazable, T>::value;
            typedef typename mpl::if_c<isObject, WriteObject, WriteEnum>::type Tx;

            Tx::Write(*this, v);

            return *this;
        } 
        

    };
    
}  // namespace SBProto 

#endif
