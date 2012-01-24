#ifndef ICLIENTTOBOOTER_H
#define ICLIENTTOBOOTER_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"

namespace BfBootCore
{
    
    using Platform::dword;
    using Platform::byte;
    

    // ��������� ���������� �������� ����������. 
    // �������� ����������� ����� �������� ������ ���������� IBootServerToClient.
    class IBootCtrl : public Utils::IBasicInterface
    {
    public:       
        // ��������� �������� � ������� ������������. ������ � ���������������� ������.
        virtual void SetUserParam(const std::string& name, const std::string& value) = 0;

        // ��������� ��������� ��������. ������ � ������-������.        
        virtual void SetFactory(Platform::dword hwNumber, Platform::dword hwType, const std::string& mac) = 0;
        
        // �������� ������ ���������������� ��������.
        virtual void GetParamList() const = 0;
        // �������� ������ ��������� ����������.
        virtual void GetAppList() const = 0;
        
        // ������� ������ ������ ������ ����������. ������ � ���������������� ������.        
        virtual void OpenWriteImg(dword size) = 0;
        
        // ���� ������ ������� ���������� ��������� ����. �������� ������ �������.
        //virtual void WriteImgChunk(const std::vector<byte>& data) = 0;
        virtual void WriteImgChunk(const void* pData, dword size) = 0;
        
        // �������� ������. 
        // ���� CRC � ��������� ������ ��������� �� ����� ���� ������������� � Config � ����������� ���������� ����������.
        // ����� ������ �����������.
        virtual void CloseNewImg(const std::string& name, dword CRC) = 0;
        // ������� ��������� ���������� ����� ����������. ������ � ���������������� ������.
        // ��������� ���������� ����������.
        virtual void DeleteLastImg() = 0;
        // ������� ��� ���������� ������ ����������. ������ � ���������������� ������.
        // ��������� ���������� ����������.
        virtual void DeleteAllImg() = 0;

        virtual void CloseUpdateTransaction(dword releaseNumber) = 0;                       
        
        // todo �� �����������
        virtual void SaveChanges() = 0;
        
        // �������� ������ ������ ����������. ��������� ������ ��� ���������� ������������ ���������.
        virtual void OpenWriteBootImg(dword size) = 0;
        // ������ ���������� ����� � �����. ��������� ������ ��� ���������� ������������ ���������, ����� ��������� ������.
        virtual void WriteBooterChunk(const void* pData, dword length) = 0;
        // �������� ������� ������ ���������� ����������. ��������� ������ ��� ���������� ������������ ���������, ����� ��������� ������.
        // ��������� ������������������ ����� ������ ���� ��������� CRC � ������, ����� ��������� ������.
        virtual void CloseNewBootImg(dword CRC) = 0;
        
        // ������� ��������� � ������� CfgDefault. ��� �����������������.
        // ��� ����������� �������� ������ �������� � ��������� ������.
        virtual void EraseDefaultCfg() = 0;
       
        virtual void GetScriptList() const = 0;
        
    };

    // ------------------------------------------------------------------------------------

    
    // ��������� ��������� ������� �� ����������� ������� � ������� ���������� ��������.
    class ILogin : public Utils::IBasicInterface
    {
    public:

        // ��������/��������� ����������� �� ����������� � ������� ����������.
        // ������������ ��������� ����������� ���������� �� ����������� Config
        virtual void SetTrace(bool on) = 0;

        // ������� ��������� ����������� � �������.
        // ����� LoginResp(bool ok) ok - true ����������� ���������
        virtual void DoLogin(const std::string& pwd, dword protoVersion) = 0;

        // ���������� �������. �������� � ����������� ����������
        virtual void DoLogout(bool withSave) = 0;        

        virtual void RunAditionalScript(const std::string& scriptName, bool withSaveChanges) = 0;
    };



    // ��������� ���������� �����������. 
    // ��������� ������ ��������� ����������� ����������� ����� ����������. ������� ���������� � �������
    // SafeBiProto � ������������� ������ (Tcp ��� Uart).
    class IClientToBooter
        : public ILogin, 
        public IBootCtrl
    {
    public:         
    }; 

    /*
    *   ������� ������ ��������� ���� ������ BootControlProto (���-������� � � �����������)!
    */
} // namespace BfBootCore

#endif
