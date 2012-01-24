#ifndef __SFXPROTORESP__
#define __SFXPROTORESP__

namespace EchoApp
{
    
    class EchoResp
    {
    public:

        // sync it with Decode() - !
        enum Code
        {
            ER_Null,

            ER_Ok,
            ER_BadVersion,
            ER_BadEchoChannel,
            ER_BadEchoTaps,
        };

        EchoResp() : m_code(ER_Null)
        {
        }

        EchoResp(Code code, const std::string &text = "") : m_code(code), m_text(text)
        {            
        }

        std::string ToString() const
        {
            std::ostringstream oss;

            oss << Decode(m_code);
            if (!m_text.empty()) oss << ": " << m_text;

            return oss.str();
        }

        Code getCode() const { return m_code; }
        const std::string& getText() const { return m_text; }

        static const char* Decode(Code c)
        {
            #define DECODE(x) if (c == x) return #x;

            DECODE(ER_Null);
            DECODE(ER_Ok);
            DECODE(ER_BadVersion);
            DECODE(ER_BadEchoChannel);
            DECODE(ER_BadEchoTaps);

            #undef DECODE

            return "(undefined)";
        }


    private:

        Code m_code;
        std::string m_text;
    };
    
    // ---------------------------------------------------------

    ESS_TYPEDEF_T(EchoException, EchoResp);
    
        
}  // namespace EchoApp

#endif
