#pragma once 

#include "Utils/ErrorsSubsystem.h"

namespace CallScp
{
    class IExecutionStream;

    ESS_TYPEDEF(BadCmdArgs);

    typedef boost::function<void  (IExecutionStream&)>  ScriptMethod;
    typedef boost::function<ScriptMethod (const QStringList&)> ScriptMethodBinder;

    // --------------------------------------------------------

    template <class Owner>
    static ScriptMethodBinder scriptMethodBind(Owner *owner, void (Owner::*fn)(IExecutionStream&))
    {
        typedef detail::ScriptMethodHelper T;
        ScriptMethod s = boost::bind(fn, owner, _1);
        return boost::bind(&T::BindArg, s, _1);
    }

    // --------------------------------------------------------

    template <class Owner, class Arg0>
    static ScriptMethodBinder scriptMethodBind(Owner *owner, void (Owner::*fn)(IExecutionStream&, Arg0))
    {
        typedef detail::ScriptMethodHelper T;
        boost::function<void (IExecutionStream&, Arg0)> s = 
            boost::bind(fn, owner, _1, _2);

        return boost::bind(&T::BindArg<Arg0>, s, _1);
    }

    // --------------------------------------------------------

    template <class Owner, class Arg0, class Arg1>
    static ScriptMethodBinder scriptMethodBind(Owner *owner, void (Owner::*fn)(IExecutionStream&, Arg0, Arg1))
    {
        typedef detail::ScriptMethodHelper T;
        boost::function<void (IExecutionStream&, Arg0, Arg1)> s = 
            boost::bind(fn, owner, _1, _2, _3);

        return boost::bind(&T::BindArg<Arg0, Arg1>, s, _1);
    }

    // --------------------------------------------------------

    template <class Owner, class Arg0, class Arg1, class Arg2>
    static ScriptMethodBinder scriptMethodBind(Owner *owner, void (Owner::*fn)(IExecutionStream&, Arg0, Arg1, Arg2))
    {
        typedef detail::ScriptMethodHelper T;
        boost::function<void (IExecutionStream&, Arg0, Arg1, Arg2)> s = 
            boost::bind(fn, owner, _1, _2, _3, _4);

        return boost::bind(&T::BindArg<Arg0, Arg1, Arg2>, s, _1);
    }

    // --------------------------------------------------------

    namespace detail
    {

        class ScriptMethodHelper : boost::noncopyable
        {
            typedef ScriptMethodHelper T;
        
        public:
            
            static ScriptMethod BindArg(boost::function<void (IExecutionStream&)> fn, 
                const QStringList &args)
            {
                ArgCountCheck(args, 0);
                return fn;
            }

            template<class Arg0> 
            static ScriptMethod BindArg(
                boost::function<void (IExecutionStream&, Arg0)> fn, 
                const QStringList &args)
            {
                ArgCountCheck(args, 1);
                return boost::bind( fn, _1, ArgTo<Arg0>(args.at(0)) );
            }

            template<> 
            static ScriptMethod BindArg<QStringList>(
                boost::function<void (IExecutionStream&, QStringList)> fn, 
                const QStringList &args)
            {
                return boost::bind( fn, _1, args);
            }

            template<class Arg0, class Arg1> 
            static ScriptMethod BindArg(
                boost::function<void (IExecutionStream&, Arg0, Arg1)> fn, 
                const QStringList &args)
            {
                ArgCountCheck(args, 2);
                return boost::bind( fn, _1, ArgTo<Arg0>(args.at(0)), ArgTo<Arg1>(args.at(1)) );
            }

            template<class Arg0, class Arg1, class Arg2> 
            static ScriptMethod BindArg(
                boost::function<void (IExecutionStream&, Arg0, Arg1, Arg2)> fn, 
                const QStringList &args)
            {
                ArgCountCheck(args, 3);
                return boost::bind( fn, _1, 
                    ArgTo<Arg0>(args.at(0)), 
                    ArgTo<Arg1>(args.at(1)), 
                    ArgTo<Arg1>(args.at(2)) );
            }
        
        private:
        
            template<class T>
            static T ArgTo(const QString &val);

            template<>
            static QString ArgTo<QString>(const QString &val) { return val; }

            template<>
            static int ArgTo<int>(const QString &s) 
            { 
                bool ok;
                int val = s.toInt(&ok);

                if (!ok) 
                {
                    QString msg = QString("Can't convert to int '%1'").arg(s);
                    ESS_THROW_MSG(BadCmdArgs, msg.toStdString());
                }

                return val;
            }

            template<>
            static bool ArgTo<bool>(const QString &s) 
            { 
                if (s == "t" || s == "true")  return true;
                if (s == "f" || s == "false") return false;

                QString msg = QString("Can't convert to bool '%1'").arg(s);
                ESS_THROW_MSG(BadCmdArgs, msg.toStdString());

                return false;
            }

            static void ArgCountCheck(const QStringList &args, int val)
            {
                if (args.size() != val) 
                {
                    QString msg = QString("Bad arg count, wait %1 is %2").arg(val).arg(args.size());
                    ESS_THROW_MSG(BadCmdArgs, msg.toStdString());
                }
            }
        };
    } // detail    
}  // namespace CallScp

