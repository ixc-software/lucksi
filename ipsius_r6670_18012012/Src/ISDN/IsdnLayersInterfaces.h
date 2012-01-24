/*
    ���� ����������� ������������ �������������� Isdn
*/
#ifndef ISDNLAYERSINTERFACES_H
#define ISDNLAYERSINTERFACES_H

//��� moc-�����, ������� �� ����� �������� � ObjLinkInterfaces.h?
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

			virtual void PullConf() = 0; // ���� ������� ����� � "����" (PullInd(pack))
			virtual void PauseInd() = 0;
			virtual void PauseConf() = 0;

            //virtual void SetDownIntf(BinderToIDriver drvBinder) = 0;
	     
	};


	class IL2ToDriver : public ObjLink::IObjectLinkInterface
	{
	public:
	  
            // ��������� �����
			virtual void DataRequest(QVector<byte> packet) = 0;

            // ��������� ��������� ���. ������
			virtual void ActivateRequest() = 0;

            // ��������� ����������� ���. ������
			virtual void DeactivateRequest() = 0;

            // ������ ���������� ������� ����� ��� ���������� ��������
			virtual void PullRequest() = 0;

            // ������� ����� ��� ���������� ��������
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

        // ��������� ������������ ���� ��������
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

	class TeiMng : public Utils::IBasicInterface// �� ������ ������ ���������� ����� st->l3.l3l2
	{
	public:

		virtual void MAssignReq(int arg) = 0;
		virtual void MRemoveReq() = 0;
		virtual void MErrorResponse() = 0;
	};
	
};

#endif
