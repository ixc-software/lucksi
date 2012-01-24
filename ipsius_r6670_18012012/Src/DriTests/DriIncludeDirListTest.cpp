

#include "stdafx.h"
#include "DriIncludeDirListTest.h"
#include "DRI/DriIncludeDirList.h"
#include "Utils/ExeName.h"
#include "Utils/QtHelpers.h"

namespace
{
    using namespace DRI;
    
    /*
    // pathList - list of pathes separeted by ';'
    // default: .exe path
    // do not check if path is valid(!)
    DriIncludeDirList(const QString &pathList = "");
    bool Verify(QStringList *pInvalidPathes = 0) const;
    bool operator==(const DriIncludeDirList &other) const; 
    bool operator!=(const DriIncludeDirList &other) const { return !operator==(other); }
    QString ToString() const;
    */
    void TestCreation(bool silentMode)
    {
        // check parsing
        {
            DriIncludeDirList dl("../path1; path2/path21; ./");
            TUT_ASSERT(dl.Size() == 3); // "./" already in list
            TUT_ASSERT(dl.ToString().contains("path1"));
            TUT_ASSERT(dl.ToString().contains("path2/path21"));
        }
        {
            DriIncludeDirList dl("../; ../../; ");
            TUT_ASSERT(dl.Size() == 3);
        }
        // valid
        {
            DriIncludeDirList dl("../; ../../");
            TUT_ASSERT(dl.Verify());
        }
        // invlaid
        {
            DriIncludeDirList dl("../; ../../;_invalid_path_");
            TUT_ASSERT(!dl.Verify());
            
            QStringList invalidList;
            TUT_ASSERT(!dl.Verify(&invalidList));
            TUT_ASSERT(invalidList.size() == 1);
            TUT_ASSERT(invalidList.at(0).contains("_invalid_path_"));
        }
        // compare
        {
            DriIncludeDirList dl1;
            DriIncludeDirList dl2;
            TUT_ASSERT(dl1 == dl2);
        }
        {
            QString path("../path1; path2/; path3");
            DriIncludeDirList dl1(path);
            DriIncludeDirList dl2(path);
            TUT_ASSERT(dl1 == dl2);
        }
        {
            QString path("../; ../../");
            DriIncludeDirList dl1(path);
            DriIncludeDirList dl2("../");
            dl2.Add("../../");
            TUT_ASSERT(dl1 == dl2);
        }
        // different order --> != (?)
        {
            QString path("../; ../../");
            DriIncludeDirList dl1(path);
            DriIncludeDirList dl2;
            dl2.Add("../../");
            dl2.Add("../");
            TUT_ASSERT(dl1 != dl2);
        }
        {
            DriIncludeDirList dl1;
            DriIncludeDirList dl2("/path");
            TUT_ASSERT(dl1 != dl2);
        }
    }

    // --------------------------------------------------------

    /*
    ESS_TYPEDEF(InvalidPath);
    // path is absolute or .exe-relative (it should end with '/')
    // do not check if path is valid(!)
    void Add(const QString &pathList); // can throw
    */
    void TestAdd(bool silentMode)
    {
        // add valid
        {
            DriIncludeDirList dl;
            TUT_ASSERT(dl.Size() == 1);
            dl.Add("../../");
            dl.Add("../");
            TUT_ASSERT(dl.Size() == 3);
        }
        // add invalid -> no exception
        {
            DriIncludeDirList dl;
            dl.Add("_invalid_path_");
            TUT_ASSERT(!dl.Verify());
        }
    }

    // --------------------------------------------------------

    /*
    ESS_TYPEDEF(InvalidPath);
    // path is absolute or relative to the pathes in list 
    QString Find(const QString &file, bool throwException = true) const;
    */
    void TestFind(bool silentMode)
    {
        const bool withException = true;
        
        // ok found
        {
            DriIncludeDirList dl;
            QString file(Utils::ExeName::GetExeName().c_str());
            TUT_ASSERT(dl.Find(file, withException).contains(file));
        }
        // !found 
        {
            DriIncludeDirList dl;
            TUT_ASSERT(dl.Find("_unknown_.file", !withException) == QString());
            bool wasException = false;
            try
            {
                dl.Find("_unknown_.file", withException);
            }
            catch (DriIncludeDirList::InvalidPath &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        // dublicate found: 
        /*{
            DriIncludeDirList dl("../isc; ../_isc");
            
            bool wasException = false;
            try
            {
                dl.Find("TestMain.isc", withException);
            }
            catch (DriIncludeDirList::InvalidPath &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }*/
    }
    
} // namespace

// --------------------------------------------------------

namespace DriTests
{
    void DriIncludeDirListTest(bool silentMode)
    {
        TestCreation(silentMode);
        
        TestAdd(silentMode);
        TestFind(silentMode);

        if (!silentMode) std::cout << "DriIncludeDirList test: OK" << std::endl;
    }
    
} // namespace DriTests
