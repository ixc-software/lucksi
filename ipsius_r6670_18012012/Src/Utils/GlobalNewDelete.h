#ifndef GLOBALNEWDELETE_H
#define GLOBALNEWDELETE_H

#ifdef ENABLE_GLOBAL_NEW_DELETE

void * operator new(std::size_t size);

void operator delete(void* ptr);

void * operator new[](std::size_t size);

void operator delete[](void* ptr);

int GetMemAllocatorCounter();

#endif



#endif
