/* 

 This file is part of the SOL project.
 Copyright © 1995-1997 Thomas Tanner <tanner@gmx.net>
 
 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.
 
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the included GNU General
 Public License file for details.
 
 You should have received a copy of the GNU General Public License
 along with this library.  If not, see <http://www.gnu.org/licenses/>.
 For more details see the file LICENSE.

*/

#ifndef SOL_HPP
#define SOL_HPP

//#define BITS
// 16, 32, 64

//#define OS
#define DOS     1
#define WIN     2
#define OS2     3
#define LINUX   4

//#define IO
#define CONSOLE 1
#define XWIN    2
#define MSWIN   3
#define OS2WIN  4

//#define COMP
#define GNUC    1
#define TURBOC  2
#define WATCOMC 3
#define MSC     4

/*
#define BIGENDIAN
#define NOANSI
#define BOOL
#define NOCOMPILER
#define DEBUG
*/

#if COMP == TURBOC
#define BOOL
#define NOANSI
#endif

#ifdef BOOL
#define false   0
#define true    1
typedef char bool;
#endif

typedef unsigned char uchar;
#if   BITS == 16
typedef unsigned char Byte;     // byte
typedef unsigned int Word;      // word
typedef unsigned long int DWord; // portable index
typedef unsigned long int UINT;
typedef long int INT;
#elif BITS == 32
typedef unsigned char Byte;     // 8 bit
typedef unsigned short Word;   // 16 bit
typedef unsigned int DWord;     // 32 bit
typedef unsigned int UINT;
typedef int INT;
#else // BITS == 64
typedef unsigned char Byte;    // 8 bit
typedef unsigned short int Word; // 16 bit
typedef unsigned int DWord; // 32 bit
typedef unsigned long long int UINT;
typedef long long int INT;
typedef unsigned long long int uint;
#endif

#define ptrSize         (sizeof(void *))
#define ptrBits         (sizeof(void *) << 3)

#define BytesPerPtr     ptrSize
#define WordsPerPtr     (ptrSize / sizeof(Word))

#ifdef BIGENDIAN
#define convw(value)    swapw(value)
#define conv(value)     swap(value)
#else
#define convw(value)    (value)
#define conv(value)     (value)
#endif
//#ifndef NOANSI
#define min(a,b)        ((a) < (b) ? (a) : (b))
#define max(a,b)        ((a) > (b) ? (a) : (b))
//#else
//#include <stdlib.h>
//#endif

#if BITS == 16
#define iabs(x)         labs(x)
#else
#define iabs(x)         abs(x)
#endif

#define errNone         0
#define errIntern       1
#define errNotSupported 2
#define errInit         3
#define errLessMem      16
#define errHeapAlloc    17
#define errHeapFree     18

#define MAX_IDLEN       32
#define MAX_STRLEN      256

#define notSupported()  error(errNotSupported)

extern struct TModules{
  int debug:1;
  int heap:1;
  int objects:1;
  int other:1;
  int interpreter:1;
  int streams:1;
  int codeGen:1;
} modulesStarted;

extern int pargc;
extern char **pargv;
extern bool test, show, debug, disableGC;

extern Word swapw(Word);
extern DWord swap(DWord);

#ifndef NOANSI
#include <string.h>
extern int stricmp(const char *s1,const char *s2);
extern int strnicmp(const char *s1,const char *s2, size_t size);
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

extern void error(uint);
extern void fatalError(uint);

extern void SOLmain();

#endif /*SOL_HPP*/
