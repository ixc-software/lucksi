
#include "stdafx.h"

#include "ContainerMemFindTests.h"
#include "Utils/ContainerMemFind.h"
#include "Utils/ErrorsSubsystem.h"
#include "ContMemFindTestHelpers.h"

/*
    Check MemFind<> with 4 types of returning values: T, T&, const T&, T* (1-4)
    and 6 types method signatures:

    a) const T& Fn() const;     // class Colour
    b) T Fn() const;            // class Lamp --v
    c) T& Fn();
    d) T* Fn();
    e) bool Fn() const;
    f) bool Fn(const T&) const;
    
*/

namespace
{
    using namespace Utils;
    using namespace UtilsTests;
    using namespace boost;
    using namespace std;

    // -----------------------------------------------

    const string CErrMsg = "MemFindTestError";

    ESS_TYPEDEF(MemFindTestError);
    
    // -----------------------------------------------
    
    class Colour
    {
        std::string m_val;
        
    public:
        Colour(const std::string &val) : m_val(val) {}

        const std::string &get() const { return m_val; }                    // a, 1

        bool IsMulti() const { return false; } // always false
        
        bool operator==(const Colour &other) const
        {
            return m_val == other.m_val;
        }
    };

    // -----------------------------------------------

    // if (m_id == 0, 2, ...) lamps - retail
    class Lamp
    {
        int m_id;
        QString m_name;
        Colour m_colour;
        bool m_isRetail;
        
    public:
        Lamp(int lampId, const QString &name, const Colour &colour) 
            : m_id(lampId), m_name(name), m_colour(colour), m_isRetail(false)
        {
            if ((m_id % 2) == 0) m_isRetail = true;
        }
            
        int getId() const { return m_id; }                                  // b, 2
        bool IsRetail() const { return m_isRetail; }                        // e, 2
        bool IsSameColour(const Colour &c) const { return m_colour == c; }  // f, 2

        QString& getName() { return m_name; }                               // c, 3
        Colour* getColour() { return &m_colour; }                           // d, 4

        bool operator==(const Lamp& other) const 
        { 
            return (m_id == other.m_id) && (m_colour == other.m_colour)
                    && (m_name == other.m_name) && (m_isRetail == other.m_isRetail); 
        }
    };

    // -----------------------------------------------

    const QString CLampName = "lamp";
    const string CSecondColour = "red";
    
    // -----------------------------------------------

    class MemFindTest
    {
        typedef vector<Colour> ColoursValStd;
        typedef QList<Colour> ColoursValQt;
        
        typedef vector<shared_ptr<Lamp> > LampsShared;
        typedef QList<Lamp*> LampsPtrQt;
        typedef ptr_vector<Lamp> LampsPtrB;

        typedef QVector<const Lamp*> LampConstPtrQt;
        typedef vector<const Colour*> ColourConstPtrSt;
        
        ColoursValStd m_colours;
        ColoursValQt m_coloursQt;
        
        LampsShared m_storStd;
        LampsPtrQt m_storQt;
        LampsPtrB m_storBoost;

        LampConstPtrQt m_storConstQt;
        ColourConstPtrSt m_coloursConstPtrStd;

       
        void InitStorages()
        {
            // value
            m_colours.push_back( Colour("white") );
            m_colours.push_back( Colour(CSecondColour) );
            m_colours.push_back( Colour("green") );
            m_colours.push_back( Colour("black") );
            
            
            for (size_t i = 0; i < m_colours.size(); ++i)
            {
                Colour currCol = m_colours.at(i).get();

                // val 
                m_coloursQt.push_back(currCol);

                // shared_ptr<>
                m_storStd.push_back( shared_ptr<Lamp>( new Lamp( i, CLampName, currCol ) ) );
                // ptr
                m_storQt.push_back( new Lamp( i, CLampName, currCol ) );

                // ptr
                m_storBoost.push_back( new Lamp( i, CLampName, currCol ) );

                // const ptr
                m_storConstQt.push_back( new Lamp( i, CLampName, currCol ) );

                // const ptr
                m_coloursConstPtrStd.push_back( new Colour( currCol ) );
            }
        }

