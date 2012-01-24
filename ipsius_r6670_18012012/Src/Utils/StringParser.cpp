
#include "stdafx.h"

#include "StringParser.h"

namespace 
{
    using std::string;

    // for string parser
    void AddData(const string& source, 
                 Utils::ManagedList<string>& dest,
                 bool removeEmpty)
    {
        if ((source.empty()) && (removeEmpty)) return;

        dest.Add(new string(source));
    }
}

// -----------------------------------

namespace Utils
{
    void StringParser(const std::string& inStr, 
                      const std::string& inSeparator, 
                      Utils::ManagedList<std::string>& outData,
                      bool removeEmpty)
    {
        outData.Clear();

        //if no separator, return full input string
        if (inSeparator.empty()) 
        {
            AddData(inStr, outData, removeEmpty);
            return;
        }

        // if separtor exists, parse string 
        size_t prev = 0;
        while (true)
        {
            size_t next = inStr.find(inSeparator, prev);// inStr.find_first_of(inSeparator, prev);

            // copy symbols after last separator || 
            // copy all string if separator's not found
            if (next == string::npos)  
            {
                AddData(inStr.substr(prev), outData, removeEmpty);
                break;
            }

            AddData(inStr.substr(prev, (next - prev)), outData, removeEmpty);

            prev = next + inSeparator.size();
        }
    }

} // namespace Utils
