
#include "stdafx.h"

#include "TypeFactoryTest.h"
#include "Utils/TypeFactory.h"


namespace
{
    const std::string CFruitName = "Fruit";
    const std::string CDeviceName = "Device";
    
    const std::string CFruitParam = "apple";
    const std::string CDeviceParam = "mp3-player";

    // ---------------------------

    // Base interface for all types
    class IBoxContent 
        : public Utils::IBasicInterface
    {
    public:
        virtual std::string getParam() const = 0;
    };

    // ---------------------------

    class ClassFruit
        : public IBoxContent
    {
        std::string m_param;

    public:
        ClassFruit(const std::string &param) : m_param(param)
        {}

    // IBaseClassStr impl
    public:
        std::string getParam() const { return m_param; }
    };

    // ---------------------------

    class ClassDevice 
        : public IBoxContent
    {
        std::string m_param;

    public:
        ClassDevice(const std::string &param) : m_param(param)
        {}

    // IBaseClassStr impl
    public:
        std::string getParam() const { return m_param; }
    };

    // ---------------------------
    
    typedef Utils::TypeFactory<IBoxContent, std::string, std::string> BaseBoxContentFactory;

    class BoxContentFactory 
        : public BaseBoxContentFactory
    {
    public:
        BoxContentFactory()
        {
            BaseBoxContentFactory::Register<ClassFruit>(CFruitName);
            BaseBoxContentFactory::Register<ClassDevice>(CDeviceName);
        }
    };

    // ---------------------------

    class MagicBox
    {
        BoxContentFactory m_factory;

    public:
        MagicBox()
        {}

        IBoxContent* get(const std::string &whatName, const std::string &whatParams)
        {
            return m_factory.Create(whatName, whatParams);
        }
    };

} // namespace

// ---------------------------------

namespace UtilsTests
{
    void TypeFactoryTest()
    {
        MagicBox magicBox;
    
        // check cretion / check values 
        boost::scoped_ptr<IBoxContent> fruit( magicBox.get(CFruitName, CFruitParam) );
        TUT_ASSERT(fruit->getParam() == CFruitParam);
    
        boost::scoped_ptr<IBoxContent> device( magicBox.get(CDeviceName, CDeviceParam) );
        TUT_ASSERT(device->getParam() == CDeviceParam);
    
        // check type-not-found exception
        bool wasException = false;
        try
        {
            boost::scoped_ptr<IBoxContent> beyondMagic(magicBox.get(std::string("NonBoxContent"),
                                                                    std::string()));
        } 
        catch (BaseBoxContentFactory::UnknownTypeName &e)
        {
            wasException = true;
        }
        TUT_ASSERT(wasException && "No exception about unknown type");
    }

} // namespace UtilsTests
