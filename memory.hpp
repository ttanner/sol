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

#ifndef MEMORY_HPP
#define MEMORY_HPP

#ifndef SOL_HPP
#include "sol.hpp"
#endif

/*--------------------------------------------------------------------------*/

typedef struct TChunk {
  TChunk *next;
  INT   size;
} TChunk;

/*
inline uint heapSearchD(void *data, uint count, UINT value)
{
  uint i = 0;
  UINT *p = (UINT*)data;
  for (; i < count; i++, p++)
    if (*p == value)
      return i+1;
  return 0;
}
*/

extern TChunk *freeList;
extern INT heapSize;

extern uint ByteLimit(uint);
extern uint WordLimit(uint);

extern void *heapAlloc(uint);
extern void heapDealloc(void *, uint);
extern UINT heapFree();

extern void heapCopy(void *, void *, uint);
extern void heapCopyD(void *, void *, uint);
extern void heapMove(void *, void *, uint);
extern void heapMoveD(void *, void *, uint);
extern void heapFillB(Byte *, uint, Byte);
extern void heapFillW(Word *, uint, Word);
extern void heapFillD(void *, uint, UINT);
extern short heapCmp(void *, void *, uint);
extern INT  heapCmpD(void *, void *, uint);
extern uint heapSearchB(Byte *, uint, Byte);
extern uint heapSearchW(Word *, uint, Word);
extern uint heapSearchD(void *, uint, UINT);

extern void runGC();

#endif /*MEMORY_HPP*/
