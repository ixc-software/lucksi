#ifndef STATEINFOCODE_H
#define STATEINFOCODE_H

namespace BfBootCore
{
    using Platform::int32;

    // коды ответов от BootServer клиенту
    enum StateInfoCode
    {
        _MinValueInfoCode_ = 0, // для проверки при преобразовании

        infoSaveBegin,   
        infoSaveProgress,
        infoSaveFailed,
        infoSaveComplete,

        _MaxValueInfoCode_
    };



    class SrvStateInfo 
    {
        StateInfoCode m_code;
        mutable std::string m_info;

        void Resolve() const
        {
            #define RESOLVE(errCode) if (errCode == m_code) {m_info = #errCode; return;}

            RESOLVE(infoSaveBegin);            
            RESOLVE(infoSaveFailed);
            RESOLVE(infoSaveComplete);

            #undef RESOLVE
        };

    public:

        SrvStateInfo(StateInfoCode code) : m_code(code){}

        explicit SrvStateInfo(int32 userInfoCode)
        {
            ESS_ASSERT(userInfoCode > _MinValueResponseCode_ && userInfoCode < _MaxValueResponseCode_ );
            m_code = StateInfoCode(userInfoCode);
        }

        const std::string& AsString() const
        {
            if( m_info.empty() ) Resolve();
            return m_info;
        }

        StateInfoCode AsCode() const
        {
            return m_code;
        }        
    };

} // namespace BfBootCore

#endif