        /*
        // bool Name(void) const;
        __________________________________________________
         
        T* FindContRetPtr(cont, memFn)
        Iter FindContRetIter(cont, memFn)

        T* FindRangeRetPtr(iterBegin, iterEnd, memFn)
        Iter FindRangeRetIter(iterBegin, iterEnd, memFn)

        void FindAllCont(inCont, memFn, outCont)
        void FindAllRange(iterBegin, iterEnd, memFn, outCont)
        */
        
        // bool Lamp::IsRetail(void) const;
        template<class TCont, class TIter>
        void CheckRetBool(TCont &cont)
        {
            CheckFoundPtr(MemFind<>::FindContRetPtr(cont, &Lamp::IsRetail), &cont.at(0));

            //typename TCont::const_iterator itrBegin;
            TIter itrBegin = cont.begin(); // left it const

            CheckFoundPtr(MemFind<>::FindRangeRetPtr( ++itrBegin, cont.end(), &Lamp::IsRetail),
                          &cont.at(2));
            
            // typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter(cont, &Lamp::IsRetail);
            TUT_ASSERT(*itrFound == cont.at(0));
            
            itrBegin = cont.begin();
            itrFound = MemFind<>::FindRangeRetIter(++itrBegin, cont.end(), &Lamp::IsRetail);
            TUT_ASSERT(*itrFound == cont.at(2));
        
            // vector<typename TCont::const_iterator> outCont;
            vector<TIter> outCont;
            MemFind<>::FindAllCont(cont, &Lamp::IsRetail, outCont);
            TUT_ASSERT(outCont.size() == 2);

            outCont.clear();
            itrBegin = cont.begin();
            TIter iterEnd = ++itrBegin;
            MemFind<>::FindAllRange(cont.begin(), iterEnd, &Lamp::IsRetail, outCont);
            TUT_ASSERT(outCont.size() == 1);         
        }

        
        /*
        // all other
        __________________________________________________

        T* FindContRetPtr(cont, memFn, arg)   
        T* FindRangeRetPtr(iterBegin, iterEnd, memFn, arg) 
        
        Iter FindContRetIter(cont, memFn, arg)   
        Iter FindRangeRetIter(iterBegin, iterEnd, memFn, arg)    

        void FindAllCont(inCont, memFn, arg, outCont)
        void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont)
        */

        // bool Lamp::IsSameColour(const Colour& c) const;
        template<class TCont, class TIter>
        void CheckRetBoolWithArg(TCont &cont)
        {
            CheckFoundPtr( MemFind<>::FindContRetPtr( cont, &Lamp::IsSameColour, m_colours.at(0) ),
                           &cont.at(0) );
            CheckFoundPtr( MemFind<>::FindRangeRetPtr( cont.begin(), cont.end(), 
                                                       &Lamp::IsSameColour, m_colours.at(1) ),
                           &cont.at(1) );
            
            // typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter( cont, &Lamp::IsSameColour, m_colours.at(0) );
            TUT_ASSERT( *itrFound == cont.at(0) );

            itrFound = MemFind<>::FindRangeRetIter( cont.begin(), cont.end(), &Lamp::IsSameColour, 
                                               m_colours.at(2) );
            TUT_ASSERT( *itrFound == cont.at(2) );

            // vector<typename TCont::const_iterator> outCont;
            vector<TIter> outCont;
            MemFind<>::FindAllCont(cont, &Lamp::IsSameColour, m_colours.at(0), outCont);
            TUT_ASSERT(outCont.size() == 1);

            outCont.clear();
            MemFind<>::FindAllRange(cont.begin(), cont.end(), &Lamp::IsSameColour, 
                                    m_colours.at(2), outCont);
            TUT_ASSERT(outCont.size() == 1);
        }

