#ifndef NOBJECHOCANCELER_H
#define NOBJECHOCANCELER_H

#include "Domain/NamedObject.h"

namespace TestRealEcho
{

    class NObjEchoCanceler : public Domain::NamedObject
    {
        Q_OBJECT;
        QTextStream* CreateStream( QFile& txFile );
    public:
        NObjEchoCanceler(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : NamedObject(pDomain, name)
        {}

        // input data is linear txt signal, out file is 16bits mono wave
        Q_INVOKABLE void Process(QString txFile, QString rxFile, int echoTaps, QString out);

    };
} // namespace TestRealEcho

#endif
