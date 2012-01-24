#ifndef __BFCONFIGDISKIO__
#define __BFCONFIGDISKIO__


typedef std::vector<std::string> StringList;


namespace BfConfig
{

    // Класс-загрузчик текстового ini-файла в строку/вектор строк
    class BfConfigDiskIO
    {
        BfConfigDiskIO();
        ~BfConfigDiskIO();

    public:
        static bool ReadConfig(const std::string &fileName, StringList &stringsVector);
        static bool ReadConfig(const std::string &fileName, std::string &stringContainer);

        static void StoreConfig(const std::string &fileName, const StringList &stringsVector);
    };

} // namespace BfConfig


#endif
