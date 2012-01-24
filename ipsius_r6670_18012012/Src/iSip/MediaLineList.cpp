#include "stdafx.h"
#include "undeferrno.h"
#include "MediaLineList.h"
#include "MediaLine.h"
#include "SdpUtils.h"
#include "IRemoteParticipant.h"
#include "Sdp/SdpHelperResip.h"
#include "Utils/GetDefLocalHost.h"

namespace {

	resip::SdpContents::Session::Medium
		CreateRejectMedium(const resip::Data &name, const resip::Data &protocol)
	{
		return resip::SdpContents::Session::Medium(name,
			0,
			1,  // Reject medium by specifying port 0 (RFC3264)	
			protocol);
	}

	// -------------------------------------------------------------------------------------

	resip::SdpContents::Session::Medium
		CreateRejectMedium(const resip::SdpContents::Session::Medium &original)
	{
		return CreateRejectMedium(original.name(), original.protocol());
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine_MakeAnswer(const sdpcontainer::SdpMediaLine &offer,
		iSip::MediaLine &localMedia,
		resip::SdpContents &answer,
		bool potential)
	{
		resip::SdpContents::Session::Medium result;

		if (!localMedia.AnswerMediaLine(offer, result, potential)) return false;

		answer.session().addMedium(result);

		return true;
	}
}

namespace iSip
{
	
	MediaLineList::Manager::Manager(MediaLineList &list,
		Utils::SafeRef<IRemoteParticipant> call) :
		m_list(list),
		m_call(call),
		m_modified(false)
	{}

	// -------------------------------------------------------------------------------------

	MediaLineList::Manager::~Manager()
	{
		if (!m_call.IsEmpty() && m_modified) m_call->UpdateMedia();
	}

	// -------------------------------------------------------------------------------------

	const MediaLineProfile&
		MediaLineList::Manager::Profile(const ISipMediaLineEvents &user) const
	{
		MediaLine *line = m_list.MediaLinePtr(user);

		ESS_ASSERT (line != 0);

		return line->Profile();
	}

	// -------------------------------------------------------------------------------------

