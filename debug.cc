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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#if OS == DOS && BITS == 16
#include <dos.h>
#endif

#ifndef COMPILER_HPP
#include "compiler.hpp"
#endif

#ifndef MEMORY_HPP
#include "memory.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

#ifndef OTHER_HPP
#include "other.hpp"
#endif

#ifndef SYSDEP_HPP
#include "sysdep.hpp"
#endif

#ifndef STREAMS_HPP
#include "streams.hpp"
#endif

#include "debug.hpp"

extern void heapDone();

/*--------------------------------------------------------------------------*/

static FILE     *logf;
bool    halt = false;

void doHalt()
{
//  debugDone();
  heapDone();
  exit(EXIT_SUCCESS);
}

int logfile(const char *format, ...)
{
  va_list param;
  int result;

  va_start(param,format);
  result = vfprintf(logf,format,param);
  va_end(param);
  fflush(logf);
  return result;
}

void TString::show()
{
  char tmp[256];
  getStr(tmp);
  console(tmp); console("\n");
}

void printHeap()
{
  logfile("heap: size=%lu free=%lu used=%lu\n",
           heapSize,heapFree(),heapSize-heapFree());
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#ifdef DEBUG

UINT heapAllocs = 0, heapDeallocs = 0;
UINT bytecodes = 0;
uint GCcalls = 0;
clock_t start;

void initStat();
void printStat();

bool debugInit()
{
  if (modulesStarted.debug)
        return true;
  if (!(logf = fopen("sol.log",WRITETEXT)))
        return false;
  logfile("##### BEGIN ######\n\n");
  initStat();
  modulesStarted.debug = 1;
  return true;
}


void debugDone()
{
  if (!modulesStarted.debug)
        return;
  printStat();
//  objectsShow(true);
//  testObjs();
//  runGC();
  heapShow();
  printHeap();
// objectsShow(false);
  logfile("GC calls: %u \n", GCcalls);
  logfile("\n###### END #######\n");
  fclose(logf);
  modulesStarted.debug = 0;
}

/*--------------------------------------------------------------------------*/

#ifndef NOCOMPILER

bool statOpcodes;
uint opcodeUsage[OPC_LAST];

static const char *opcodeNames[] = {
 "push system",
 "push literal",
 "push block",
 "push instance",
 "push temp",
 "push blockparam",
 "push blocktemp",
 "push instvar",
 "push global",
 "push cast",
 "store temp",
 "store blocktemp",
 "store instvar",
 "store global",
 "pop temp",
 "pop blocktemp",
 "pop instvar",
 "pop global",
 "pop list",
 "pop",
 "return",
 "return block",
 "call",
 "call special",
 "call blockspec",
 "new list",
 "end list",
 "end block"
};

void statOpcode(uint code)
{
  statOpcodes = true;
  opcodeUsage[code]++;
}

#endif

void initStat()
{
  start = clock();
#ifndef NOCOMPILER
  statOpcodes = false;
  memset(&opcodeUsage[0], 0, sizeof(opcodeUsage));
#endif
}

void printStat()
{
  int i;
  clock_t stop;

  stop = clock();
  logfile("\n");
#ifndef NOCOMPILER
  if (statOpcodes) {
   logfile("Bytecodes:\n\n");
   for (i = 0; i < OPC_LAST; i++)
    if (opcodeUsage[i])
     logfile("%-15s : %u\n",opcodeNames[i],opcodeUsage[i]);
   logfile("\n");
  }
#endif
 logfile("heap allocs  : %lu\n", heapAllocs);
 logfile("heap deallocs: %lu\n", heapDeallocs);
 logfile("bytecodes executed: %lu\n", bytecodes);
 console("time used: %f\n",(stop - start) / CLOCKS_PER_SEC);
/*
 logfile("sizeTHeap: %u\n",sizeof(THeapObj));
 logfile("sizeObj: %u\n",sizeObj);
 logfile("sizeTObj: %u\n",sizeof(TObj));
 logfile("sizeTObjOpt: %u\n",sizeof(TObjOptions));
*/
}

/*--------------------------------------------------------------------------*/

void heapShow()
{
  TChunk *cur;
  UINT  size = 0;

  cur = freeList;
  while (cur) {
    logfile("(%p %lu)\n",cur,cur->size);
    size += cur->size;
    cur = cur->next;
  }
  logfile("\n -> %lu bytes \n",size);
}

/*--------------------------------------------------------------------------*/

void printObj(uint n, Obj o)
{
  logfile("\t\t%3d ", n);
  switch (objType(o)) {

  case otObj:
    if (o != NULL && o->_opt.varType == vtReal)
      logfile("real= %6.9f\n", asReal(o));
    else
      logfile("obj-> %08p\n", o);
    break;

  case otInt:
    logfile("int = %d\n", asInt(o));
    break;

  case otChar:
    logfile("chr = %c\n", asChar(o));
    break;
  }
}

UINT showObject(Obj o, bool values)
{
  UINT size;
  uint iv, lim, i;
  uchar vt;
  char tmp[256];

  static const char *vtn[vtObjs - vtNone + 1] = {
    "none", "byte", "word", "", "obj "
  };

#if BITS == 16
  logfile("%08p ", o);
#else
  logfile(" %08p ", o);
#endif
  if (o->_opt.marked)
    putc('x',logf);
  else
    putc('.',logf);
  if (o->_opt.binary)
    putc('x',logf);
  else
    putc('.',logf);
  putc(' ',logf);
  vt = o->_opt.varType;
  if (vt == vtReal) {
    logfile("real = %6.9f\n", ((Real)o)->value);
    size = sizeReal;
    return size;
  }
  iv = o->instVars();
  lim = o->getLimit();
  logfile("%5d %s %5d %s\n",iv, vtn[vt - vtNone], lim,
          asStr(tmp, (Obj)o->iClass->name));
  size = sizeObj + o->instVars();
  if (lim != 0)
    size += lim + 1;
  if (!values)
    return size;
  if (o->iClass != clsString) {
    for (i = 1; i <= iv; i++)
      printObj(i, o->ivarAt[i]);
    logfile("\t\t------------\n");
  }
  switch (vt) {

  case vtBytes:
    if (!lim)
      break;
    lim *= BytesPerPtr;
    logfile("\t\t");
    for (i = 1; i <= lim; i++) {
      logfile("%02X ",o->atByte[Byte0 + i]);
      if (!(i & 15) && i != lim)
        logfile("\n\t\t");
    }
    putc('\n',logf);
    break;

  case vtWords:
    logfile("\t\t\"%s\"\n", asStr(tmp, o));
    break;

  case vtObjs:
    for (i = 1; i <= lim; i++)
      printObj(i, o->at[i]);
    break;
  }
  return size;
}

extern Obj objectList;

void objectsShow(bool values)
{
  Obj cur;
  UINT count= 0, size= 0;

  putc('\n',logf);
  count = 0;
  size = 0;
  cur = objectList;
  logfile("adress    mb ivars type limit class\n");
  while (cur) {
    size += showObject(cur, values);
    count++;
    cur = cur->_next;
  }
  logfile("%lu objects %lu ptrs %lu bytes\n", count, size,
          (UINT)size * sizeof(UINT));
}

/*--------------------------------------------------------------------------*/

static void testAll(Obj from)
{
  uint i, l;

  if ( (!from) || (objType(from) != otObj) || (from->_opt.marked) )
    return;
  if (!objectExists(from)) {
    logfile("FATAL: object doesn't exist!!!\n");
    showObject(from,true);
    objectsShow(true);
    exit(EXIT_FAILURE);
    return;
  }
  from->_opt.marked = 1;
  if (from->_opt.varType == vtReal || from->_opt.binary)
    return;
  l = from->instVars();
  if (l > 50)
    logfile("%p too many instvars?\n",from);
  else
    for (i = 0; i <= l; i++)
      testAll(from->ivarAt[i]);
  if (from->_opt.varType != vtObjs)
    return;
  l = from->getLimit();
  if (l > 3000)
    logfile("%p too many vars?\n",from);
  else
    for (i = 1; i <= l; i++)
      testAll(from->at[i]);
}

void testObjs()
{
  Obj cur;

  testAll((Obj)root);
  cur = objectList;
  while (cur) {
    cur->_opt.marked = 0;
    cur = cur->_next;
  }
}

bool objectExists(Obj obj)
{
  Obj cur;

  if (!obj || objType(obj) != otObj)
    return true;
  cur = objectList;
  while (cur) {
    if (cur == obj)
      return true;
    cur = cur->_next;
  }
  return false;
}

/*--------------------------------------------------------------------------*/

bool TExtStream::debugModuls()
{
  uint  i;
  DWord n;
  char  mn[MAX_IDLEN];
  Modul m;

  logfile("imported moduls:\n");
  for (i = 1; i <= exc->header.moduls; i++) {
    if (!fread(&n,  sizeof(n), 1, exc->stream))
      return false;
    n = conv(n);
    if (fread(&mn[0], sizeof(char), (size_t)n, exc->stream) != n)
      return false;
    mn[(uint)n] = '\0';
    logfile(" %s\n",mn);
    m = loadModul(mn);
    if (!m)
      return false;
    exc->impModuls->ins(m);
  }
  return true;
}

bool TExtStream::debugLinks()
{
  uint  i;
  DWord n;
  Modul m;
  char  mn[256];

  logfile("imported links:\n");
  for (i = 1; i <= exc->header.moduls; i++) {
    m = (Modul)exc->impModuls->at[i];
    logfile("modul %lu %s:\n",i,asStr(mn,m->name));
    for (;;) {
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
      n = conv(n);
      if (!n) break;
      switch (n & 3) {
        case 1 :
         logfile(" class  ");
         exc->impLinks->ins(m->classes->at[(uint)n >> 2]);
         break;
        case 2 :
         logfile(" global ");
         exc->impLinks->ins(m->globals->at[(uint)n >> 2]);
         break;
        case 3 :
         logfile(" genMeth");
         exc->impLinks->ins(m->genericMethods->at[(uint)n >> 2]);
         break;
      }
      logfile(" %08lu\n",n >> 2);
    }
  }
  return true;
}

bool TExtStream::debugObj(Obj *obj, uint nr)
{
  Obj   tmp;
  DWord n;

  logfile("\t\t%4u ", nr);
  if (!fread(&n, sizeof(n), 1, exc->stream))
    return false;
  tmp = (Obj)(uint)conv(n);
  if ((objType(tmp) == otObj) && (tmp)) {
    n = (DWord)(uint)tmp >> 2;
    if (n <= exc->linkStart) {
      logfile("modul %lu\n",n);
      tmp = exc->impModuls->at[(uint)n];
    }
    else if (n > exc->instStart + 2) {
      logfile("obj -> %08lu\n",n - exc->instStart - 2);
      tmp = exc->instances->at[(uint)n - exc->instStart - 2];
    }
    else if (n == exc->instStart + 1) {
      logfile("true\n");
      tmp = objTrue;
    }
    else if (n == exc->instStart + 2) {
      logfile("false\n");
      tmp = objFalse;
    }
    else if (n <= exc->instStart) {
     if (!exc->sysModul) {
       logfile("link -> %08lu\n",n - exc->linkStart);
       tmp = exc->impLinks->at[(uint)n - exc->linkStart];
     }
     else {
       logfile("class -> %08lu\n",n);
       tmp = classes->at[(uint)n];
     }
    }
  } else {
    n >>= 2;
    switch (objType(tmp)) {
     case otObj: logfile("obj -> 00000000\n", n); break;
     case otInt: logfile("int = %li\n", asInt(tmp)); break;
     case otChar: logfile("chr = %c\n", (char)n); break;
     default: logfile("??? %lu\n", (UINT)tmp);
    }
  }
  *obj = tmp;
  return true;
}

bool TExtStream::debugInstances()
{
  uint  i, j, l;
  DWord n;
  Obj   inst;
  char  s[256];

  static const char *vtn[vtObjs - vtNone + 1] = {
    "none", "byte", "word", "", "obj "
  };

// read class of each instance
  logfile("instance classes:\n");
  for (i = 1; i <= exc->header.instances; i++) {
    if (!debugObj(&inst, i))
      return false;
    if (inst != clsReal)
      exc->instances->ins(((Class)inst)->create());
    else
      exc->instances->ins(newReal(0));
  }
  if (exc->sysModul) {
    logfile("sys classes:\n");
    j = size(classes);
    for (i = 1; i <= j; i++) {
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
     n = conv(n);
     if (n)
       exc->instances->at[(uint)n] = classes->at[i];
     logfile("\t\t%4u -> %08lu = %s\n", i, n,
                asStr(s, ((Class)classes->at[i])->name));
    }
  }
// read each instance
  logfile("instances:\n");
  for (i = 1; i <= exc->header.instances; i++) {
    inst = exc->instances->at[i];
    logfile("%08u ", i);
    if (inst->_opt.varType != vtReal) {
      l = inst->instVars();
      logfile("%5d %s %s\n",l, vtn[inst->_opt.varType - vtNone],
                asStr(s,inst->iClass->name));
      for (j = 1; j <= l; j++)
        if (!debugObj(&inst->ivarAt[j], j))
          return false;
      if (inst->_opt.varType == vtNone)
        continue;
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
      n = conv(n);
      logfile("\t\t%lu -------\n",n);
      switch (inst->_opt.varType) {
        case vtBytes:
          inst->setLimit(byteLimit((uint)n));
          if (fread(&inst->atByte[Byte0+1], 1, (size_t)n, exc->stream) != n)
            return false;
          logfile("\t\t");
          for (j = 1; j <= l; j++) {
            logfile("%02X ",inst->atByte[Byte0 + j]);
            if (!(j & 15) && j != l)
              logfile("\n\t\t");
            }
          logfile("\n");
          break;
        case vtWords:
          inst->setLimit(wordLimit((uint)n));
          if (fread(&inst->atWord[Word0+1], sizeof(Word), (size_t)n, exc->stream) != n)
            return false;
#ifdef BIGENDIAN
          for (j = 1; j <= n; j++)
            inst->atWord[Word0+j] = convw(inst->atWord[Word0+j]);
#endif
          logfile("\t\t\"%s\"\n",asStr(s,inst));
          break;
        case vtObjs:
          inst->setLimit((uint)n);
          for (j = 1; j <= n; j++)
            if (!debugObj(&inst->at[j], j))
              return false;
          break;
      }
    } else {    // write real
// ??? PORTABLE ???
      if (!fread(&((Real)inst)->value, sizeof(double), 1, exc->stream))
        return false;
      logfile("real = %6.9f\n", ((Real)inst)->value);
    };
  }
  return true;
}

Obj TExtStream::debug()
{
  if (status != objTrue)
    return NULL;
    //return false;
  if (fread(&exc->header, sizeof(exc->header), 1, exc->stream)) {
    exc->header.moduls = conv(exc->header.moduls);
    exc->header.links = conv(exc->header.links);
    exc->header.instances = conv(exc->header.instances);
    exc->impModuls->setLimit((uint)exc->header.moduls);
    exc->impLinks->setLimit((uint)exc->header.links);
    exc->instances->setLimit((uint)exc->header.instances);
    exc->linkStart = (uint)exc->header.moduls;
    exc->sysModul = (uint)exc->header.moduls == 0;
    if (!exc->sysModul)
      exc->instStart = exc->linkStart + (uint)exc->header.links;
    else {
      exc->instStart = size(classes);
    }
    logfile("moduls    %u\n",exc->header.moduls);
    logfile("links     %u\n",exc->header.links);
    logfile("insts     %u\n",exc->header.instances);
    logfile("linkstart %u\n",exc->linkStart);
    logfile("inststart %u\n",exc->instStart);
    if (debugModuls()) // read imported moduls
     if (debugLinks()) // read imported links
      if (debugInstances()) { // read classes + instances
        Obj result;
            sysObjs->ins(result = exc->instances->at[1]);
        exc->impModuls->resizeTo(0);
        exc->impLinks->resizeTo(0);
        exc->instances->resizeTo(0);
        sysObjs->del(result);
        return result;
        //return true;
      }
  }
  return NULL;
  //return false;
}

/*--------------------------------------------------------------------------*/

#else

bool debugInit()
{
  if (modulesStarted.debug)
        return true;
/*
  if (!(logf = fopen("SOL.log",WRITETEXT)))
        return false;
  logfile("##### BEGIN ######\n\n");
*/
  modulesStarted.debug = 1;
  return true;
}

void debugDone()
{
  if (!modulesStarted.debug)
        return;
/*
  logfile("\n###### END #######\n");
  fclose(logf);
*/
}

/*--------------------------------------------------------------------------*/

#endif

/*--------------------------------------------------------------------------*/
