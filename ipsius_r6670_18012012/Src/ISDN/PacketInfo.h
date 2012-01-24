#ifndef PACKETINFO_H
#define PACKETINFO_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "isdnpack.h"


// for LAPD && mod128, without start/stop flag
// Use dataref! - Not work with dead ref!

namespace ISDN
{
    
    class PacketInfo
    {
    public:

        PacketInfo(const IsdnPacket& pack, bool IsTE):
          m_data( pack.ReadFullData() ), m_IsTE(IsTE), m_pPack(&pack)
        {
  
        }


        std::string AsBinary()
        {
            std::stringstream ss;
            int size = m_data.size(), count = 0;
            ss.setf(std::ios::uppercase);
            while(count < size)
            {
                ss.width(2);ss.fill('0');
                ss << std::hex << (int)m_data[count++] << " ";                
            }
            return ss.str();
        }

        std::string CommonInfo()
        {
            if(BadPacket()) return "BadPacket!";
            std::stringstream ss;
            Type t = PacketType();
            ss << "PackID(&)=" << m_pPack << " SAPI = " << Sapi() << ", TEI = " << Tei() << ", C/R bit = " << CR();
            char pf = (CR() == 'R') ? 'F' : 'P';

            switch(t)
            {
            case(I): 
                if (m_data.size() <= 3) return "BadPacket!";
                ss << "\nPacketType: Info, " << pf << " = " << PF(t) 
                    << ", N(s) = " << Ns() << ", N(r) = " << Nr();
                break;
            case(U):
                ss << "\nPacketType: U, ";
                if( IsSabme() ) 
                {ss << "SABME"; break;}
                if( IsUa() ) 
                {ss << "UA"; break; }
                if( IsDisc() ) {ss << "DISC"; break;}
                if( IsDm() ) {ss << "DM"; break; }
                if( IsFrmr() ) {ss << "FRMR"; break;}
                ss << "FrameError";
                break;
            case(Utei):
                if (m_data.size() <= 7) return "BadPacket!";
                if(Sapi() == 0)  ss << "\nPacketType: Ui "<< pf << " = " << PF(t);
                if(Sapi() == 63) ss << "\nPacketType: U-TEI, "<< pf << " = " << PF(t) << ", ri = " << Ri()
                    << ", ai = " << Ai() << ", com = " << Com();
                break;
            case(S):
                {
                if (m_data.size() <= 3) return "BadPacket!";
                ss << "\nPacketType: S, " << pf << " = " << PF(t)<< ", N(s) = " << Ns()
                    <<" N(r) = " << Nr() <<" Comand: ";
                Command cmd = CMD();
                if(cmd == RR)  {ss << "RR"; break;}
                if(cmd == RNR) {ss << "RNR"; break;}
                if(cmd == REJ) {ss << "REJ"; break;}
                if(cmd == NOC) {ss << "???"; break;}
                break;
                }
            case(Bad):
                ss << "\nPacketType: Unknown! (Wrong packet!) ";
            }

            return ss.str();
        }

        bool BadPacket()
        {
            if( m_data.size()<3 )return true;
            // ... other if
            //else
            return false;
        }

    private:

        enum Type {I, U, Utei, S, Bad};
        enum Command {RR, RNR, REJ, NOC};
        const std::deque<byte>& m_data;
        bool m_IsTE; // TE side if true, else NT side
        const IsdnPacket* m_pPack;


        Type PacketType()
        {
            /*Size validate*/

            byte  contr0 = m_data.at(2); // first control byte
            if (!(contr0 & 1)) return I;
            if (((contr0 & 0xf3) == 1) && ((contr0 & 0x0c) != 0x0c)) return S;
            if ((contr0 & 0xef) == 0x03) return Utei;
            //if ((contr0 & 0xef) == 0x63) return U;
            
            return U;
        }

        Command CMD()
        {
            byte contr0 = m_data.at(2);
            if(contr0 == 0x1) return RR;
            if(contr0 == 0x9) return REJ;
            if(contr0 == 0x5) return RNR;
            return NOC;
        }

        //bool

        char CR()
        {
            bool crBit = m_data.at(0) & 2;
            if(m_IsTE) crBit = !crBit;
            return crBit ? 'C' : 'R';
        }

        bool PF(Type t)
        {
            ESS_ASSERT(m_data.size() > 3);

            switch(t)
            {
            case(I):
            case(S):
                return m_data.at(3) & 1;
            case(U):
            case(Utei):
                return m_data.at(2) & 0x10;
                break;
            default: 
                ESS_ASSERT(0 && "Imposible assert");
                return 0;
            }
        }

        int Ns()
        {
            return (byte)m_data.at(2) >> 1;
        }

        int Nr()
        {
            ESS_ASSERT(m_data.size() > 3);
            return (byte)m_data.at(3) >> 1;
        }
        
        int Tei()
        {
            return (byte)(m_data.at(1) >> 1);
        }
        
        int Sapi()
        {
            return (byte)(m_data.at(0) >> 2);
        }
        
        bool IsSabme()
        {
            return( (m_data.at(2) & ~0x10) == 0x6f );
        }

        bool IsUa()
        {
            return ((m_data.at(2) & 0xef) == 0x63);
        }

        bool IsDisc()
        {
            return ( (m_data.at(2) & 0xef) == 0x43 );
        }
        
        bool IsDm()
        {
            return ( (m_data.at(2) & 0xef) == 0x0f );
        }

        bool IsFrmr()
        {
            return ( (m_data.at(2) & 0xef) == 0x87 );
        }

        Platform::word Ri()
        {
            ESS_ASSERT(m_data.size() > 5);
            return (m_data.at(4) << 8) | (m_data.at(5));
        }

        int Ai()
        {
            if (m_data.size() <= 7) return -1;
            return (byte) m_data.at(7) >> 1;
        }

        std::string Com()
        {   
            if(m_data.size() < 7)return "NoComand";
            int com = (byte) m_data.at(6);
            switch(com)
            {
                case(1):return "ID_REQUEST";
                case(2):return "ID_ASSIGNED";
                case(3):return "ID_DENIED";
                case(4):return "ID_CHK_REQ";
                case(5):return "ID_CHK_RES";
                case(6):return "ID_REMOVE";
                case(7):return "ID_VERIFY";
            }
            return "NoComand";
        }
        
    };
}//namespace ISDN

#endif
