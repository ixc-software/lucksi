
#ifndef __PROGRAMOPTIONS__
#define __PROGRAMOPTIONS__

// ProgramOption.h

#include "Utils/IBasicInterface.h"
#include "Utils/StringList.h"
#include "Utils/ManagedList.h"

namespace Utils
{
    class ProgramOptionsExceptions
    {
    public:
        ESS_TYPEDEF(Base);
        ESS_TYPEDEF_FULL(ProcessError, Base);
        ESS_TYPEDEF_FULL(DublicateName, Base);
        
        static void Error(const std::string &desc, const std::string &src = "");
        static void DublicateNameError(const std::string &name);
    };
    
    // --------------------------------------------
    
    // Base option handler
    class OptionBase
    {
        typedef ProgramOptionsExceptions PE;
        
        std::vector<std::string> m_names;
        std::string m_desc;

    protected:
        virtual void Exec(const std::vector<std::string> &data) = 0;

    public:
        OptionBase(const std::vector<std::string> &names, const std::string &desc) :
            m_names(names), m_desc(desc)
        {
            ESS_ASSERT(!m_names.empty());
        }

        virtual ~OptionBase()
        {
        }

        const std::string& Description() const { return m_desc; }
        std::string Names(const std::string &key) const;
        
        int Find(const std::string &name) const;
        
        void CheckDublicates(const std::vector<std::string> &names) const; // can throw
        void CheckDublicates(const OptionBase &other) const; // can throw

        void Execute(const std::vector<std::string> &data) { Exec(data); }
    };

    // --------------------------------------------
    
    // Using to call function depend on option
    template<class TContext>
    class OptionCallFunc : 
        public OptionBase
    {
        typedef ProgramOptionsExceptions PE;
        
        typedef void (TContext::*FnPtr)();
        typedef void (TContext::*FnWithStringPtr)(const std::string&);
        typedef void (TContext::*FnWithListPtr)(const std::vector<std::string>&);
        
    public:
        class ICallFunc : public Utils::IBasicInterface
        {
        public:
            virtual void Call(const std::vector<std::string> &data) = 0;
        };

    private:
        boost::scoped_ptr<ICallFunc> m_fnPtr;

        void Exec(const std::vector<std::string> &data) // override
        {
            m_fnPtr->Call(data);
        }
        
    public:
        OptionCallFunc(TContext &context,
                       FnPtr fnPtr,
                       const std::vector<std::string> &names,
                       const std::string &desc) :
           OptionBase(names, desc)
        {
            class CallWithoutParam : public ICallFunc
            {
                TContext &m_context;
                FnPtr m_fnPtr;
                std::string m_optName;

            // ICallFunc impl
            private:
                void Call(const std::vector<std::string> &data)
                {
                    if (!data.empty())
                    {
                        std::ostringstream err;
                        err << "Parameter for " << m_optName 
                            << "option which does't have parameters";
                        PE::Error(err.str());
                    }
                    (m_context.*m_fnPtr)();
                }
                
            public:
                CallWithoutParam(TContext &context, 
                                 FnPtr fnPtr,
                                 const std::string &optName):
                    m_context(context), m_fnPtr(fnPtr), m_optName(optName)
                {
                }
            };

            m_fnPtr.reset(new CallWithoutParam(context, fnPtr, Names("")));
        }
        
        OptionCallFunc(TContext &context,
                       FnWithStringPtr fnPtr,
                       const std::vector<std::string> &names,
                       const std::string &desc) :
            OptionBase(names, desc)
        {
            class CallWithString : public ICallFunc
            {
                TContext &m_context;
                FnWithStringPtr m_fnPtr;
                std::string m_optName;

            // ICallFunc impl
            private:
                void Call(const std::vector<std::string> &data)
                {
                    if (data.size() != 1) 
                    {
                        std::ostringstream err;
                        err << "Invalid '" << m_optName 
                            << "' option (string-type) value: '";
                        for (size_t i = 0; i < data.size(); ++i)
                        {
                            err << data.at(i);
                            if (i != (data.size() - 1)) err << " ";
                        }
                        err << "'";
                        
                        PE::Error(err.str());
                    }

                    (m_context.*m_fnPtr)(data.at(0));
                }
                
            public:
                CallWithString(TContext &context, 
                               FnWithStringPtr fnPtr,
                               const std::string &optName):
                    m_context(context), m_fnPtr(fnPtr), m_optName(optName)
                {
                }
            };
            
            m_fnPtr.reset(new CallWithString(context, fnPtr, Names("")));
        }
        
        OptionCallFunc(TContext &context,
                       FnWithListPtr fnPtr,
                       const std::vector<std::string> &names,
                       const std::string &desc) :
            OptionBase(names, desc)
        {
            class CallWithStringList : public ICallFunc
            {
                TContext &m_context;
                FnWithListPtr m_fnPtr; // void (TContext::*m_fnPtr)(const std::vector<std::string>&);

            // ICallFunc impl
            private:
                void Call(const std::vector<std::string> &data)
                {
                    // ESS_ASSERT(!data.empty());
                    (m_context.*m_fnPtr)(data);
                }
                
            public:
                CallWithStringList(TContext &context, 
                                   FnWithListPtr fnPtr):
                    m_context(context), m_fnPtr(fnPtr)
                {
                }
            };
            
            m_fnPtr.reset(new CallWithStringList(context, fnPtr));
        }
    };

