#ifndef __TOBOOL__
#define __TOBOOL__

namespace Utils
{
	
	template<class T>
	void ToBool(T &val)
	{
		return (val) ? true : false;
	}
		
}  // namespace Utils

#endif

