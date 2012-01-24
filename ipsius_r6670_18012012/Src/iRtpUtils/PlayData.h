#ifndef _PLAY_DATA_RTP_TEST_H_
#define _PLAY_DATA_RTP_TEST_H_

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "IRtpTest.h"
#include "IDataSrcForRtpTest.h"

#include "Utils/StatisticElement.h"

namespace iRtp {    class RtpPcSession; };

namespace iRtpUtils
{    

	// Выполняет запись в файл пока не прийдет отбой
	class PlayData : boost::noncopyable,
		public iCore::MsgObject, 
		public IRtpTest
	{   
		typedef PlayData T;

	public:

		PlayData(iCore::MsgThread& thread, 
			Utils::SafeRef<iRtp::RtpPcSession>,
            IDataSrcForRtpTest *dataSource);

        ~PlayData();

    // IRtpTest impl
	private:
		void StartSend(const iMedia::Codec &sendCodec);
		void StartReceive(const iMedia::Codec &receiveCodec);
		void ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
	private:
		void Send(iCore::MsgTimer* );
		int getRate() const;

        void SendPack(int size);

	private:

		Utils::SafeRef<iRtp::RtpPcSession> m_rtp;
        boost::scoped_ptr<IDataSrcForRtpTest> m_dataSource;
        iCore::MsgTimer m_txTimer;
		bool m_isStarted;
		bool m_prevTickInited;
		Platform::ddword m_prevTick;
		int m_rate;
        Platform::dword m_timestamp; // == byte count
        Utils::StatElementForInt m_statSendSize;
        int m_mustSendBytes;
	};
};

#endif
