#ifndef __SPECIALIZATIONTEST__
#define __SPECIALIZATIONTEST__

namespace SpTest
{
	
    template<class T>
    class Base
    {
    public:

        void Write(T &value);
    };

    template<>
    class Base<int>
    {
    public:

        void Write(int &value)
        {
            value = 0;
        }

    };

    template<>
    class Base<std::string>
    {
    public:

        void Write(std::string &value)
        {
            value.clear();
        }

    };

    class Instance
    {
    public:

        static void Do()
        {
            {
                Base<int> i;
                int val = 15;
                i.Write(val);
            }

            {
                Base<std::string> i;
                std::string val = "15";
                i.Write(val);
            }
        }
    };

	
}  // namespace SpTest

#endif

