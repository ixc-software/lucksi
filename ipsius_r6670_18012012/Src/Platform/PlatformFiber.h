#pragma once

namespace Platform
{
    
    class Fiber : boost::noncopyable
    {
        class FiberImpl;
        
        boost::scoped_ptr<FiberImpl> m_body;
        
    public:
        
        Fiber(const boost::function<void ()> &fn, int stackSize = -1);
        ~Fiber();
        
        void Run();      // run fiber
        void Return();   // return from fiber (from Run())                
    };
    

    /*

        Alt. API

        class Fiber
        {
        public:
            Fiber(...);
            ~Fiber();

            void SwitchTo();
            bool IsMain(); 

            static Fiber& GetCurrentFiber();
        };
       
    */
    
}  // namespace Platform