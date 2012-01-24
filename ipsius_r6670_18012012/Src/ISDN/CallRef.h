#ifndef CALLREF_H
#define CALLREF_H

#include "stdafx.h"
#include "IIsdnAlloc.h"
#include "Platform/Platform.h"

namespace ISDN
{
    using boost::shared_ptr;
    using Platform::byte;
    using Platform::dword;

    class CallRefGenerator;

    // Метка соединения. Содержит значение и флаг стороны
    class CallRef
    {
        friend class CallRefGenerator;

        dword m_value;
        bool m_Flag; // true if generated on this side

        dword m_maxValue; // assign cref size


        void IncrValue();
        void ReplaceValue(dword value);
        explicit CallRef(int value, dword maxValue);

    public:        

        // Extract CallRef from L2Packet
        CallRef(QVector<byte> l2data, IIsdnAlloc& alloc);

        bool operator == (const CallRef& rhs) const;

        bool operator != (const CallRef& rhs) const;
        
		dword GetValue() const;

        std::string ToString(bool onlyValue = true) const;

        void AddBackCRef (std::vector<byte>& out) const;

        bool IsGlobal() const;
    
    };

} // ISDN

#endif

