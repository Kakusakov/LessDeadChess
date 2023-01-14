#pragma once
typedef unsigned __int64 U64;    // for the old microsoft compilers 
typedef unsigned long long  U64; // supported by MSC 13.00+ and C99 
typedef unsigned char BYTE;
typedef unsigned short WORD;
#define C64(constantU64) constantU64##ULL
#define MIN(a, b) a > b ? b : a

//#define DEBUG
//#define DEBUG_VERBOSE

#if defined DEBUG_VERBOSE and !defined DEBUG
#define DEBUG
#endif

#if defined DEBUG_VERBOSE and !defined LOG_TO_FILE
#error Please use logging if you want a verbose debug.
#endif
