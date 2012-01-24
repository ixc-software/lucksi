#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"
#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"
#include "UdpCommandsOfAutoSender.h"

namespace 
{
    using Platform::byte;

    void Copy(std::vector<byte>& out, const byte* in, int size)
    {
        out.clear();        
        
        for (int i = 0; i < size; ++i)
        {
            out.push_back( *(in + i) );
        }
    }
} // namespace 

namespace BfBootCore
{   

    void CmdReload::getBin(std::vector<Platform::byte>& bin)
    {
        Platform::byte cmd[] = "ReloadBoard";
        Copy(bin, cmd, sizeof(cmd));                
    }

    void CmdInfoRequest::getBin(std::vector<Platform::byte>& bin)
    {
        Platform::byte cmd[] = "InfoRequest";
        Copy(bin, cmd, sizeof(cmd));                
    }
} // namespace BfBootCore

