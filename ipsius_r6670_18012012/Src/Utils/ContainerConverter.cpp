#include "stdafx.h"
#include "ContainerConverter.h"
#include "QtHelpers.h"
#include "ErrorsSubsystem.h"

// ---------------------------------------------

namespace
{

    template<class>
    class IntegerToQString
    {
    public:
        template<class TNumberType>
        QString operator()(const TNumberType &src)
        {
            return QString::number(src);
        }
    };

    class Base
    {
    protected:

        QString m_content;

        Base (const QString& msg) : m_content(msg) {}

    public:        
        
        virtual QString getContent() = 0;

        virtual ~Base(){}
    };

    class Derived : public Base
    {
        QString getContent()
        {
            return m_content;
        }

    public:

        Derived(const QString& msg) : Base(msg){}
    };

}

// ---------------------------------------------

namespace Utils
{
    void TestConverter()
    {        

        typedef std::list<std::string> LS;
        LS ls1;
        ls1.push_back("A"); ls1.push_back("b"); ls1.push_back("citrus");
       
        typedef std::deque<std::string> DS;
        DS ds;
        
        Converter<>::DoConvert(ls1, ds);
        
        typedef QList<std::string> QS;
        QS qs;
        Converter<>::DoConvert(ds, qs);
                
        typedef QVector<QString> QQ;
        QQ qq;
        Converter<QStd>::DoConvert(qs, qq);

        typedef QLinkedList<std::string> QLLS;
        QLLS qlls; 
        qlls.push_back(""); qlls.push_back(""); qlls.push_back("");
        Converter<QStd>::DoConvert(qq.begin(), qq.end(), qlls.begin()); // итеративная форма

        typedef std::vector<QString> VQ;
        VQ vq;
        Converter<QStd>::DoConvert(qlls, vq);  
        
        LS ls2;
        Converter<QStd>::DoConvert(vq, ls2);

        TUT_ASSERT(ls1 == ls2); 

        // another test
        {            
            QVector<long> src;
            src.push_back(1);
            src.push_back(2);

            std::vector<QString> dst;

            Converter<IntegerToQString>::DoConvert(src, dst);
            TUT_ASSERT(dst.at(0) == "1");
            TUT_ASSERT(dst.at(1) == "2");

            std::vector<Derived*> vd;
            vd.push_back(new Derived("A")); vd.push_back(new Derived("b")); vd.push_back(new Derived("citrus"));
            std::vector<Base*> vb;

            Converter<>::DoConvert(vd, vb);
            TUT_ASSERT(vb.size()==3);
            TUT_ASSERT(vb.at(0)->getContent() == "A");
            TUT_ASSERT(vb.at(1)->getContent() == "b");
            TUT_ASSERT(vb.at(2)->getContent() == "citrus");
            for (int i = 0; i < vb.size(); ++i)
            {
                delete vb.at(i);
            }

            long lArray[2];
            Converter<>::DoConvert(src.begin(), src.end(), lArray);
            TUT_ASSERT( lArray[0] == src.at(0) );
            TUT_ASSERT( lArray[1] == src.at(1) );

        }        
    }
} // namespace Utils

