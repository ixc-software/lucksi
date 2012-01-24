#ifndef __KEYS__
#define __KEYS__

#include "Platform/Platform.h"

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys/exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>
#include "uart/uart0.h"
#include "utils/IBasicInterface.h"

namespace BfDev
{
    using namespace Platform;

    /*
        Keys class
        .....
    */
	
    class IKeyObserver : public Utils::IBasicInterface
    {
    public:
    	virtual void KeyPressed(int key) = 0;
    };
    
    
	class Keys : public boost::noncopyable
	{
	public:

		static void Init(IKeyObserver &observer);
		static void PressedInterrupt();
		static Keys &Instance();	
		void OnTimer();
		
    private:    
        Keys(IKeyObserver &observer);        
		void Pressed(int key);
    private:
		IKeyObserver &m_observer;
		int m_lastKey;
	};

}//namespace BFKeys

#endif