        // int Lamp::getId() const;
        template<class TCont, class TIter>
        void CheckRetT(TCont &cont)
        {
            CheckFoundPtr(MemFind<>::FindContRetPtr(cont, &Lamp::getId, 0), &cont.at(0) );
            CheckFoundPtr(MemFind<>::FindRangeRetPtr(cont.begin(), cont.end(), &Lamp::getId, 1),
                          &cont.at(1) );

            // typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter(cont, &Lamp::getId, 0);
            TUT_ASSERT( *itrFound == cont.at(0) );
            
            itrFound = MemFind<>::FindRangeRetIter(cont.begin(), cont.end(), &Lamp::getId, 2);
            TUT_ASSERT( *itrFound == cont.at(2) );

            // QList<typename TCont::const_iterator> outCont;
            QList<TIter> outCont;
            MemFind<>::FindAllCont(cont, &Lamp::getId, 0, outCont);
            TUT_ASSERT(outCont.size() == 1);

            outCont.clear();
            MemFind<>::FindAllRange(cont.begin(), cont.end(), &Lamp::getId, 2, outCont);
            TUT_ASSERT(outCont.size() == 1);
        }
        
        
        // Colour* Lamp::getColour();
        template<class TCont, class TIter>
        void CheckRetTPtr(TCont &cont)
        {
            CheckFoundPtr( MemFind<>::FindContRetPtr( cont, &Lamp::getColour, m_colours.at(0) ),
                           &cont.at(0) );
            CheckFoundPtr( MemFind<>::FindRangeRetPtr( cont.begin(), cont.end(), 
                                                       &Lamp::getColour, m_colours.at(0) ),
                           &cont.at(0) );
            
            //typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter( cont, &Lamp::getColour, m_colours.at(0) );
            TUT_ASSERT( *itrFound == cont.at(0) );
            
            itrFound = MemFind<>::FindRangeRetIter( cont.begin(), cont.end(), 
                                                    &Lamp::getColour, m_colours.at(0) );
            TUT_ASSERT( *itrFound == cont.at(0) );

            // vector<typename TCont::const_iterator> outCont;
            vector<TIter> outCont;
            MemFind<>::FindAllCont(cont, &Lamp::getColour, m_colours.at(0), outCont);
            TUT_ASSERT(outCont.size() == 1);
            
            outCont.clear();
            MemFind<>::FindAllRange(cont.begin(), cont.end(), &Lamp::getColour, 
                                    m_colours.at(0), outCont);
            TUT_ASSERT(outCont.size() == 1);
        }

        // QString& Lamp::getName()
        template<class TCont, class TIter>
        void CheckRetTRef(TCont &cont)
        {
            CheckFoundPtr( MemFind<>::FindContRetPtr(cont, &Lamp::getName, CLampName), 
                           &cont.at(0) ); // ret first
            CheckFoundPtr( MemFind<>::FindRangeRetPtr(cont.begin(), cont.end(), 
                                                      &Lamp::getName, CLampName), 
                           &cont.at(0) );
            
            // typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter(cont, &Lamp::getName, CLampName);
            TUT_ASSERT( *itrFound == cont.at(0) );

            itrFound = MemFind<>::FindRangeRetIter(cont.begin(), cont.end(), &Lamp::getName, 
                                                   CLampName);
            TUT_ASSERT( *itrFound == cont.at(0) );

            // vector<typename TCont::iterator> outCont;
            vector<TIter> outCont;            
            MemFind<>::FindAllCont(cont, &Lamp::getName, CLampName, outCont);
            TUT_ASSERT( outCont.size() == cont.size() ); // ret all

            outCont.clear();
            MemFind<>::FindAllRange(cont.begin(), cont.end(), &Lamp::getName, CLampName, 
                                    outCont);
            TUT_ASSERT( outCont.size() == cont.size() );
        }
        
        
        // const string& Colours::get() const;
        // cont -- Container<T>, where T - stored by value
        template<class TCont, class TIter>
        void CheckRetTRefConst(TCont &cont)
        {
            // const std::string& Colour::get() const;
            CheckFoundPtr( MemFind<>::FindContRetPtr( cont, &Colour::get, CSecondColour),
                           &cont.at(1) );
            CheckFoundPtr( MemFind<>::FindRangeRetPtr(cont.begin(), cont.end(), 
                                                      &Colour::get, CSecondColour),
                           &cont.at(1) );

            // typename TCont::const_iterator itrFound;
            TIter itrFound;
            itrFound = MemFind<>::FindContRetIter(cont, &Colour::get, CSecondColour);
            TUT_ASSERT( *itrFound == cont.at(1) );

            itrFound = MemFind<>::FindRangeRetIter(cont.begin(), cont.end(), 
                                              &Colour::get, CSecondColour);
            TUT_ASSERT( *itrFound == cont.at(1) );
            
            // QList<typename TCont::const_iterator> outCont;
            QList<TIter> outCont;
            MemFind<>::FindAllCont(cont, &Colour::get, CSecondColour, outCont);
            TUT_ASSERT(outCont.size() == 1); // unique

            outCont.clear();
            MemFind<>::FindAllRange(cont.begin(), cont.end(), &Colour::get, 
                                    CSecondColour, outCont);
            TUT_ASSERT(outCont.size() == 1);
        }

