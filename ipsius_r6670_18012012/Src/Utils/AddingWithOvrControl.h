#ifndef ADDINGWITHOVRCONTROL_H
#define ADDINGWITHOVRCONTROL_H

namespace Utils
{
    // выполняет приращение oldVal на add, возвращает признак переполнения
    template<class T1, class T2>
    bool AddingWithOvrControl(T1& oldVal, const T2& add) 
    {
        if (add == 0) return false;

        T1 newVal = oldVal + add;
        bool overflow = false;

        if (add > 0)
        {                
            if (newVal <= oldVal) overflow = true;
        }
        else
        {
            if (newVal >= oldVal) overflow = true;
        }

        oldVal = newVal;

        return overflow;
    }
} // namespace Utils

#endif
