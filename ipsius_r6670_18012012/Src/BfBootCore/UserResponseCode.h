#ifndef USERRESPONSECODE_H
#define USERRESPONSECODE_H

#include "Utils/ErrorsSubsystem.h"

namespace BfBootCore
{
    using Platform::int32;
   
    // ���� ������� �� BootServer �������
    enum UserResponseCode
    {
        _MinValueResponseCode_ = 0, // ��� �������� ��� ��������������

        Ok,                 // - ���������

        errWrongProtoVersion, 
        errWrongPassword,
        errServerBusy, 

        errSizeNull,
        errDelNothing,          // - ������ �������       
        errInvalidPairNameVal, // ��������� � ����� ������ �� ���������� ��� ������������ ��������        
        errIsUploadState,      // ��������� ��� �������� ����� ������                            
        errNoEnoughtMemory,	// ��� ����� ��� ���������� ������ ������
        errNotOpened, 		// ������� ������ ����� ������ ��� �������� ������
        errWriteApwardsOfDeclare,    // ����� �� ������� ��� ������ ����� ������        
        errUnavailableInThisMode,
        errCRC,			// ������ ��� �������� CRC
        errSaveNothing,        // ��� ���������
        errHaveUnsavedChanges,	// ������� ������������� ���������
        errAlreadyOpened,
        errInvalidDescr,
        errWrongReleaseNumber, // �� ������� ���������� ����������(������ ������������ ��� ���� ��� ����������)
        errNoOpenedUpdateTrasaction, // ��������� ������� �������� ���������� �����������
        errDuplicateScriptName,      
        errCantRunScript,        
        errSaveForbiddenBooterUpdated, // ���������� ��������� �.�. ��������� ��� ��������
        //errFlashWriting, // ������ ������ �� ����
        errNextCmdTimeout,
        errWrongParamName,
        //errInvalidMac,

        _MaxValueResponseCode_
    };

   

    class UserResponseInfo // todo rename BoardError
    {
        UserResponseCode m_code;
        mutable std::string m_info;

        void Resolve() const
        {
            #define RESOLVE(errCode) if (errCode == m_code) {m_info = #errCode; return;}

            RESOLVE(Ok);            
            RESOLVE(errWrongProtoVersion);
            RESOLVE(errWrongPassword);
            RESOLVE(errServerBusy);
            RESOLVE(errSizeNull);
            RESOLVE(errDelNothing);
            RESOLVE(errInvalidPairNameVal);
            RESOLVE(errIsUploadState);
            RESOLVE(errNoEnoughtMemory);
            RESOLVE(errNotOpened);
            RESOLVE(errWriteApwardsOfDeclare);
            RESOLVE(errCRC);
            RESOLVE(errSaveNothing);
            RESOLVE(errHaveUnsavedChanges);
            RESOLVE(errAlreadyOpened);
            RESOLVE(errInvalidDescr);
            RESOLVE(errWrongReleaseNumber);
            RESOLVE(errNoOpenedUpdateTrasaction);
            RESOLVE(errDuplicateScriptName);
            RESOLVE(errCantRunScript);
            //RESOLVE(errFlashWriting);
            RESOLVE(errNextCmdTimeout);

            #undef RESOLVE
        };

    public:

        UserResponseInfo(UserResponseCode code) : m_code(code){}

        explicit UserResponseInfo(int32 userResponseCode)
        {
            ESS_ASSERT(userResponseCode > _MinValueResponseCode_ && userResponseCode < _MaxValueResponseCode_ );
            m_code = UserResponseCode(userResponseCode);
        }
      
        const std::string& AsString() const
        {
            if( m_info.empty() ) Resolve();
            return m_info;
        }

        UserResponseCode AsCode() const
        {
            return m_code;
        }        
    };

    // -------------------------------------------------------------------------------------

    ESS_TYPEDEF_T(BoardException, UserResponseInfo);

} // namespace BfBootCore

#endif

