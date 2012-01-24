#ifndef L2ERROR_H
#define L2ERROR_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{
    // L2 error. Described in table II-1/Q.921 (page 145)
    class L2Error
    {
        const char m_MDLErrorCode;

    public:
        explicit L2Error(char errorCode)
            : m_MDLErrorCode(errorCode)
        {
            ESS_ASSERT( errorCode >= 'A' && errorCode <= 'O');
        }

        std::string getAsString() const
        {
            std::string result;
            if (m_MDLErrorCode >= 'A' && m_MDLErrorCode <= 'E')  result = "Receipt of unsolicited response.";
            if (m_MDLErrorCode >= 'F')  result = "Peer initiated reestablish.";
            if (m_MDLErrorCode >= 'G' && m_MDLErrorCode <= 'I')  result = "Unsuccessful retransmission (T200 max expired).";
            if (m_MDLErrorCode >= 'J' && m_MDLErrorCode <= 'O')  result = "Receipt of unsolicited response.";

            result.append(" Error code ").push_back(m_MDLErrorCode);
            return result;
        }

        char getErrorCode() const
        {
            return m_MDLErrorCode;
        }
    };
} // namespace ISDN

#endif
