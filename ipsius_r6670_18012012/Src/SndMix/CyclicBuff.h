#ifndef __CYCLICBUFF__
#define __CYCLICBUFF__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"

namespace SndMix
{
    using Platform::int16;

    // ---------------------------------------------------------

    class IFreqFixer : public Utils::IBasicInterface
    {
    public:

        struct StateInfo
        {
            int ReadCount, WriteCount;

            StateInfo(int readCount, int writeCount) : ReadCount(readCount), WriteCount(writeCount)
            {
            }
        };

        virtual void OnReadCompleted(const StateInfo &info) = 0;
    };


    // ---------------------------------------------------------

    struct CyclicBuffState
    {
        int TimeDurationMs;

        int DataWrite;
        int WriteErrors, WriteErrDataCount;

        int DataRead;
        int ReadErrors,  ReadErrDataCount;

        bool NoErrors() const
        {
            return (WriteErrors == 0) && (ReadErrors == 0);
        }

        std::string ToString() const
        {
            std::ostringstream oss;

            oss << "time "  << TimeDurationMs << "(ms); "
                << "read "  << Dir(DataRead,  ReadErrors,  ReadErrDataCount) << "; "
                << "write " << Dir(DataWrite, WriteErrors, WriteErrDataCount);

            return oss.str();
        }

    private:

        static std::string Dir(int total, int errCount, int errDataCount)
        {
            std::ostringstream oss;

            oss << "tot_data " << total;

            if (errCount > 0)
            {
                oss << " overrun " << errCount << " (" << errDataCount << " bytes)";
            }

            return oss.str();
        }
    };

    // -------------------------------------------------------------

    class CyclicBuff;

    class ICyclicBuffDataReady : public Utils::IBasicInterface
    {
    public:

        // return true, if more calls allowed
        virtual bool OnCyclicBuffDataReady(CyclicBuff *pSender) = 0;
    };

    // -------------------------------------------------------------

    /*
        Cyclic read/write buff
        We can implemnent here read/write freq fix thru write iterator 
        (write speed to read speed adoptation)
    */
    class CyclicBuff : boost::noncopyable
    {

        static int Ticks()
        {
            return Platform::GetSystemTickCount();
        }

        struct Direction
        {
            int Pos;
            int StartTime;
            int TotalCount;
            int OverrunErrors;
            int OverrunCount;
            bool Active;

            Direction()
            {
                Pos = 0;
                StartTime = CyclicBuff::Ticks();
                TotalCount = 0;
                OverrunErrors = 0;
                OverrunCount = 0;
                Active = false;
            }
        };

        std::vector<int16> m_buff;

        Direction m_read;
        Direction m_write;

        ICyclicBuffDataReady *m_pNotify; 
        int m_notifyDataSize;

        boost::shared_ptr<IFreqFixer> m_fixer;

        bool m_rawFillOnReadOverrun;

        static void AssertPos(int pos, int buffSize)
        {
            ESS_ASSERT(pos >= 0 && pos < buffSize);
        }

        static int AvaibleCount(int fromPos, int toPos, bool read, int buffSize)
        {
            if (fromPos == toPos) return read ? 0 : buffSize;

            if (fromPos < toPos) return (toPos - fromPos);

            return buffSize - fromPos + toPos;
        }

        /*
        static int MovePosBack(int pos, int count, int buffSize)  // can be optimised
        {
            ESS_ASSERT( count < buffSize );
            AssertPos(pos, buffSize);

            while(count--)
            {
                if (pos == 0) pos = buffSize - 1;
                         else pos--;
            }

            return pos;
        } */

        void MovePosBack(Direction &dir, int count, bool withFill)
        {
            int buffSize = m_buff.size();
            ESS_ASSERT( count < buffSize );

            while(count--)
            {
                if (dir.Pos == 0) dir.Pos = buffSize - 1;
                             else dir.Pos--;

                if (withFill) m_buff.at(dir.Pos) = 0;
            }
        }

