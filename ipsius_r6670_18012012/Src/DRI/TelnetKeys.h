#ifndef __TELNETKEYS__
#define __TELNETKEYS__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "Utils/QtEnumResolver.h"

namespace TelnetKey
{

/*    
    enter = 0d 0a
    up = 1b 5b 41
    dn = 1b 5b 42
    left = 1b 5b 44
    right = 1b 5b 43
    bs = 8
    tab = 9
    ins = 1b 5b 32 7e
    del = 7f
    home = 1b 5b 31 7e
    end = 1b 5b 34 7e 
    pg up = 1b 5b 35 7e
    pg down = 1b 5b 36 7e
    esc =  1b 

    f1-f4 =
        1b 4f 50 
        1b 4f 51 
        1b 4f 52
        1b 4f 53 

    f5-f12 = 
        1b 5b 31 35 7e  | ?[15~
        1b 5b 31 37 7e  | ?[17~
        1b 5b 31 38 7e  | ?[18~
        1b 5b 31 39 7e  | ?[19~
        1b 5b 32 30 7e  | ?[20~
        1b 5b 32 31 7e  | ?[21~
        1b 5b 32 33 7e  | ?[23~
        1b 5b 32 34 7e  | ?[24~


    LINUX:

        enter       -- 0d 00 
        backspace   -- 7f
        del         -- 1b 5b 33 7e
        home        -- 1b 5b 48
        end         -- 1b 5b 46


*/


    // ----------------------------------------------------

    class Map : public QObject
    {
        Q_OBJECT
        Q_ENUMS(KeyID)

    public:

        enum KeyID
        {
            None = -1,

            BS,
            Tab,
            Escape,
            Del,

            Enter,

            Up,
            Down,
            Left,
            Right,

            Ins,
            Home,
            End,
            PageUp,
            PageDown,

            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
        };

        struct Pair
        {
            KeyID Key; 
            QString Scan;
        };

        Map(bool linuxMode)
        {
            Add(Tab,            "\x09");
            Add(Escape,         "\x1b");

            Add(Up,             "\x1b\x5b\x41");
            Add(Down,           "\x1b\x5b\x42");
            Add(Left,           "\x1b\x5b\x44");
            Add(Right,          "\x1b\x5b\x43");

            Add(Ins,            "\x1b\x5b\x32\x7e");
            Add(PageUp,         "\x1b\x5b\x35\x7e");
            Add(PageDown,       "\x1b\x5b\x36\x7e");

            Add(F1,             "\x1b\x4f\x50");
            Add(F2,             "\x1b\x4f\x51");
            Add(F3,             "\x1b\x4f\x52");
            Add(F4,             "\x1b\x4f\x53");
            Add(F5,             "\x1b\x5b\x31\x35\x7e");
            Add(F6,             "\x1b\x5b\x31\x37\x7e");
            Add(F7,             "\x1b\x5b\x31\x38\x7e");
            Add(F8,             "\x1b\x5b\x31\x39\x7e");
            Add(F9,             "\x1b\x5b\x32\x30\x7e");
            Add(F10,            "\x1b\x5b\x32\x31\x7e");
            Add(F11,            "\x1b\x5b\x32\x33\x7e");
            Add(F12,            "\x1b\x5b\x32\x34\x7e");   

            if (linuxMode)
            {
                Add(Enter,          LinuxEnter());
                Add(BS,             "\x7f");
                Add(Del,            "\x1b\x5b\x33\x7e");
                Add(Home,           "\x1b\x5b\x48");
                Add(End,            "\x1b\x5b\x46");
            }
            else
            {
                Add(Enter,          "\x0d\x0a");
                Add(BS,             "\x08");
                Add(Del,            "\x7f");
                Add(Home,           "\x1b\x5b\x31\x7e");
                Add(End,            "\x1b\x5b\x34\x7e");
            }

            Sort();
        }

        QString Peek(const QString &s, KeyID &result) const
        {
            result = None;

            MapType::const_iterator i = m_map.begin();

            while(i != m_map.end())
            {
                QString val = (*i).Scan;

                if (s.startsWith(val))
                {
                    result = (*i).Key;
                    return s.mid(val.size());
                }

                ++i;
            }

            return s;
        }

        // used for left/right/enter only
        QString Resolve(KeyID id) const
        {
            if (id == Enter) return "\x0d\x0a";

            const Pair *p = FindByID(id);
            if (p == 0) return "";

            return p->Scan;
        }

        QString KeyAsString(KeyID id) const
        {
            return m_enumResolver.Resolve(id).c_str();
        }


    private:

        // can't encode as char*, it's contains \0
        static QString LinuxEnter() 
        {
            QString s;
            s.append(QChar(0x0d));
            s.append(QChar(0));

            ESS_ASSERT(s.size() == 2);

            return s;
        }

        static bool SortCmp(const Pair &first, const Pair &second)
        {
            return (first.Scan.size() > second.Scan.size());
        }

        void Sort()
        {
            // desc sort by Pair::Scan.size()
            std::sort(m_map.begin(), m_map.end(), &SortCmp);
        }

        void Add(KeyID id, const QString &scan)
        {
            ESS_ASSERT( FindByID(id) == 0 );

            Pair p;
            p.Key = id;
            p.Scan = scan;

            m_map.push_back(p);
        }

        const Pair* FindByID(KeyID id) const
        {
            for(size_t i = 0; i < m_map.size(); ++i)
            {
                if (m_map.at(i).Key == id) return &m_map.at(i);
            }

            return 0;
        }

        typedef std::vector<Pair> MapType;

        MapType m_map;
        Utils::QtEnumResolver<Map, Map::KeyID> m_enumResolver;
    };
    
}  // namespace

#endif
