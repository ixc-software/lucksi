#include "stdafx.h"

#include "qextserialport.h"
#include "NameUartPort.h"
#include "UartStatistics.h"
#include "UartProfile.h"
#include "Uart.h"

namespace 
{
    QextSerial::ParityType ConvertParity(iUart::ParityType parity)
    {
        if(parity == iUart::ParityNone) return QextSerial::PAR_NONE;
        if(parity == iUart::ParityOdd) return QextSerial::PAR_ODD;
        if(parity == iUart::ParityEven) return QextSerial::PAR_EVEN;
        if(parity == iUart::ParityMark) return QextSerial::PAR_MARK;
        if(parity == iUart::ParitySpace) return QextSerial::PAR_SPACE;
        ESS_HALT("Unknown parity");
        return QextSerial::PAR_NONE;
    };

    QextSerial::DataBitsType ConvertDataBits(iUart::DataBits dataBits)
    {
        if (dataBits == iUart::DataBits_5) return QextSerial::DATA_5;
        if (dataBits == iUart::DataBits_6) return QextSerial::DATA_6;
        if (dataBits == iUart::DataBits_7) return QextSerial::DATA_7;
        if (dataBits == iUart::DataBits_8) return QextSerial::DATA_8;

        ESS_HALT("Unknown data bits");
        return QextSerial::DATA_5;
    };

    QextSerial::StopBitsType ConvertDataBits(iUart::StopBits stopBits)
    {
        if (stopBits == iUart::StopBits_1) return QextSerial::STOP_1;
        if (stopBits == iUart::StopBits_2) return QextSerial::STOP_2;
        if (stopBits == iUart::StopBits_1_5) return QextSerial::STOP_1_5;

        ESS_HALT("Unknown stop bits");
        return QextSerial::STOP_1;
    }

	
    QextSerial::BaudRateType ConvertBaudRate(int baudRate)
    {
        switch(baudRate)
        {
            case 50:
                return QextSerial::BAUD50;
            case 75:
                return QextSerial::BAUD75;
            case 110:
                return QextSerial::BAUD110;
            case 134:
                return QextSerial::BAUD134;
            case 150:
                return QextSerial::BAUD150;
            case 200:
                return QextSerial::BAUD200;
            case 300:
                return QextSerial::BAUD300;
            case 600:
                return QextSerial::BAUD600;
            case 1200:
                return QextSerial::BAUD1200;
            case 1800:
                return QextSerial::BAUD1800;
            case 2400:
                return QextSerial::BAUD2400;
            case 4800:
                return QextSerial::BAUD4800;
            case 9600:
                return QextSerial::BAUD9600;
            case 14400:
                return QextSerial::BAUD14400;
            case 19200:
                return QextSerial::BAUD19200;
            case 38400:
                return QextSerial::BAUD38400;
            case 56000:
                return QextSerial::BAUD56000;
            case 57600:
                return QextSerial::BAUD57600;
            case 76800:
                return QextSerial::BAUD76800;
            case 115200:
                return QextSerial::BAUD115200;
            case 128000:
                return QextSerial::BAUD128000;
            case 256000:
                return QextSerial::BAUD256000;
        } 
        ESS_HALT("Unknown stop rate");
        return QextSerial::BAUD50;
    };

    QextSerial::FlowType ConvertFlowType(iUart::FlowType flowType)
    {
        if(flowType == iUart::FlowOff) return QextSerial::FLOW_OFF;
        if(flowType == iUart::FlowHardware) return QextSerial::FLOW_HARDWARE;
        if(flowType == iUart::FlowXonXoff) return QextSerial::FLOW_XONXOFF;

        ESS_HALT("Unknown flow type");
        return QextSerial::FLOW_OFF;
    };

};

namespace iUart
{
    using Platform::byte;

