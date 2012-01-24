#ifndef __RECURSIVETHROW__
#define __RECURSIVETHROW__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

namespace BfPort
{

	namespace Detail
	{
		
        class IExecutive : Utils::IBasicInterface
        {
        public:
           virtual void Execute() = 0;
        };

        template<class TException>	
        class ThrowException : public IExecutive
        {
        // IExecutive impl
        private:
            void Execute()
            {
                ESS::DoException<TException>("", __FILE__, __LINE__);
            }
        };

        int ExecuteAtRecurseLevel(int level, IExecutive &exe);
		
	}  // namespace Detail


	template<class TException>	
	void RecursiveThrow(int atLevel)
	{
        Detail::ThrowException<TException> t;
        Detail::ExecuteAtRecurseLevel(atLevel, t);
	}

    inline void InlineFn()
    {
    }
		
}  // namespace BfPort

#endif
