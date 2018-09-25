#ifndef MOBULA_INC_CTYPES_H_
#define MOBULA_INC_CTYPES_H_

typedef wchar_t wchar;
typedef char byte;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long longlong;
typedef unsigned long long ulonglong;
typedef long double longdouble;
typedef char* char_p;
typedef wchar_t* wchar_p;
typedef void* void_p;

#ifdef __x86_64__
typedef uint64_t PointerValue;
#else
typedef uint32_t PointerValue;
#endif

#endif  // MOBULA_INC_CTYPES_H_