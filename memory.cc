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

#define NOHEAPCACHE
//#define NODYNHEAP
//#define NOSORTFREE
//#define NOBESTMATCH

#define minHeap 200L
#define maxHeap 1024L

#define growSize 61440L

#define maxItems        32
#define minSize         2  // ptrs
#define maxCache        32 // ptrs

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if OS == DOS && BITS == 16
#include <dos.h>
#endif

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#include "memory.hpp"

INT heapSize, heapAvail;

extern void runGC();

/*--------------------------------------------------------------------------*/

TChunk  *freeList;
#ifdef NODYNHEAP
TChunk  *heapStart;
#else
static void *heapBlocks;
#endif

/*--------------------------------------------------------------------------*/

static void *primAlloc(UINT count);
static void primDealloc(void *data, UINT count);
static UINT primFree();

#ifndef NOHEAPCACHE
typedef struct TCache {
  UINT  *item[maxItems];
  uint  count;
} TCache;

static TCache   cache[maxCache-minSize+1];
#endif

void *heapAlloc(uint count)
{
  UINT *tmp;

  if (!count)
    return NULL;
  if (count < minSize)
    count = minSize;
#ifndef NOHEAPCACHE
  if (count <= maxCache) {
    if (!cache[count-minSize].count) {
      tmp = (UINT*)primAlloc((maxItems+1)*(UINT)count*sizeof(UINT));
       // maxItems + one new
      cache[count-minSize].count = maxItems;
      uint i;
      for (i = 0; i < maxItems; i++) {
        cache[count-minSize].item[i] = tmp;
        tmp += count;
      }
    } else
      tmp = cache[count-minSize].item[--cache[count-minSize].count];
  } else
#endif
    tmp = (UINT*)primAlloc((UINT)count*sizeof(UINT));
  heapAvail -= (UINT)count*sizeof(UINT);
  assert(heapAvail >= 0);
  return tmp;
}

void heapDealloc(void *data, uint count)
{
  if (!count)
    return;
  if (count < minSize)
    count = minSize;
#ifndef NOHEAPCACHE
  if (count <= maxCache && cache[count-minSize].count != maxItems)
    cache[count-minSize].item[cache[count-minSize].count++] = (UINT*)data;
  else
#endif
    primDealloc(data, (UINT)count*sizeof(UINT));
  heapAvail += (UINT)count*sizeof(UINT);
  assert(heapAvail >= 0);
}

UINT heapFree()
{
  return heapAvail;//primFree();
}

/*--------------------------------------------------------------------------*/

#ifndef NODYNHEAP

bool heapInit()
{
  if (modulesStarted.heap)
    return true;
#if COMP == TURBOC
  logfile("free dos heap: %lu\n", coreleft());
#endif
  freeList = NULL;
  heapBlocks = NULL;
  heapSize = 0;
  heapAvail = 0;
#ifndef NOHEAPCACHE
  uint i;
  for (i = minSize; i <= maxCache; i++)
    cache[i-minSize].count = 0;
#endif
  modulesStarted.heap = 1;
  return true;
}

void heapDone()
{
  void *cur, *next;

  if (!modulesStarted.heap)
    return;
  cur = heapBlocks;
  while (cur) {
   next = *(void**)cur;
   free(cur);
   cur = next;
  }
  modulesStarted.heap = 0;
}

#else

bool heapInit()
{
  uint bad;
  UINT s;

  if (modulesStarted.heap)
    return true;
#if COMP == TURBOC
  logfile("free dos heap: %lu\n", coreleft());
#endif
#if BITS != 16
  heapSize = maxHeap << 10;
  heapStart = (TChunk*)malloc((size_t)heapSize);
  if (!heapStart) {
    for (s = maxHeap - 4; s >= minHeap; s -= 4) {
     heapSize = s << 10;
     heapStart = (TChunk*)malloc((size_t)heapSize);
     if (heapStart)
       break;
    }
    if (!heapStart)
      return false;
  }
  freeList = heapStart;
  bad = (uint)((UINT)heapStart & (BytesPerPtr - 1));
  if (bad)
    freeList = (TChunk*)((uchar*)heapStart + ptrSize - bad);
  freeList->next = NULL;
  freeList->size = heapSize;
#else
  TChunk *cur = freeList, *last = freeList;
  UINT  count = heapSize;

  while (count) {
    cur->size = min(count,0x10000L-FP_OFF(cur));
    count -= cur->size;
    last->next = cur;
    last = cur;
    cur = (TChunk*)MK_FP(FP_SEG(cur) + 0x1000,0);
  }
  last->next = NULL;
/*
  while (count > (cur->size = min(0x10000L,count) - FP_OFF(cur))) {
    count -= cur->size;
    last->next = cur;
    last = cur;
    cur = (TChunk*)MK_FP(FP_SEG(cur) + 0x1000,0);
  }
  last->next = cur;
  cur->next = NULL;
*/
#endif
  heapAvail = heapSize;
#ifdef HEAPCACHE
  uint i;
  for (i = minSize; i <= maxCache; i++)
    cache[i-minSize].count = 0;
#endif
  modulesStarted.heap = 1;
  return true;
}

