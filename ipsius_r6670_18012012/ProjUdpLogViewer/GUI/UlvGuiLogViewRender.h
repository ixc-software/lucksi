#ifndef __ULVGUILOGVIEWRENDER__
#define __ULVGUILOGVIEWRENDER__

#include "Core/IUdpLogViewRender.h"
#include "Core/ModelToViewParams.h"
#include "Core/UdpLogDB.h"

#include "UlvGuiCtrlToModelParams.h"
#include "UlvGuiLogToQImage.h"
#include "Core/UlvTypes.h"


namespace UlvGui
{
    using Ulv::ddword;

    class GuiRenderResult :
        public Ulv::RenderResult
    {
        boost::shared_ptr<QImage> m_img;

    public:
        GuiRenderResult(boost::shared_ptr<QImage> img) : m_img(img)
        {}

        boost::shared_ptr<QImage> Get() const { return m_img; }
    };

    // -------------------------------------------------------------------

    class  GuiLogViewRender :
        public Ulv::IUdpLogViewRender
    {
    // IUdpLogViewRender impl
    private:
        boost::shared_ptr<Ulv::RenderResult> Render(const Ulv::UdpLogDB &db,
                                                    const Ulv::CtrlToModelParams *pParams)
        {
            ESS_ASSERT(pParams != 0);

            // cast common params into GUI params
            const GuiCtrlToModelParams *p = dynamic_cast<const GuiCtrlToModelParams*>(pParams);
            ESS_ASSERT(p != 0);

            // render image
            LogToQImageOutParams outParams;
            boost::shared_ptr<QImage> img = LogToQImage(db, *p, outParams);

            // make result
            boost::shared_ptr<GuiRenderResult> res(new GuiRenderResult(img));
            res->Records(outParams.StartRecIndex, outParams.RenderedRecordsCount, db.RecordCount());
            res->Lines(outParams.RenderedLinesCount, outParams.MaxImageLinesCount);

            return res;
        }

    public:
        GuiLogViewRender()
        {}
    };


} // namespace UlvGui

#endif
