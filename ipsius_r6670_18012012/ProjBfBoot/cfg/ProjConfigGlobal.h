#ifndef __PROJCONFIGGLOBAL__
#define __PROJCONFIGGLOBAL__

	/*
	     ���������� ��������� ���������� �������.
	     ���������� � stdafx.h
	*/

    // ------------------------------------------------------
	
	// ��������� ���������� new/delete ��� �������
	// #define ENABLE_GLOBAL_NEW_DELETE

    // ------------------------------------------------------

    // ������������ ����������� ESS_ASSERT, ��������� ������ ��� �������������� �����������
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
                // ������������ ������ ������ � �����
                CUseStackTrace = true,              

                /* ������������ stack trace ��� �����, �������������� �� ESS::Exception;
                ���������� -- ������ ���� �����������, �.�. ������������ stack trace 
                �������� ��������� ��������, � ����� ������� ������������������ ����, 
                � ������� ������� ��������� ����������
                */
                CStackTraceInExceptions = false,   
            };  

        };

    }  // namespace ProjConfig






    // ------------------------------------------------------
    // ------------------------------------------------------

    // ��������� ���! ������������ ��� ��������������� ��������� ����� CfgInfo.cpp

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