        /*
        all with errors: MemFind<TException>
        ____________________________________
        
        EssAssertion:
        T* FindContRetPtr(cont, memFn, err)
        Iter FindRangeRetIter(iterBegin, iterEnd, memFn, err)
        void FindAllCont(inCont, memFn, arg, outCont, err)
        Iter FindContRetIter(cont, memFn, arg, err)   
        
        TutAssert:
        void FindAllCont(inCont, memFn, outCont, err)
        T* FindRangeRetPtr(iterBegin, iterEnd, memFn, err)
        T* FindContRetPtr(cont, memFn, arg, err)   
        void FindAllRange(iterBegin, iterEnd, memFn, arg, outCont, err)
        
        MemFindTestError:
        Iter FindContRetIter(cont, memFn, err)
        void FindAllRange(iterBegin, iterEnd, memFn, outCont, err)
        T* FindRangeRetPtr(iterBegin, iterEnd, memFn, arg, err)
        Iter FindRangeRetIter(iterBegin, iterEnd, memFn, arg, err)
        */

        void CheckEssAssert()
        {
            // 1
            MemFind<EssAssertion>::FindContRetIter(m_colours, &Colour::IsMulti); // all false            
            
            // 2
            LampsShared::iterator itr = m_storStd.begin(); 
            MemFind<EssAssertion>::FindRangeRetIter(++itr, itr, &Lamp::IsRetail); // second -- false
            // 3
            vector<LampsPtrQt::iterator> outCont;
            MemFind<EssAssertion>::FindAllCont(m_storQt, &Lamp::getId, (m_storQt.size() + 4), outCont);
            // 4
            /*
            Colour fake("_fake_colour_"); 
            MemFind<EssAssertion>::FindContRetIter(m_storBoost, &Lamp::getColour, &fake, CErrMsg); 
            */
            MemFind<EssAssertion>::FindContRetIter(m_storBoost, &Lamp::getId, static_cast<int>(m_storBoost.size() + 4));
        }

        void CheckTutAssert()
        {
            int estimateEx = 0;
            int realEx = 0;            

            // 1
            try
            {                
                ++estimateEx;
                vector<ColoursValStd::const_iterator> outCont;
                MemFind<TutAssertion>::FindAllCont(m_colours, &Colour::IsMulti, outCont);                
            }
            catch(const ESS::TutException& e) { ++realEx; }
            
            // 2
            try
            {                
                ++estimateEx;
                LampsShared::iterator itr = m_storStd.begin(); 
                ++itr;
                MemFind<TutAssertion>::FindRangeRetPtr(itr, itr, &Lamp::IsRetail); // second -- false                
            }
            catch(const ESS::TutException& e) { ++realEx; }
            
            // 3
            try
            {
                ++estimateEx;
                MemFind<TutAssertion>::FindContRetPtr(m_storConstQt, &Lamp::getId, 
                    (m_storConstQt.size() + 4));                
            }
            catch(const ESS::TutException& e) { ++realEx; }
            
            // 4
            try
            {
                ++estimateEx;
                vector<LampsPtrB::iterator> outCont2;
                MemFind<TutAssertion>::FindAllRange(m_storBoost.begin(), m_storBoost.end(), 
                    &Lamp::getId, static_cast<int>(m_storBoost.size() + 4), 
                    outCont2);                
            }
            catch(const ESS::TutException& e) { ++realEx; }

            TUT_ASSERT(realEx == estimateEx);
        }


