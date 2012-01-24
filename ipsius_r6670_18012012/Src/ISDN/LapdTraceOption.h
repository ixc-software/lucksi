#ifndef LAPDTRACEOPTION_H
#define LAPDTRACEOPTION_H

namespace ISDN
{

    struct LapdTraceOption
    {
        LapdTraceOption()
            : m_traceOn(false),
            m_traceRaw(false)
        {}        

        bool m_traceOn;
        bool m_traceRaw;
    };

} // namespace ISDN

#endif