void heapDone()
{
  if (!modulesStarted.heap)
    return;
  free(heapStart);
}

#endif

/*--------------------------------------------------------------------------*/

#ifndef NODYNHEAP

static bool growHeap(uint min)
{
  void *block = NULL;
  uint size;

  min += ptrSize;
  if (min < growSize) {
    block = malloc((size_t)growSize+ptrSize);
    size = (uint)growSize;
  }
  if (!block) {
    block = malloc((size_t)min+ptrSize);
    size = min;
  }
  if (!block)
    return false;
  *(void**)block = heapBlocks;
  heapBlocks = block;
  uchar *p = (uchar*)block + ptrSize;
  uint bad = (uint)((UINT)p & (ptrSize - 1));
  if (bad)
    p += ptrSize - bad;
#ifdef NOSORTFREE
  TChunk *c = (TChunk*)p;
  TChunk *cur, *last;
  cur = last = freeList;
  while (cur) {
    last = cur;
    cur = cur->next;
  };
  if (last)
    last->next = c;
  else
    freeList = c;
  c->next = NULL;
  c->size = size;
#else
  primDealloc(p, size);
#endif
  heapSize += size;
  heapAvail += size;
  assert(heapAvail >= 0);
  assert(heapSize >= 0);
  return true;
}

#endif

#define DELENTRY() if (cur != last) last->next = cur->next; else freeList = cur->next;

static void *primAlloc(UINT count)
{
  TChunk *cur,*last;
search:
  cur = last = freeList;
#ifdef DEBUG
  heapAllocs++;
#endif
#if BITS == 16
  if (count > 65530L)
    return false;
#endif
#ifndef NOBESTMATCH
  TChunk *best = NULL;

  while (cur) {
    if (cur->size == count) {
      DELENTRY();
      return cur;
    };
    if (!best && cur->size - sizeof(UINT)*2 >= count)
      best = cur;
    last = cur;
    cur = cur->next;
  };
  if (best) {
    best->size -= count;
    return ((uchar*)best + (uint)best->size);
  }
#else
  while (cur) {
    if (cur->size == count) {
      DELENTRY();
      return cur;
    };
    if (cur->size - sizeof(UINT)*2 >= count) {
      cur->size -= count;
      return ((uchar*)cur + cur->size);
    };
    last = cur;
    cur = cur->next;
  };
#endif
#ifndef NODYNHEAP
  if (growHeap((uint)count))
    goto search;
#endif
  assert(false);
  return NULL;
}

