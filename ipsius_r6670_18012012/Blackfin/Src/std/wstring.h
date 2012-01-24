#ifndef __WSTRING__
#define __WSTRING__

namespace std
{
	
	// wstring implementation; don't use it!
	class wstring
	{
		std::vector<wchar_t> m_data;
			
		template<class T>
		static int ValCompare(const T &base, const T &other)
		{
			if (base == other) return 0;
			return (base > other) ? 1 : -1;
		}
		
		int Compare(const wstring &other)
		{
			int cmpLen = ValCompare(size(), other.size());
			if (cmpLen != 0) return cmpLen;
			
			for(int i = 0; i < m_data.size(); ++i)
			{
				int cmp = ValCompare(m_data.at(i), other.at(i));
				if (cmp != 0) return cmp;
			}
			
			return 0;
		}
		
	public:
		
		wstring()
		{
		}
		
		wstring(const wchar_t *p)
		{
			m_data.reserve(64);
			
			while(wchar_t c = *p++)
		    {
		    	m_data.push_back(c);
		    }
		}
		
		bool operator==(const wstring &other)
		{
			return (Compare(other) == 0);
		}
		
		int size() const
		{
			return m_data.size();
		}
		
		const wchar_t& at(int indx) const
		{
			return m_data.at(indx);
		}
		
		wchar_t& at(int indx)
		{
			return m_data.at(indx);
		}
				
		void resize(int newSize)
		{
			m_data.resize(newSize);
		}
		
		void clear()
		{
			m_data.clear();
		}
						
	};	
	
}  // namespace std

#endif

