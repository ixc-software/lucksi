#include "stdafx.h"
#include "Utils/IntToString.h"
#include "AdiDeviceResult.h"

namespace BfDev
{
	std::string AdiDeviceResult::ToString(Platform::word code)	
	{
		switch(code)
		{
		case ADI_DEV_RESULT_SUCCESS:
			return "Generic success";
		case ADI_DEV_RESULT_FAILED: 
			return "Generic failure";
		case ADI_DEV_RESULT_NOT_SUPPORTED: 
			return "Not supported";
		case ADI_DEV_RESULT_DEVICE_IN_USE: 
			return "Device already in use";
		case ADI_DEV_RESULT_NO_MEMORY: 
			return "Insufficient memory for operation";
		case ADI_DEV_RESULT_NOT_USED_1: 
			return "No longer used (was ADI_DEV_RESULT_TOO_MANY_DEVICES)";
		case ADI_DEV_RESULT_BAD_DEVICE_NUMBER: 
			return "Bad device number";
		case ADI_DEV_RESULT_DIRECTION_NOT_SUPPORTED: 
			return "Data direction not supported";
		case ADI_DEV_RESULT_BAD_DEVICE_HANDLE: 
			return "Bad device handle";
		case ADI_DEV_RESULT_BAD_MANAGER_HANDLE: 
			return "Bad device manager handle";
		case ADI_DEV_RESULT_BAD_PDD_HANDLE: 
			return "Bad physical driver handle";
		case ADI_DEV_RESULT_INVALID_SEQUENCE: 
			return "Invalid sequence of commands";
		case ADI_DEV_RESULT_ATTEMPTED_READ_ON_OUTBOUND_DEVICE: 
			return "Attempted read on outbound device";
		case ADI_DEV_RESULT_ATTEMPTED_WRITE_ON_INBOUND_DEVICE: 
			return "Attempted write on inbound device";
		case ADI_DEV_RESULT_DATAFLOW_UNDEFINED: 
			return "Dataflow method is undefined";
		case ADI_DEV_RESULT_DATAFLOW_INCOMPATIBLE: 
			return "Operation is incompatible with the dataflow method";
		case ADI_DEV_RESULT_BUFFER_TYPE_INCOMPATIBLE: 
			return "Device does not support the given buffer type";
		case ADI_DEV_RESULT_NOT_USED_2: 
			return "No longer used (was ADI_DEV_RESULT_CALLBACK_ERROR)";
		case ADI_DEV_RESULT_CANT_HOOK_INTERRUPT: 
			return "Can't hook an interrupt";
		case ADI_DEV_RESULT_CANT_UNHOOK_INTERRUPT: 
			return "Can't unhood an interrupt";
		case ADI_DEV_RESULT_NON_TERMINATED_LIST: 
			return "Non-NULL terminated buffer list";
		case ADI_DEV_RESULT_NO_CALLBACK_FUNCTION_SUPPLIED: 
			return "No callback function was provided to the open function";
		case ADI_DEV_RESULT_REQUIRES_UNIDIRECTIONAL_DEVICE: 
			return "Requires uni-directional device";
		case ADI_DEV_RESULT_REQUIRES_BIDIRECTIONAL_DEVICE: 
			return "Requires bidirectional device";
		case ADI_DEV_RESULT_TWI_LOCKED: 
			return "TWI locked in other operation";
		case ADI_DEV_RESULT_REQUIRES_TWI_CONFIG_TABLE: 
			return "Requires configuration table for the TWI driver";
		case ADI_DEV_RESULT_CMD_NOT_SUPPORTED: 
			return "Command not supported";
		case ADI_DEV_RESULT_INVALID_REG_ADDRESS: 
			return "Accessing invalid Device Register address";
		case ADI_DEV_RESULT_INVALID_REG_FIELD: 
			return "Accessing invalid Device Register field location";
		case ADI_DEV_RESULT_INVALID_REG_FIELD_DATA: 
			return "Providing invalid Device Register field value";
		case ADI_DEV_RESULT_ATTEMPT_TO_WRITE_READONLY_REG: 
			return "Attempt to write a readonly register";
		case ADI_DEV_RESULT_ATTEMPT_TO_ACCESS_RESERVE_AREA: 
			return "Attempt to access a reserved location";
		case ADI_DEV_RESULT_ACCESS_TYPE_NOT_SUPPORTED: 
			return "Access type provided by the driver is not supported";
		case ADI_DEV_RESULT_DATAFLOW_NOT_ENABLED: 
			return "In sync mode, buffers provided before dataflow enabled";
		case ADI_DEV_RESULT_BAD_DIRECTION_FIELD: 
			return "In sequential I/O mode, buffers provided with an invalid direction value";
		case ADI_DEV_RESULT_BAD_IVG: 
			return "Bad IVG number detected";
		case ADI_DEV_RESULT_SWITCH_BUFFER_PAIR_INVALID: 
			return "Invalid buffer pair provided with Switch/Update switch buffer type";
		case ADI_DEV_RESULT_DMA_CHANNEL_UNAVAILABLE: 
			return "No DMA channel is available to process the given command/buffer";
		case ADI_DEV_RESULT_ATTEMPTED_BUFFER_TABLE_NESTING: 
			return "Buffer table nesting is not allowed";
		}
		
        return std::string("Unknown result. Code ") + Utils::IntToString(code);
	}
};	

