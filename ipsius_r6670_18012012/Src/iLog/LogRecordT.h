#ifndef __LOGRECORDT__
#define __LOGRECORDT__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "LogSessionBody.h"

namespace iLog
{

    /*
        ������ � ����
        TTimestamp      -- ��� ��������� ����� (��������� ���� ��� ����/�����), ��. LogTimestamp
        TData           -- ������ ������; ��� ������; ��� ���� ������ 
                           (std::string, QString ��� ����� ��������)

        TConvert        -- ���, ����������� ����������� ������� ��� �������������� 
                           char* � TData
    */	
	template<class TTimestamp, class TData, class TConvert>
    class LogRecordT : boost::noncopyable
    {
    public:

        typedef TData                                           RecordData;
        typedef TTimestamp                                      RecordTimestamp;
        typedef typename TTimestamp::CaptureModeType            RecordTimestampMode;
        typedef RecordKind<TData>                               RecordTag;

        LogRecordT(LogSessionBody<TData> &session, 
                   const TData &data, 
                   RecordTimestampMode tsMode,
                   const TData &recordKind) : 
            m_timestamp(tsMode), 
            m_session(session), 
            m_data(data),
            m_recordKind(recordKind)            
        {
            m_session.IncUsage();
        }

        ~LogRecordT()
        {
            // record always destroyed in safe context (LogStore lock mutex)
            m_session.DecUsage(true);
        }

        bool TimeStampEmpty() const
        {
            return m_timestamp.IsEmpty();
        }

        int TimeStampCompare(const LogRecordT &other) const
        {
            return m_timestamp.Compare(other.m_timestamp);
        }

        const RecordTimestamp& Timestamp() const { return m_timestamp; }
        const TData& GetRecordKind() const          { return m_recordKind; }
        const TData& Session() const             { return m_session.Name(); }
        const RecordData& Data() const           { return m_data; }

        LogSessionBody<TData>* SessionBody()     { return &m_session; }

        static RecordData CharToData(const char *p)
        {
            return TConvert::CharToData(p);
        }

        static std::string DataToString(const TData &data)
        {
            return TConvert::DataToString(data);
        }

    private:

        TTimestamp m_timestamp;             // ����� �������� ������
        LogSessionBody<TData> &m_session;   // ���� ������
        TData m_data;                       // ������ 
        const TData &m_recordKind;          // ��� ������

    };
	
}  // namespace iLog

#endif

