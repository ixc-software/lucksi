#ifndef __MODELTOVIEWPARAMS__
#define __MODELTOVIEWPARAMS__

#include "Utils/ErrorsSubsystem.h"

#include "IUdpLogViewRender.h"
#include "UdpLogRecordSrc.h"


namespace Ulv
{
    class ModelToViewParams
    {
        boost::shared_ptr<RenderResult> m_renderRes;
        UdpLogRecordSrcInfoList m_srcList;

    public:
        ModelToViewParams(boost::shared_ptr<RenderResult> renderRes,
                          const UdpLogRecordSrcInfoList &srcList) :
            m_renderRes(renderRes), m_srcList(srcList)
        {
            ESS_ASSERT(renderRes != 0);
        }

        boost::shared_ptr<RenderResult> RenderRes() const { return m_renderRes; }
        const UdpLogRecordSrcInfoList& SrcList() const { return m_srcList; }
    };

} // namespace Ulv


#endif
