#ifndef __DIFF__
#define __DIFF__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"

namespace DRI
{

    struct DiffItem
    {
        QString Left;
        int LeftLine;
        
        QString Right;
        int RightLine;

        QString Desc;
        
        DiffItem() : LeftLine(-1), RightLine(-1) {}

        DiffItem(const QString &left, int leftLine,
                 const QString &right, int rightLine,
                 const QString &desc = "") :
            Left(left), LeftLine(leftLine), Right(right), RightLine(rightLine)
        {
        }

        QString ToString() const
        {
            // return QString("Diff at line %1\n%2\n%3\n\n").arg(Line).arg(Left).arg(Right);
            QString res = QString("Diff:\n"
                                  "Original (line %1): %2\n"
                                  "Current  (line %3): %4\n")
                          .arg(LeftLine).arg(Left).arg(RightLine).arg(Right); 

            if (!Desc.isEmpty()) res += QString("<%1>").arg(Desc);
            res += "\n";

            return res;
        }
    };

    // -------------------------------------------------

    class DiffList
    {
        std::vector<DiffItem> m_list;

    public:

        DiffList()
        {
        }

        void Add(const DiffItem &i)
        {
            m_list.push_back(i);
        }

        void Clear()
        {
            m_list.clear();
        }

        int Count() const { return m_list.size(); }

        QString ToString() const
        {
            QStringList sl;

            for(size_t i = 0; i < m_list.size(); ++i)
            {
                sl.push_back( m_list.at(i).ToString() );
            }

            return sl.join("\n");
        }

    };

    // -------------------------------------------------

    class IDiffLinesCompare : public Utils::IBasicInterface
    {
    public:
        virtual int CompareLine(QString left, QString right,
                                bool &needCompareNextAsBlock) = 0;
        virtual int CompareBlock() = 0;
        // returns true if need next
        virtual bool ComparingBlockAddLeft(QString left) = 0;
        virtual bool ComparingBlockAddRight(QString right) = 0;
    };

    // -------------------------------------------------

    // generalize and move to Utils::
    class Diff
    {
        static QString NormalizeString(QString s)
        {
            QString res;

            for(int i = 0; i < s.size(); ++i)
            {
                QChar c = s.at(i);
                if (c < QChar(' ')) continue;
                res += c;
            }

            return res;
        }

        static QStringList Split(QString s)
        {
            QStringList sl;

            while(!s.isEmpty())
            {
                int pos = s.indexOf(QChar(10));

                if (pos < 0)
                {
                    sl.push_back( NormalizeString(s) );
                    break;
                }

                sl.push_back( NormalizeString(s.left(pos)) );
                s = s.mid(pos + 1);
            }

            return sl;
        }

        static bool CompareLine(QString left, int leftLine,
                                QString right, int rightLine,
                                DiffList &diff,
                                IDiffLinesCompare *pComparator,
                                bool &needCompareNextAsBlock)
        {
            needCompareNextAsBlock = false;
            
            if (pComparator != 0)
            {
                if (pComparator->CompareLine(left, right, needCompareNextAsBlock) == 0)
                { 
                    return true;
                }
            }
            else
            {
                if (left == right) return true;
            }

            diff.Add( DiffItem(left, leftLine, right, rightLine) );

            return false;
        }

        static bool CompareBlock(QString leftBlock, int leftStartLine,
                                 QString rightBlock, int rightStartLine,
                                 DiffList &diff,
                                 IDiffLinesCompare *pComparator)
        {
            if (pComparator != 0)
            {
                if (pComparator->CompareBlock() == 0) return true;
            }
            else
            {
                if (leftBlock == rightBlock) return true;
            }

            diff.Add(DiffItem(leftBlock, leftStartLine, rightBlock, rightStartLine));

            return false;
        }

        static bool ComparingBlockAddLeft(QString left,
                                          IDiffLinesCompare *pComparator)
        {
            if (pComparator == 0) return false;
            
            return pComparator->ComparingBlockAddLeft(left);
        }
        
        static bool ComparingBlockAddRight(QString right,
                                           IDiffLinesCompare *pComparator)
        {
            if (pComparator == 0) return false;
            
            return pComparator->ComparingBlockAddRight(right);
        }

        static QString ExtractBlock(const QStringList &list, int first, int last)
        {
            ESS_ASSERT(first <= last);
            ESS_ASSERT(last < list.size());

            QString res;
            for (int i = first; i <= last; ++i)
            {
                if (i != first) res += "| ";
                res += list.at(i);                
                res += "\n";
            }

            return res;
        }
    private:
        struct CompareBlockParams
        {
            bool IsCompareBlock;
            int LeftBeginLine;      // index + 1
            int RightBeginLine;     // index + 1
            bool IsEndOfLeftBlock;
            bool IsEndOfRightBlock;

