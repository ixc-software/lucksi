#ifndef ISDNROLE_H
#define ISDNROLE_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace ISDN
{
    class IsdnRole : public Utils::IBasicInterface
    {
    public:
        
        virtual bool IsLeft() const = 0;

        virtual std::string Name() const = 0;
        
        bool IsRight()  const { return !IsLeft(); }

        virtual bool IsUserSide() const = 0;

        bool IsNetworkSide() const { return !IsUserSide(); }
        
    };

    //-----------------

    class LeftDrvRole: public IsdnRole
    {
    public:

        bool IsLeft() const {return true;} // override

        bool IsUserSide() const {return true;} // override

        std::string Name() const {return "LeftDrv";} // override
    };

    //-----------------

    class RightDrvRole: public IsdnRole
    {
    public:

        bool IsLeft()  const {return false;} // override

        bool IsUserSide() const {return false;} // override

        std::string Name() const {return "RightDrv";} // override
    };

    class LeftL2Role: public IsdnRole
    {
    public:

        bool IsLeft() const {return true;} // override

        bool IsUserSide() const {return true;} // override

        std::string Name() const {return "LeftL2";} // override
    };

    class RightL2Role: public IsdnRole
    {
    public:

        bool IsLeft() const {return false;} // override

        bool IsUserSide() const {return false;} // override

        std::string Name() const {return "RightL2";} // override
    };

  

    //class LeftL3UserRole: public IsdnRole
    //{
    //public:

    //    bool IsLeft() const {return true;} // override

    //    bool IsUserSide() const {return true;} // override

    //    std::string Name() const {return IsUserSide() ? "Left(User)L3" : "Left(Net)L3";} //override
    //};

    //class RightL3NetRole: public IsdnRole
    //{
    //public:

    //    bool IsLeft() const {return false;} // override

    //    bool IsUserSide() const {return false;} // override

    //    std::string Name() const {return IsUserSide() ? "Right(User)L3" : "Right(Net)L3";} //override
    //};

} // ISDN

#endif 
