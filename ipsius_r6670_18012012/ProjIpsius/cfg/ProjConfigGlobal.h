#ifndef __PROJCONFIGGLOBAL__
#define __PROJCONFIGGLOBAL__

	/*
	     √лобальные настройки компил€ции проекта.
	     ¬ключаютс€ в stdafx.h
	*/

    // ------------------------------------------------------
	
	// включение глобальных new/delete дл€ отладки
	// #define ENABLE_GLOBAL_NEW_DELETE

    // ------------------------------------------------------

    // использовать подстановку ESS_ASSERT, отключать только при параноидальной оптимизации
    #define ENABLE_ESS_ASSERTIONS           

    // ------------------------------------------------------

    namespace ProjConfig
    {

        // used in ThreadStrategy.h for boost::shared_ptr
        class CfgSharedPtr
        {
        public:

            static const bool CEnableThreadSafety = true;
        };

    }  // namespace ProjConfig






    // ------------------------------------------------------
    // ------------------------------------------------------

    // —лужебный код! предназначен дл€ принудительного включени€ файла CfgInfo.cpp

    namespace Utils
    {
        void CfgInfoDummi();
    }

    /*namespace
    {

        class ForceLink_CfgInfo
        {
        public:
            ForceLink_CfgInfo()
            {
                Utils::CfgInfoDummi();
            }
        };

        ForceLink_CfgInfo GForceLink_CfgInfo;
    }*/

#endif

