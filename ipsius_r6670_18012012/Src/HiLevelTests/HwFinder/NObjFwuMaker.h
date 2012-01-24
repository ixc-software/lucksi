#ifndef NOBJFWUMAKER_H
#define NOBJFWUMAKER_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

namespace HiLevelTests
{
    namespace HwFinder
    {    


        // Обертка позволяющая создать 
        class NObjFwuMaker : public Domain::NamedObject
        {   
            Q_OBJECT;

            QString m_ldrPath;
            QString m_ldrFile;
            int m_dummyLdrSize;
            QString m_FwuPath;
            
        public:        

            NObjFwuMaker(Domain::IDomain *pDomain, const Domain::ObjectName &name)
                : Domain::NamedObject(pDomain, name), m_dummyLdrSize(-1)
            {
            }                 

            // Ldr-file for BOOT-script:
            Q_PROPERTY(QString LdrPath READ m_ldrPath WRITE m_ldrPath); 
            Q_PROPERTY(QString LdrFile READ getMainLdrFile WRITE m_ldrFile); // игнорируется если DummyLdrSize > 0
            Q_PROPERTY(int DummyLdrSize READ m_dummyLdrSize WRITE m_dummyLdrSize); // -1 не генерировать, взять реальный
            Q_PROPERTY (QString FwuPath READ m_FwuPath WRITE m_FwuPath); // путь куда будут сохранятся прошивки                

            Q_INVOKABLE void MakeFwu(QString fwuName, int revision, QString boardTypeRange, QString boardNumRange);

        private:                 

            QString getMainLdrFile();       
            void ValidateRangeSyntax(QString str);
        };

    } // namespace HwFinder
} // namespace HiLevelTests

#endif
