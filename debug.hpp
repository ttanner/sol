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

#ifndef DEBUG_HPP
#define DEBUG_HPP

/*--------------------------------------------------------------------------*/

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

/*--------------------------------------------------------------------------*/

extern bool halt;
#define checkHalt()     if (halt) doHalt()
extern void doHalt();

extern int console(const char *format, ...);
extern int logfile(const char *format, ...);
extern void printHeap();

#ifdef DEBUG

extern UINT heapAllocs, heapDeallocs;
extern UINT bytecodes;
extern uint GCcalls;

extern void heapTest();
extern void heapShow();
extern void testObjs();
extern UINT showObject(Obj,bool);
extern void objectsShow(bool);
extern bool objectExists(Obj);

#ifndef NOCOMPILER
extern void statOpcode(uint code);
#endif

#else

#define statOpcode(c)

#endif

/*--------------------------------------------------------------------------*/

#endif // DEBUG_HPP
