#ifndef IFIXEDMEMBLOCKPOOL_H
#define IFIXEDMEMBLOCKPOOL_H

#include "IBasicInterface.h"

namespace Utils
{
    
    // ��������� ���������� ������ �������������� �������
    class IFixedMemBlockPool : public IBasicInterface
    {
    public:
        
        virtual void* alloc(int size, const char* file = 0, int line = 0) = 0;

        // allocate block with max available size
        virtual void* allocFullBlock(const char* file = 0, int line = 0) = 0;

        virtual void free(void *) = 0;

        virtual int getBlockSize() const = 0;

        // ��� ���������� � "�� Fixed" ������������. size ��� ������������ ��� assert
        //virtual void* alloc(int size) = 0;
    };

} // namespace Utils

#endif

