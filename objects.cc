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

#include <assert.h>
#include <string.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef MEMORY_HPP
#include "memory.hpp"
#endif

#ifndef INTRPRTR_HPP
#include "intrprtr.hpp"
#endif

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#include "objects.hpp"

/*--------------------------------------------------------------------------*/

Class   clsObject,
        clsSmallInteger, clsReal, clsCharacter,
        clsByteList, clsString, clsList,
        clsModul, clsClass,
        clsAssociation, clsGlobal,
        clsGenericMethod, clsMethods,
        clsCompiledMethod, clsPrimitiveMethod,
        clsContext, clsBlockContext,
        clsBlock, clsError;
List    root, classes, moduls;
List    sysObjs, compObjs;
UINT    objCount, realCount;
Obj     objectList;

/*--------------------------------------------------------------------------*/

Class classOf(Obj obj)
{
  switch (objType(obj)) {

  case otObj:
    if (!obj)
      return clsObject;
    if (obj->_opt.varType != vtReal)
      return obj->iClass;
    else
      return clsReal;

  case otInt:
    return clsSmallInteger;

  case otChar:
    return clsCharacter;

  case otLink:
  default:
    return NULL;
  } 
}


bool isEqual(Obj a, Obj b)
{
  uint t, v;

  if (a == b)
    return true;
  t = objType(a);
  if (t != objType(b))
    return false;
  if ((!a) || (!b))
    return false;
  switch (t) {

  case otObj:
    if (!b)
      return false;
    if (a->_opt.binary)
      return false;
    v = a->_opt.varType;
    if (v != b->_opt.varType)
      return false;
    if (v != vtReal) {
      if (heapCmpD((UINT*)a->ivarAt, (UINT*)b->ivarAt, a->instVars() + 1))
        return false;
      if ((a->at == NULL) != (b->at == NULL))
        return false;
      if (a->at) {
        if (heapCmpD((UINT*)a->at, (UINT*)b->at, limit(a) + 1) != 0)
          return false;
      }
      return true;
    } else
      return (((Real)a)->value == ((Real)b)->value);

  case otInt:
  case otChar:
    return false;

  case otLink:
  default:
    return false;
  }
}


Obj copyOf(Obj o)
{
  Obj theCopy;

  switch (objType(o)) {

  case otObj:
    if (!o)
      return NULL;
    if (o->_opt.binary)
      return o;
    if (o->_opt.varType != vtReal) {
      if (o->iClass == clsClass)
        return o;
      theCopy = o->iClass->createVar(limit(o));
      if (theCopy) {
        heapCopyD(&theCopy->ivarAt[1], &o->ivarAt[1], o->instVars());
        if (o->at)
          heapCopyD(&theCopy->at[1], &o->at[1], limit(o));
      }
      return theCopy;
    } else
      return newReal(((Real)o)->value);

  case otInt:
  case otChar:
    return o;

  case otLink:
  default:
    return NULL;
  }
}

/*--------------------------------------------------------------------------*/

uint THeapObj::size4(uint size)
{
  switch (_opt.varType) {

  case vtBytes:
    return byteLimit(size);

  case vtWords:
    return wordLimit(size);

  case vtObjs:
    return size;

  default:
    return 0;
  }
}

/*--------------------------------------------------------------------------*/

bool TObj::setLimit(uint aLimit)
{
  uint limit;
  Obj  *vars;

  limit = getLimit();
  if (limit == aLimit)
    return true;
  if (aLimit == 0) {
    heapDealloc(at, limit + 1);
    at = NULL;
    return true;
  }
  vars = (Obj*)heapAlloc(aLimit + 1);
  if (!vars)
    return false;
  if (!limit) {
    heapFillD(&vars[1], aLimit, 0);
  checkHalt();
  } else {
    if (limit < aLimit) {
      heapCopyD(&vars[1], &at[1], limit);
      heapFillD(&vars[limit + 1], aLimit - limit, 0);
    } else
      heapCopyD(&vars[1], &at[1], aLimit);
    heapDealloc(at, limit + 1);
  }
  at = vars;
  at[0] = (Obj)aLimit;
  return true;
}

bool TObj::growLimit()
{
  uint l;

  l = getLimit();
  if (l < 4)
    l++;
  else if (l < 16)
    l += 4;
  else if (l < 256)
    l += 16;
  else
    l += 64;
  return setLimit(l);
}

/*--------------------------------------------------------------------------*/

