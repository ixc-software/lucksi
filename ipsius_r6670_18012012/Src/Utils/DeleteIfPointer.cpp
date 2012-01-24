
#include "stdafx.h"
#include "DeleteIfPointer.h"

// ----------------------------------------------

namespace
{
    using namespace Utils;

    class NotDeleted : public INonDestructable 
    {
        ~NotDeleted() {}  // private
    };

    class SpecDestroy : public ISpecialDestructable
    {
        int m_i;

    public:

        void ProcessSpecialDelete()  // override
        {
            m_i++;
        }

    };

    class UsualClass 
    {
        int m_i;

    public:

        ~UsualClass()
        {
            m_i = 0;
        }
    };

}

// ----------------------------------------------

namespace Utils
{

    void DeletePointerTest()
    {
        // DeleteNop
        int i = 0;
        DeleteIfPointer(i);

        // TryDeletePointer, delete
        UsualClass *pI = new UsualClass;
        DeleteIfPointer(pI);

        // TryDeletePointer, nothing
        NotDeleted *pM = new NotDeleted();
        DeleteIfPointer(pM);

        // TryDeletePointer, ->ProcessSpecialDelete()
        SpecDestroy *pSD = new SpecDestroy();
        DeleteIfPointer(pSD);
    }

}

