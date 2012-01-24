#ifndef __ILOG_HDLC__
#define __ILOG_HDLC__


namespace Ds2155
{
    using namespace Platform;

    class ILogHdlc : public Utils::IBasicInterface
    {
    	
    public:
    
    	virtual void Add(const std::string str) = 0;
    	
    };
	
}; //namespace Uart

#endif







