/*
    Файл интерфейсов межуровневых взаимодействий Isdn
*/
#ifndef ISDNLAYERSINTERFACES_H
#define ISDNLAYERSINTERFACES_H

//для moc-файла, наверно их нужно включить в ObjLinkInterfaces.h?
#include "stdafx.h"
#include "ObjLink/ObjectLinkMeta.h"
#include "ObjLink/ObjLinkInterfaces.h"

#include "Utils/IBasicInterface.h"
#include "Platform/Platform.h"
#include "L2Error.h"
#include "Dss1Binders.h"
#include "isdnpack.h"

namespace ISDN
{
    using Platform::byte;	
    
	class IDriverToL2 : public ObjLink::IObjectLinkInterface
	{
	public:
	   
			virtual void DataInd(QVector<byte> packet) = 0;
			virtual void ActivateInd() = 0;
			virtual void DeactivateInd() = 0;
			virtual void ActivateConf() = 0;
			virtual void DeactivateConf() = 0;

			virtual void PullConf() = 0; // могу принять пакет в "пулл" (PullInd(pack))
			virtual void PauseInd() = 0;
			virtual void PauseConf() = 0;

            //virtual void SetDownIntf(BinderToIDriver drvBinder) = 0;
	     
	};


	class IL2ToDriver : public ObjLink::IObjectLinkInterface
	{
	public:
	  
            // отправить пакет
			virtual void DataRequest(QVector<byte> packet) = 0;

            // выполнить активацию физ. уровня
			virtual void ActivateRequest() = 0;

            // выполнить деактивацию физ. уровня
			virtual void DeactivateRequest() = 0;

            // запрос готовности принять пакет для отложенной отправки
			virtual void PullRequest() = 0;

            // принять пакет для отложенной отправки
			virtual void PullInd(QVector<byte> packet) = 0;

            virtual void SetUpIntf(BinderToIDrvToL2 pIntfUp) = 0;	    
	};    

	class IL2ToL3 : public ObjLink::IObjectLinkInterface
	{
	public:

		virtual void EstablishInd() = 0;
		virtual void EstablishConf() = 0;
			
		virtual void ReleaseConf() = 0;
		virtual void ReleaseInd() = 0;
					
        virtual void DataInd(QVector<byte> l2data) = 0;		
        virtual void UDataInd(QVector<byte> l2data) = 0;

        // Индикация освобождения окна отправки
        virtual void FreeWinInd(){}

        virtual void SetDownIntf(BinderToIL3ToL2 binderToL2) = 0;

        // standard MDL error indications code
        virtual void ErrorInd(L2Error errorCode) = 0;
	};

    class ForceMoc_IsdnLayersInterfaces : QObject { Q_OBJECT };

	class IL3ToL2  : public ObjLink::IObjectLinkInterface
	{
	public:

		virtual void EstablishReq() = 0;
		virtual void ReleaseReq() = 0;
			
		virtual void DataReq(QVector<byte> l3data) = 0;
		virtual void UDataReq(QVector<byte> l3data) = 0;

        virtual void SetUpIntf(BinderToIL2ToL3 binderToL3) = 0;
	
		/*virtual void MAssignReq(int arg) = 0;
		virtual void MRemoveReq() = 0;
		virtual void MErrorResponse() = 0;*/
	};

	class TeiMng : public Utils::IBasicInterface// на втором уровне вызывались через st->l3.l3l2
	{
	public:

		virtual void MAssignReq(int arg) = 0;
		virtual void MRemoveReq() = 0;
		virtual void MErrorResponse() = 0;
	};
	
};

#endif