void TIndexedCollection::moveSpace(uint from, uint to, uint count)
{
  if (!count)
    return;
  assert(from > 0 && from <= asInt(size));
  assert(to > 0 && to <= asInt(size));
  switch (_opt.varType) {

  case vtBytes:
    heapMove(&atByte[Byte0+to], &atByte[Byte0+from], count);
    break;

  case vtWords:
    heapMove(&atWord[Word0+to], &atWord[Word0+from], count * sizeof(Word));
    break;

  case vtObjs:
    heapMoveD(&at[to], &at[from], count);
    break;

  default:
    break;
  }
}

bool TIndexedCollection::insSpace(uint index, uint count)
{
  uint Size;

  if (!count)
    return true;
  Size = asUInt(size);
  if (!resizeTo(Size + count))
    return false;
  moveSpace(index, index + count, Size - index + 1);
  return true;
}

bool TIndexedCollection::delSpace(uint index, uint count)
{
  uint Size;

  if (!count)
    return true;
  Size = asUInt(size);
  if (index + count - 1 > Size)
    return false;
  moveSpace(index + count, index, Size - index - count + 1);
  resizeTo(Size - count);
  return true;
}

bool TIndexedCollection::setData(void *data, uint count)
{
  switch (_opt.varType) {

  case vtBytes:
    return (((TByteList*)this)->setData(data, count));

  case vtWords:
    return (((TString*)this)->setData(data, count));

  case vtObjs:
    return (((TList*)this)->setData(data, count));

  default:
    return false;
  }
}

bool TIndexedCollection::resizeTo(uint newSize)
{
  uint Size;

  Size = asUInt(size);
  if (Size == newSize)
    return true;
  if (newSize > Size) {
    if (!setLimit(size4(newSize)))
      return false;
  } else
  switch (_opt.varType) {

  case vtBytes:
    ((TByteList*)this)->fillSpace(newSize + 1, Size - newSize, 0);
    break;

  case vtWords:
    ((TString*)this)->fillSpace(newSize + 1, Size - newSize, 0);
    break;

  case vtObjs:
    ((TList*)this)->fillSpace(newSize + 1, Size - newSize, NULL);
    break;

  default:
    break;
  }
  size = newInt(newSize);
  return true;
}

/*--------------------------------------------------------------------------*/

void TByteList::fillSpace(uint index, uint count, Byte value)
{
  if (!count)
    return;
  assert(index > 0 && index <= asInt(size));
  assert(index + count - 1 <= asInt(size));
  heapFillB(&atByte[Byte0+index], count, value);
}

bool TByteList::setData(void *data, uint count)
{
  if (!setLimit(byteLimit(count)))
    return false;
  heapCopy(&atByte[Byte0+1], data, count);
  size = newInt(count);
  return true;
}


uint TByteList::indexOf(Byte value, uint start)
{
  if (start > 0 && start <= asInt(size))
    return (heapSearchB(&atByte[Byte0+start], asUInt(size) - start + 1, value));
  else
    return 0;
}

/*--------------------------------------------------------------------------*/

void TString::fillSpace(uint index, uint count, Word value)
{
  if (!count)
    return;
  assert(index > 0 && index <= asInt(size));
  assert(index + count - 1 <= asInt(size));
  heapFillW(&atWord[Word0+index], count, value);
}

bool TString::setData(void *data, uint count)
{
  if (!setLimit(wordLimit(count)))
    return false;
  heapCopy(&atWord[Word0+1], data, count * sizeof(Word));
  size = newInt(count);
  return true;
}

char *TString::getStr(char *s)
{
  uint i,Size;
  char *p;

  p = s;
  Size = asUInt(size);
  for (i = 1; i <= Size; i++)
     *p++ = (char)atWord[Word0+i];
  *p = '\0';
  return s;
}

bool TString::setStr(const char *str)
{
  uint i,l;

  l = strlen(str);
  if (!setLimit(wordLimit(l)))
    return false;
  heapFillD(&at[1],getLimit(),0);
  size = newInt(l);
  for (i = 1; i <= l; i++)
        atWord[Word0+i] = str[i - 1];
  return true;
}

uint TString::indexOf(Word value, uint start)
{
  if (start > 0 && start <= asInt(size))
    return (heapSearchW(&atWord[Word0+start], asUInt(size) - start + 1, value));
  else
    return 0;
}

/*--------------------------------------------------------------------------*/

