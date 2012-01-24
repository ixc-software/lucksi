#ifndef BOOTIMAGE_H
#define BOOTIMAGE_H

#include "stdafx.h"
#include "BfBootCore/IHwFirmware.h"


namespace IpsiusService
{    
    class BootImage :  public BfBootCore::IHwFirmwareImage
    {
        QString m_name;
        QByteArray m_data;

        // IHwFirmwareImage
    public:        

        std::string Info() const 
        {
            return m_name.toStdString();
        }
        QByteArray Data()
        {
            return m_data;
        }

    public:   

        ESS_TYPEDEF(Err);

        QByteArray Data()const
        {
            return m_data;
        }

        BootImage(QString fileName) : m_name(fileName)
        {
            QFile file(fileName);
            std::string err = "Can`t open file " + fileName.toStdString();
            if( !file.open(QIODevice::ReadOnly) ) ESS_THROW_MSG(Err, err);
            m_data = file.readAll();        
            // todo: image analize
        }
    };
} // namespace IpsiusService

#endif