            CompareBlockParams() :
                IsCompareBlock(false),
                LeftBeginLine(0),
                RightBeginLine(0),
                IsEndOfLeftBlock(true),
                IsEndOfRightBlock(true)
            {
            }

            CompareBlockParams(int leftBeginLine, int rightBeginLine) :
                IsCompareBlock(true),
                LeftBeginLine(leftBeginLine),
                RightBeginLine(rightBeginLine),
                IsEndOfLeftBlock(false),
                IsEndOfRightBlock(false)
            {
            }
        };

    public:

        static bool Do(QString stringLeft, 
                       QString stringRight, 
                       DiffList &diff, 
                       IDiffLinesCompare *pComparator = 0)
        {
            diff.Clear();

            QStringList slLeft      = Split(stringLeft);
            QStringList slRight     = Split(stringRight);

            int diffCount = 0;
            int iLeft = 0;
            int iRight = 0;
            CompareBlockParams bp;

            // Left and right can have non-comparable parts with different size.
            // So even if their full size is different Diff::Do() may return false
            while ((iLeft < slLeft.size()) && (iRight < slRight.size()))
            {
                QString left = slLeft.at(iLeft);
                QString right = slRight.at(iRight);

                if (bp.IsCompareBlock)
                {
                    if (!bp.IsEndOfLeftBlock)
                    {
                        if (ComparingBlockAddLeft(left, pComparator)) ++iLeft;
                        else bp.IsEndOfLeftBlock = true;
                    }

                    if (!bp.IsEndOfRightBlock)
                    {
                        if (ComparingBlockAddRight(right, pComparator)) ++iRight;
                        else bp.IsEndOfRightBlock = true;
                    }

                    if ((bp.IsEndOfLeftBlock) && (bp.IsEndOfRightBlock))
                    {
                        QString leftBlock = ExtractBlock(slLeft, bp.LeftBeginLine - 1, iLeft);
                        QString rightBlock = ExtractBlock(slRight, bp.RightBeginLine - 1, iRight);

                        bool isSame = CompareBlock(leftBlock, bp.LeftBeginLine,
                                                   rightBlock, bp.RightBeginLine,
                                                   diff, pComparator);
                        if (!isSame) ++diffCount;

                        bp.IsCompareBlock = false;
                    }
                    continue;
                }
                
                bool needCompareNextAsBlock = false;
                bool isSame = CompareLine(left, iLeft + 1, right, iRight + 1,
                                          diff, pComparator, needCompareNextAsBlock);
                if (!isSame) ++diffCount;

                if (needCompareNextAsBlock) bp = CompareBlockParams(iLeft + 1, iRight + 1);
            
                ++iLeft;
                ++iRight;
            }
            
            // Size mismatch even after processing
            for(int i = iLeft; i < slLeft.size(); ++i) // left bigger
            {
                diff.Add( DiffItem(slLeft.at(i), i + 1, "", slRight.size()) );
                diffCount++;
            }
            for(int i = iRight; i < slRight.size(); ++i) // right bigger 
            {
                diff.Add( DiffItem("", slLeft.size(), slRight.at(i), i + 1) );
                diffCount++;
            }
            
            return (diffCount == 0);
        }
        
        /*static bool Do(QString stringLeft, QString stringRight, 
            DiffList &diff, IDiffLinesCompare *pComparator = 0)
        {
            diff.Clear();

            QStringList slLeft      = Split(stringLeft);
            QStringList slRight     = Split(stringRight);

            int diffCount = 0;            
            int count = std::min(slLeft.size(), slRight.size());

            // common part
            for(int i = 0; i < count; ++i)
            {
                QString left = slLeft.at(i);
                QString right = slRight.at(i);

                if (!LinesCompare(i + 1, left, right, diff, pComparator)) 
                {
                    diffCount++;
                }
            }

            // size mismatch
            if (slLeft.size() != slRight.size())
            {
                if (slLeft.size() > slRight.size())
                {
                    for(int i = slRight.size(); i < slLeft.size(); ++i)
                    {
                        diff.Add( DiffItem(i + 1, slLeft.at(i), "") );
                        diffCount++;
                    }
                }
                else
                {
                    for(int i = slLeft.size(); i < slRight.size(); ++i)
                    {
                        diff.Add( DiffItem(i + 1, "", slRight.at(i)) );
                        diffCount++;
                    }
                }
            }

            return (diffCount == 0);
        }*/

    };

    
}  // namespace DRI


#endif