    class Uart::Impl : boost::noncopyable,
        public IUartIO,
        public IUartManagement
    {
    public:
        Impl(const UartProfile &profile, Utils::SafeRef<IUartEvents> user) : 
            m_port(NameUartPort(profile.Port()).c_str()),
            m_user(user)
        {
			UartReturnCode res = Open();
			
			if (!res.isSuccess()) ESS_THROW_MSG(InitializationFailed, res.ToString());

            SetBaudRate(profile.m_baudRate);  
            SetFlowControl(profile.m_flowType);
            SetParityCheck(profile.m_parityType);	
            SetDataBits(profile.m_dataBits);
            SetStopBits(profile.m_stopBits);
            SetReadWriteTimeout(profile.m_readWriteTimeoutMs);
            
        }
        ~Impl()
        {
            m_port.close();
        }

        void BindUser(Utils::SafeRef<IUartEvents> user)
        {
            ESS_ASSERT(m_user.IsEmpty());

            m_user = user;
        }

		void UnbindUser(const IUartEvents *user)
		{
			ESS_ASSERT(m_user.IsEqualIntf(user));
				
			m_user.Clear();
		}

    // IUartIO
    private:
        int AvailableForRead() const
        {
            return m_port.bytesAvailable();
        }

        int AvailableForWrite() const
        {
            return 0;
        }

        int ReadImpl(void *data, int size, bool throwInd)
        {
			size = m_port.read(static_cast<char *>(data), size);
			m_statistic.m_packReceived++;
	        m_statistic.m_byteReceived += size;
			return  size;
        }

        void ResetInboundData()
        {
            ESS_HALT("unsupported on PC");
        }

        void ResetOutboundData()
        {
            ESS_HALT("unsupported on PC");
        }

        int WriteImpl(const void *data, int size)
        {
            const int CMaxWriteTries = 8;

            int totalWrited = 0;
            int tries = 0;


            while(tries < CMaxWriteTries)
            {
                const char *pData = (const char*)(data) + totalWrited;
                totalWrited += m_port.write(pData, size - totalWrited);
                if (totalWrited >= size) break;

                ++tries;
            }

            ESS_ASSERT(totalWrited <= size);

			m_statistic.m_packSend++;
			m_statistic.m_byteSend += totalWrited;

			return size;
        }

    // IUartManagement
    private:

        UartReturnCode Open()
        {
            bool result = m_port.open(QIODevice::ReadWrite);
			return UartReturnCode((result) ? QextSerial::E_NO_ERROR : QextSerial::E_INVALID_FD);
        }

        void Close()
        {
            m_port.close();
        }

        void SetParityCheck(ParityType parity)
        {
            m_port.setParity(ConvertParity(parity));
        }

        void SetDataBits(DataBits dataBits)
        {
            m_port.setDataBits(ConvertDataBits(dataBits));
        }

        void SetStopBits(StopBits stopBits)
        {
            m_port.setStopBits(ConvertDataBits(stopBits));
        }

        void SetBaudRate(int rate)
        {
            m_port.setBaudRate(ConvertBaudRate(rate));
        }

        void SetFlowControl(FlowType flowType)
        {
            m_port.setFlowControl(ConvertFlowType(flowType));
        }

        QextSerial::LineStatus LineStatus() const
        {
            return static_cast<QextSerial::LineStatus>(m_port.lineStatus());
        }

        void SetReadWriteTimeout(int intervalMs)
        {
            m_port.setTimeout(0, intervalMs);
        }

    private:
        mutable QextSerial::QextSerialPort m_port;
        Utils::SafeRef<IUartEvents> m_user;
		UartStatistics m_statistic;
    };

    // ---------------------------------------------------------------

    Uart::Uart(const UartProfile &profile, Utils::SafeRef<IUartEvents> user) : 
        m_impl(new Impl(profile, user))
    {}

    // ---------------------------------------------------------------

    Uart::~Uart()
    {}

    // ---------------------------------------------------------------

	Uart *Uart::CreateUart(const UartProfile &profile, Utils::SafeRef<IUartEvents> user, std::string &error)
	{
		Uart *uart = 0;
		try
		{
			uart = new Uart(profile, user);
		}
		catch (InitializationFailed& e)
		{
            error = e.getTextMessage();
			return 0; 
		}
		return uart;
	}

	// ---------------------------------------------------------------

    void Uart::BindUser(Utils::SafeRef<IUartEvents> user)
    {
        m_impl->BindUser(user);
    }

	// ---------------------------------------------------------------

	void Uart::UnbindUser(const IUartEvents *user)
	{
        m_impl->UnbindUser(user);
	}

    // ---------------------------------------------------------------

    IUartIO & Uart::IO()
    {
        return *m_impl;
    }

    const IUartIO &Uart::IO() const
    {
        return *m_impl;
    }

    // ---------------------------------------------------------------

    IUartManagement &Uart::Management()
    {
        return *m_impl;
    }

    const IUartManagement &Uart::Management() const
    {
        return *m_impl;
    }
};

