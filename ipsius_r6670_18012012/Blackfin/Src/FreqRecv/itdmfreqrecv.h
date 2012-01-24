#ifndef __ITDMFREQRECV__
#define __ITDMFREQRECV__

namespace FreqRecv
{
    
    // интерфейс управления приемником частоты (удаленным)
    class ITdmFreqRecvRemote : public Utils::IBasicInterface
    {
    public:

        virtual bool Equal(const std::string &params) = 0;

        virtual void On()    = 0;
        virtual void Off()   = 0;
    };

    class ITdmFreqRecvLocal : public ITdmFreqRecvRemote
    {
    public:

        virtual bool WaitData() = 0;
        virtual void ProcessData(const std::vector<Platform::int16> &data) = 0;

        virtual std::string PeekEvent() = 0;        

    };
    
    
}  // namespace FreqRecv

#endif
