#ifndef USERRESPONSECODE_H
#define USERRESPONSECODE_H

#include "Utils/ErrorsSubsystem.h"

namespace BfBootCore
{
    using Platform::int32;
   
    // коды ответов от BootServer клиенту
    enum UserResponseCode
    {
        _MinValueResponseCode_ = 0, // для проверки при преобразовании

        Ok,                 // - выполнено

        errWrongProtoVersion, 
        errWrongPassword,
        errServerBusy, 

        errSizeNull,
        errDelNothing,          // - нечего удалять       
        errInvalidPairNameVal, // параметра с таким именем не существует или недопустимый аргумент        
        errIsUploadState,      // запрещено при открытой сесси записи                            
        errNoEnoughtMemory,	// Нет места для размещения нового образа
        errNotOpened, 		// попытка записи блока данных без открытия записи
        errWriteApwardsOfDeclare,    // выход за пределы при записи блока данных        
        errUnavailableInThisMode,
        errCRC,			// ошибка при проверке CRC
        errSaveNothing,        // нет изменений
        errHaveUnsavedChanges,	// имеются несохраненные изменения
        errAlreadyOpened,
        errInvalidDescr,
        errWrongReleaseNumber, // не закрыта транзакция обновления(ошибка пользователя или сбой при обновлении)
        errNoOpenedUpdateTrasaction, // ожидается наличие открытой транзакции обнеовления
        errDuplicateScriptName,      
        errCantRunScript,        
        errSaveForbiddenBooterUpdated, // сохранение запрещено т.к. загрузчик был обновлен
        //errFlashWriting, // ошибка записи на флеш
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

