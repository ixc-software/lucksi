#ifndef __LOGTIMESTAMP__
#define __LOGTIMESTAMP__

#include "stdafx.h"
#include "Utils/DateTimeCapture.h"
// #include "LogTimestampT.h"

namespace iLog
{

    class Timestamp
    {
        Utils::DateTimeCapture m_dt;

    public:

        enum CaptureModeType
        {
            CM_None,
            CM_Full,
        };

        Timestamp(const CaptureModeType &mode = DefaultCaptureMode()) 
        {
            if (mode == CM_Full) 
            {
                Capture();
            }
        }

        bool IsEmpty() const
        {
            return m_dt.IsNull();
        }

        int Compare(const Timestamp &other) const
        {
            return m_dt.Compare(other.m_dt);
        }

        const Utils::DateTime& get() const 
        { 
            return m_dt.DateAndTime(); 
        } 

        void Capture()
        {
            m_dt.Capture();

            /*
            Utils::DateTimeCapture c;
            c.Capture();

            m_dt = c.DateAndTime(); */
        }

        static CaptureModeType DefaultCaptureMode()
        {
            return CM_Full;
        }

        static CaptureModeType DefaultSystemCaptureMode()
        {
            return DefaultCaptureMode();
        }

        static Timestamp EmptyTimestamp();

    };


}  // namespace iLog

#endif

