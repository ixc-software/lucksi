
#include "stdafx.h"
#include "ManagedListTest.h"

#include "Utils/ManagedList.h"
#include "Utils/DebugAllocationCounter.h"


namespace
{
    class TestClass
    {
        int m_value;
        int &m_allocCounter;
        
    public:
        TestClass(int value, int &allocCounter) : 
            m_value(value), m_allocCounter(allocCounter)
        {
            ++m_allocCounter;
        }

        int Val() const { return m_value; }
        
        ~TestClass()
        {
            --m_allocCounter;
        }
    };
    
} // namespace

// ---------------------------------------------------------

// tests
namespace
{
    using namespace Utils;
    
    /*
    ManagedList(bool managed = true, size_t reserveCount = 0, bool allowNullItems = false);
    ~ManagedList();
    void Reserve(size_t size);
    size_t Size() const;
    size_t Capacity() const;
    bool IsEmpty() const;
    */
    void TestConstructorAndDesctuctor(int &allocCounter)
    {
        int value = 5;
        
        // constructors
        // without params 
        {
            ManagedList<TestClass> m;
            ESS_ASSERT(m.Size() == 0);
            ESS_ASSERT(m.Capacity() == 0);
            ESS_ASSERT(m.IsEmpty());
        }
        // managed -- true/false
        {
            const bool managed = true;
            
            TestClass *p = new TestClass(value, allocCounter);
            {
                ManagedList<TestClass> m(managed);
                m.Add(p);
            }
        }
        {
            const bool managed = false;

            TestClass* p = new TestClass(value, allocCounter);
            {
                ManagedList<TestClass> m(managed);
                m.Add(p);
            }
            TUT_ASSERT(p != 0);
            delete p; // Ok
        }
        // reserveCount
        size_t reserveCount = 5;
        {
            ManagedList<TestClass> m;
            m.Reserve(reserveCount);
            TUT_ASSERT(m.Size() == 0);
            TUT_ASSERT(m.Capacity() == reserveCount);
            TUT_ASSERT(m.IsEmpty());
        }
        {
            ManagedList<TestClass> m(true, reserveCount);
            TUT_ASSERT(m.Size() == 0);
            TUT_ASSERT(m.Capacity() == reserveCount);
            TUT_ASSERT(m.IsEmpty());
        }
        // allowNullItems -- true/false
        {
            const bool allowNullItems = false;
            
            try 
            { 
                ManagedList<TestClass> m(true, 0, allowNullItems);
                m.AddEmptyItems(1);                   // (1) assert: !allowNullItems
            }
            catch (const ESS::HookRethrow &e) {}
        }
        {
            const bool allowNullItems = true;
            
            ManagedList<TestClass> m(true, 0, allowNullItems);
            m.AddEmptyItems(1);
        }
    }

    // ---------------------------------------------------------

    /*
    void AddEmptyItems(int count);  // work only in m_allowNullItems mode(assert);
    void Add(T* item);
    */
    void TestAddAndAddNullItem(int &allocCounter)
    {
        const bool managed = true;
        const size_t nullCount = 5;
        int value = 10;
        
        {
            const bool allowNullItems = true;

            ManagedList<TestClass> m(managed, 0, allowNullItems);

            m.AddEmptyItems(nullCount);
            TUT_ASSERT(m.Size() == nullCount);
            TUT_ASSERT(m[0] == 0);

            TestClass *p = new TestClass(value, allocCounter);
            m.Add(p);
            TUT_ASSERT(m.Size() == nullCount + 1);
            TUT_ASSERT(m[m.Size() - 1] == p);

            m.Add(0);
            TUT_ASSERT(m[m.Size() - 1] == 0);
        }
        {
            const bool allowNullItems = false;

            ManagedList<TestClass> m(managed, 0, allowNullItems);

            TestClass *p = new TestClass(value, allocCounter);
            m.Add(p);
            TUT_ASSERT(m.Size() == 1);
            
            try
            {
                m.AddEmptyItems(nullCount);     // (3) assert: !allowNullItems
            }
            catch (const ESS::HookRethrow &e) {}

            try
            {
                m.Add(0);                       // (2) assert: p != 0
            }
            catch (const ESS::HookRethrow &e) {}
        }
    }
    
    // ---------------------------------------------------------

