#ifndef ICLIENTTOBOOTER_H
#define ICLIENTTOBOOTER_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"

namespace BfBootCore
{
    
    using Platform::dword;
    using Platform::byte;
    

    // Интерфейс управления сервером загрузчика. 
    // Ответами клиекнтской части являются методы интерфейса IBootServerToClient.
    class IBootCtrl : public Utils::IBasicInterface
    {
    public:       
        // Изменения настроек в текущей конфигурации. Только в пользовательском режиме.
        virtual void SetUserParam(const std::string& name, const std::string& value) = 0;

        // Установка фабричных настроек. Только в сервис-режиме.        
        virtual void SetFactory(Platform::dword hwNumber, Platform::dword hwType, const std::string& mac) = 0;
        
        // Получить список пользовательских настроек.
        virtual void GetParamList() const = 0;
        // Получить список имеющихся приложений.
        virtual void GetAppList() const = 0;
        
        // Открыть сессию записи нового приложения. Только в пользовательском режиме.        
        virtual void OpenWriteImg(dword size) = 0;
        
        // Если сессия открыта записывает очередной блок. Возможна ошибка размера.
        //virtual void WriteImgChunk(const std::vector<byte>& data) = 0;
        virtual void WriteImgChunk(const void* pData, dword size) = 0;
        
        // Закрытие сессии. 
        // Если CRC и ожидаемый размер совпадают то новый файл прописывается в Config и открывается транзакция обновления.
        // иначе сессия прерывается.
        virtual void CloseNewImg(const std::string& name, dword CRC) = 0;
        // Удалить последний сохраненый образ приложения. Только в пользовательском режиме.
        // Открывает транзакцию обновления.
        virtual void DeleteLastImg() = 0;
        // Удалить все сохраненые образы приложения. Только в пользовательском режиме.
        // Открывает транзакцию обновления.
        virtual void DeleteAllImg() = 0;

        virtual void CloseUpdateTransaction(dword releaseNumber) = 0;                       
        
        // todo не использован
        virtual void SaveChanges() = 0;
        
        // Открытие сессии записи загрузчика. Разрешено только при отсутствии несохраненых изменений.
        virtual void OpenWriteBootImg(dword size) = 0;
        // Запись очередного блока в буфер. Разрешено только при отсутствии несохраненых изменений, иначе прерывает сессию.
        virtual void WriteBooterChunk(const void* pData, dword length) = 0;
        // Закрытие текущей сессии обновления загрузчика. Разрешено только при отсутствии несохраненых изменений, иначе прерывает сессию.
        // Сохраняет забуферизированный образ только если совпадает CRC и размер, иначе прерывает сессию.
        virtual void CloseNewBootImg(dword CRC) = 0;
        
        // Стирает настройки в области CfgDefault. Для суперпользователя.
        // При последующей загрузке сервер стартует в сервисном режиме.
        virtual void EraseDefaultCfg() = 0;
       
        virtual void GetScriptList() const = 0;
        
    };

    // ------------------------------------------------------------------------------------

    
    // Доступный интерфейс сервера до подключения клиента к объекту управления сервером.
    class ILogin : public Utils::IBasicInterface
    {
    public:

        // Включить/выключить трассировку не подключаясь к объекту управления.
        // Используются параметры трассировки полученные из актуального Config
        virtual void SetTrace(bool on) = 0;

        // Команда выполнить подключение к серверу.
        // Ответ LoginResp(bool ok) ok - true подключение выполнено
        virtual void DoLogin(const std::string& pwd, dword protoVersion) = 0;

        // Отключение клиента. Приводит к перезапуску загрузчика
        virtual void DoLogout(bool withSave) = 0;        

        virtual void RunAditionalScript(const std::string& scriptName, bool withSaveChanges) = 0;
    };



    // Интерфейс управления загрузчиком. 
    // Удаленный клиент управляет загрузчиком посредством этого интерфейса. Команды передаются с помощью
    // SafeBiProto и транспортного уровня (Tcp или Uart).
    class IClientToBooter
        : public ILogin, 
        public IBootCtrl
    {
    public:         
    }; 

    /*
    *   Изменяя методы интерфеса надо менять BootControlProto (тип-команда и её регистрация)!
    */
} // namespace BfBootCore

#endif
