#include "stdafx.h"
#include "Utils/QtEnumResolver.h"

namespace Utils
{

	std::string QtEnumResolverImpl::ResolveSet(const  QMetaEnum &e, int value)
	{
		QByteArray res(e.valueToKeys(value));
		std::ostringstream out;
		bool temp = false;
		for(int i = 0; i < e.keyCount (); ++i)
		{
			if(value & e.value(i))
			{
				if(temp) out << " | ";
				out << e.key(i);
				temp = true;
			}
		}
		return out.str();
	}

	// -----------------------------------------------------

	const char* QtEnumResolverImpl::Resolve(const  QMetaEnum &e, int value)
	{
		const char *p = e.valueToKey(value);
		return p;
	}

	// -----------------------------------------------------

	bool QtEnumResolverImpl::Resolve(const  QMetaEnum &e, const char *inValue, int &outValue)
	{
		ESS_ASSERT(inValue != 0);

		for(int i = 0; i < e.keyCount(); ++i)
		{
			const char *pCurrKey = e.key(i);
			ESS_ASSERT(pCurrKey != 0);

			if (std::strcmp(inValue, pCurrKey) != 0) continue;
			outValue = e.value(i);
			return true;
		}
		outValue = -1;
		return  false;
	}

};


