
#include "stdafx.h"

#include "TelnetTests.h"

#include "Utils/Random.h"
#include "iCore/MsgThread.h"

using namespace Platform;


namespace TelnetTests
{

    void Display(const QByteArray &data, 
                 const std::string &prevComment)
    {
        if (!prevComment.empty()) std::cout << prevComment << " :\t";

        for (int i = 0; i < data.size(); ++i)
        {
            std::cout << (int)(byte)data.at(i) << ",";
        }

        std::cout << std::endl;
    }

    // ---------------------------------------------

    void TestOk(const std::string &comment)
    {
        std::cout << "OK!";
        if (!comment.empty()) std::cout << " : " << comment;
        std::cout << std::endl; 
    }

} // namespace TelnetTest
