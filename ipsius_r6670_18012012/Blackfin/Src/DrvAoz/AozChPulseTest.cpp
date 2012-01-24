#include "stdafx.h"
#include "AozChPulseTest.h"

#include "AozChPulse.h"

namespace
{
    using namespace DrvAoz;
    using iCmp::AbLineParams;
    using std::string;

    const int CPulseCapturePeriodMs = 4;

    ESS_TYPEDEF(TestErrorBase);

    // -----------------------------------------------------

    // default line params for tests
    AbLineParams LineParams()
    {
        AbLineParams params;

        // damage
        params.LineDamageDetectionEnable = true;
        params.LineDamageSwitchPeriodMs = 100; 
        params.LineDamageSwitchPercent = 75;   

        // pulse params (ms)
        params.TProtBounceHookDown = 15;
        params.TProtBounceHookUp = 20;
        params.TProtHookUp = 200;
        params.TMaxDialPulce = 160;
        params.TMinSerialDial = 240;
        params.TProtHookDown = 700;

        return params;
    }

    // -----------------------------------------------------

    // output interface
    class IOutput : public Utils::IBasicInterface
    {
    public:
        virtual void Log(const string &s) = 0;
    };

    // null output
    class NullOutput : public IOutput
    {
        void Log(const string &s) { }
    };

    // output to std::cout
    class CoutOutput : public IOutput
    {
        void Log(const string &s) 
        { 
            std::cout << s << std::endl;
        }
    };

    // -----------------------------------------------------

    // pulse & hook programmed generator
    class Generator
    {
        struct Stage
        {
            int DurationMs;
            bool Hook;
            bool Pulse;

            Stage(int durationMs = 0, bool hook = false, bool pulse = false) :
            DurationMs(durationMs), Hook(hook), Pulse(pulse)
            {
                ESS_ASSERT(durationMs > 0);
            }
        };

        int m_timeStepMs;
        int m_currTimeMs;
        int m_currStageIndx;
        int m_currStageStartTimeMs;

        std::vector<Stage> m_stages;

    public:

        Generator(int timeStepMs) : m_timeStepMs(timeStepMs)
        {
            m_currTimeMs = 0;
            m_currStageIndx = -1;
            m_currStageStartTimeMs = 0;
        }

        void Add(int durationMs, bool hook)
        {
            m_stages.push_back( Stage(durationMs, hook, hook) );
        }

        void Add(int durationMs, bool hook, bool pulse)
        {
            m_stages.push_back( Stage(durationMs, hook, pulse) );
        }

        bool PeekNext(bool *pHook, bool *pPulse = 0)
        {
            ESS_ASSERT(pHook != 0);

            if (m_stages.empty()) return false;

            if (m_currStageIndx < 0)
            {
                m_currStageIndx = 0;
                m_currStageStartTimeMs = 0;
            }

            // current stage
            Stage stage = m_stages.at(m_currStageIndx);

            if (m_currTimeMs > m_currStageStartTimeMs + stage.DurationMs)
            {
                // end of stages
                if (m_currStageIndx + 1 == m_stages.size()) return false;

                m_currStageStartTimeMs += stage.DurationMs;
                m_currStageIndx++;
                stage = m_stages.at(m_currStageIndx);
            }

            *pHook = stage.Hook;
            if (pPulse != 0) *pPulse = stage.Pulse;

            // next time
            m_currTimeMs += m_timeStepMs;

            return true;
        }

    };

    // -----------------------------------------------------

    // SL like actions with Generator
    class GeneratorSL
    {
        Generator m_gen;
        bool m_isHookUp;
        int m_pulseTime, m_pulseSilence, m_digitSilence;

        static int DigitToPulseCount(char c)
        {
            ESS_ASSERT(c >= '0' && c <= '9');
            if (c == '0') return 10;
            return c - '1' + 1;
        }

    public:

        GeneratorSL(int timeStepMs, int pulseTime, int pulseSilence, int digitSilence) : m_gen(timeStepMs)
        {
            m_pulseTime = pulseTime;
            m_pulseSilence = pulseSilence;
            m_digitSilence = digitSilence;

            m_isHookUp = false;
        }

