#pragma once

#include "Utils/ErrorsSubsystem.h"
#include "ImgDescriptor.h"
#include "ImgDescrList.h"
#include "ConfigLine.h"
#include "ScriptList.h"



namespace BfBootCore
{   
    //template<class T>
    class StringToTypeConverter
    {
    public:
        ESS_TYPEDEF(CantConvert);

        template<class T>
        static void fromString(const std::string& s, T &field)
        {
            std::istringstream ss;
            ss.str(s);
            ss >> field;
            if (ss.fail()) ESS_THROW(CantConvert);            
        }

        template<class T>
        static std::string toString(const T &field)
        {
            std::ostringstream ss;
            ss << field;
            return ss.str();
        }

        // ------------------------------------------------------------------------------------
        
        static void fromString(const std::string& s, std::string &field)
        {
            field = s;
        }

        
        static std::string toString(const std::string &field)
        {            
            return field;
        }

        // ------------------------------------------------------------------------------------

        static void fromString(const std::string& s, ImgDescriptor &field)
        {
            if (!field.setFromString(s)) 
                ESS_THROW(CantConvert);
        }
       
        static std::string toString(const ImgDescriptor &field)
        {            
            return field.ToString();
        }

        // ------------------------------------------------------------------------------------    

        static void fromString(const std::string& s, ConfigLine &out)
        {
            //if ( out.Read(s.c_str(), s.size()) == 0) ESS_THROW(CantConvert);
            try 
            {
                ConfigLine line(s);
                out = line;
            }
            catch(ConfigLine::ParseError&)
            {
                ESS_THROW(CantConvert);
            }
            

        }

        static std::string toString(const ConfigLine &line)
        {            
            return line.getRawString();
        }   

        // ------------------------------------------------------------------------------------

        static void fromString(const std::string& s, NamedScript &out)
        {            
            if(!out.Assign(s)) 
                ESS_THROW(CantConvert);;            
        }

        static std::string toString(const NamedScript& in)
        {            
            return in.getRawString();
        }

        // ------------------------------------------------------------------------------------

        static void fromString(const std::string& s, ScriptList &out)
        {          
            std::vector<NamedScript> list;
            fromString(s, list);
            if (!out.SetFrom(list)) ESS_THROW(CantConvert);            
        }

        static std::string toString(const ScriptList& in)
        {            
            return toString(in.AsVector());
        }

        // ------------------------------------------------------------------------------------

        static void fromString(const std::string& s, ImgDescrList &out)
        {          
            std::vector<ImgDescriptor> list;
            fromString(s, list);
            out.SetFrom(list);            
        }

        static std::string toString(const ImgDescrList& in)
        {            
            return toString(in.AsVector());
        }
       
        // ------------------------------------------------------------------------------------
        
        // vector<T> save format ("val" is type T object, "|" is VectorValueSep):
        // |val|val|val| 
        static std::string VectorValueSep()
        {            
            return "<V>";
        }

        static std::string VectorBeginMarker()
        {            
            return "<VB>";
        }

        static std::string VectorEndMarker()
        {            
            return "<VE>";
        }

        template<class T>
        static void fromString(const std::string& s, std::vector<T> &field)
        {
            
            if (s == VectorBeginMarker() + VectorEndMarker()) return;                        
            
            if (s.find(VectorBeginMarker()) != 0) ESS_THROW(CantConvert);
            if (s.rfind(VectorEndMarker()) != s.size() - VectorEndMarker().size()) ESS_THROW(CantConvert);

            field.clear();

            //отрезаем vb VectorEndMarker()      
            int beg = VectorBeginMarker().size() + VectorValueSep().size();
            std::string body = s.substr(beg, s.size() - VectorEndMarker().size() - beg);

            int curr = 0;
            while (curr < body.size())
            {             
                size_t vS = body.find(VectorValueSep(), curr);
                size_t vB = body.find(VectorBeginMarker(), curr);

                int length;
                int step;
                if (vS == std::string::npos && vB == std::string::npos)
                {
                    length = body.size() - curr;
                    step = length;
                }      
                else
                if (vB == std::string::npos || vS < vB) 
                {
                    length = vS - curr;                 
                    step = length + VectorValueSep().size();
                }
                else
                {
                    size_t vE = body.find(VectorEndMarker(), vB + VectorBeginMarker().size());
                    if (vE == std::string::npos) ESS_THROW(CantConvert);
                    size_t next = body.find(VectorValueSep(), vE);
                    if (next == std::string::npos)
                    {
                        length = vE - curr + VectorEndMarker().size();                 
                        step = length;
                    }
                    else 
                    {
                        length = next - curr;
                        step = length + VectorValueSep().size();
                    }
                }
                
                
                T val;
                fromString(body.substr(curr, length), val);
                field.push_back(val);
                
                curr += step;
            }
            
        }

        template<class T>
        static std::string toString(const std::vector<T> &field)
        {       
            std::string result = VectorBeginMarker();

            for (int i = 0; i < field.size(); ++i)
            {
                result += VectorValueSep() + toString( field.at(i) );
            }

            result += VectorEndMarker(); //VectorValueSep();
            return result;
        }

        
    };

    
    
} // namespace BfBootSrv


