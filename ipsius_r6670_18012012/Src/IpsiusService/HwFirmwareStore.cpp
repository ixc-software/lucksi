#include "stdafx.h"
#include "HwFirmwareStore.h"

#include "HwFirmware.h"

namespace IpsiusService
{

    HwFirmwareStore::HwFirmwareStore( const QString &path ) : m_path(path)
    {
        Synchronize();
    }

    // ------------------------------------------------------------------------------------

    void HwFirmwareStore::ListFirmware( Utils::StringList &sl, bool withBadItems ) const
    {
        ItemList::const_iterator i;
        for(i = m_items.begin(); i != m_items.end(); ++i)
        {
            sl.push_back( (*i)->Info() );
        }            

        if (withBadItems)
        {
            for(int i = 0; i < m_badItems.size(); ++i)
            {
                sl.push_back( QString("BAD: %1").arg(m_badItems.at(i)) );
            }
        }
        // summary
        QString s = QString("OK: %1; Bad: %2")
            .arg(m_items.size()).arg(m_badItems.size());
        sl.push_back(s);
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<HwFirmware> HwFirmwareStore::UpdateRequest( const BfBootCore::BroadcastMsg &msg, bool rollbackMode ) /*const */  
    {
        // синхронизация прошивок с содержимым на диске
        Synchronize();            

        // поиск подходящей по Type+Num прошивки с наибольшим номером релиза
        shared_ptr<HwFirmware> pFw;        
        for(ItemList::iterator i = m_items.begin(); i != m_items.end(); ++i)
        {
            shared_ptr<HwFirmware> pCurr = *i;
            if (!pCurr->Accepted(msg)) continue;

            if (pFw == 0) pFw = pCurr;
            else
            {
                if (pCurr->ReleaseCompare(*pFw) > 0) pFw = pCurr;
            }
        }  

        if (pFw == 0) return pFw; // нет подходящей прошивки

        // сравнение с версией релиза на плате        
        if (!rollbackMode && pFw->ReleaseCompare(msg) < 0) pFw.reset(); // pFw ver < msg
        if (pFw && pFw->ReleaseCompare(msg) == 0) pFw.reset();                 // pFw ver == msg

        return pFw;
    }

    // ------------------------------------------------------------------------------------

    void HwFirmwareStore::Synchronize() /* can throw */
    {
        QFileInfoList files = QDir(m_path).entryInfoList(QStringList("*.fwu"), QDir::Files);

        // удаление прошивок если файлов уже нет
        ItemList::iterator i = m_items.begin();
        while (i != m_items.end())
        {                
            ItemList::iterator curr = i;                
            ++i;
            if ( !files.contains((*curr)->getFileInfo()) ) m_items.erase(curr);                                
        }

        // добавление прошивок           
        for(int i = 0; i < files.size(); ++i)
        {                
            try
            {
                if (!IsContain(files.at(i))) 
                    m_items.push_back( shared_ptr<HwFirmware>(new HwFirmware( files.at(i)) ) );
            }
            catch(const ESS::BaseException &e)
            {
                QString s = QString("%1 // %2").arg(files.at(i).fileName())
                    .arg(e.getTextAndPlace().c_str());
                m_badItems.push_back(s);
            }
        }
    }

    // ------------------------------------------------------------------------------------

    bool HwFirmwareStore::IsContain( const QFileInfo& info )
    {
        for(ItemList::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
        {
            if ( (*i)->getFileInfo() == info ) return true;
        }
        return false;
    }
} // namespace IpsiusService
