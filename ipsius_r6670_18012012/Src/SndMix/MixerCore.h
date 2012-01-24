#ifndef __MIXERCORE__
#define __MIXERCORE__

#include "iLog/LogWrapper.h"

#include "MixerError.h"
#include "MixMode.h"
#include "IMixPoint.h"

namespace SndMix
{   
    using Utils::SafeRef;
    using boost::scoped_ptr;
    
    /*
            MixerCore is passive, driven by external frequrency -- DoMixCycle() calls 
    */
    class MixerCore : 
        virtual public Utils::SafeRefServer, 
        public IMixPointOwner,
        boost::noncopyable
    {
        const int m_blockSize;

        const bool m_autoMode;
        const std::string m_name;
        scoped_ptr<iLogW::LogSession> m_log; 

        // оптимизация: разряженный нулями массив, 
        // IMixPoint.Handle() есть индекс в этом массиве

		typedef std::vector< SafeRef<IMixPoint> > PointList;
        PointList m_points; 

        MixMode m_mode;

        int FindPoint(const IMixPoint *p) const
        {
            for(int i = 0; i < m_points.size(); ++i)
            {
                if (m_points.at(i).IsEqualIntf(p) )
                {
                    return i;
                }
            }

            return -1;
        }

        int FindPoint(const SafeRef<IMixPoint> &p) const
        {
            return FindPoint( p.operator->() );
        }


        bool PointIndexesIsCorrect(const std::vector<int> &indexes)
        {
            for(int i = 0; i < indexes.size(); ++i)
            {
                int currIndex = indexes.at(i);

                if ( currIndex >= m_points.size() )     return false;
                if ( m_points.at(currIndex).IsEmpty() ) return false;
            }

            return true;
        }


        void VerifyMode(const MixMode &mode)
        {
            if( !PointIndexesIsCorrect( mode.AllInputChs() ) )
            {
                ESS_THROW(Error::MixerCoreBadMode);
            }

            if( !PointIndexesIsCorrect( mode.AllOutputChs() ) )
            {
                ESS_THROW(Error::MixerCoreBadMode);
            }
        }

        /*
        bool ReadyToMix()
        {
            const std::vector<int> &inputPoints = m_mode.AllInputChs();
            	
            if ( inputPoints.empty() ) return false;

            for(int i = 0; i < inputPoints.size(); ++i)
            {
                int index = inputPoints.at(i);

                if (m_points.at(index)->ReadPtr(m_blockSize) == 0)
                {
                    return false;
                }
            }

            return true;
        } */

        /* perfomance: root of all evil
            - везде код написан безопасно, через at() вместо []
            - итераторы на чтение хранятся в куче (readIters) 
        */
        void MakeOutputForPoint(int index)
        {
            const ChOutputDesc &desc = m_mode.OutputDesc(index);
           
            // collect all read iterators to readIters
            Utils::ManagedList<CyclicBuff::RdIter> readIters;
            {
                const std::vector<ChInput> &inputPoints = desc.InputChannels;

                for(int i = 0; i < inputPoints.size(); ++i)
                {
                    int index = inputPoints.at(i).ChInputIndex;
                    CyclicBuff &cb = m_points.at(index)->ReadBuff();
                    readIters.Add( new CyclicBuff::RdIter(cb, m_blockSize, true) );
                }

            }

            // clear output buffer
            CyclicBuff::WrIter wr( m_points.at(index)->WriteBuff(), m_blockSize );

            while( !wr.End() )  // per sample loop
            {
                int sum = 0;

                for(int i = 0; i < readIters.Size(); ++i)
                {
                    sum += readIters[i]->ReadAndMove();
                }

                wr.WriteAndMove(sum); // saturation control - ?
            }

        }

        void DoMix()
        {
            // mix for all output
            const std::vector<int> &outputPoints = m_mode.AllOutputChs();
            
            for(int i = 0; i < outputPoints.size(); ++i)
            {
                MakeOutputForPoint( outputPoints.at(i) );
            }

            // next blocks for all input
            {
                const std::vector<int> &inputPoints = m_mode.AllInputChs();

                for(int i = 0; i < inputPoints.size(); ++i)
                {
                    int index = inputPoints.at(i);
                    m_points.at(index)->ReadBuff().MoveReadPosition(m_blockSize);
                }
            }
        }

        void AddModeFor(MixMode &mode, int index) const
        {
            std::vector<ChInput> inputs;

            for(int i = 0; i < m_points.size(); ++i)
            {
                if (i == index) continue;
                if ( m_points[i].IsEmpty() ) continue;

                PointMode m = m_points[i]->GetMode();
                if (m == SndMix::PmRecv) continue;

                inputs.push_back( ChInput( m_points[i]->Handle() ) );
            }

            if ( inputs.empty() ) return;

            // add mode for channel
            ChOutputDesc desc;
            desc.ChOutputIndex = m_points[index]->Handle();
            desc.InputChannels = inputs;

            mode.ChOutputMode(desc);
        }

        void UpdateMode()
        {
            if (!m_autoMode) return;

            MixMode mode;

            for(int i = 0; i < m_points.size(); ++i)
            {
                if ( m_points.at(i).IsEmpty() ) continue;

                PointMode m = m_points[i]->GetMode();
                if (m == SndMix::PmSend) continue;

                AddModeFor(mode, i);
            }

            SetMode(mode);
        }

        bool PointIsCorrect(const IMixPoint &point) const
        {
            int index = point.Handle();
            if (index >= m_points.size()) return false;
            if ( !m_points.at(index).IsEqualIntf(&point)) return false;

            return true;
        }

    // IMixPointOwner impl
    private:

        void Bind(SafeRef<IMixPoint> point)
        {
            ESS_ASSERT( !point.IsEmpty() );

            int index = point->Handle();

            if (index >= m_points.size())
            {
                m_points.resize(index + 1);
            }

            ESS_ASSERT( m_points.at(index).IsEmpty() );
            m_points.at(index) = point;

            UpdateMode();
        }

		int CreateIndex()
		{
			for(int i = 0; i < m_points.size(); ++i)
			{
				if(m_points[i].IsEmpty()) return i;
			}
			
			int index = m_points.size();
			m_points.resize(index + 1);
			return index;
		}

        void Unbind(const IMixPoint &point)
        {
            ESS_ASSERT( PointIsCorrect(point) );

            m_points.at(point.Handle()).Clear();

            // mode update/verify
            UpdateMode();
            VerifyMode(m_mode);
        }


    public:

        // (directMixMode == true) -> mix делается только при вызове ProcessMix()
        MixerCore(iLogW::ILogSessionCreator &logCreator, 
                  bool autoMode, int id, int blockSize) :
            m_blockSize(blockSize),
            m_autoMode(autoMode),
            m_name("Conf" + Utils::IntToString(id)),
            m_log( logCreator.CreateSession(m_name, true) )
        {
        }

        ~MixerCore()
        {
            for(int i = 0; i < m_points.size(); ++i)
            {
                ESS_ASSERT( m_points.at(i).IsEmpty() );
            }
        }

        void SetMode(const MixMode &mode)
        {
            VerifyMode(mode);

            m_mode = mode;
        }

        const std::string& Name()
        {
            return m_name;
        }

        std::vector<MixPointState> State() const
        {
            std::vector<MixPointState> res;

            for(int i = 0; i < m_points.size(); ++i)
            {
                if ( m_points.at(i).IsEmpty() ) continue;
                res.push_back( m_points.at(i)->PointState() );
            }

            return res;
        }
        
        void DoMixCycle()
        {
	        DoMix();
        } 

        bool PointIn(IMixPoint *p)
        {
            if (p == 0) return false;

            return (FindPoint(p) >= 0);
        }

                
    };
    
    
}  // namespace SndMix


#endif
