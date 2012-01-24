#ifndef COMADDR_H
#define COMADDR_H

namespace BfBootDRI
{
    // Format: COM num @ rate
    bool ResolveComAddr(QString addr, int& num, int& rate);    
} // namespace BfBootDri

#endif
