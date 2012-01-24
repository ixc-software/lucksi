#ifndef __MOC_EXT__

#define  __MOC_EXT__

#include <stdio.h>
#include <stdlib.h>

// cross reference, must be defined later in moc.h
struct ClassDef;
class Moc;

namespace MocExt
{

    // ---------------------------------------------------------------
    // Список структур ClassDef для которых необходимо генерация Proxy	
    class ExtentionMoc
    {
    public:

        ExtentionMoc(Moc &moc);
        ~ExtentionMoc();

		void Append(const ClassDef &class_def, QString classNamespaces);
        void Generate(FILE *out) const;
        
        static void warning(const char *msg);
        static void error(const char *msg);

    private:

        void warningImpl(const char *msg);
        void errorImpl(const char *msg);

        class Impl; 
        Impl *m_impl; 

    };  // class ExtentionMioc  

} // namespace MocExt

#endif





