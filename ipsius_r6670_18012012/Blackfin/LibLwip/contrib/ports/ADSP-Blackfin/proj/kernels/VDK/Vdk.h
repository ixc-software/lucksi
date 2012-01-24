/* =============================================================================
 *
 *  Description: This VDK Configuration file is automatically generated
 *               by the VisualDSP++ IDDE and should not be modified.
 *
 * ===========================================================================*/

#ifndef VDK_H_    
#define VDK_H_

#ifdef __ECC__	
#pragma no_implicit_inclusion
#ifndef _ADI_THREADS
#warning "VDK projects should be compiled with -threads option"
#endif
#endif

/* Define which optional VDK features are to be used in this build. */

/**
 * Semaphores are used in this project 
 */
 
#define VDK_INCLUDE_SEMAPHORES_
//#define VDK_BOOT_SEMAPHORES_

/**
 * MemoryPools are used in this project 
 */
 
//#define VDK_INCLUDE_MEMORYPOOLS_
//#define VDK_BOOT_MEMORYPOOLS_

/**
 * DeviceFlags are used in this project 
 */
 
//#define VDK_INCLUDE_DEVICE_FLAGS_
//#define VDK_BOOT_DEVICE_FLAGS_

/**
 * IO templates are used in this project
 */
 
//#define VDK_INCLUDE_IO_
//#define VDK_BOOT_OBJECTS_IO_

/* 
 * There are heaps specified 
 */

#define VDK_MULTIPLE_HEAPS_

/**
 * Messages are used in this project
 */

//#define VDK_INCLUDE_MESSAGES_

/**
 * Multiprocessor messaging is used in this project
 */

//#define VDK_MULTIPROCESSOR_MESSAGING_

/**
 * RoutingThreads are defined in this project
 */

//#define VDK_INCLUDE_ROUTING_THREADS_

/**
 * Routing Nodes are defined in this project
 */

//#define VDK_INCLUDE_ROUTING_NODES_

/**
 * Marshalled Maessage Types are defined in this project
 */

//#define VDK_INCLUDE_MARSHALLED_MESSAGES_

/**
 * Events and EventBits are used in this project 
 */
 
//#define VDK_INCLUDE_EVENTS_

/**
 * Enhanced API error checking and runtime Instrumentation is included
 */
 
#define VDK_INSTRUMENTATION_LEVEL_ 2

#ifndef VDK_LDF_

#ifdef __ADSPBF533__
#include <defBF533.h>
#endif

#ifdef __ADSPBF561__
#include <defBF561.h>
#endif

#ifdef __ADSPBF535__
#include <defBF535.h>
#endif

/**
 * ALL of the following enums are generated in format suitable for C/C++
 * and a format suitable for assembly inclusion
 */

/*************************************************************************
 * ThreadType
 *
 * These are used in calls to VDK::CreateThread().
 */
 
#ifdef __ECC__	// for C/C++ access
enum ThreadType
{
kDynamicThreadType = -1,
	klwip_sysboot_threadtype,
	kADI_TOOLS_IOEThreadType,
};
#else	// for asm access
#define kDynamicThreadType  -1
#define klwip_sysboot_threadtype 0	// enum
#define kADI_TOOLS_IOEThreadType 1	// enum
#endif 	// __ECC__

/*************************************************************************
 * ThreadID
 *
 * These are used in calls to VDK::CreateThread().
 */
 
#ifdef __ECC__	// for C/C++ access
enum ThreadID
{
	kIdleThread=0,
	klwipBootThread=1

};
#else	// for asm access
#define kIdleThread 0
#define klwipBootThread 1	// enum

#endif 	// __ECC__


/*************************************************************************
 * SemaphoreID
 *
 * Used in all semaphore APIs
 *
 */

#ifdef __ECC__	// for C/C++ access
enum SemaphoreID 
{

	last_semaphore__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__

/*************************************************************************
 * MemoryPoolID
 *
 * Used in all Memory Pool APIs
 *
 */

#ifdef __ECC__        // for C/C++ access
enum PoolID
{

      last_mempool__3VDK=-1
};
#else // for asm access

#endif        // __ECC__

/*************************************************************************
 * EventID
 *
 * Used in Event(...) APIs.
 *
 * There cannot be more than NATURAL_WORD_SIZE
 * EventIDs.
 */
 
#ifdef __ECC__	// for C/C++ access
enum EventID
{

	last_event__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__


/*************************************************************************
 * EventBitID
 *
 * Used in EventBit(...) APIs
 *
 * There cannot be more than NATURAL_WORD_SIZE
 * EventBitIDs.
 */
 
#ifdef __ECC__	// for C/C++ access
enum EventBitID
{

	last_eventbit__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__

/*************************************************************************
 * DeviceID
 *
 * Used in Device Drivers
 *
 */
 
#ifdef __ECC__	// for C/C++ access
enum IOID
{

	last_bootdevice__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__

/*************************************************************************
 * IO template IDs
 *
 * Used in Device Drivers
 *
 */
 
#ifdef __ECC__	// for C/C++ access
enum IOTemplateID
{

	last_iotemplate__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__

/*************************************************************************
 * DeviceFlagID
 *
 * Used in Device Drivers
 *
 */
 
#ifdef __ECC__	// for C/C++ access
enum DeviceFlagID
{

	last_deviceflag__3VDK=-1
};
#else	// for asm access

#endif 	// __ECC__

#ifdef __ECC__  // for C/C++ access
enum MessageID
{
        last_message__3VDK=-1
};
#endif  // __ECC__

#ifdef VDK_MULTIPLE_HEAPS_
#ifdef __ECC__  // for C/C++ access
enum HeapID {
	ksystem_heap
};
#else // for asm access
#define ksystem_heap 0	// enum
#endif // __ECC__
#endif

#ifdef VDK_MULTIPROCESSOR_MESSAGING_

#ifdef __ECC__  // for C/C++ access
enum RoutingNodeID {

};
#else // for asm access

#endif // __ECC__

#ifdef VDK_INCLUDE_MARSHALLED_MESSAGES_
#ifdef __ECC__  // for C/C++ access
enum MarshalledTypeID {

};
#else // for asm access

#endif // __ECC__
#endif // VDK_INCLUDE_MARSHALLED_MESSAGES_

#endif

// include the VisualDSP Kernel API and library
#include <VDK_Public.h>

#endif // VDK_LDF_
#endif // VDK_CONFIG_H_

/* ========================================================================== */
