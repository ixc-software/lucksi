#ifndef __BFPIO__
#define __BFPIO__

namespace BfDev
{
    
    enum PioPort
    {
        PortF,
        PortG,
        PortH,        
    };
    
    struct BfPioInfo
    {
        typedef volatile unsigned short *Ptr;
        
        Ptr Dir, Clear, Set;
        
        BfPioInfo(Ptr dir, Ptr clear, Ptr set) : Dir(dir), Clear(clear), Set(set) {}
    };
    
    struct BfPio
    {
        static BfPioInfo Get(PioPort pio);
    };
        
}  // namespace BfDev

#endif