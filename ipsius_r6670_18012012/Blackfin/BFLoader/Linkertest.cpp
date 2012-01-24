// #include "stdafx.h"
// #include <string>

#include <cstdlib>

void* LinkerAlloc();

/*
void* operator new(std::size_t size)
{
	return 0;
}
*/

char* RunLinkertest()
{
	int *p = new int(-1);
    return "RunLinkertest";
}

char* RunLinkertest2()
{
	//int *p;
	// delete p;
	
	void *p = LinkerAlloc();
	
    // int *p = (int*)malloc(4); //  new int(-1);
	
    return "RunLinkertest2";
}

int fn()
{
	LinkerAlloc();
	return 0;
}

int fn2()
{
	return fn();
}

