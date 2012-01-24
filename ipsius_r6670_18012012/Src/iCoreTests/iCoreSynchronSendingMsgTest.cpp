
#include "stdafx.h"

#include "iCoreSynchronSendingMsgTest.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/SyncTimer.h"

#include "Utils/Random.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ManagedList.h"
#include "iCoreTests/TestUtils.h"

#include "Platform/Platform.h"

namespace
{
    using namespace iCore;
    using namespace Utils;
    using namespace TestUtils;    

    enum SendingConst
    {
        CSenders = 4,
        CSessions = 5,
        CMsgs = 15,
        CMsgTimeoutUS = 50, //microsec

        CSleepDuration = CSenders * CSessions * CMsgs 
                            * (CMsgTimeoutUS/1000 + 1) * 4,
    };

	class Sender;

	struct SendData
	{
		Sender* pSender;
		double val;
		bool isLast;
	};

	class Receiver : public MsgObject, boost::noncopyable
	{
		void OnPutData(SendData* pSendData);
	public:
		Receiver(MsgThread& thread);

		void PutData(SendData* pSendData);
	};

    // -------------------------------------------------------------

	class Sender : public MsgObject, boost::noncopyable
	{
		Receiver& m_receiver;
		int m_sessionCount;
		double m_receivedSumm;
        double m_sentSumm;
        Utils::Random m_random;

        AtomicBool m_finishReq;    
        AtomicBool m_finishConf;    
        AtomicBool m_insideSend;

		
        void Send();
        void SendBody();
		void CompareSumms() const;
        void ResetSumms();

        //event
        void OnFinishSession();
        void OnSetReceivedSumm(double val);
        void OnStartSession();


    public:
		Sender( MsgThread& thread, int senderNo, Receiver& receiver, int sessions);
        ~Sender();

        bool getFinished() const;
        void setFinished();

        //msg
        void StartSession();
		void FinishSession();
		void SetReceivedSumm(double val);
	};

	// ----------------------------------------------

	Receiver::Receiver(MsgThread& thread) : MsgObject(thread)
	{
	}

	// ----------------------------------------------

	void Receiver::OnPutData(SendData* pSendData)
	{
        //if (pSendData->pSender->Finished()) return;
        TUT_ASSERT(pSendData->pSender);

		pSendData->pSender->SetReceivedSumm(pSendData->val);
		if (pSendData->isLast)
		{
			pSendData->pSender->FinishSession();
		}
	}

	// ----------------------------------------------

	void Receiver::PutData(SendData* pSendData)
	{
		PutMsg(this, &Receiver::OnPutData, pSendData);
	}

	// ----------------------------------------------
	// Sender implementation
	// ----------------------------------------------

	Sender::Sender(MsgThread& thread, int senderNo, Receiver& receiver, int sessions)
		: MsgObject(thread), m_receiver(receiver), m_sessionCount(sessions), 
            m_receivedSumm(0), m_sentSumm(0),// m_finish(false), 
            m_random(senderNo)//, m_randomVal(senderNo)
	{}

    Sender::~Sender()
    {
        m_finishReq.Set(true);

        //wait until finish sending
        while (!m_finishConf.Get())
        {
            Platform::Thread::Sleep(1);
        }
    }

    // ----------------------------------------------

    void Sender::ResetSumms()
    {
        m_sentSumm = 0;
        m_receivedSumm = 0;
    }

	// ----------------------------------------------	
    void Sender::CompareSumms() const
	{
		TUT_ASSERT(m_sentSumm == m_receivedSumm);
	}

    // ----------------------------------------------

    bool Sender::getFinished() const
    {
        return m_finishConf;
    }

    // ----------------------------------------------

    void Sender::Send()
    {
        m_insideSend.Set(true);
        SendBody();
        m_insideSend.Set(false);
    }


	void Sender::SendBody()
	{
        if (m_finishReq.Get()) 
        {
            AsyncSetReadyForDeleteFlag(m_finishConf);
            return;
        }

        int msgsDif = CMsgs / 10; //10%
        int msgCount = m_random.Next(msgsDif) + (CMsgs - msgsDif);// 50;

        while(msgCount--)
		{
            //create pack
			SendData* pSendData = new SendData;
			pSendData->pSender = this;
			pSendData->val = m_random.Next(100) + 100;
			pSendData->isLast = (msgCount == 0) ? true : false;

            //save summ
			m_sentSumm += pSendData->val;

            if (m_finishReq.Get())
            {
                AsyncSetReadyForDeleteFlag(m_finishConf);
                return;
            }

            //send it to rcvr
            m_receiver.PutData(pSendData);

            //wait before send next
            Platform::ThreadWaitUS(m_random.Next(CMsgTimeoutUS - 1) + 1);   //microsec
		}

	}

	// ----------------------------------------------

    void Sender::OnFinishSession()
    {        
        // sent msgs = received msgs
        CompareSumms();

        --m_sessionCount;
        if (m_sessionCount == 0) 
        {
            m_finishConf.Set(true);
            return;
        }

        StartSession();
    }

    // ----------------------------------------------

    void Sender::FinishSession()
    {
        PutMsg(this, &Sender::OnFinishSession);
    }

    // ----------------------------------------------

    void Sender::OnStartSession()
    {
        ResetSumms();
        Send();
    }

    // ----------------------------------------------

    void Sender::StartSession()
    {
        PutMsg(this, &Sender::OnStartSession);
    }

    // ----------------------------------------------

	void Sender::OnSetReceivedSumm(double val)
	{
		m_receivedSumm += val;
	}

	// ----------------------------------------------

    void Sender::SetReceivedSumm(double val)
	{
		PutMsg(this, &Sender::OnSetReceivedSumm, val);
	}

    // ----------------------------------------------
    
    class Process : public TestProcess,  public boost::noncopyable
    {
        class SenderThread 
        {
            MsgThread m_thread;
            Sender m_sender;
        public:
            SenderThread(int senderNo, Receiver& receiver, int sessionCount) : 
              m_thread("SenderThread", Platform::Thread::LowPriority, true), 
              m_sender(m_thread, senderNo, receiver, sessionCount)
            {
                m_sender.StartSession();
            }

            bool getFinished() const
            {
                return m_sender.getFinished();
            }
        };

        class ReceiverThread
        {
            MsgThread m_thread;
            Receiver m_receiver;

        public:
            ReceiverThread() : 
              m_thread("ReceiverThread", Platform::Thread::LowPriority, true), 
              m_receiver(m_thread)
            {}

            Receiver& getReceiver()
            {
                return m_receiver;
            }
        };

        ReceiverThread m_receiverThread;

        ManagedList<SenderThread> m_senderThreads;

        bool IsProcessed()
        {
            for (size_t i = 0; i < m_senderThreads.Size(); ++i)
            {
                if (!m_senderThreads[i]->getFinished()) return false;
            }

            return true;
        }

     public:
        Process(int senderCount, int sessionCount) 
            : TestProcess(CSleepDuration)
        {
            //create senders && run their threads
            m_senderThreads.Reserve(senderCount);
            //for (int i = 0; i < senderCount; ++i)
            while (senderCount--)
            {
                m_senderThreads.Add(new SenderThread(senderCount, m_receiverThread.getReceiver(), sessionCount));
            }
        }
    };
} //namespace

// --------------------------------

namespace iCoreTests
{
    void iCoreSynchronSendingMsgTest()
    {
        Process p(CSenders, CSessions);
        p.Run();
    }

} // namespace iCoreTests
