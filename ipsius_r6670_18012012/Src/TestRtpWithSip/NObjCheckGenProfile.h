#ifndef _NOBJ_CHECK_GENERATOR_PROFILE_H_
#define _NOBJ_CHECK_GENERATOR_PROFILE_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "CheckGenProfile.h"

namespace TestRtpWithSip
{   
	class NObjCheckGenProfile : public Domain::NamedObject,
		public DRI::INonCreatable,
		CheckGenProfile
	{
		Q_OBJECT;
	public:
		NObjCheckGenProfile(Domain::NamedObject *pParent, 
			const Domain::ObjectName &name) :
			NamedObject(&pParent->getDomain(), name, pParent)
		{
			m_turnOn = false;
		}		

		Q_PROPERTY(bool Enabled READ IsEnabled WRITE m_turnOn);
		bool IsEnabled() const {	return m_turnOn;	}
		Q_PROPERTY(int AnalisSample READ m_countAnalisSample WRITE m_countAnalisSample);
		Q_PROPERTY(int CountSkipSample READ m_countSkipSample WRITE m_countSkipSample);
		Q_PROPERTY(int Freq READ m_freq WRITE m_freq);
		Q_PROPERTY(int Power READ m_power WRITE m_power);
		Q_PROPERTY(bool UseAlaw READ m_useAlaw WRITE m_useAlaw);
		bool UseAlaw() const {	return m_useAlaw; }
		CheckGenProfile Profile() const 
		{
			return *this;
		}
	private:
		bool m_turnOn;
    };
};

#endif