        bool IsReadDir(const Direction &dir) const
        {
            bool isRead = (&dir == &m_read);
            if (!isRead) ESS_ASSERT(&dir == &m_write);
            return isRead;
        }

        /* 
            Iterator open
         
            Read & Write pos difference:
            - if (readPos == writePos) then (readAvailble == 0), (writeAvaible == buffSize)
            - readPos can jump to writePos (it means to get all availble data), but
              writePos can't jump to readPos 'couse that drops all data
        */
        void BeginOperation(Direction &dir, int count)
        {
            ESS_ASSERT(count > 0 && count < m_buff.size());

            ESS_ASSERT( !dir.Active );
            dir.Active = true;

            // fix pos if data is not enough avaible
            {
                bool isRead = IsReadDir(dir);                
                int avaible = isRead ? AvaibleForRead() : AvaibleForWrite();

                int need = count - avaible;
                if (!isRead) need += 1;  // see comment upper
                if (need <= 0) return;   // no overrun 

                bool withFill = isRead && m_rawFillOnReadOverrun;
                MovePosBack(dir, need, withFill);

                dir.OverrunErrors++;
                dir.OverrunCount += need;

            }

        }

        /* 
            Iterator close 
         
            count can be zero then new position will be ignored 
        */
        void EndOperation(Direction &dir, int newPos, int count)
        {
            AssertPos(newPos, m_buff.size());

            ESS_ASSERT( dir.Active );
            dir.Active = false;

            if (count <= 0) return;

            // move position
            dir.Pos = newPos;
            dir.TotalCount += count;

            // notify in the end of write op
            if ( m_pNotify != 0 && !IsReadDir(dir) )
            {
                while(true)
                {
                    int avaible = AvaibleForRead();

                    if (avaible >= m_notifyDataSize || m_notifyDataSize <= 0)
                    {
                        bool ok = m_pNotify->OnCyclicBuffDataReady(this);
                        if (!ok) break;
                    }
                    else
                    {
                        break;
                    }

                    ESS_ASSERT(AvaibleForRead() < avaible);
                }
            }

            // notify about end of read op
            if (m_fixer && IsReadDir(dir))
            {
                m_fixer->OnReadCompleted( IFreqFixer::StateInfo(m_read.TotalCount, m_write.TotalCount) );
            }
        }

        int MovePosNext(int pos, int count = 1) // can be optimised
        {
            int buffSize = m_buff.size();

            ESS_ASSERT( count < buffSize );
            AssertPos(pos, buffSize);  // is it too expensive - ?

            while(count--)
            {
                pos++;
                if (pos == buffSize) pos = 0;
            }

            return pos;
        }


    public:


        // only one instance of each type of iterator allowed, but using of iterator class 
        // give us RAII (automatic closing read or write operation)
        class IterBase : boost::noncopyable
        {
            CyclicBuff &m_owner;
            Direction &m_dir; 
            const int m_count;
            const bool m_dontMovePos;

            int m_remains;
            int m_pos;

        protected:
            
            IterBase(CyclicBuff &owner, bool isRead, int count, bool dontMovePos) : 
              m_owner(owner), 
              m_dir(isRead ? owner.m_read : owner.m_write), 
              m_count(count), 
              m_dontMovePos(dontMovePos)
            {
                m_owner.BeginOperation(m_dir, count);

                m_remains = count;
                m_pos = m_dir.Pos;
            }

            ~IterBase()
            {
                ESS_ASSERT(m_remains == 0);

                m_owner.EndOperation(m_dir, m_pos, m_dontMovePos ? 0 : m_count);
            }

            int16& Data() const
            {
                return m_owner.m_buff.at(m_pos);
            }

        public:

              bool End() const
              {
                  return (m_remains == 0);
              }

              void Move()
              {
                  ESS_ASSERT( !End() );

                  m_pos = m_owner.MovePosNext(m_pos);
                  m_remains--;
              }

        };

        class RdIter : public IterBase
        {
        public:

