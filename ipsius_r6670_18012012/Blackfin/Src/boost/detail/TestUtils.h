
#ifndef __BOOST_DETAIL_TESTUTILS__
#define __BOOST_DETAIL_TESTUTILS__

namespace boost
{
    namespace detail
    {
        // Using for out test info in CDebugMode
        class TestDestructor
        {
            const char *m_s;
            bool m_show;
        public:
            TestDestructor(const char *pClassName, bool show)
            : m_s(pClassName), m_show(show)
            {}

            virtual ~TestDestructor()
            {
                if (m_show) std::cout << "call ~" << m_s << "()\n";
            }
        };

        std::ostream& DebugStream();

    } // namespace detail

} // namespace boost

#endif
