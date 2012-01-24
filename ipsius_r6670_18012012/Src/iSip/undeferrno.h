#ifndef _UNDEF_ERRNO_IPSIUS_H_
#define _UNDEF_ERRNO_IPSIUS_H_

// При сборке библиотеки rutil из Resiprocate
// возникает конфликт define в файлах 
// ..\resip\rutil\socket.hxx и 
// ..\boost\cerrno.hpp(

#ifdef WIN32

#undef EWOULDBLOCK
#undef EINPROGRESS            
#undef EALREADY                
#undef ENOTSOCK                
#undef EDESTADDRREQ            
#undef EMSGSIZE                
#undef EPROTOTYPE              
#undef ENOPROTOOPT             
#undef EPROTONOSUPPORT         
#undef ESOCKTNOSUPPORT         
#undef EOPNOTSUPP              
#undef EPFNOSUPPORT            
#undef EAFNOSUPPORT            
#undef EADDRINUSE              
#undef EADDRNOTAVAIL           
#undef ENETDOWN                
#undef ENETUNREACH             
#undef ENETRESET               
#undef ECONNABORTED            
#undef ECONNRESET              
#undef ENOBUFS                 
#undef EISCONN                 
#undef ENOTCONN                
#undef ESHUTDOWN               
#undef ETOOMANYREFS            
#undef ETIMEDOUT               
#undef ECONNREFUSED            
#undef ELOOP                   
#undef EHOSTDOWN               
#undef EHOSTUNREACH            
#undef EPROCLIM                
#undef EUSERS                  
#undef EDQUOT                  
#undef ESTALE                  
#undef EREMOTE                 

#endif

#endif
