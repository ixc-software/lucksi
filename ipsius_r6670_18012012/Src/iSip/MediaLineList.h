#pragma once

#include "ISipMediaLine.h"

#include "iSip/Sdp/SdpMediaLine.h"
#include "ConversationProfile.h"
#include "ISipMedia.h"

#include "iLog/LogWrapper.h"
#include "Utils/ManagedList.h"
#include "Utils/SafeRef.h"


namespace sdpcontainer	{	class SdpMediaLine;	}
namespace resip			{	class SdpContents ;	}

namespace iSip
{

	class  MediaLine;
	struct MediaLineProfile;
	class  IRemoteParticipant;

	class MediaLineList : boost::noncopyable,
		public ISipMedia
	{
		typedef sdpcontainer::SdpMediaLine ML;
	
	public:

		typedef boost::function<void (boost::shared_ptr<resip::SdpContents>)>
			OfferProcessor;

        typedef boost::function<void (bool)> AdjustCompleteObserver;

		MediaLineList(iLogW::ILogSessionCreator &logCreator,
			OfferProcessor offerProcessor = OfferProcessor(),
            AdjustCompleteObserver adjustCompleteObserver = AdjustCompleteObserver());

        ~MediaLineList();

		class Manager : boost::noncopyable
		{
		public:

			Manager(MediaLineList &list, 
				Utils::SafeRef<IRemoteParticipant> call = Utils::SafeRef<IRemoteParticipant>());

			~Manager();

		public:

			const MediaLineProfile& 
				Profile(const ISipMediaLineEvents &user) const;

			MediaLineProfile& 
				Profile(const ISipMediaLineEvents &user);

			void AddMediaLine(ISipMediaLineEvents &user,
				const MediaLineProfile &profile);

			void DeleteMediaLine(const ISipMediaLineEvents &user);
			
		private:
			bool m_modified;
			MediaLineList &m_list;
			Utils::SafeRef<IRemoteParticipant> m_call;
		};


	// iSip::ISipMedia
	private:

		void SetupNatIpConverter(NatIpConverter converter);

		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer);

		void AdjustRTPStreams(bool isOffer);

		bool GenerateAnswer(resip::SdpContents &answer) const;

		void GenerateOffer(resip::SdpContents &offer) const;

	private:

		typedef std::vector<MediaLine*> MediaLinesList;

		void FillTempLocalMediaList(MediaLinesList &list) const;

		bool GenerateAnswer(const sdpcontainer::Sdp &offer, 
			resip::SdpContents &answer) const;

		static bool AnswerMediaLines(const sdpcontainer::SdpMediaLine &offer,
			MediaLinesList &localLines,
			resip::SdpContents &answer, 
			bool potential);

		bool AnswerMediaLines( const sdpcontainer::SdpMediaLine::SdpMediaLineList &offerList,
			MediaLinesList &localLines,
			resip::SdpContents &answer, 
			bool potential ) const;

		void AdjustRTPStreams( const sdpcontainer::Sdp::MediaLineList &offerList,
			MediaLinesList &localLines) const;

		void InitReceiveProcess();

		void AddMediaLine(ISipMediaLineEvents &user,
			const MediaLineProfile &profile);

		void DeleteMediaLine(const ISipMediaLineEvents &user);

		int  FindMediaLine(const ISipMediaLineEvents &user) const;

		const MediaLine* MediaLinePtr(const ISipMediaLineEvents &user) const;
		MediaLine* MediaLinePtr(const ISipMediaLineEvents &user);

	private:

		static bool ProcessMedia(const sdpcontainer::SdpMediaLine &offer,
			MediaLinesList &localLines,
			boost::function<bool (const sdpcontainer::SdpMediaLine &offer, MediaLine &)> fn);

		bool ProcessMedia( const sdpcontainer::SdpMediaLine::SdpMediaLineList &offer,
			MediaLinesList &localLines,
			boost::function<bool (const sdpcontainer::SdpMediaLine &offer, MediaLine &)> fn) const;

	private:

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;

		OfferProcessor m_offerProcessor;
        AdjustCompleteObserver m_adjustCompleteObserver;

		boost::shared_ptr<sdpcontainer::Sdp> m_remoteSdp;

		NatIpConverter m_natIpConverter;

		typedef Utils::ManagedList<MediaLine> MediaList;
		
		MediaList m_lines;
	};

}


