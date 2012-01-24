#include "stdafx.h"
#include "BfConfigDiskIO.h"
#include "Utils/ErrorsSubsystem.h"


using std::fstream;
using std::string;


namespace BfConfig
{

    BfConfigDiskIO::BfConfigDiskIO()
    {
    }


    //-------------------------------------------------------------------------


    BfConfigDiskIO::~BfConfigDiskIO()
    {
    }


    //-------------------------------------------------------------------------


    bool BfConfigDiskIO::ReadConfig(const string &fileName, StringList &stringsVector)
    {
        fstream fileStream;
        string  temp;

        fileStream.open( fileName.c_str(), fstream::in );
        ESS_ASSERT(fileStream.is_open() == true);

        // todo: обработка ошибок
        while (getline(fileStream, temp))
        {
            if (!temp.empty())
            {
                stringsVector.push_back(temp);
            }
        }

        fileStream.close();

        return true;
    }


    //-------------------------------------------------------------------------


    void BfConfigDiskIO::StoreConfig(const string &fileName, const StringList &stringsVector)
    {
        fstream fileStream;
        string  temp;

        fileStream.open( fileName.c_str(), fstream::out );
        ESS_ASSERT(fileStream.is_open() == true);

        StringList::const_iterator it = stringsVector.begin();

        for (; it != stringsVector.end(); ++it)
        {
            fileStream << (*it) << std::endl;
        }

        fileStream.close();
    }

} // namespace BfConfig
