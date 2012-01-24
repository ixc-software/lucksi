#ifndef __BFFREQINIT__
#define __BFFREQINIT__

namespace BfDev
{

    class BfFreqInit
    {
        
        static void FreqInit(int pllDiv, int pllCtl, int &frequencyCpu, int &frequencySys)
        {
        	const int CFrequencyGen = 25000000;
        	
        	*pPLL_DIV = pllDiv; 
            *pPLL_CTL = pllCtl; 
            idle();	
            
            // info
        	int t = (*pPLL_DIV) & 0x0F;
        
        	int frequencyGen = CFrequencyGen;
        	frequencyCpu = *pPLL_CTL;
        	if (frequencyCpu & 1) frequencyGen /= 2;
        
        	frequencyCpu >>= 9;
        	frequencyCpu &= 0x3F;
        	
        	if(frequencyCpu == 0) frequencyCpu = frequencyGen * 64; 
        	else                  frequencyCpu *= frequencyGen;
        	
        	frequencySys = frequencyCpu / t;            	
        }
        
    public:
                        
        static void FreqInit120MHz(int &frequencyCpu, int &frequencySys)
        {
        	const int CBusFreq = 120 * 1000 * 1000;
        	const int C_PLL_DIV = 0x0005;
            const int C_PLL_CTL = 0x3000;
            
        	FreqInit(C_PLL_DIV, C_PLL_CTL, frequencyCpu, frequencySys);    
        	ESS_ASSERT(frequencySys == CBusFreq);
        }
                
    };    
    
    
}  // namespace BfDev

#endif

