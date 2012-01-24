#ifndef __ULVGUISTARTCONFIG__
#define __ULVGUISTARTCONFIG__

#include "Utils/MetaConfig.h"


namespace UlvGui
{
    struct StartConfigInfo
    {
        bool ShowFilterBox;
        bool ShowDebugBox;
        bool ShowInfoBox;
        int TimestampModeIndex;
        int FormWidth;
        int FormHeight;
        int FormTop;
        int FormLeft;

        // bool EnableUseRcvrTimestamp;
        // bool EnableUseSrcInLog;

        StartConfigInfo() :
            ShowFilterBox(false), ShowDebugBox(false), ShowInfoBox(false)/*,
            EnableUseRcvrTimestamp(false), EnableUseSrcInLog(false)*/,
            TimestampModeIndex(0), FormWidth(780), FormHeight(400), FormTop(0), FormLeft(0)
        {
        }

    };

    // -------------------------------------------------------------------

    class StartConfig
    {
        Utils::MetaConfig m_config;
        QString m_fileName;

        StartConfigInfo m_info;

        void RegisterMeta();
        void LoadConfig();
        void WriteConfigToFile(const QByteArray &config);

    public:
        ESS_TYPEDEF(StartConfigError);

    public:
        // if file doesn't exists or config in file is invalid,
        // it'll create default config file with given name
        StartConfig(const QString &fileName); // can throw
        // save params to config file
        ~StartConfig();

        bool ShowFilterBox() const { return m_info.ShowFilterBox; }
        bool ShowDebugBox() const { return m_info.ShowDebugBox; }
        bool ShowInfoBox() const { return m_info.ShowInfoBox; }
        // bool EnableUseRcvrTimestamp() const { return m_info.EnableUseRcvrTimestamp; }
        // bool EnableUseSrcInLog() const { return m_info.EnableUseSrcInLog; }
        int TimestampModeIndex() const { return m_info.TimestampModeIndex; }
        int FormWidth() const { return m_info.FormWidth; }
        int FormHeight() const { return m_info.FormHeight; }
        int FormTop() const { return m_info.FormTop; }
        int FormLeft() const { return m_info.FormLeft; }

        void ShowFilterBox(bool val) { m_info.ShowFilterBox = val; }
        void ShowDebugBox(bool val) { m_info.ShowDebugBox = val; }
        void ShowInfoBox(bool val) { m_info.ShowInfoBox = val; }
        // void EnableUseRcvrTimestamp(bool val) { m_info.EnableUseRcvrTimestamp = val; }
        // void EnableUseSrcInLog(bool val) { m_info.EnableUseSrcInLog = val; }
        void TimestampModeIndex(int val);
        void FormWidth(int val) { m_info.FormWidth = val; }
        void FormHeight(int val) { m_info.FormHeight = val; }
        void FormTop(int val) { m_info.FormTop = val; }
        void FormLeft(int val) { m_info.FormLeft = val; }

    };

} // namespace UlvGui


#endif