static void primDealloc(void *data, UINT count)
{
  TChunk *cur,*last;

#ifdef DEBUG
  heapDeallocs++;
#endif
  #ifdef NOSORTFREE
search:
  cur = last = freeList;
  while (cur) {
    if ((uchar*)data + count == cur) {
      count += cur->size;
      DELENTRY();
      goto search;
    };
    if (data == (uchar*)cur + cur->size) {
      count += cur->size;
      data = cur;
      DELENTRY();
      goto search;
    };
    last = cur;
    cur = cur->next;
  }
  cur = (TChunk*)data;
  cur->size = count;
  cur->next = freeList;
  freeList = cur;
#else
  cur = last = freeList;
  TChunk *tmp = (TChunk*)data;
#if BITS == 16
  while (cur && (FP_SEG(cur) < FP_SEG(data) ||
        FP_SEG(cur) == FP_SEG(data) && FP_OFF(cur) < FP_OFF(data) )) {
#else
  while (cur && (cur < data)) {
#endif
    last = cur; cur = cur->next;
  }
  if (!cur) {
    if (last)
      if ((uchar*)data == (uchar*)last + (uint)last->size)
        last->size+= count;   // ld
      else {
        last->next = tmp;     // l d
        tmp->size = count;
        tmp->next = NULL;
      }
    else {
        freeList = tmp;     // d
        tmp->size = count;
        tmp->next = NULL;
    }
  } else
  if (last == cur) {
    if ((uchar*)cur == (uchar*)data + (uint)count) {
      tmp->next = cur->next; // dc
      tmp->size = count + cur->size;
    } else {
      tmp->next = cur;  // d c
      tmp->size = count;
    }
    freeList = tmp;
  } else
  if ((uchar*)data == (uchar*)last + (uint)last->size) {
    if ((uchar*)cur == (uchar*)data + (uint)count) {
      last->next = cur->next; // ldc
      last->size+= count + cur->size;
    } else
      last->size+= count;     // ld c
  } else
  if ((uchar*)cur == (uchar*)data + (uint)count) {
    last->next = tmp;        // l dc
    tmp->next = cur->next;
    tmp->size = count + cur->size;
  } else {
    last->next = tmp;        // l d c
    tmp->next = cur;
    tmp->size = count;
  }
#endif
}

static UINT primFree()
{
  UINT count=0;
  TChunk *cur= freeList;

  while (cur) {
    count += cur->size;
    cur = cur->next;
  };
  return count;
};

/*--------------------------------------------------------------------------*/

void heapCopy(void *dest, void *src, uint count)
{
  memcpy(dest,src,count);
}

void heapCopyD(void *dest, void *src, uint count)
{
  memcpy(dest,src,count*sizeof(UINT));
}

void heapMove(void *dest, void *src, uint count)
{
  memmove(dest,src,count);
}

void heapMoveD(void *dest, void *src, uint count)
{
  memmove(dest,src,count*sizeof(UINT));
}


void heapFillB(Byte *dest, uint count, Byte value)
{
  memset(dest,value,count);
}

void heapFillW(Word *dest, uint count, Word value)
{
  while (count--) *dest++ = value;
}

void heapFillD(void *dest, uint count, UINT value)
{
  UINT *p = (UINT*)dest;
  while (count--) *p++ = value;
}


short heapCmp(void *a, void *b, uint count)
{
  return memcmp(a,b,count);
}

INT heapCmpD(void *a, void *b, uint count)
{
  UINT *p1 = (UINT*)a, *p2 = (UINT*)b;
  while (count--) {
    INT d = *p1++ - *p2++;
    if (d) return d;
  }
  return 0;
}

uint heapSearchB(Byte *data, uint count, Byte value)
{
  for (uint i = 0; i < count; i++)
    if (data[i] == value)
      return i+1;
  return 0;
}

uint heapSearchW(Word *data, uint count, Word value)
{
  for (uint i = 0; i < count; i++)
    if (data[i] == value)
      return i+1;
  return 0;
}


uint heapSearchD(void *data, uint count, UINT value)
{
  uint i = 0;
  UINT *p = (UINT*)data;
  for (; i < count; i++, p++)
    if (*p == value)
      return i+1;
/*
  for (uint i = 0; i < count; i++)
    if (((UINT*)data)[i] == value)
      return i+1;
*/
  return 0;
}

/*--------------------------------------------------------------------------*/

#define maxNewObjs      1024

static uint newObjs;

void addNewObj()
{
  newObjs++;
  if (disableGC)
    return;
  if (newObjs <= maxNewObjs)
    return;
  newObjs = 0;
#ifdef DEBUG
/*
  UINT before = heapFree();
  logfile("running GC...(%lu,%lu->",heapSize,before);
  runGC();
  logfile("%lu=%lu)\n",heapFree(),heapFree()-before);
  heapShow();
*/
  runGC();
#else
  runGC();
#endif
}

static void markAll(Obj from)
{
  uint i, l;

  if ( (objType(from) != otObj) || !from || from->_opt.marked)
    return;
  from->_opt.marked = 1;
  if (from->_opt.varType == vtReal || from->_opt.binary)
    return;
  l = from->instVars();
  for (i = 0; i <= l; i++)
    markAll(from->ivarAt[i]);
  if (from->_opt.varType != vtObjs)
    return;
  l = limit(from);
  for (i = 1; i <= l; i++)
    markAll(from->at[i]);
}

void runGC()
{
  Obj cur, last, tmp;

//  objectsShow(true);
  markAll((Obj)root);
//  objectsShow(true);
  cur = last = objectList;
  while (cur) {
    if (!cur->_opt.marked) {
      tmp = cur;
      cur = cur->_next;
      if (tmp != last)
        last->_next = cur;
      else {
        objectList = cur;
        last = cur;
      }
      if (tmp->_opt.varType != vtReal) {
        if (tmp->at)
          heapDealloc(tmp->at, limit(tmp) + 1);
        heapDealloc(tmp, sizeObj + tmp->instVars());
        objCount--;
      } else {
        heapDealloc(tmp, sizeReal);
        realCount--;
      }
    } else {
      cur->_opt.marked = 0;
      last = cur;
      cur = cur->_next;
    }
  }
#ifdef DEBUG
  GCcalls++;
#endif
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