        const Generator& Get() const
        {
            return m_gen;
        }

        void HookUp(int delayAfter = 0)
        {
            m_isHookUp = true;
            Delay(delayAfter);
        }

        void HookDown(int delayAfter = 0)
        {
            m_isHookUp = false;
            Delay(delayAfter);
        }

        void Delay(int delay)
        {
            if (delay == 0) return;
            ESS_ASSERT(delay > 0);
            m_gen.Add(delay, !m_isHookUp);
        }

        void Dial(const std::string &number)
        {
            ESS_ASSERT(m_isHookUp);

            for(int i = 0; i < number.size(); ++i)
            {
                int pulseCount = DigitToPulseCount(number.at(i));
                for(int j = 0; j < pulseCount; ++j)
                {
                    HookUp(m_pulseSilence);
                    HookDown(m_pulseTime);
                }
                HookUp(m_digitSilence);                
            }
        }

    };

    // -----------------------------------------------------

    // reference events list for tests
    class ScenarioVerify
    {
        struct Item
        {
            int TimeMinMs; 
            int TimeMaxMs; 
            string Event;

            string ToString() const
            {
                string s;
                s += Event + ", " + Utils::IntToString(TimeMinMs);
                s += "-" + Utils::IntToString(TimeMaxMs);
                return s;
            }
        };

        std::vector<Item> m_items;
        int m_currItemIndex;

    public:

        ScenarioVerify()
        {
            m_currItemIndex = 0;
        }

        void Add(int timeMs, const string &event)
        {
            Add(timeMs, timeMs, event);
        }

        void Add(int timeMinMs, int timeMaxMs, const string &event)
        {
            Item i;
            i.TimeMinMs = timeMinMs;
            i.TimeMaxMs = timeMaxMs;
            i.Event = event;

            m_items.push_back(i);
        }

        // return false on error
        bool VerifyNextEvent(int timeMs, const string &event, string &errMsg)
        {
            ESS_ASSERT(!Completed());

            bool ok = true;
            errMsg.clear();
            Item i = m_items.at(m_currItemIndex);

            string eventIn = "event-in (" + event + ", " + Utils::IntToString(timeMs) + ")";
            string eventRef = "event-ref (" + i.ToString() + ")";

            // verify time
            if (timeMs < i.TimeMinMs || timeMs > i.TimeMaxMs)
            {
                ok = false;
                errMsg = "Time mismatch. " + eventIn + " vs " + eventRef;
            }

            // verify event
            if (ok && (event != i.Event))
            {
                ok = false;
                errMsg = "Event mismatch. " + eventIn + " vs " + eventRef;
            }

            // next item
            ++m_currItemIndex;

            return ok;
        }

        bool Completed() const
        {
            return (m_currItemIndex >= m_items.size());
        }
    };

    // -----------------------------------------------------

    // single step AozChPulse results: messages and events
    struct ProcessResult
    {
        struct Item
        {
            bool IsEvent;
            string Data;

            Item(bool isEvent = false, const string &data = "") : IsEvent(isEvent), Data(data)
            {
            }
        };

        std::vector<Item> Items;
        int TimeMs;

        ProcessResult()
        {
            Clear();
        }

        void Clear(int time = 0)
        {
            Items.clear();
            TimeMs = time;
        }

        void AddTrace(const string &s)
        {
            Items.push_back( Item(false, s) );
        }

        void AddEvent(const string &s)
        {
            Items.push_back( Item(true, s) );
        }

        bool Empty() const
        {
            return Items.empty();
        }

