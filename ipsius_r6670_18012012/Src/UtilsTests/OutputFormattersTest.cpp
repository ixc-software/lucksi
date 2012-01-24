
#include "stdafx.h"

#include "OutputFormattersTest.h"
#include "Utils/OutputFormatters.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
    using namespace Utils;

    void TestAdd(bool silentMode)
    {
        TableOutput t("|", '*');

        // add empty
        t.AddRow("");

        // add one by one
        t.AddRow("11");
        t.AddRow("21", "22");

        t.AddRow("31", "32", "33");
        t.AddRow("41", "42", "43", "44");
        t.AddRow("51", "52", "53", "54", "55");

        // add empty between not empty
        t.AddRow("", "62", "63", "64", "", "66");

        // add list
        QStringList row;
        row << "71" << "72" << "73" << "74" << "75" << "76" << "77";
        t.AddRow(row);

        QStringList toCheck;
        toCheck << "**|**|**|**|**|**|**"
                << "11|**|**|**|**|**|**"
                << "21|22|**|**|**|**|**"
                << "31|32|33|**|**|**|**"
                << "41|42|43|44|**|**|**"
                << "51|52|53|54|55|**|**"
                << "**|62|63|64|**|66|**"
                << "71|72|73|74|75|76|77";

        if (!silentMode)
            std::cout << t.Get().join("\n").toStdString() << std::endl << std::endl;

        TUT_ASSERT(t.Get() == toCheck);
    }

    // ------------------------------------------------------

    void TestGetNonFixed(bool silentMode)
    {
        QStringList toCheck;
        toCheck << "0|str*********|"
                << "1|str123******|"
                << "2|str123456***|"
                << "3|str123456789|";

        // column fixed size = -1 --> infinite line width
        {
            TableOutput t("|", '*');
            t.AddRow("0", "str", "");
            t.AddRow("1", "str123", "");
            t.AddRow("2", "str123456", "");
            t.AddRow("3", "str123456789", "");

            if (!silentMode)
                std::cout<< t.Get().join("\n").toStdString() << std::endl << std::endl;


            TUT_ASSERT(t.Get() == toCheck);
        }        
    }

    // ------------------------------------------------------

    void SetWidthes(bool silentMode)
    {        
        TableOutput t("|", '*');
        t.AddRow("123", "123756", "123456", "123456789");
        t.AddRow("1", "", "" ,"", "", "0");

        // specify 4 ranges for 6 columns, 5th and 6th will have max width
        t.SetColumnWidthes((QVector<int>() << -1 << 8 << 5 << 3));

        QStringList toCheck;
        toCheck << "123|123756**|12345|123||*"
                << "***|********|6****|456||*"
                << "***|********|*****|789||*"
                << "1**|********|*****|***||0";

        if (!silentMode)
            std::cout << t.Get().join("\n").toStdString() << std::endl << std::endl;

        TUT_ASSERT(t.Get() == toCheck);
    }


} // namespace

// ------------------------------------------------------

namespace UtilsTests
{
    void TableOutputTest(bool silentMode)
    {
        TestAdd(silentMode);
        TestGetNonFixed(silentMode);
        SetWidthes(silentMode);
    }

} // namespace UtilsTests
