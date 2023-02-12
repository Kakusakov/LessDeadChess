/*
* Macros:
*   DEBUG:
*     Description:
*		Turns on addintional engine state messages.
* 
*   LOG_TO_FILE:
*     Description:
*       Creates a log file instead of writing to the standard output.
* 
*	DEBUG_VERBOSE:
*	  Defines:
*		[ DEBUG, LOG_TO_FILE ]
*     Description:
*       Create a log file with more debug data than usual DEBUG.
* 
*   SPLIT_SLIDER_GENERATION:
*	  Description:
*       Allow generating slider attacks and slider quiets separately.
* 
*   MOVE_ORDERING_MODE_ATTACKS_AND_QUIETS:
*     Defines:
*       [ SPLIT_SLIDER_GENERATION ]
*     Description:
*       Use a move ordering strategy that splits attacks' and 
*         quiets' generation.
* 
*/
#pragma once
typedef unsigned __int64 U64;    // for the old microsoft compilers 
typedef unsigned long long  U64; // supported by MSC 13.00+ and C99 
typedef unsigned char BYTE;
typedef unsigned short WORD;
#define C64(constantU64) constantU64##ULL
#define MIN(a, b) a > b ? b : a

// define macro dependencies

#if defined DEBUG_VERBOSE 
#if !defined DEBUG
#define DEBUG
#endif

#if !defined LOG_TO_FILE
#define LOG_TO_FILE
#endif
#endif

#if defined MOVE_ORDERING_MODE_ATTACKS_AND_QUIETS
#if !defined SPLIT_SLIDER_GENERATION
#define SPLIT_SLIDER_GENERATION
#endif
#else

#endif

#if defined DEBUG
#include <iostream>
#endif
