// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#ifndef __STDAFX__
#define __STDAFX__


#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4311) // 'variable' : pointer truncation from 'type' to 'type'
    #pragma warning(disable: 4312) // 'operation' : conversion from 'type1' to 'type2' of greater size

    #define COMPILER_FORWARD_TYPEID_ALLOWED

#endif // _MSC_VER

// Global project config
#include "ProjConfigGlobal.h"

// STD
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <exception>
#include <ostream>
#include <fstream>
#include <typeinfo> 
#include <functional>
#include <stack>
#include <algorithm>
#include <limits>

// C-lib
#include <cmath>
#include <climits>
#include <cstdlib>
#include <cctype>
#include <cassert>

// Qt
#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QtGui/QtGui>
#include <QtMultimedia/QtMultimedia>

// Boost
#define BOOST_ENABLE_ASSERT_HANDLER

#include <boost/utility.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp> 
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/filesystem.hpp> 
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/functional/value_factory.hpp>
#include <boost/functional/factory.hpp>


// boost::ptr_container
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_list.hpp>

// Boost::mpl
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/prior.hpp>
#include <boost/mpl/count.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/greater.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/vector/vector20.hpp>
#include <boost/mpl/vector/vector50.hpp>
#include <boost/mpl/transform.hpp>

//#include <boost/mpl/size.hpp>
//#include <boost/mpl/find.hpp>
//#include <boost/mpl/at.hpp>

// Overload global operators new, new[], delete, delete[];
// Use for allocation count. Switched by definition ENABLE_GLOBAL_NEW_DELETE
#include "Utils/DebugAllocationCounter.h"


#ifdef _MSC_VER
    #pragma warning(pop)
#endif // _MSC_VER


#endif

