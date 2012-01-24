#include "stdafx.h"
#include "CallRecord.h"

namespace iReg
{
	// CallSideRecord

	CallSideRecord::CallSideRecord( boost::function<void ()> endCallObserver ) : 
		m_endCallObserver(endCallObserver)
	{
		m_endCode = 0;
	}

	// -------------------------------------------------------------------------------------------------------

	QString CallSideRecord::ToString() const
	{
		if (IsEmpty()) return "empty";

		QString str;
		str = QString("Name : %1, Calling info : %2, Called info : %3")
			.arg((m_name.isEmpty()) ? "Unknown" : m_name)
			.arg((m_callingInfo.isEmpty()) ? "Unknown" : m_callingInfo)
			.arg((m_calledInfo.isEmpty()) ? "Unknown" : m_calledInfo);

		if (!m_additionalInfo.isEmpty()) str += ", " + m_additionalInfo;
		if (m_endCode != 0)  str += QString(", End code : %1").arg(m_endCode);

		return str + ", Destroy reason : " + m_destroyReason;
	}

	// -------------------------------------------------------------------------------------------------------

	void CallSideRecord::SetName( const QString &str )
	{
		m_name = str;
	}

	// -------------------------------------------------------------------------------------------------------

	void CallSideRecord::SetEndCode( int code, const QString &description )
	{
		m_endCode = code;
		m_destroyReason = description;

		m_endCallObserver();
	}

	// -------------------------------------------------------------------------------------------------------

	void CallSideRecord::SetAdditionalInfo(const QString &info)
	{
		m_additionalInfo = info;
	}

	// -------------------------------------------------------------------------------------------------------

	void CallSideRecord::SetCalledInfo( const QString &calledInfo )
	{
		m_calledInfo = calledInfo;
	}

	// -------------------------------------------------------------------------------------------------------

	void CallSideRecord::SetCallingInfo( const QString &callingInfo )
	{
		m_callingInfo = callingInfo;
	}

	// -------------------------------------------------------------------------------------------------------

	bool CallSideRecord::IsEmpty() const
	{
		return m_name.isEmpty() && 
			m_calledInfo.isEmpty() && 
			m_callingInfo.isEmpty() && 
			m_endCode == 0 &&
			m_destroyReason.isEmpty();
	}
	// ~CallSideRecord
	// -------------------------------------------------------------------------------------------------------

	CallRecord::CallRecord() : 
	m_srcSide(boost::bind(&T::EndCall, this)),
		m_dstSide(boost::bind(&T::EndCall, this))
	{
		m_beginCallTime = QDateTime::currentDateTime();
		m_talkDuration = 0;
	}

	// -------------------------------------------------------------------------------------------------------

	QString CallRecord::ToString() const
	{
		QString res = "Call begin time: " + m_beginCallTime.toString() +
			", Call end time: " + m_endCallTime.toString() +
			", Talk duration (sec): " + QString().setNum(m_talkDuration) +
			", Src: {" + m_srcSide.ToString() + "}";

		QString dst(m_dstSide.ToString());
		if (!dst.isEmpty()) res += ", Dst: {" + dst + "}";
		return res;
	}

	// -------------------------------------------------------------------------------------------------------

	void CallRecord::EndCall()
	{
		QDateTime currentDateTime = QDateTime::currentDateTime();

		if (!m_endCallTime.isNull()) return;

		m_endCallTime = currentDateTime;

		if (!m_beginTalkTime.isNull())	m_talkDuration += m_beginTalkTime.secsTo(currentDateTime);
	}
}  // namespace iReg