        string ToString(bool eventsOnly = false, bool ignoreEmpty = true) const
        {
            const string CIdent("    ");
            const string CSep("\n");

            string res;

            for(int i = 0; i < Items.size(); ++i)
            {
                bool isEvent = Items.at(i).IsEvent;
                if (eventsOnly && !isEvent) continue;

                res += CIdent;
                if (isEvent) 
                {
                    res += "* " + Items.at(i).Data + " *";
                }
                else 
                {
                    res += "\"" + Items.at(i).Data + "\"";
                }
                res += CSep;
            }

            if (res.empty()) 
            {
                if (ignoreEmpty) return "";
                res = CIdent + "<empty>" + CSep;
            }

            res = Utils::IntToString(TimeMs) + ":" + CSep + res;

            return res;
        }

    };

    // -----------------------------------------------------

    // list of ProcessResult 
    class FullProcessResult
    {
        std::vector<ProcessResult> m_list;

        struct Event
        {
            int TimeMs;
            string Data;

            Event(int timeMs, const string &data) : TimeMs(timeMs), Data(data)
            {
            }
        };

        std::vector<Event> GetAllEvents() const
        {
            std::vector<Event> result;

            for(int i = 0; i < m_list.size(); ++i)
            {
                const ProcessResult &curr = m_list.at(i);
                for(int j = 0; j < curr.Items.size(); ++j)
                {
                    if (curr.Items.at(j).IsEvent)
                    {
                        result.push_back( Event(curr.TimeMs, curr.Items.at(j).Data) );
                    }
                }
            }

            return result;
        }

    public:

        FullProcessResult()
        {
        }

        void Add(const ProcessResult &result)
        {
            if (result.Empty()) return;
            m_list.push_back(result);
        }

        string DumpTrace(bool eventsOnly) const
        {
            string res;

            for(int i = 0; i < m_list.size(); ++i)
            {
                string s = m_list.at(i).ToString(eventsOnly);
                if (s.empty()) continue;
                res += s + "\n";
            }

            return res;
        }

        string DumpEvents(const string &specPrefix = "")
        {
            std::ostringstream oss;
            std::vector<Event> events = GetAllEvents();

            for(int i = 0; i < events.size(); ++i)
            {
                oss << specPrefix << "(" << events.at(i).TimeMs 
                    << ", " << "\"" << events.at(i).Data << "\"" << ")" << "\n";
            }

            return oss.str();
        }

        bool Verify(ScenarioVerify &sv, string &errMsg)
        {
            errMsg.clear();

            std::vector<Event> events = GetAllEvents();

            for(int i = 0; i < events.size(); ++i)
            {
                const Event &e = events.at(i);

                if (sv.Completed())
                {
                    errMsg = "Out of scenario!";
                    return false;
                }

                if (!sv.VerifyNextEvent(e.TimeMs, e.Data, errMsg)) return false;
            }

            if (!sv.Completed())
            {
                errMsg = "Extra verify data";
                return false;
            }

            return true;
        }

    };

    // -----------------------------------------------------

    // AozChPulse test shell: Generator -> FullProcessResult
    class AozPulseHolder : public IAozChannel
    {
        int m_pulseCapturePeriodMs;
        AozChPulse m_pulse;
        ProcessResult m_result;
        int m_currTimeMs;

        static string BoolToStr(bool val)
        {
            return val ? "true" : "false";
        }

        static string IntToStr(int val)
        {
            return Utils::IntToString(val);
        }

        ProcessResult Process(bool hook, bool pulse)
        {
            m_result.Clear(m_currTimeMs);
            m_currTimeMs += m_pulseCapturePeriodMs;

            m_pulse.Process(hook, pulse);

            return m_result;
        }

    // IAozChannel impl
    private:

        void OnLineConditionChanged(bool damaged)
        {
            m_result.AddEvent("damaged-" + BoolToStr(damaged));
        }

        void OnLineReset()
        {
            m_result.AddEvent("line-reset");
        }

        void OnDigit(int pulseCount, int digitCount)
        {
            m_result.AddEvent("pulse-count-" + IntToStr(pulseCount));
        }

        void OnDigitBegin(int digitCount)
        {
            m_result.AddEvent("on-digit-begin");
        }

        void OnHook(bool up)
        {
            m_result.AddEvent("hook-up-" + BoolToStr(up));
        }