        void CheckException()
        {            
            int estimateExCount = 0;
            int realEx = 0;
            // 1
            try
            {
                ++estimateExCount;
                MemFind<MemFindTestError>::FindContRetIter(m_colours, &Colour::IsMulti, CErrMsg);
            }
            catch(const MemFindTestError& e) { ++realEx; }                                  
              
            // 2
            try
            {
                ++estimateExCount;
                LampsShared::iterator itr = m_storStd.begin(); 
                ++itr;
                vector<LampsShared::iterator> outCont;
                
                MemFind<MemFindTestError>::FindAllRange(itr, itr, &Lamp::IsRetail, outCont, 
                    CErrMsg);
            }
            catch(const MemFindTestError& e) { ++realEx; }
            
            // 3            
            try
            {
                ++estimateExCount;
                MemFind<MemFindTestError>::FindRangeRetPtr(m_colours.begin(), m_colours.end(), 
                    &Colour::get, /*QString("_fake_name_"),*/ std::string("_fake_name_"),
                    CErrMsg);
            }
            catch(const MemFindTestError& e) { ++realEx; }
            
            // 4            
            try
            {
                ++estimateExCount;
                MemFind<MemFindTestError>::FindRangeRetIter(m_storConstQt.begin(), 
                    m_storConstQt.end(), &Lamp::getId, 
                    (m_storConstQt.size() + 4), CErrMsg);
            }
            catch(const MemFindTestError& e) { ++realEx; }

            // 5
            try
            {
                ++estimateExCount;
                MemFind<MemFindTestError>::FindRangeRetPtr(m_storQt.begin(), m_storQt.end(), 
                                                       &Lamp::getName, QString("_fake_name_"),
                                                       CErrMsg);
            
            } 
            catch (const MemFindTestError& e) { ++realEx; }
            
            ESS_ASSERT(realEx == estimateExCount);
        }
        
    public:

        MemFindTest(bool checkWithAssertionsAlso)
        {
            InitStorages();

            /*
            template<class TCont, class TIter> void CheckRetX(const TCont &cont):
                - for Container<T> use Container::iterator/const_iterator;
                - for Container<const T> use Container::const_iterator;
                - for const Container<T> use Container::const_iterator;
            */
            
            // Lamp's conts
            // bool Fn() const;
            CheckRetBool<LampsPtrQt, LampsPtrQt::iterator>(m_storQt);
            CheckRetBool<LampsShared, LampsShared::iterator>(m_storStd);
            CheckRetBool<LampsPtrB, LampsPtrB::iterator>(m_storBoost);
           
            CheckRetBool<const LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);
            CheckRetBool<LampsShared, LampsShared::iterator>(m_storStd);            
            CheckRetBool<LampsPtrB, LampsPtrB::iterator>(m_storBoost);

            CheckRetBool<const LampConstPtrQt, LampConstPtrQt::const_iterator>(m_storConstQt);            
            
            // bool Fn(const T&) const;
            CheckRetBoolWithArg<LampsPtrQt, LampsPtrQt::iterator>(m_storQt);
            CheckRetBoolWithArg<LampsShared, LampsShared::iterator>(m_storStd);
            CheckRetBoolWithArg<LampsPtrB, LampsPtrB::iterator>(m_storBoost);

