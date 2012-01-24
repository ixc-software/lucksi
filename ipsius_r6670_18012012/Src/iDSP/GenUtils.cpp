#include "stdafx.h"
#include "GenUtils.h"

namespace iDSP
{
	const Gen2xProfile Gen2xProfile::Silence(425, -15, 0, 0, 0,     1000);
	const Gen2xProfile Gen2xProfile::Ready  (425, -15, 0, 0, 10000, 0);
	const Gen2xProfile Gen2xProfile::Alert  (425, -15, 0, 0, 1000,  4000);
	const Gen2xProfile Gen2xProfile::Busy   (425, -15, 0, 0, 300,   400);
	const Gen2xProfile Gen2xProfile::Ticker (425, -3,  0, 0, 200,   5000);

	bool operator==(const Gen2xProfile &par1, const Gen2xProfile &par2)
	{
		return par1.Freq0 == par2.Freq0 &&
			par1.Freq0Lev == par2.Freq0Lev &&
			par1.Freq1 == par2.Freq1 &&
			par1.Freq1Lev == par2.Freq1Lev &&
			par1.GenInterval == par2.GenInterval &&
			par1.SilenceInterval == par2.SilenceInterval;
	}

	bool operator!=(const Gen2xProfile &par1, const Gen2xProfile &par2)
	{
		return !(par1 == par2);
	}

	const std::string Gen2xProfile::ToString() const
	{
		if(*this == Silence) return "Silence";
		if(*this == Ready) return "Ready";
		if(*this == Alert) return "Alerting";
		if(*this == Busy) return "Busy";

		std::ostringstream out;
		out << "Freq0: " << Freq0 << ','
			<< "Freq0 Lev: " << Freq0Lev << ';'
			<< "Freq1: " << Freq1 << ','
			<< "Freq1 Lev: " << Freq1Lev << ';'
			<< "Tone interval: " << GenInterval << ';'
			<< "Silence interval: " << SilenceInterval << '.';
		return out.str();
	}
}