        void PulseTrace(const string &s)
        {   
            m_result.AddTrace(s);
        }

    public:

        AozPulseHolder(AbLineParams &params, int pulseCapturePeriodMs) :
          m_pulseCapturePeriodMs(pulseCapturePeriodMs),
          m_pulse(*this, pulseCapturePeriodMs, params, true),
          m_currTimeMs(0)
        {
        }

        FullProcessResult Run(const Generator &genIn)
        {
            Generator gen = genIn;  // un-const
            FullProcessResult result;

            while(true)
            {
                bool hook, pulse;
                if (!gen.PeekNext(&hook, &pulse)) break; // EOF

                ProcessResult res = Process(hook, pulse);
                result.Add(res);
            }

            return result;
        }


    };

    // -----------------------------------------------------

    void DamageDetectionTest(IOutput &out)
    {
        AbLineParams lineParams = LineParams();
        AozPulseHolder pulseObj(lineParams, CPulseCapturePeriodMs);

        // make generator; bool -> line breaked; false for hook-up
        Generator gen(CPulseCapturePeriodMs);
        gen.Add(1000, true, true);    // трубка положена
        gen.Add(1000, false, false);  // трубка снята
        gen.Add(1000, true, false);   // авария
        gen.Add(1000, true, true);    // трубка положена 

        // verify scenario
        ScenarioVerify sv;
        sv.Add(1248, "hook-up-true");
        sv.Add(2096, "damaged-true");
        sv.Add(3096, "damaged-false");
        sv.Add(3796, "hook-up-false");

        // run
        FullProcessResult result = pulseObj.Run(gen);

        // print
        out.Log( result.DumpTrace(true) );

        // do verify
        string err;
        if (!result.Verify(sv, err))
        {
            ESS_THROW_MSG(TestErrorBase, err);
        }
    }

    // -----------------------------------------------------

    /*

    TProtBounceHookDown     <   TDialPulce    <= TMaxDialPulce         default 60mc
    TProtBounceHookUp       <   TDialPause    < TMinSerialDial         default 40mc 
    TMinSerialDial          <=  TSerialDial   < 1 СЕК ДЛЯ ТЕСТОВ       default 500mc
    TMaxDialPulce           <   TFlash        < TProtHookDown          default 300 мс

    */

    void PulseDetectionTest(IOutput &out)
    {
        AbLineParams params = LineParams();
        AozPulseHolder pulseObj(params, CPulseCapturePeriodMs);

        // make generator
        int pulseTime = params.TMaxDialPulce - 5; 
        int pulseSilence = params.TMinSerialDial - 5; 
        int digitSilence = 500;

        GeneratorSL gen(CPulseCapturePeriodMs, pulseTime, pulseSilence, digitSilence);
        gen.HookDown(1000);
        gen.HookUp(1000);
        gen.Dial("471206");
        gen.Delay(1000);
        gen.HookDown(1000);

        // verify scenario
        ScenarioVerify sv;
        sv.Add(1248, "hook-up-true");
        sv.Add(3804, "pulse-count-4");
        sv.Add(7032, "pulse-count-7");
        sv.Add(7924, "pulse-count-1");
        sv.Add(9204, "pulse-count-2");
        sv.Add(13604, "pulse-count-10");
        sv.Add(16444, "pulse-count-6");
        sv.Add(18404, "hook-up-false");

        // run
        FullProcessResult result = pulseObj.Run(gen.Get());

        // print
        out.Log( result.DumpEvents("sv.Add") );

        // do verify        
        string err;
        if (!result.Verify(sv, err))
        {
            ESS_THROW_MSG(TestErrorBase, err);
        } 

    }

}

namespace DrvAoz
{

    void RunAozChPulseTest(bool silenceMode)
    {
        boost::scoped_ptr<IOutput> out;
        if (silenceMode) out.reset( new NullOutput() );
                    else out.reset( new CoutOutput() );

        // DamageDetectionTest(*out);
        PulseDetectionTest(*out);
    }
    
}  // namespace DrvAoz
