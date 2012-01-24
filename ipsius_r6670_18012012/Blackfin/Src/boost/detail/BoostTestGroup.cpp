
#include "stdafx.h"

#include "BoostTestGroup.h"
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"
#include "boost/mpl/if.hpp"


namespace
{
    using namespace boost::detail;
	using namespace TestFw;

    class BoostTestsGroup : public TestGroup<BoostTestsGroup>
    {
        BoostTestsGroup(ILauncher &launcher) 
		: TestGroup<BoostTestsGroup>(launcher, "Boost")             
        {
            bool silentMode = true;
            
            Add("shared_ptr<> tests", &shared_ptr_test, silentMode);
            Add("scoped_ptr<> tests", &scoped_ptr_test, silentMode);
            // static tests 
            Add("Static assert test", &boost_static_assert_test);
            Add("is_base_of<> test", &is_base_of_test);
            Add("is_same<> test", &is_same_test);
            Add("mpl::if_<> / if_c<> test", &BoostMplIfTest);
			// ... 
       }

    public:
        static void AddToLauncher(ILauncher &launcher)
        {
            new BoostTestsGroup(launcher);  // free at launcher
        }
    }; 
    
} // namespace

// ------------------------------------------------

namespace boost
{
    namespace detail
    {
        void RegisterTestGroup(TestFw::ILauncher &launcher)
        {
            BoostTestsGroup::AddToLauncher(launcher);
        }

    } // namespace detail

} // namespace boost 

