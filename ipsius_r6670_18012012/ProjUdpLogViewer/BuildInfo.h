

#ifndef __BUILDINFO__
#define __BUILDINFO__

    // Don't edit this file, except path to SVN

    // point to .svn dir (revision extracted from this path)
    // SVN_PATH = ../.svn

    struct AutoBuildInfo
    {
        static const char* FullInfo()
        {
            return "AutoBuildInfo: rev 4325; build Mon Feb 15 17:47:54 2010 by Alex";   // FULL_INFO 
        }
        
        static int Revision()
        {
            return 4325;    // REVISION
        }
    };

#endif


