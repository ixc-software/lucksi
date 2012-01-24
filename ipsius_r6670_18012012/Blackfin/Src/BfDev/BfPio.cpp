#include "stdafx.h"

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "BfPio.h"

// -----------------------------------------

namespace BfDev
{

    BfPioInfo BfPio::Get(PioPort pio)
    {
        if (pio == PortF) return BfPioInfo(pPORTFIO_DIR, pPORTFIO_CLEAR, pPORTFIO_SET); 
        if (pio == PortG) return BfPioInfo(pPORTGIO_DIR, pPORTGIO_CLEAR, pPORTGIO_SET); 
        if (pio == PortH) return BfPioInfo(pPORTHIO_DIR, pPORTHIO_CLEAR, pPORTHIO_SET); 

        ESS_HALT("");                
        return BfPioInfo(0, 0, 0);
    }   

}  // namespace BfDev