void TList::fillSpace(uint index, uint count, Obj value)
{
  if (!count)
    return;
  assert(index > 0 && index <= asInt(size));
  assert(index + count - 1 <= asInt(size));
  heapFillD(&at[index], count, (UINT)value);
}

bool TList::setData(void *data, uint count)
{
  if (!setLimit(count))
    return false;
  heapCopyD(&at[1], data, count);
  size = newInt(count);
  return true;
}

bool TList::atIns(uint index, Obj value)
{
  if (!insSpace(index, 1))
    return false;
  at[index] = value;
  return true;
}

bool TList::del(Obj o)
{
  uint i;

  i = indexOf(o, 1);
  if (i) {
    atDel(i);
    return true;
  } else
    return false;
}


void TList::ins(Obj value)
{
  uint limit, curSize;

  curSize = asUInt(size);
  limit = getLimit();
  if (curSize == limit)
    growLimit();
  at[curSize + 1] = value;
  size = newInt(curSize+1);
}

void TList::insAll(List from)
{
  uint i, l;

  l = size(from);
  for (i = 1; i <= l; i++)
    ins(from->at[i]);
}

void TList::delAll(List from)
{
  uint i, l;

  l = size(from);
  for (i = 1; i <= l; i++)
    del(from->at[i]);
}

uint TList::indexOf(Obj value, uint start)
{
  if (start > 0 && start <= asUInt(size))
/*
   return (heapSearchD(&at[start], asUInt(size) - start + 1,(UINT)value));
  else
   return 0;
*/
  {
    uint count = asUInt(size) - start + 1;
    Obj *p = &at[start];
    for (uint i = 0; i < count; i++, p++)
      if (*p == value)
        return i + 1;
  }
  return 0;

}

uint TList::indexOfEq(Obj value, uint start)
{
  uint i, l;

  l = asUInt(size);
  for (i = start; i <= l; i++)
    if (isEqual(value, at[i]))
      return i;
  return 0;
}

/*--------------------------------------------------------------------------*/

bool TModul::doInit()
{
  uint i, l;
  Methods mth;
  Modul m;

  l = size(methods);
  for (i = 1; i <= l; i++) {
    mth = (Methods)methods->at[i];
    mth->genericMethod->insAll(mth);
  }
  l = size(imports);
  for (i = 1; i <= l; i++) {
    m = (Modul)imports->at[i];
    m->references = newInt(asInt(m->references) + 1);
  }
  if (init)
    return execute(init);
  return true;
}

void TModul::doEnd()
{
  uint i, l;
  Methods mth;
  Modul m;

  l = size(methods);
  for (i = 1; i <= l; i++) {
    mth = (Methods)methods->at[i];
    mth->genericMethod->delAll(mth);
  }
  l = size(imports);
  for (i = 1; i <= l; i++) {
    m = (Modul)imports->at[i];
    m->references = newInt(asInt(m->references) - 1);
    if (m->references == newInt(0))
      m->doEnd();
  }
  moduls->del(this);
}

/*---------------------------------------------------*/

extern void addNewObj();

Obj newReal(double x)
{
  Real r;

  addNewObj();
  r = (Real)heapAlloc(sizeReal);
  if (!r)
    return NULL;
  r->_opt.varType = vtReal;
  r->value = x;
  r->_next = objectList;
  objectList = (Obj)r;
  realCount++;
  return ((Obj)r);
}

String newStr(const char *str)
{
  String s;

  s = newInst(String);
  if (!s)
    return NULL;
  s->setStr(str);
  return s;
}

char *asStr(char *s, Obj str)
{
  if (str)
    return ((String)str)->getStr(s);
  else {
    *s = '\0';
    return s;
  }
}

/*--------------------------------------------------------------------------*/

Obj TClass::createVar(uint aLimit)
{
  Obj inst;
  uint ivars;

  addNewObj();
  ivars = asUInt(instanceVars);
  inst = (Obj)heapAlloc(sizeObj + ivars);
  if (!inst)
    return NULL;
  heapFillD(&inst->ivarAt[1], ivars, 0);
  inst->iClass = this;
  inst->_opt.varType = (unsigned)(asUInt(options) & 15);
  inst->_opt.binary = (asUInt(options) & optBinary) != 0;
  inst->_opt.marked = 0;
  inst->at = NULL;
  if (!inst->setLimit(aLimit)) {
    heapDealloc(inst,sizeObj + ivars);
    return NULL;
  }
  inst->_next = objectList;
  objectList = inst;
  objCount++;
  return inst;
}

