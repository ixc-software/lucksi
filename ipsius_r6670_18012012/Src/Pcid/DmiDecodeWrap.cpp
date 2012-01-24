#include "stdafx.h"
#include "DmiDecodeWrap.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"
#include "Utils/CryptUtils.h"


namespace 
{
	class SectionParser
	{
	public:
		SectionParser(const QString &section,
			const QStringList &headers) : 
			m_isComplete(false),
			m_section(section),
			m_headers(headers)
		{
			ESS_ASSERT(!m_headers.isEmpty());
			m_isComplete = false;
		}

		QString Section() const
		{
			return m_section;
		}

		bool IsComplete() const
		{
			return m_isComplete;
		}

		int ResultSize() const
		{
			return m_result.size();
		}

		void Process(const QString &line)
		{
			for(int i = 0; i != m_headers.size(); ++i)
			{
				if(!line.startsWith(m_headers[i])) continue;
				m_result << line;
				m_headers.removeAt(i);
				if (m_headers.isEmpty()) m_isComplete = true;
				break;
			}
		}

		void Result(QStringList &result) const
		{
			if(!m_result.isEmpty()) result << m_section << m_result;
		}

		static const SectionParser CBiosInfo;
		static const SectionParser CSystemInfo;
		static const SectionParser CBaseBoardInfo;
		static const SectionParser CProcessorInfo;

		static SectionParser BiosInfoParser() 
		{ 
			QStringList headers;
			headers << "Vendor" << "Version" << "Release Date";
			return SectionParser("BIOS Information", headers); 
		}

		static SectionParser SystemInfoParser() 
		{ 
			QStringList headers;
			headers << "Manufacturer" << "Product Name" << "Version" << "Serial Number";
			return SectionParser("System Information", headers); 
		}

		static SectionParser BaseBoardInfoParser() 
		{ 
			QStringList headers;
			headers << "Manufacturer" << "Product Name" << "Version" << "Serial Number";
			return SectionParser("Base Board Information", headers); 
		}

		static SectionParser ProcessorInfoParser() 
		{ 
			QStringList headers;
			headers << "Socket Designation" << "Type" 
				<< "Family" << "Manufacturer" << "ID" << "Signature";
			return SectionParser("Processor Information", headers); 
		}
	private:
		bool m_isComplete;
		QString m_section;
		QStringList m_headers;
		QStringList m_result;
	};

	const SectionParser SectionParser::CBiosInfo(SectionParser::BiosInfoParser());
	const SectionParser SectionParser::CSystemInfo(SectionParser::SystemInfoParser());
	const SectionParser SectionParser::CBaseBoardInfo(SectionParser::BaseBoardInfoParser());
	const SectionParser SectionParser::CProcessorInfo(SectionParser::ProcessorInfoParser());


	// -------------------------------------------------------------------------------

	class DmiDecodeSections
	{
		typedef std::vector<SectionParser> List;
	public:
		DmiDecodeSections()
		{
			m_sections.push_back(SectionParser::CProcessorInfo);
			m_sections.push_back(SectionParser::CBaseBoardInfo);
			m_sections.push_back(SectionParser::CSystemInfo);
			m_sections.push_back(SectionParser::CBiosInfo);
		}

		int Parse(QTextStream &in, QStringList &out)
		{
			SectionParser *currentParser = 0;

			for(QString line = in.readLine();
				!line.isNull();
				line = in.readLine()) 
			{
				line = line.trimmed();
				if (line.isEmpty()) 
					currentParser = 0;
				else if(currentParser == 0) 
					currentParser = SearchIncompleteSection(line);
				else if (!currentParser->IsComplete()) 
					currentParser->Process(line); 
			}	
			return Result(out);
		}
	private:
		int Result(QStringList &result) const
		{
			int completedSection = 0;
			for(List::const_iterator i = m_sections.begin();
				i != m_sections.end();
				++i)
			{
				completedSection += (i->ResultSize() == 0) ? 0 : 1;
				i->Result(result);
			}
			return completedSection;
		}

		SectionParser *SearchIncompleteSection(const QString &str)
		{
			if (str.isEmpty()) return 0;

			for(List::iterator i = m_sections.begin();
				i != m_sections.end();
				++i)
			{
				if(!i->IsComplete() && i->Section() == str) return &(*i);
			}
			return 0;
		}
	private:
		List m_sections;
	};

	const int CMaxCompleteSection = 2;
};


namespace Pcid
{
	bool DmiDecodeWrap::GetMachineID(QTextStream &in, std::string &out)
	{
		QStringList result;
		DmiDecodeSections dmiDecodeSections;
		int countSection = dmiDecodeSections.Parse(in, result);

		if(countSection < CMaxCompleteSection) return false;

		// volume id -> md5 -> base64
		std::string idHash;
		Utils::MD5::Digest(result.join(";").toStdString(), idHash);
		Utils::Base64::Encode(idHash, out);

		return true;
	}

}; // namespace Pcid
