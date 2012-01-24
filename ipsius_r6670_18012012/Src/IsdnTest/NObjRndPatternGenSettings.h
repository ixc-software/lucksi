#ifndef NOBJRNDPATTERNGENSETTINGS_H
#define NOBJRNDPATTERNGENSETTINGS_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "RandomPatternGen.h"

namespace IsdnTest
{
    class NObjRndPatternGenSettings :         
        public Domain::NamedObject,
        public DRI::INonCreatable        
    {
        Q_OBJECT;

        RandomPatternGen::Profile m_prof;

        int getSeed() const {return m_prof.Seed;}
        void setSeed(int seed) {m_prof.Seed = seed;}

        int getChangeBytesChancePercent() const {return m_prof.ChangeBytesChancePercent;}
        void setChangeBytesChancePercent(int val) {m_prof.ChangeBytesChancePercent = val;}

        int getMaxChangeBits() const {return m_prof.MaxChangeBits;}
        void setMaxChangeBits(int val) {m_prof.MaxChangeBits = val;}

        int getCutBytesChancePercent() const {return m_prof.CutBytesChancePercent;}
        void setCutBytesChancePercent(int val) {m_prof.CutBytesChancePercent = val;}

        int getMaxCutBytes() const {return m_prof.MaxCutBytes;}
        void setMaxCutBytes(int val) {m_prof.MaxCutBytes = val;}

        int getAddBytesChancePercent() const {return m_prof.AddBytesChancePercent;}
        void setAddBytesChancePercent(int val) {m_prof.AddBytesChancePercent = val;}

        int getMaxAddBytes() const {return m_prof.MaxAddBytes;}
        void setMaxAddBytes(int val) {m_prof.MaxAddBytes = val;}
    
    public:          

        const RandomPatternGen::Profile& getProf() const {return m_prof;}

        NObjRndPatternGenSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent)
        {        
        }        

        Q_PROPERTY(int Seed READ getSeed  WRITE setSeed);
        Q_PROPERTY(int ChangeBytesChancePercent READ getChangeBytesChancePercent WRITE setChangeBytesChancePercent);
        Q_PROPERTY(int MaxChangeBits READ getMaxChangeBits WRITE setMaxChangeBits);
        Q_PROPERTY(int CutBytesChancePercent READ getCutBytesChancePercent WRITE setCutBytesChancePercent);
        Q_PROPERTY(int MaxCutBytes READ getMaxCutBytes WRITE setMaxCutBytes);
        Q_PROPERTY(int AddBytesChancePercent READ getAddBytesChancePercent WRITE setAddBytesChancePercent);
        Q_PROPERTY(int MaxAddBytes READ getMaxAddBytes WRITE setMaxAddBytes);
    };

} // namespace IsdnTest

#endif