/*--------------------------------------------------------------------------*/

void TInstCache::init(Class cls, uint maxSize)
{
  type = cls;
  cache = newList(maxSize);
}

Obj  TInstCache::create(uint limit)
{
  uint i, j = size(cache);
  Obj  tmp;

  if (j) {
    for (i = 1; i <= j; i++)
     if (limit(tmp = cache->at[i]) >= limit) {
      cache->atDel(i);
      return tmp;
     }
    tmp = cache->at[j];
    cache->delLast();
    tmp->setLimit(limit);
    return tmp;
  } else
    return type->createVar(limit);
}

void TInstCache::free(Obj inst)
{
  if (size(cache) < limit(cache))
    cache->ins(inst);
}

/*--------------------------------------------------------------------------*/

Class createClass(const char *name, uint instVars, uchar opt)
{
  Class newClass;

  newClass = newInst(Class);
  if (!newClass)
    return NULL;
  if (clsString)
    newClass->name = newStr(name);
  newClass->instanceVars = newInt(instVars);
  newClass->options = newInt(opt);
  if (classes) {
    if (*name != '%')
      classes->ins(newClass);
    else
      sysObjs->ins(newClass);
  }
  return newClass;
}

bool createClassCls()
{
  clsClass = (Class)heapAlloc(sizeof(TClass) / ptrSize);
  if (!clsClass)
    return false;
  heapFillD(clsClass, sizeof(TClass) / ptrSize, 0);
  clsClass->_next = NULL;
  clsClass->_opt.varType = vtNone;
  clsClass->_opt.marked = 0;
  clsClass->iClass = clsClass;
  clsClass->instanceVars = newInt(IVars(TClass));
  clsClass->options = newInt(vtNone);
  objectList = (Obj)clsClass;
  objCount++;
  return true;
}

void createStdClasses()
{
  clsObject = createClass("Object", IVars(TObj), vtNone);
  clsSmallInteger = createClass("SmallInteger", 0, vtNone);
  clsReal = createClass("Real", 0, vtNone + optBinary);
  clsCharacter = createClass("Character", 0, vtNone);
  clsByteList = createClass("ByteList", IVars(TByteList), vtBytes);
  clsModul  = createClass("Modul", IVars(TModul), vtNone);
  clsAssociation = createClass("Association", IVars(TAssociation), vtNone);
  clsGlobal = createClass("Global", IVars(TGlobal), vtNone);
  clsGenericMethod = createClass("GenericMethod", IVars(TGenericMethod), vtObjs);
  clsMethods = createClass("Methods", IVars(TMethods), vtObjs);
  clsCompiledMethod = createClass("CompiledMethod", IVars(TCompiledMethod), vtBytes);
  clsPrimitiveMethod = createClass("PrimitiveMethod", IVars(TPrimitiveMethod), vtNone);
  clsContext = createClass("Context", IVars(TContext), vtObjs);
  clsBlockContext = createClass("BlockContext", IVars(TBlockContext), vtObjs);
  clsBlock = createClass("Block", IVars(TBlock), vtNone);
  clsError = createClass("Error", IVars(TError), vtNone);
}

bool objectsInit()
{
  bool tmp;

  if (modulesStarted.objects)
        return true;
  classes = NULL;
  clsString = NULL;
  tmp = disableGC;
  disableGC = true;
#ifdef DEBUG
  GCcalls = 0;
#endif
  if (!createClassCls())
    return false;
  clsString = createClass("", IVars(TString), vtWords);
  clsClass->name = newStr("Class");
  clsString->name = newStr("String");
  clsList = createClass("List", IVars(TList), vtObjs);
  root    = newList(8);
  root->ins(NULL); // CurContext
  root->ins(NULL); // reserved
  root->ins(NULL); // tmp1
  root->ins(NULL); // tmp2
//  halt = true;
//  objectsShow(true);
// heapShow();
  root->ins(classes = newList(16));
  root->ins(moduls = newList(4));
  root->ins(sysObjs = newList(16));
  root->ins(compObjs = newList(16));
  classes->ins(clsClass);
  classes->ins(clsString);
  classes->ins(clsList);

  createStdClasses();
  disableGC = tmp;

  modulesStarted.objects = 1;

  return true;
}


void objectsDone()
{
  if (!modulesStarted.objects)
        return;
  modulesStarted.objects = 0;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
