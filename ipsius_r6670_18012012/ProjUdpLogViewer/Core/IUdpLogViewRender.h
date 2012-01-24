#ifndef __IUDPLOGVIEWRENDER__
#define __IUDPLOGVIEWRENDER__

#include "Utils/IBasicInterface.h"

namespace Ulv
{
    // Base class for render results
    class RenderResult
    {
        int m_startRecord;
        int m_recordsDisplayed;
        int m_recordsAll;

        int m_usedLinesCount;
        int m_maxLinesCount;

    public:
        RenderResult() :
            m_startRecord(0), m_recordsDisplayed(0), m_recordsAll(0),
            m_usedLinesCount(0), m_maxLinesCount(0)
        {}

        virtual ~RenderResult()
        {}

        void Records(int start, int displayed, int all)
        {
            m_startRecord = start;
            m_recordsDisplayed = displayed;
            m_recordsAll = all;
        }

        int StartRecord() const { return m_startRecord; }
        int RecordsDisplayed() const { return m_recordsDisplayed; }
        int RecordsAll() const { return m_recordsAll; }

        void Lines(int used, int max)
        {
            m_usedLinesCount = used;
            m_maxLinesCount = max;
        }

        int UsedLinesCount() const { return m_usedLinesCount; }
        int MaxLinesCount() const { return m_maxLinesCount; }
    };

    // ------------------------------------------------------

    class UdpLogDB;
    class CtrlToModelParams;

    // Base interface for log view render
    class IUdpLogViewRender : Utils::IBasicInterface
    {
    public:
        virtual boost::shared_ptr<RenderResult>
                Render(const UdpLogDB &db, const CtrlToModelParams *pParams) = 0;
    };


} // namespace Ulv

#endif // IUDPLOGVIEWRENDER_H
