#ifndef __GENUTILS__
#define __GENUTILS__

namespace iDSP
{

	struct Gen2xProfile
	{
	public:
		Gen2xProfile()
		{
			Freq0 = -1;
			Freq0Lev = -1;
			Freq1 = -1;
			Freq1Lev = -1;
			GenInterval = 0;
			SilenceInterval = 0;
		}

		Gen2xProfile(int freq0, int freq0Lev,
			int freq1, int freq1Lev,  
			int genInterval, int silenceInterval)
		{
			Freq0 = freq0;
			Freq0Lev = freq0Lev;
			Freq1 = freq1;
			Freq1Lev = freq1Lev;
			GenInterval = genInterval;
			SilenceInterval = silenceInterval;
		}

		static const Gen2xProfile Silence;
		static const Gen2xProfile Ready;
		static const Gen2xProfile Alert;
		static const Gen2xProfile Busy;
		static const Gen2xProfile Ticker;

		const std::string ToString() const;

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
		{
			s   << data.Freq0
				<< data.Freq0Lev
				<< data.Freq1
				<< data.Freq1Lev
				<< data.GenInterval
				<< data.SilenceInterval;
		}

	public:
		int Freq0; 
		int Freq0Lev;
		int Freq1;
		int Freq1Lev;
		int GenInterval;
		int SilenceInterval;
	};

	bool operator==(const Gen2xProfile &par1, const Gen2xProfile &par2);
	bool operator!=(const Gen2xProfile &par1, const Gen2xProfile &par2);

}  // namespace iDSP


#endif