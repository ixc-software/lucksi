#ifndef _CH_DATA_CAPTURE_H_
#define _CH_DATA_CAPTURE_H_

//#include "iCmp/ChMngProto.h"

#include "Utils/WeakRef.h"

namespace iCmpExt
{

	class ChDataCaptureMode : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;
	public:
		enum Value
		{ 
			None,
			Bin,
			Wav
		};
		Q_ENUMS(Value);
	};

	// -------------------------------------------------------------------------------	

	class IChDataCaptureSender : Utils::IBasicInterface 
	{
	public:
		virtual void SendDataCapture(int channel, bool enable) = 0;
	};

	// -------------------------------------------------------------------------------	

	class IChDataCapture : Utils::IBasicInterface 
	{
	public:

		virtual int  ChannelNumber() const = 0;

		virtual void Event(const std::vector<Platform::byte> &rx, 
			const std::vector<Platform::byte> &tx, 
			const std::vector<Platform::byte> &rxFixed) = 0;

	};

	// -------------------------------------------------------------------------------	

    class IStore;

	class ChDataCapture : boost::noncopyable,
		IChDataCapture
	{        
		Utils::WeakRefHost m_selfRefHost;
        boost::scoped_ptr<IStore> m_store;        
		Utils::WeakRef<IChDataCaptureSender&> m_sender;
		int m_channel;

	public:

		ChDataCapture(const std::string &ownerDesc, 
			ChDataCaptureMode::Value mode,
			Utils::WeakRef<IChDataCaptureSender&> sender,
			int channel);

		~ChDataCapture();

		Utils::WeakRef<IChDataCapture&> SelfRef() 
		{
			return m_selfRefHost.Create<IChDataCapture&>(*this);
		}

	// IChDataCapture
	private:

		int  ChannelNumber() const;

		void Event(const std::vector<Platform::byte> &rx, 
			const std::vector<Platform::byte> &tx, 
			const std::vector<Platform::byte> &rxFixed);
	};
}

#endif