    // --------------------------------------------

    // Default help-option handler
    class OptionExecHelp : public OptionBase // OptionSetField
    {
        bool &m_helpRequested;
        
        void Exec(const std::vector<std::string> &data); // override
        
    public:
        OptionExecHelp(bool &helpRequested);
    };

    // --------------------------------------------

    class OptionList
    {
        ManagedList<OptionBase> m_opt;
        std::string m_key;

        void AllNames(std::vector<std::string> &res, size_t &longestSize) const;
        void AllDescriptions(std::vector<std::string> &res) const;
        static std::string OptionHelpInfo(const std::string &names, 
                                          const std::string &desc,
                                          int descMinOffset);
        
    public:
        OptionList(const std::string &key);

        OptionBase* Find(const std::string &name) const;

        template<class TContext, class TFuncPtr>
        void AddFunc(TContext &context, TFuncPtr fnPtr, 
                     const std::vector<std::string> &names,
                     const std::string &desc)
        {
            for (size_t i = 0; i < m_opt.Size(); ++i)
            { 
                m_opt[i]->CheckDublicates(names);
            }
            
            m_opt.Add(new OptionCallFunc<TContext>(context, fnPtr, names, desc));
        }

        template<class T>
        void Add(const T &setter)
        {
            enum { CCheck = boost::is_base_of<OptionBase, T>::value };
            BOOST_STATIC_ASSERT( CCheck );

            for (int i = 0; i < m_opt.Size(); ++i)
            {
                m_opt[i]->CheckDublicates(setter);
            }
            
            m_opt.Add(new T(setter));
        }

        std::string HelpInfo() const;
        std::string HelpKeys() const;
        const std::string& Key() const { return m_key; }  
    };

    // --------------------------------------------
    
    // Process command line arguments: save key values to fields or call function
    template <class TContext>
    class ProgramOptions
    {
        typedef ProgramOptionsExceptions PE;
        
        typedef void (TContext::*FnPtr)();
        typedef void (TContext::*FnWithStringPtr)(const std::string&);
        typedef void (TContext::*FnWithListPtr)(const std::vector<std::string>&);
        
        TContext &m_context;
        OptionList m_opt;
        bool m_helpRequested;
        
        // return empty string if it's not a key
        std::string ExtractName(const std::string &item)
        {
            std::string res;
            size_t kSize = m_opt.Key().size();
            
            if (item.size() < (kSize + 1)) return res;
            if (item.substr(0, kSize) == m_opt.Key()) res = item.substr(kSize, -1);

            return res;
        }
        
    public:
        ProgramOptions(TContext &context, const std::string &key = "-") : 
            m_context(context), m_opt(key), m_helpRequested(false)
        {
        }
            
        void Add(FnPtr fnPtr, 
                 const std::vector<std::string> &names,
                 const std::string &desc = "")
        {
            m_opt.AddFunc(m_context, fnPtr, names, desc);
        }
        
        void Add(FnWithStringPtr fnPtr, 
                 const std::vector<std::string> &names,
                 const std::string &desc = "")
        {
            m_opt.AddFunc(m_context, fnPtr, names, desc);
        }
        
        void Add(FnWithListPtr fnPtr, 
                 const std::vector<std::string> &names,
                 const std::string &desc = "")
        {
            m_opt.AddFunc(m_context, fnPtr, names, desc);
        }
        
        template<class T>
        void Add(const T &setter) { m_opt.Add(setter); }

        void AddDefaultHelpOption() 
        {
            Add(OptionExecHelp(m_helpRequested));
        }

        void Process(const Utils::StringList &args)
        {
            int i = 1; // skip exe name
            while (i < args.size())
            {
                // try extract key name
                std::string key = ExtractName(args.at(i).toStdString());
                if (key.empty()) PE::Error("Value without option", args.at(i).toStdString());

                OptionBase *p = m_opt.Find(key);
                if (p == 0) PE::Error("Unknown option", args.at(i).toStdString());

                ++i;

                std::vector<std::string> params;
                while ((i < args.size()) 
                        && (ExtractName(args.at(i).toStdString()).empty()))
                {
                    params.push_back(args.at(i).toStdString());
                    ++i;
                }
                p->Execute(params);
            }
        }

        bool HelpRequested() const { return m_helpRequested; }

        std::string HelpInfo() const { return m_opt.HelpInfo(); }
        std::string HelpKeys() const { return m_opt.HelpKeys(); }
    };

} // namespace Utils

#endif
