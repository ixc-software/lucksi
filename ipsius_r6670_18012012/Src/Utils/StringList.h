#pragma once

#include "IStoragable.h"

namespace Utils
{
	
	class StringList : public QStringList, public IStoragable
	{
	protected:

		void Save(QTextStream &stream) const  // override
		{
			for(int i = 0; i < size(); i++)
			{
				stream << at(i) << endl;
			}
		}

		void Load(QTextStream &stream) // override
		{
			clear();

			while (!stream.atEnd()) 
			{
				push_back(stream.readLine());
			}
		}

	public:

        StringList(){}

        StringList(const QStringList &other) : QStringList(other)
        {}

        StringList(int argc, char* argv[])
        {
            for(int i = 0; i < argc; ++i)
            {
                push_back(argv[i]);
            }
        }

        // make from binary dump
        // linefeed 0a (linux) vs 0d 0a (win32)
        StringList(void *pData, int dataSize, const char *pCodecName = "Windows-1251");
				
	}; 


	/*
	// не работает, т.к. останавливается на whitespaces в данных
	class StringList : public IStoragableList<QString>
	{
		// ...
	};
	*/
	
	
}
	


