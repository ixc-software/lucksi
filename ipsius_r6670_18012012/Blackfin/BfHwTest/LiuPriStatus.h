#ifndef __LIUPRISTATUS__
#define __LIUPRISTATUS__

namespace Ds2155
{
    
	// ���������� LIU		
	struct PriStatus
	{

		int rxSignalLevel;				// ������� �������� �1 ������� (0-15)
		bool rxLOS;						// ������ "��� ������� �����"
		bool rxAIS;						// ������ "�������� ��� �������"		
		bool rxLOF;						// ������ "��� ������������� Fas"
		bool rxLOMF;					// ������ "��� ������������� ������������� MFas"		
		bool rxLSS;						// ������ "��� ������������� �� ���"
		
		PriStatus()
		{
		}
						
		std::string ToString(bool withLf = true) const
		{				
            std::string lf = withLf ? "\n" : "  ";

			std::ostringstream resStream;

			resStream << "RxLOS = " << BoolToString(rxLOS) << lf;
			resStream << "RxAIS = " << BoolToString(rxAIS) << lf;
			resStream << "RxLOF = " << BoolToString(rxLOF) << lf;			
			resStream << "RxLOMF = " << BoolToString(rxLOMF) << lf;						
			resStream << "RxLSS = " << BoolToString(rxLSS) << lf;									
			resStream << "PriRecieveLevel = -" << (rxSignalLevel * 2.5) << " dB" << lf;			
	
			return resStream.str();
		}
		
	private:
		
		std::string BoolToString(bool val) const
		{
			return val ? "1" : "0";
		}
		
	};	
    
    
}  // namespace Ds2155


#endif
