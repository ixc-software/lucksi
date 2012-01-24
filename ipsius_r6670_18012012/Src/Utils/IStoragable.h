
#ifndef __ISTORAGABLE__

#define __ISTORAGABLE__

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"

namespace Utils
{

	class IStoragable : IBasicInterface
	{
	protected:

		virtual void Save(QTextStream &stream) const = 0;
		virtual void Load(QTextStream &stream) = 0;


	public:

		ESS_TYPEDEF(IoError);

		static const char* DefaultCodec()
		{
			return "Windows-1251";
		}

		void SaveToFile(const QString &fileName, const char *pCodec = DefaultCodec()) const
		{
			QFile f(fileName);

			if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) ESS_THROW(IoError);
			if (!f.resize(0)) ESS_THROW(IoError);
			
			Save(f, pCodec);
		}

		void SaveToFile(const char *pFileName, const char *pCodec = DefaultCodec()) const
		{
			SaveToFile(QString(pFileName), pCodec);
		}

		void LoadFromFile(const QString &fileName, const char *pCodec = DefaultCodec())
		{
			QFile f(fileName);

            if (!QFileInfo(fileName).isFile()) ESS_THROW(IoError);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) ESS_THROW(IoError);

			Load(f, pCodec);
		}

		void LoadFromFile(const char *pFileName, const char *pCodec = DefaultCodec())
		{
			LoadFromFile(QString(pFileName), pCodec);
		}


		void Save(QIODevice &device, const char *pCodec = DefaultCodec()) const
		{
			QTextStream ts(&device);
			ts.setCodec(pCodec);
			Save(ts);
		}

		void Load(QIODevice &device, const char *pCodec = DefaultCodec())
		{
			QTextStream ts(&device);
			ts.setCodec(pCodec);
			Load(ts);
		}

        
		/*

				TODO: Add binary support - ?
				Qt support write to QDataStream (binary) as <<, >> for containers 

				TODO: Save to std::istream, load from std::ostream - ?

				TODO: Global template functions for load/save containers 
				vs IStoragableContainer<>

		*/

	};

	// -------------------------------------------

	/*
		ƒобавление интерфейса IStoragable дл€ QList<T>
		ƒл€ T& и T* не работает, нужна специализаци€
	*/
	
	template<class T>
	class IStoragableList : public QList<T>, public IStoragable
	{
	protected:

        typedef typename QList<T>::const_iterator ConstIter;

		void Save(QTextStream &stream) const  // override
		{
			ConstIter i = this.begin();

			while(i != this.end())
			{
				stream << (*i) << endl;
				++i;
			}

		}

		void Load(QTextStream &stream)  // override
		{
			this.clear();

			while(!stream.atEnd())
			{
				T t;
				stream >> t;
				push_back(t);
			}
		}

	};


	// -------------------------------------------

	
	/* 
	   ƒл€ контейнеров C<T>, возможность описать специализацию T* -- можно
	   было бы победить ручную установку параметра elementsAsPtr
	*/
	/*
	template<class T, template<class> class C>   
	class IStoragableContainer
	{

	protected:

		virtual void Save(QTextStream &stream) const = 0;
		virtual void Load(QTextStream &stream) = 0;


	};  */


}  // namespace


#endif

