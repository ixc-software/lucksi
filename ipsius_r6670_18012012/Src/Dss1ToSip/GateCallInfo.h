#pragma once

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/TimerTicks.h"
#include "Utils/UtilsDateTime.h"
#include "Utils/WeakRef.h"
#include "Utils/FullEnumResolver.h"

namespace Dss1ToSip 
{

	class GateCallState : public QObject
	{
		Q_OBJECT;
		Q_ENUMS(Value);
	public:

		enum Value
		{ 
			Trying,
			Dialing,
			Ringing,
			Talk
		};
	};
	
	// ------------------------------------------------------------------

	class GateCallInfo : boost::noncopyable
	{
	public:
		GateCallInfo(bool isIncomingCall,
			const QString &srcCall,
			const QString &dstCall)
		{
			m_isIncomingCall = isIncomingCall;
			m_srcCall = srcCall;
			m_dstCall = dstCall;

			State = (m_isIncomingCall) ? GateCallState::Dialing : GateCallState::Trying;
			BeginCall = Utils::TimerTicks::Ticks();
			BeginTalk = 0;
		}

		bool IsIncomingCall() const {	return m_isIncomingCall; }

		const QString &SrcCall() const {	return m_srcCall; }
		const QString &DstCall() const {	return m_dstCall; }
		void DstCall(const QString &dstCall) {	m_dstCall = dstCall; }

		Utils::WeakRef<GateCallInfo&> SafeRef()
		{
			return m_selfRefHost.Create<GateCallInfo&>(*this);
		}

		GateCallState::Value GetState() const
		{
			return State;
		}

		QString StateToString() const
		{
			return Utils::EnumResolve(State).c_str();
		}

		void ChangeState(GateCallState::Value callState)
		{
			State = callState;
			
			if(State == GateCallState::Talk) BeginTalk = Utils::TimerTicks::Ticks();
		}

		QString TalkDuration() const
		{
			if(BeginTalk == 0) return " - // - ";

			return MSecToString(Utils::TimerTicks::Ticks() - BeginTalk);
		}

		QString CallDuration() const
		{
			return MSecToString(Utils::TimerTicks::Ticks() - BeginCall);
		}

	private:
		static QString MSecToString(Platform::dword msec)
		{
			Platform::dword sec = msec /1000;
			msec = msec % 1000;
			int min = sec / 60;
			sec = sec % 60;
			int h = min / 60;
			min = min % 60;

			return Utils::Time(h, min, sec, msec). ToString().c_str();
		};

	private:
		GateCallState::Value State;
		bool m_isIncomingCall;
		QString m_srcCall;
		QString m_dstCall;
		Platform::dword BeginCall;
		Platform::dword BeginTalk;
		Utils::WeakRefHost m_selfRefHost;
	};
}




