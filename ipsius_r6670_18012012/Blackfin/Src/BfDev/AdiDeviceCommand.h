#ifndef _BF_ADI_DEVICE_COMMAND_H_
#define _BF_ADI_DEVICE_COMMAND_H_

#include "stdafx.h"

#include <drivers/adi_dev.h>
#include "Utils/ErrorsSubsystem.h"

namespace BfDev
{
    class AdiDeviceCommand 
    {
    public:
        AdiDeviceCommand()  
        {
            m_command.CommandID = ADI_DEV_CMD_END;
            m_command.Value = 0;
        }

        AdiDeviceCommand(u32 command, void *value = 0) 
        {
            m_command.CommandID = command;
            m_command.Value = value;
        }

        AdiDeviceCommand(u32 command, int value) 
        {
            m_command.CommandID = command;
            m_command.Value = reinterpret_cast<void*>(value);
        }
        
        bool IsEmpty() const 
        {
            return m_command.CommandID == ADI_DEV_CMD_END && 
            	!m_command.Value;
        }
           
        void *Command()
        {
            return &m_command;
        }
    private:
        ADI_DEV_CMD_VALUE_PAIR m_command;
    };


    class AdiDeviceCommandsTable
    {
        enum {  MAX_COUNT_COMMANDS = 10 };
    public:
        AdiDeviceCommandsTable() : m_count(0)
        {}
        void Add(const AdiDeviceCommand &command)
        {
            ESS_ASSERT(m_count >=0 && m_count < MAX_COUNT_COMMANDS);

            m_commands[m_count++] = command;
        }
        
        bool IsCorrect() const 
        {
            ESS_ASSERT(m_count >=0 && m_count < MAX_COUNT_COMMANDS);
            return m_commands[m_count].IsEmpty();
        }
        
        void *Table() 
        {
            return &m_commands[0];
        }
    private:
        int m_count;
        AdiDeviceCommand m_commands[MAX_COUNT_COMMANDS];
    };

};

#endif