            RdIter(CyclicBuff &owner, int count, bool dontMovePos = false) 
                : IterBase(owner, true, count, dontMovePos)             
            {
            }

            int16 Read() const 
            {
                ESS_ASSERT( !End() );

                return Data();
            }

            int16 ReadAndMove()
            {
                int16 res = Read();
                Move();
                return res;
            }
        };

        class WrIter : public IterBase
        {
        public:

            WrIter(CyclicBuff &owner, int count) 
                : IterBase(owner, false, count, false) 
            {
            }

            void Write(int16 val)
            {
                ESS_ASSERT( !End() );

                Data() = val;
            }

            void WriteAndMove(int16 val)
            {
                Write(val);
                Move();
            }

        };

    public:

        CyclicBuff(int buffSize) : 
          m_pNotify(0), m_notifyDataSize(-1), m_rawFillOnReadOverrun(false)
        {
            m_buff.resize(buffSize);
        }

        void EnableRawFillOnReadOverrun()
        {
            m_rawFillOnReadOverrun = true;
        }

        void SetupNotifyCallback(ICyclicBuffDataReady *pNotify, int notifyDataSize)
        {
            ESS_ASSERT(m_pNotify == 0);
            
            m_pNotify = pNotify;
            m_notifyDataSize = notifyDataSize;
        }

        void SetupFreqFixer(boost::shared_ptr<IFreqFixer> fixer)
        {
            m_fixer = fixer;
        }

        void MoveReadPosition(int count)
        {
            // prev version, it's bad: don't update stats, don't do callbacks 
            // m_read.Pos = MovePosNext(m_read.Pos, count);

            RdIter rd(*this, count);
            while( !rd.End() ) rd.Move();
        }

        int AvaibleForRead() const 
        {
            return AvaibleCount( m_read.Pos, m_write.Pos, true, m_buff.size() );
        }

        int AvaibleForWrite() const 
        {
            return AvaibleCount( m_write.Pos, m_read.Pos, false, m_buff.size() );
        }

        void ReadBlock(int16 *pBlock, int count)
        {
            ESS_ASSERT(pBlock != 0 && count > 0);

            RdIter i(*this, count);

            while( !i.End() )
            {
                *pBlock++ = i.ReadAndMove();
            }
        }

        void ReadBlock(std::vector<int16> &block, int count)
        {
            ESS_ASSERT(count > 0);

            block.resize(count);
            ReadBlock(&block[0], count);
        }

        void WriteBlock(const int16 *pBlock, int count)
        {
            ESS_ASSERT(pBlock != 0 && count > 0);

            WrIter i(*this, count);

            while( !i.End() )
            {
                i.WriteAndMove( *pBlock++ );
            }
        }

        void WriteBlock(const std::vector<int16> &block)
        {
            ESS_ASSERT(block.size() > 0);

            WriteBlock(&block[0], block.size());
        }

        CyclicBuffState GetState() const
        {
            CyclicBuffState st;

            st.TimeDurationMs = CyclicBuff::Ticks() - m_read.StartTime;

            {
                const Direction &d      = m_write;
                st.DataWrite            = d.TotalCount;
                st.WriteErrors          = d.OverrunErrors;
                st.WriteErrDataCount    = d.OverrunCount;
            }

            {
                const Direction &d      = m_read;
                st.DataRead             = d.TotalCount;
                st.ReadErrors           = d.OverrunErrors;
                st.ReadErrDataCount     = d.OverrunCount;
            }

            return st;
        }

        std::string DebugInfo() const
        {
            std::ostringstream oss;

            oss << "rd = " << m_read.Pos << "; wr = " << m_write.Pos << "; ";

            for(int i = 0; i < m_buff.size(); ++i)
            {
                oss << m_buff.at(i) << " ";
            }

            return oss.str();
        }
                
    };    
    
    void CycBuffTestNormal();
    void CycBuffWriteOverrunTest();
    void CycBuffReadOverrunTest();
    
}   // namespace SndMix


#endif