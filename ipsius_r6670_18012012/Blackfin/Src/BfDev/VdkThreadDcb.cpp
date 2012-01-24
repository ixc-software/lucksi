#include "stdafx.h"

#include "VdkThreadDcb.h"


// ---------------------------------------------

namespace 
{
	
	int GThreadDcbCount = 0;
	
}  // namespace 


namespace BfDev
{
	
	// This is NOT thread safe code!
	void VdkThreadDcb::AllocDcb()
	{
		ESS_ASSERT(GThreadDcbCount == 0);
		
		GThreadDcbCount++;
	}
	
	void VdkThreadDcb::FreeDcb()
	{
		ESS_ASSERT(GThreadDcbCount > 0);
		
		GThreadDcbCount--;
	}


}  // namespace BfDev

// ---------------------------------------------

namespace 
{

    struct Data
    {
        int *pValue;
        int diff;
    };

    class Exec
    {
    public:

        void Execute(Data data)
        {
            ESS_ASSERT(data.pValue != 0);

            *(data.pValue) = *(data.pValue) + data.diff;
        }
    };

}  // namespace 

// ---------------------------------------------

namespace BfDev
{
		
    void VdkThreadDcbTest()
    {
        const int CQueueSize = 32;

        VdkThreadDcb queue(CQueueSize);

        int value = 0;
        int summ = 0;

        Exec exec;

        Data data;
        data.pValue = &value;

        for(int i = 0; i < 16; ++i)
        {
            // put
            data.diff = (i + 1);
            queue.PutMsg(&Exec::Execute, exec, data);
            summ += data.diff;

            // put
            data.diff = (i + 2);
            queue.PutMsg(&Exec::Execute, exec, data);
            summ += data.diff;

            // sleep
            Platform::ThreadSleep(1);
        }


        TUT_ASSERT(summ == value);

    }

}  // namespace BfDev
