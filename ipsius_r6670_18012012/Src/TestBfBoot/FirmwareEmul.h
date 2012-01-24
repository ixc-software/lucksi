#ifndef FIRMWAREEMUL_H
#define FIRMWAREEMUL_H

#include "BfBootCore/IHwFirmware.h"
#include "BfBootCore/NamedScript.h"
#include "BfBootCore/ScriptList.h"
#include "Utils/ExeName.h"
#include "BfBootCli/BinaryImage.h"

namespace TestBfBoot
{
    // заглушечная реализация IHwFirmware не связанная с архиватором
    class FirmwareEmul : public BfBootCore::IHwFirmware
    {
        std::vector<BfBootCli::BinaryImage> m_img;
        BfBootCore::ScriptList m_script;
        int m_relNum;
    private:
    public:
        QString Info() const {ESS_UNIMPLEMENTED; return QString();}
        int Release() const
        {
            return m_relNum;
        }

        int ImageCount() const
        {
            return m_img.size();
        }
        BfBootCore::IHwFirmwareImage& Image(int index) const 
        {
            ESS_ASSERT(index < m_img.size());
            FirmwareEmul* pThis = const_cast<FirmwareEmul*>(this);
            return pThis->m_img.at(index);
        }

        int ScriptCount() const
        {
            return m_script.Size();
        }
        const BfBootCore::IHwFirmwareScript& Script(int index) const
        {
            return m_script[index];
        }
    public:
        void AddDataFromFile(QString filename) //in path = ExeDir()/../
        {
            QDir dir(Utils::ExeName::GetExeDir().c_str());
            dir.cdUp();
            QDir::setCurrent( dir.absolutePath() );

            QFile file(filename);
            ESS_ASSERT( file.open(QIODevice::ReadOnly) );

            QByteArray buff;
            buff = file.readAll();

            AddDataFromBin(filename.toStdString(), buff);
        }
        void AddDataFromBin (const std::string& name, QByteArray data)
        {            
            m_img.push_back(BfBootCli::BinaryImage(name, data));
        }
        void AddScript(const std::string& name, const std::string& body)
        {
            m_script.Add(BfBootCore::NamedScript(name, body));
        }
        void SetRelease(int rel)
        {
            ESS_ASSERT(rel >= 0);
            m_relNum = rel;            
        }

        FirmwareEmul() : m_relNum(-1){}

        bool Validate()
        {
            return
                m_relNum >= 0
                && !m_img.empty()
                && m_script.Size() != 0;
        }
    };
} // namespace BfBootCli

#endif
