#ifndef _NOBJ_CALL_MEETING_H_
#define _NOBJ_CALL_MEETING_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

namespace Pcid
{

	class NObjMachineId : public Domain::NamedObject
	{
		Q_OBJECT;
		typedef NObjMachineId T;
	public:
		NObjMachineId(Domain::IDomain *pDomain, const Domain::ObjectName &name); 
		
		Q_INVOKABLE void GenPcid(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void GenPcidDmiDecode(DRI::ICmdOutput *pOutput, const QString &fileName);
		Q_INVOKABLE void TestSerialFile(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void TestPcid();
		Q_INVOKABLE void GenSerial(DRI::ICmdOutput *pOutput, const QString &password);
	private:
		void SaveResultFile(const QString &name, const QString &info);
	};

}; // namespace Pcid

#endif