    /*
    void Set(int indx, TestClass *pNewItem);
    */
    void TestSet(int &allocCounter)
    {
        // set when managed
        {
            const bool managed = true;
            const bool allowNullItems = false;
    
            TestClass *p0 = new TestClass(10, allocCounter);
            TestClass *p1 = new TestClass(20, allocCounter);

            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.Add(p0);
            TUT_ASSERT(m[0] == p0);
            
            m.Set(0, p1);
            TUT_ASSERT(m.Size() == 1);
            TUT_ASSERT(m[0] == p1);
        }
        {
            const bool managed = true;
            const bool allowNullItems = false;
            
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            try
            {
                m.Set(1, 0);                    // (4) assert: !allowNullItems
            }
            catch (const ESS::HookRethrow &e) {}
        }
        // set when !managed
        {
            const bool managed = false;
            const bool allowNullItems = false;
            
            TestClass *p0 = new TestClass(30, allocCounter);
            TestClass *p1 = new TestClass(40, allocCounter);
            {
                ManagedList<TestClass> m(managed, 0, allowNullItems);
                m.Add(p0);
                TUT_ASSERT(m[0] == p0);
                
                m.Set(0, p1);
                TUT_ASSERT(m[0] == p1);
                TUT_ASSERT(p0 != 0);
            }
            delete p0;
            delete p1;
        }
        // set to empty list 
        {
            const bool managed = true;
            const bool allowNullItems = true;

            ManagedList<TestClass> m(managed, 0, allowNullItems);
            try
            {
                m.Set(0, 0);                        // (5) assert: index > size
            }
            catch (const ESS::HookRethrow &e) {}
        }
        // set at out of range position
        {
            const bool managed = true;
            const bool allowNullItems = true;
            
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.AddEmptyItems(2);
            
            try
            {
                m.Set(8, 0);                        // (6) assert: index > size
            }
            catch (const ESS::HookRethrow &e) {}
        }
    }
    
    // ---------------------------------------------------------
    
    /*
    void Delete(size_t index);
    void Clear();
    */
    void TestDeleteAndClear(int &allocCounter)
    {
        const bool allowNullItems = true;
        {
            const bool managed = true;

            TestClass *p0 = new TestClass(50, allocCounter);
            TestClass *p1 = new TestClass(60, allocCounter);
            TestClass *p2 = new TestClass(70, allocCounter);
            TestClass *p3 = new TestClass(80, allocCounter);

            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.Add(p0);
            m.Add(p1);
            m.Add(p2);
            m.Add(p3);
            
            m.Delete(0);
            TUT_ASSERT(m.Size() == 3);
            TUT_ASSERT(m[0] == p1);
            TUT_ASSERT(m[1] == p2);
            TUT_ASSERT(m[2] == p3);

            m.Delete(2);
            TUT_ASSERT(m.Size() == 2);
            TUT_ASSERT(m[0] == p1);
            TUT_ASSERT(m[1] == p2);

            m.Clear();
            TUT_ASSERT(m.Size() == 0);
        }
        {
            const bool managed = false;

            TestClass *p0 = new TestClass(10, allocCounter);
            TestClass *p1 = new TestClass(20, allocCounter);

            {
                ManagedList<TestClass> m(managed, 0, allowNullItems);
                m.Add(p0);
                m.Add(p1);
    
                m.Delete(0);
                TUT_ASSERT(m.Size() == 1);
                TUT_ASSERT(p0 != 0);
                TUT_ASSERT(m[0] == p1);
    
                m.Clear();
                TUT_ASSERT(m.Size() == 0);
                TUT_ASSERT(p1 != 0);
            }
            
            delete p0;
            delete p1;
        }
        // Delete() from empty list
        {
            ManagedList<TestClass> m;
            try
            {
                m.Delete(0);                            // (7) assert: index > size
            }
            catch (const ESS::HookRethrow &e) {}
        }
        // Delete() from empty list with capacity > 0
        {
            ManagedList<TestClass> m(true, 5, allowNullItems);
            try
            {
                m.Delete(0);                            // (8) assert: index > size
            }
            catch (const ESS::HookRethrow &e) {}
        }
        // Delete() with too big index
        {
            ManagedList<TestClass> m(true, 0, allowNullItems);
            m.Add(new TestClass(10, allocCounter));
            try
            {
                m.Delete(1);                            // (9) assert: index > size
            }
            catch (const ESS::HookRethrow &e) {}
        }
        // Clear() with empty list
        {
            ManagedList<TestClass> m;
            m.Clear();
        }
        // Clear() with list that contains null items
        {
            ManagedList<TestClass> m(true, 0, allowNullItems);
            m.AddEmptyItems(5);
            TUT_ASSERT(m.Size() == 5);
            
            m.Clear();
            TUT_ASSERT(m.Size() == 0);
        }
    }
    
