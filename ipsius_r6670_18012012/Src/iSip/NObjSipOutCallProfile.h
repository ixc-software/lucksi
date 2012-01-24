#pragma once

#include "stdafx.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "IRemoteParticipant.h"

namespace iSip
{

	// profile for calls from user to sip

	class NObjSipOutCallProfile : public Domain::NamedObject,
		public DRI::INonCreatable
	{

		Q_OBJECT;

		bool m_useSourceNameInContact;
		bool m_useSourceNameInFrom;
		bool m_useSourceNameAsDisplay;

	private:

		void UseSourceNameInContact(bool val)
		{	
			if (m_useSourceNameInFrom && !val) ThrowRuntimeException("Not allowed if 'UseSourceNameInFrom == true'");

			m_useSourceNameInContact = val; 
		}

		void UseSourceNameInFrom(bool val)
		{	
			if (val && !m_useSourceNameInContact) ThrowRuntimeException("Not allowed if 'UseSourceNameInContact == false'");

			m_useSourceNameInFrom = val; 
		}

	public:

		NObjSipOutCallProfile(Domain::NamedObject *pParent, const Domain::ObjectName &name) :
			NamedObject(&pParent->getDomain(), name, pParent)
		{
			m_useSourceNameInContact = false;

			m_useSourceNameInFrom = false;

			m_useSourceNameAsDisplay = false;
		}

		bool UseSourceNameInContact()	const {	return m_useSourceNameInContact; }
		bool UseSourceNameInFrom()		const {	return m_useSourceNameInFrom; }
		bool UseSourceNameAsDisplay()	const {	return m_useSourceNameAsDisplay; }

		void UpdateCallParameters(IRemoteParticipantCreator::Parameters &parameters) const
		{
			ESS_ASSERT (parameters.IsValid());

			parameters.UseSourceNameInContact = 
				UseSourceNameInContact();

			parameters.UseSourceUserNameInFrom = 
				UseSourceNameInFrom();

			parameters.UseSourceUserNameAsDisplayName = 
				UseSourceNameAsDisplay();
		}

	// DRI
	public:

		Q_PROPERTY(bool UseSourceNameInContact	READ  m_useSourceNameInContact WRITE UseSourceNameInContact);

		Q_PROPERTY(bool UseSourceNameInFrom	READ  m_useSourceNameInFrom WRITE UseSourceNameInFrom);

		Q_PROPERTY(bool UseSourceNameAsDisplay READ  m_useSourceNameAsDisplay WRITE m_useSourceNameAsDisplay);

	};


}