            CheckRetBoolWithArg<LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);
            CheckRetBoolWithArg<LampsShared, LampsShared::const_iterator>(m_storStd);
            CheckRetBoolWithArg<LampsPtrB, LampsPtrB::const_iterator>(m_storBoost);
            CheckRetBoolWithArg<LampConstPtrQt, 
                                LampConstPtrQt::const_iterator>(m_storConstQt);
            CheckRetBoolWithArg<const LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);                        
            CheckRetBoolWithArg<const LampConstPtrQt, 
                                LampConstPtrQt::const_iterator>(m_storConstQt);
            
            // T Fn() const
            CheckRetT<LampsPtrQt, LampsPtrQt::iterator>(m_storQt);
            CheckRetT<LampsShared, LampsShared::iterator>(m_storStd);
            CheckRetT<LampsPtrB, LampsPtrB::iterator>(m_storBoost);
            CheckRetT<LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);
            CheckRetT<LampsShared, LampsShared::const_iterator>(m_storStd);
            CheckRetT<LampsPtrB, LampsPtrB::const_iterator>(m_storBoost);
            CheckRetT<const LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);                        

            CheckRetT<LampConstPtrQt, LampConstPtrQt::const_iterator>(m_storConstQt);
            
            // T* Fn()
            CheckRetTPtr<LampsPtrQt, LampsPtrQt::iterator>(m_storQt);
            CheckRetTPtr<LampsShared, LampsShared::iterator>(m_storStd);
            CheckRetTPtr<LampsPtrB, LampsPtrB::iterator>(m_storBoost);

            CheckRetTPtr<LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);
            CheckRetTPtr<LampsShared, LampsShared::const_iterator>(m_storStd);
            CheckRetTPtr<LampsPtrB, LampsPtrB::const_iterator>(m_storBoost);

            // CheckRetTPtr<LampConstPtrQt, LampConstPtrQt::const_iterator>(m_storConstQt);
                        
            // T& Fn()
            CheckRetTRef<LampsPtrQt, LampsPtrQt::iterator>(m_storQt);
            CheckRetTRef<LampsShared, LampsShared::iterator>(m_storStd);
            CheckRetTRef<LampsPtrB, LampsPtrB::iterator>(m_storBoost);

            CheckRetTRef<LampsPtrQt, LampsPtrQt::const_iterator>(m_storQt);
            CheckRetTRef<LampsShared, LampsShared::const_iterator>(m_storStd);
            CheckRetTRef<LampsPtrB, LampsPtrB::const_iterator>(m_storBoost);

            // CheckRetTRef<const LampConstPtrQt, LampConstPtrQt::iterator>(m_storConstQt);
            
            // Colour's conts
            // const T& Fn() const;
            CheckRetTRefConst<ColoursValStd, ColoursValStd::iterator>(m_colours);
            CheckRetTRefConst<ColoursValQt, ColoursValQt::iterator>(m_coloursQt);

            CheckRetTRefConst<ColoursValStd, ColoursValStd::const_iterator>(m_colours);
            CheckRetTRefConst<ColoursValQt, ColoursValQt::const_iterator>(m_coloursQt);

            CheckRetTRefConst<ColourConstPtrSt, 
                              ColourConstPtrSt::const_iterator>(m_coloursConstPtrStd);

            // with errors
            CheckException();
            CheckTutAssert();

            if (!checkWithAssertionsAlso) return;
            CheckEssAssert(); // ESS_ASSERT - terminate process, не проверишь
        }

        ~MemFindTest()
        {
            for (int i = 0; i < m_storQt.size(); ++i)
            {
                delete m_storQt.at(i);
            }
            m_storQt.clear();

            for (int i = 0; i < m_storConstQt.size(); ++i)
            {
                delete m_storConstQt.at(i);
            }
            m_storConstQt.clear();

            for (size_t i = 0; i < m_coloursConstPtrStd.size(); ++i)
            {
                delete m_coloursConstPtrStd.at(i);
            }
            m_coloursConstPtrStd.clear();
        }
    };
    
} // namespace 

// --------------------------------------------

namespace UtilsTests
{
    void ContainerMemFindTests()
    {
        bool checkWithAssertionAlso = false;
        
        MemFindTest test(checkWithAssertionAlso);
        
        CheckFoundPtrTest::Run();
    }
    
} // namesapce UtilsTests
