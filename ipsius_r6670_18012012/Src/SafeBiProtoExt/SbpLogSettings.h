#ifndef _SBP_LOG_SETTINGS_H_
#define _SBP_LOG_SETTINGS_H_


namespace SBProtoExt
{   
    class SbpLogSettings
    {
    public:
		SbpLogSettings(int countPar = 0, int binaryMaxBytes = -1)
		{	
			Init(countPar, binaryMaxBytes);
		}
		void Init(int countPar, int binaryMaxBytes)
		{
			m_countPar = countPar;
			m_binaryMaxBytes = binaryMaxBytes;
		}
		int  BinaryMaxBytes() const {	return m_binaryMaxBytes; }
		void BinaryMaxBytes(int  val) {	m_binaryMaxBytes = val; }
		int CountPar() const {	return m_countPar; }
		void CountPar(int  val) {	m_countPar = val; }
	private:
		int m_binaryMaxBytes; // count bytes for trace, if -1 trace all, if 0 bytes not trace
		int m_countPar; // count par for trace, if 0 trace all
	};
} // namespace SBProto

#endif

