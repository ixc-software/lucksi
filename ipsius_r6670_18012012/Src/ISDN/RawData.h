#ifndef RAWDATA_H
#define RAWDATA_H

#include "stdafx.h"

namespace ISDN
{
    // упаковка для ссылочной передачи фрагмента QVector<byte>
    class RawData : boost::noncopyable
    {
        const QVector<byte> m_data;
        int m_offset;

    public:

        RawData(const QVector<byte> &data, int offset):
          m_data(data), m_offset(offset) 
          {
              ESS_ASSERT(m_offset < data.size());
          }

          byte operator[](int index) const
          {
              int pos = m_offset + index;
              ESS_ASSERT(pos < m_data.size());
              return m_data[pos];
          }

          int Size() const
          {
              return m_data.size() - m_offset;
          }

    };
} // namespace ISDN

#endif
