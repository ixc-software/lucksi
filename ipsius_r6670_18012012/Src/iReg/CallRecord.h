#pragma once

#include "Utils/IBasicInterface.h"
#include "Platform/Platform.h"

namespace iReg
{

	class CallSideRecord
	{
	public:

		CallSideRecord(boost::function<void ()> endCallObserver);

		QString ToString() const;

		void SetName(const QString &str);

		void SetEndCode(int code, const QString  &description);

		void SetAdditionalInfo(const QString &info);

		void SetCalledInfo(const QString  &calledInfo);

		void SetCallingInfo(const QString  &callingInfo);

	private:
	
		bool IsEmpty() const;

	private:

		QString m_name;

		QString m_calledInfo;

		QString m_callingInfo;

		int m_endCode;
		QString m_destroyReason;
		QString m_additionalInfo;

		boost::function<void ()> m_endCallObserver;
	};

	// -------------------------------------------------------------------------------------------------------

    class CallRecord
    {
		typedef CallRecord T;

    public:
		
		CallRecord();

        QString ToString() const;

		void Talk()
		{
			if (m_beginTalkTime.isNull()) m_beginTalkTime = QDateTime::currentDateTime();
		}

		CallSideRecord& Src() {	return m_srcSide; }
		CallSideRecord& Dst() {	return m_dstSide; }
	
	private:

		void EndCall();

	private:

		QDateTime m_beginCallTime;
		QDateTime m_endCallTime;

		QDateTime m_beginTalkTime;
		QDateTime m_endTalkTime;
		Platform::dword m_talkDuration;		

		CallSideRecord m_srcSide;
		CallSideRecord m_dstSide;
    };

    // ---------------------------------------------------------

	class CallRecordWrapper : boost::noncopyable
	{
	public:
		
		typedef boost::function<void (const CallRecord &)> ObserverFn;

        CallRecordWrapper(ObserverFn observer) : m_observer(observer)
		{

		}

		~CallRecordWrapper()
		{
			m_observer(m_record);
		}

		CallRecord& Record()	{	return m_record; }

		const CallRecord& Record() const {	return m_record; }	

    private:
		CallRecord m_record;
		ObserverFn m_observer;

	};
        
}  // namespace iReg

