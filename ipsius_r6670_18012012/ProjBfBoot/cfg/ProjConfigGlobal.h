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

        // ---------------------------------------------------------

        class CfgESS
        {
        public:

            enum 
            { 
                // использовать захват данных о стеке
                CUseStackTrace = true,              

                /* использовать stack trace дл€ типов, унаследованных от ESS::Exception;
                отключение -- своего рода оптимизаци€, т.к. формирование stack trace 
                довольно накладна€ операци€, и может снизить производительность кода, 
                в котором активно возникают исключени€
                */
                CStackTraceInExceptions = false,   
            };  

        };

    }  // namespace ProjConfig






    // ------------------------------------------------------
    // ------------------------------------------------------

    // —лужебный код! предназначен дл€ принудительного включени€ файла CfgInfo.cpp

    namespace Utils
    {
        void CfgInfoDummi();
    }

    namespace
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
    }

#endif