    // ---------------------------------------------------------

    /*
    T* Detach(int itemIndex); // remove item from list and get ownership over it;
    */
    void TestDetach(int &allocCounter)
    {
        const bool allowNullItems = true;
        
        {
            const bool managed = true;
            
            TestClass *p0 = new TestClass(10, allocCounter);
            TestClass *p1 = new TestClass(20, allocCounter);
            
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.Add(p0);
            m.Add(p1);
            m.AddEmptyItems(1);

            TestClass *pNull = m.Detach(2);
            TUT_ASSERT(pNull == 0);
            TUT_ASSERT(m.Size() == 2);
            
            TestClass *sameP0 = m.Detach(0);
            TUT_ASSERT(p0 == sameP0);
            TUT_ASSERT(m.Size() == 1);

            delete p0;
            
            TestClass *sameP1 = m.Detach(0);
            TUT_ASSERT(sameP1 = p1);
            TUT_ASSERT(m.Size() == 0);

            delete p1;
        }
        {
            const bool managed = false;

            TestClass *p0 = new TestClass(30, allocCounter);
            
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.Add(p0);
            
            TestClass *sameP0 = m.Detach(0);
            TUT_ASSERT(p0 == sameP0);
            TUT_ASSERT(m.Size() == 0);

            delete p0;
        }
    }
    
    // ---------------------------------------------------------

    /*
    T* operator[](size_t index) const;
    */
    void TestAccess(int &allocCounter)
    {
        const int capacity = 5;
        const bool allowNullItems = true;

        {
            const bool managed = false;

            ManagedList<TestClass> m(managed, capacity, allowNullItems);

            TestClass *p = new TestClass(10, allocCounter);
            {
                m.Add(p);
                TUT_ASSERT(m.Size() == 1);
                TUT_ASSERT(m[0] == p);
            }
            delete p;
        }

        const bool managed = true;
        
        {
            ManagedList<TestClass> m(managed, capacity, allowNullItems);
            m.Add(new TestClass(20, allocCounter));
            m.Add(new TestClass(30, allocCounter));
            m.Add(new TestClass(40, allocCounter));
            TUT_ASSERT(m.Size() == 3);
            TUT_ASSERT(m[0]->Val() == 20);
            TUT_ASSERT(m[1]->Val() == 30);
            TUT_ASSERT(m[2]->Val() == 40);
        }
        {
            ManagedList<TestClass> m(managed, capacity, allowNullItems);
            bool wasException = false;
            try
            {
                TestClass *p = m[0];
            }
            catch (const std::out_of_range &e) 
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        {
            ManagedList<TestClass> m(managed, capacity, allowNullItems);
            m.Add(new TestClass(50, allocCounter));
            bool wasException = false;
            try
            {
                TestClass *p = m[1];
            }
            catch (const std::out_of_range &e) 
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
    }

    // ---------------------------------------------------------

    /*
    template<class Par> 
    int Find(const Par *p) const 
    */
    void TestFind(int &allocCounter)
    {
        const bool managed = true;
        const bool allowNullItems = true; 

        TestClass *pFindNull = 0;

        {
            TestClass *pFind = new TestClass(1000, allocCounter);
            
            ManagedList<TestClass> m(managed, 0, allowNullItems);

            m.Add(new TestClass(10, allocCounter));        // 0
            m.Add(new TestClass(20, allocCounter));        // 1
            m.AddEmptyItems(1);                            // 2
            m.Add(new TestClass(40, allocCounter));        // 3
            m.Add(pFind);                                  // 4
            m.Add(new TestClass(60, allocCounter));        // 5

            int foundNullIndex = m.Find(pFindNull);
            TUT_ASSERT(foundNullIndex == 2);
            
            int foundIndex = m.Find(pFind);
            TUT_ASSERT(foundIndex == 4);

            m.Delete(4);
            foundIndex = m.Find(pFind);
            TUT_ASSERT(foundIndex < 0);
        }
        {
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            int foundNullIndex = m.Find(pFindNull);
            TUT_ASSERT(foundNullIndex < 0);
        }
        {
            ManagedList<TestClass> m(managed, 0, allowNullItems);
            m.AddEmptyItems(5);
            int foundNullIndex = m.Find(pFindNull);
            TUT_ASSERT(foundNullIndex == 0);
        }
    }

    // ---------------------------------------------------------

    /* 
    void Resize(int newSize) 
    */

    void TestResize(int &allocCounter)
    {
        const bool allowNullItems = true;
        const int resizeSize = 10; 
        {
            const bool managed = true;
            // resize empty
            {
                const size_t reserveCount = 0;
                ManagedList<TestClass> m(managed, reserveCount, allowNullItems);
                
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0] == 0);
                TUT_ASSERT(m[m.Size() - 1] == 0);

                TestClass *p = new TestClass(10, allocCounter);
                m.Set(0, p);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0] == p);

                m.Add(new TestClass(20, allocCounter));
                TUT_ASSERT(m.Size() == (resizeSize + 1));
                TUT_ASSERT(m[resizeSize]->Val() == 20);
            }
            // resize !empty
            {
                const size_t reserveCount = 0;
                ManagedList<TestClass> m(managed, reserveCount, allowNullItems);
                m.Add(new TestClass(30, allocCounter));
                TUT_ASSERT(m.Size() == 1);
                TUT_ASSERT(m[0]->Val() == 30);

                // resize to bigger
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);

