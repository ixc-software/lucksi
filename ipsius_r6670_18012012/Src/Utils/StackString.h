#ifndef __STACKSTRING__
#define __STACKSTRING__

#include "Platform/Platform.h"
#include "Utils/ComparableT.h"
#include "Utils/IntToString.h"

namespace Utils
{
    enum { CDefaultStackStringSize = 160};

    // Quick, stack-based string class
    // Overflow is silence
    template<int BuffSize>
    class StackStringT:
        public ComparableT<StackStringT<BuffSize> >
    {
        char m_buff[BuffSize + 1]; // +1 for \0
        int  m_length;
        bool m_overflowed;

        void Assert(bool exp, const char *pMsg) const
        {
            if (!exp) Platform::ExitWithMessage(pMsg);
        }

        void DoOverflowed()
        {
            m_overflowed = true;

            // change last char to special
            m_buff[m_length-1] = '~';
        }

        // This is single point of m_length modification (except Clear())
        bool AddChar(char c)
        {
            if (m_overflowed) return false;

            if (m_length == BuffSize) 
            {
                DoOverflowed();
                return false;
            }

            // never happend, paranoic
            Assert(m_length < BuffSize, "StackString::AddChar");
            
            m_buff[m_length++] = c;

            return true;
        }

    public:

        StackStringT() 
        {
            Clear();
        }

        template<int OtherBuffSize>
        StackStringT(const StackStringT<OtherBuffSize> &other)
        {
            Clear();
            Add(other);
        }

        StackStringT(const char *p)
        {
            Clear();
            Add(p);
        }

        StackStringT(const std::string &s)
        {
            Clear();
            Add(s);
        }

        template<int OtherBuffSize>
        void operator=(const StackStringT<OtherBuffSize> &other)
        {
            Clear();
            Add(other);
        }

        void Clear()
        {
            m_length = 0;
            m_overflowed = false;
        }

        int Length() const
        {
            return m_length;
        }

        bool Overflowed() const
        {
            return m_overflowed;
        }

        template<int OtherBuffSize>
        void Add(const StackStringT<OtherBuffSize> &other)
        {
            Add(other.c_str());
        }

        void Add(const std::string &s)
        {
            Add(s.c_str());
        }

        void Add(const char *p)
        {
            Assert(p != 0, "StackString::Add empty");

            if (m_overflowed) return;

            while(char c = *p++)
            {
                if (!AddChar(c)) break;
            }
        }

        void AddInt(int i)
        {
            char buff[16];

            // itoa(i, buff, 10);
            bool ok = Utils::IntToString(i, buff, sizeof(buff));
            Assert(ok, "StackString::AddInt int to string convertion failed");

            Add(buff);
        }

        void AddHex(int i, int bytesToAdd = 4)
        {
            char buff[16];

            bool ok = Utils::IntToHexString(i, buff, sizeof(buff), false, bytesToAdd);
            Assert(ok, "StackString::AddHex int to string convertion failed");

            Add(buff);
        }

        const char* c_str() const
        {
            // this method has to be const to get data from const objects
            // (using inside Add() and Compare())
            // patch \0, m_length always < BuffSize
            const_cast<StackStringT<BuffSize>* >(this)->m_buff[m_length] = 0;

            return m_buff;
        }

        char& operator[](int indx)
        {
            // Assert(indx < m_length, "StackString::[]");
            Assert(((indx >= 0) && (indx < m_length)), "StackString::[]");

            return m_buff[indx];
        }

        const char& operator[](int indx) const
        {
            // Assert(indx < m_length, "StackString::[]");
            Assert(((indx >= 0) && (indx < m_length)), "StackString::[]");

            return m_buff[indx];
        }

        int Compare(const char *p) const
        {
            return std::strcmp(c_str(), p);
        }

        template<int OtherBuffSize>
        int Compare(const StackStringT<OtherBuffSize> &other) const
        {
            return Compare(other.c_str());
        }

        template<int OtherBuffSize>
        void operator+=(const StackStringT<OtherBuffSize> &other)
        {
            Add(other.c_str());
        }

        void operator+=(const char *p)
        {
            Add(p);
        }
        
        void operator+=(const std::string &s)
        {
            Add(s.c_str());
        }

        // operators ==, !=, >, >=, <, <= -- via ComparableT
        
        // operator +
    };

    typedef StackStringT<CDefaultStackStringSize> StackString;

}  // namespace Utils

#endif

