#ifndef __IHWFIRMWARE__
#define __IHWFIRMWARE__

#include "Utils/IBasicInterface.h"
#include "IHwFirmwareScript.h"

/*перенести в клиента -- ?*/

namespace BfBootCore
{

    class IHwFirmwareImage : public Utils::IBasicInterface
    {
    public:

        virtual QByteArray Data() = 0;  // can throw 
        virtual QByteArray Data() const = 0;  // can throw 
        virtual std::string Info() const = 0;
    };    

    // --------------------------------------------------------

    class IHwFirmware : public Utils::IBasicInterface
    {
    public:
        virtual QString Info() const = 0;
        virtual int Release() const = 0;

        virtual int ImageCount() const = 0;
        virtual IHwFirmwareImage& Image(int index) const = 0; // const -- ?

        virtual int ScriptCount() const = 0;
        virtual const IHwFirmwareScript& Script(int index) const = 0;
    };

}  // namespace BfBootCore


#endif