                m.Set(1, new TestClass(40, allocCounter));
                TUT_ASSERT(m[1]->Val() == 40);
                TUT_ASSERT(m.Size() == resizeSize);
                
                m.Add(new TestClass(50, allocCounter));
                TUT_ASSERT(m.Size() == resizeSize + 1);
                TUT_ASSERT(m[m.Size() - 1]->Val() == 50);

                // resize to smaller
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0]->Val() == 30);
            }
            // resize empty with reserved space
            {
                const size_t reserveCount = 5;
                ManagedList<TestClass> m(managed, reserveCount, allowNullItems);
                m.Resize(reserveCount - 1);
                TUT_ASSERT(m.Size() == (reserveCount - 1));

                m.Resize(reserveCount + 1);
                TUT_ASSERT(m.Size() == (reserveCount + 1));

                m.Resize(0);
                TUT_ASSERT(m.IsEmpty());
            }
            // resize !empty with reserved space
            {
                const size_t reserveCount = 5;
                ManagedList<TestClass> m(managed, reserveCount, allowNullItems);
                m.Add(new TestClass(60, allocCounter));
                TUT_ASSERT(m.Size() == 1);
                
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0] != 0);
            }
        }
        {
            const bool managed = false;
            const size_t reserveCount = 0;
            
            TestClass *p0 = new TestClass(70, allocCounter);
            TestClass *p1 = new TestClass(80, allocCounter);
            TestClass *p2 = new TestClass(90, allocCounter);

            {
                ManagedList<TestClass> m(managed, reserveCount, allowNullItems);
                m.Add(p0);
                TUT_ASSERT(m.Size() == 1);
                TUT_ASSERT(m[0] == p0);
    
                // resize to bigger
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0] == p0);
    
                m.Set(1, p1);
                TUT_ASSERT(m[1] == p1);
                TUT_ASSERT(m.Size() == resizeSize);
    
                m.Add(p2);
                TUT_ASSERT(m.Size() == (resizeSize + 1));
                TUT_ASSERT(m[m.Size() - 1] == p2);
    
                // resize to smaller
                m.Resize(resizeSize);
                TUT_ASSERT(m.Size() == resizeSize);
                TUT_ASSERT(m[0] == p0);
                TUT_ASSERT(m[1] == p1);
            }

            delete p0;
            delete p1;
            delete p2;
        }
    }
    
} // namespace

// ---------------------------------------------------------

namespace UtilsTests
{
    void ManagedListTest(bool silentMode)
    {
        ESS::ExceptionHookRethrow<ESS::HookRethrow> assertCounter;

        int allocCounter = 0;
        
        TestConstructorAndDesctuctor(allocCounter);
        TestAddAndAddNullItem(allocCounter);
        TestSet(allocCounter);
        TestDeleteAndClear(allocCounter);
        TestDetach(allocCounter);
        TestAccess(allocCounter);
        TestFind(allocCounter);
        TestResize(allocCounter);

        if (!silentMode) 
        {
            std::cout << "ManagedList test: "  
                << "allocation counter = " << allocCounter 
                << ", assertion counter = " << assertCounter.getCounter() << std::endl;
        }

        TUT_ASSERT(allocCounter == 0);
        TUT_ASSERT(assertCounter.getCounter() == 9);
        
        if (!silentMode) std::cout << "ManagedList test: OK" << std::endl;
    }
    
} // namespace UtilsTests