	MediaLineProfile& MediaLineList::Manager::Profile(
		const ISipMediaLineEvents &user)
	{
		m_modified = true;

		MediaLine *line = m_list.MediaLinePtr(user);

		ESS_ASSERT (line != 0);

		return line->Profile();
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::Manager::AddMediaLine(ISipMediaLineEvents &user,
		const MediaLineProfile &profile)
	{
		m_modified = true;

		MediaLine *line = m_list.MediaLinePtr(user);

		if (line == 0)
			m_list.AddMediaLine(user, profile);
		else
			line->Profile() = profile;
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::Manager::DeleteMediaLine(const ISipMediaLineEvents &user)
	{	
		m_modified = true;

		m_list.DeleteMediaLine(user);
	}	
}

namespace iSip
{

	MediaLineList::MediaLineList(iLogW::ILogSessionCreator &logCreator,
		OfferProcessor offerProcessor,
        AdjustCompleteObserver adjustCompleteObserver) :
		m_log(logCreator.CreateSession("Media", true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_offerProcessor(offerProcessor),
        m_adjustCompleteObserver(adjustCompleteObserver)
	{
	}

    // -------------------------------------------------------------------------------------

    MediaLineList::~MediaLineList()
    {}

	// -------------------------------------------------------------------------------------
	// iSip::ISipMedia

	void MediaLineList::SetupNatIpConverter(NatIpConverter converter)
	{
		m_natIpConverter = converter;
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
	{
		m_remoteSdp.reset(
			sdpcontainer::SdpHelperResip::createSdpFromResipSdp(*offer));

		if (!m_offerProcessor.empty()) m_offerProcessor(offer);
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::AdjustRTPStreams(bool isOffer)
	{
		if (isOffer)
		{
			InitReceiveProcess();
		}
        else
        {
		    MediaLinesList localLines;
		    FillTempLocalMediaList(localLines);
		    AdjustRTPStreams(m_remoteSdp->getMediaLines(), localLines);
        }
        
        if (!m_adjustCompleteObserver.empty()) m_adjustCompleteObserver(isOffer);
	}

	// -------------------------------------------------------------------------------------

	bool MediaLineList::GenerateAnswer( resip::SdpContents &answer ) const
	{
		if (!GenerateAnswer( *m_remoteSdp, answer )) return false;
		
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "GenerateAnswer: "
				<< SipUtils::ToString(answer)
				<< iLogW::EndRecord;
		}

		return true;


	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::GenerateOffer( resip::SdpContents &offer ) const
	{

		QHostAddress hostAddress = Utils::GetDefLocalHost();

		resip::Data ip = SipUtils::ToUtf8(hostAddress.toString());

		SdpUtils::SetupOrigin(hostAddress.protocol() == QAbstractSocket::IPv4Protocol,
			(m_natIpConverter.empty()) ? ip : m_natIpConverter(ip),
			offer);


		for (int i = 0; i < m_lines.Size(); ++i)
		{
			resip::SdpContents::Session::Medium medium;
			m_lines[i]->BuildResipMedium(medium);
			offer.session().addMedium(medium);
		}

		if (!m_lines.IsEmpty()) m_lines[0]->BuildConnectionAttribute(m_natIpConverter,
			offer.session().connection());

		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "GenerateOffer: " << SipUtils::ToString(offer) << iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::FillTempLocalMediaList(MediaLinesList &list) const
	{
		for(int i = 0; i < m_lines.Size(); ++i)
		{
			ESS_ASSERT(m_lines[i] != 0);
			list.push_back(m_lines[i]);
		}
	}

	// -------------------------------------------------------------------------------------

	bool MediaLineList::GenerateAnswer( const sdpcontainer::Sdp &offer, resip::SdpContents &answer ) const
	{
		if (m_lines.IsEmpty()) return false;

		resip::Data ip = SipUtils::ToUtf8(Utils::GetDefLocalIp());

		// Set session id and version for this answer
		SdpUtils::SetupOrigin(true,
			(m_natIpConverter.empty()) ? ip : m_natIpConverter(ip),
			answer);

		if (!offer.getTimes().empty())
		{
			sdpcontainer::Sdp::SdpTime offerTime(offer.getTimes().front());

			resip::SdpContents::Session::Time time(offerTime.getStartTime(),
				offerTime.getStopTime());

			answer.session().addTime(time);
		}

		MediaLinesList localLines;
		FillTempLocalMediaList(localLines);

		bool valid = false;

		for(sdpcontainer::Sdp::MediaLineList::const_iterator offerMediumsIt = offer.getMediaLines().begin();
			offerMediumsIt != offer.getMediaLines().end();
			++offerMediumsIt)
		{
			bool mediaLineValid = AnswerMediaLines( (*offerMediumsIt)->getPotentialMediaViews(),
				localLines,
				answer, true);

			if (!mediaLineValid)
			{
				mediaLineValid = AnswerMediaLines( *(*offerMediumsIt),
					localLines,
					answer, false);
			}

			if (!mediaLineValid)
			{
				answer.session().addMedium(
					CreateRejectMedium((*offerMediumsIt)->getMediaTypeString(), (*offerMediumsIt)->getTransportProtocolTypeString()));
			}
			else
			{
				valid = true;
			}
		}  // end loop through m= offers

		return valid;
	}

	// -------------------------------------------------------------------------------------

	bool MediaLineList::AnswerMediaLines(const sdpcontainer::SdpMediaLine &offer,
		MediaLinesList &localLines,
		resip::SdpContents &answer,
		bool potential)
	{
		return ProcessMedia(offer,
			localLines,
			boost::bind(&MediaLine_MakeAnswer, _1, _2, boost::ref(answer), potential));
	}

	// -------------------------------------------------------------------------------------

	bool MediaLineList::AnswerMediaLines( const sdpcontainer::SdpMediaLine::SdpMediaLineList &offerList,
		MediaLinesList &localLines,
		resip::SdpContents &answer,
		bool potential ) const
	{
//		ESS_ASSERT(answer.session().media().empty());

		bool result = ProcessMedia(offerList,
			localLines,
			boost::bind(&MediaLine_MakeAnswer, _1, _2, boost::ref(answer), potential));
		
		if (!m_lines.IsEmpty())
		{
			m_lines[0]->BuildConnectionAttribute(m_natIpConverter, answer.session().connection());
		}

		return result;

	}

	// -------------------------------------------------------------------------------------

	bool AdjustRTPStreamsImpl(const sdpcontainer::SdpMediaLine &remoteMediaLine,
		MediaLine &media)
	{
		return media.AdjustRTPStreams(remoteMediaLine);
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::AdjustRTPStreams(const sdpcontainer::Sdp::MediaLineList &offerList,
		MediaLinesList &localLines) const
	{

		for(sdpcontainer::Sdp::MediaLineList::const_iterator offerMediumsIt = offerList.begin();
			offerMediumsIt != offerList.end();
			++offerMediumsIt)
		{
			
			bool result = ProcessMedia((*offerMediumsIt)->getPotentialMediaViews(),
				localLines,
				boost::bind(&AdjustRTPStreamsImpl, _1, _2));

			if (!result)
			{
				bool result = ProcessMedia(*(*offerMediumsIt),
					localLines,
					boost::bind(&AdjustRTPStreamsImpl, _1, _2));
			}

		}		
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::InitReceiveProcess()
	{
		for(int i = 0; i < m_lines.Size(); ++i)
		{
			m_lines[i]->InitReceiveProcess();
		}
	}

	// -------------------------------------------------------------------------------------

	// static
	bool MediaLineList::ProcessMedia(const sdpcontainer::SdpMediaLine &offer,
		MediaLinesList &localLines,
		boost::function<bool (const sdpcontainer::SdpMediaLine&, MediaLine &)> fn)
	{
		for(MediaLinesList::iterator i = localLines.begin();
			i != localLines.end(); ++i)
		{
			MediaLine *localMedia = *i;

			if (localMedia == 0) continue;
				
			if (!fn(offer, *localMedia)) continue;

			*i = 0;

			return true;
		}

		return false;

	}

	// -------------------------------------------------------------------------------------

	bool MediaLineList::ProcessMedia( const sdpcontainer::SdpMediaLine::SdpMediaLineList &offerList,
		MediaLinesList &localLines,
		boost::function<bool (const sdpcontainer::SdpMediaLine&, MediaLine &)> fn) const
	{
		bool result = false;

		for(sdpcontainer::SdpMediaLine::SdpMediaLineList::const_iterator i = offerList.begin();
			i != offerList.end(); ++i)
		{
			if(ProcessMedia(*i, localLines, fn)) result = true;
		}

		return result;
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::AddMediaLine(ISipMediaLineEvents &media,
		const MediaLineProfile &profile)
	{
		ESS_ASSERT(FindMediaLine(media) == m_lines.Size());

		m_lines.Add(new MediaLine(*m_log, media, profile));
	}

	// -------------------------------------------------------------------------------------

	void MediaLineList::DeleteMediaLine(const ISipMediaLineEvents &media)
	{
		int i = FindMediaLine(media);

		ESS_ASSERT(i < m_lines.Size());

		m_lines.Delete(i);
	}

	// -------------------------------------------------------------------------------------

	int MediaLineList::FindMediaLine(const ISipMediaLineEvents &user) const
	{
		int i = 0;

		while (i < m_lines.Size())
		{
			if (m_lines[i]->IsSame(user)) break;

			++i;
		}

		return i;
	}

	// -------------------------------------------------------------------------------------

	const MediaLine* MediaLineList::MediaLinePtr(const ISipMediaLineEvents &user) const
	{
		int i = FindMediaLine(user);

		return (i < m_lines.Size()) ? m_lines[i] : 0;
	}

	// -------------------------------------------------------------------------------------

	MediaLine* MediaLineList::MediaLinePtr(const ISipMediaLineEvents &user)
	{
		int i = FindMediaLine(user);

		return (i < m_lines.Size()) ? m_lines[i] : 0;
	}


}
