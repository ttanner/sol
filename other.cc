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

#include <sys/stat.h>
#include <stdlib.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef MEMORY_HPP
#include "memory.hpp"
#endif

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#include "other.hpp"

Obj     objNil, objTrue, objFalse;
Class   clsBoolean, clsTime, clsLargeInt, clsDictionary;

LargeInt largeZero;

/*--------------------------------------------------------------------------*/

bool otherInit()
{
  if (modulesStarted.other)
        return true;
  objNil = NULL;
  clsBoolean = createClass("Boolean", IVars(TObj), vtNone);
  clsTime = createClass("Time", IVars(TTime), vtNone);
  clsLargeInt = createClass("LargeInteger", IVars(TLargeInt), vtBytes);
  clsDictionary = createClass("Dictionary", IVars(TDictionary), vtNone);
  sysObjs->ins(objTrue = clsBoolean->create());
  sysObjs->ins(objFalse = clsBoolean->create());
  sysObjs->ins(largeZero = newInst(LargeInt));
  largeZero->sign = newInt(0);
  modulesStarted.other = 1;
  return true;
}


void otherDone()
{
  if (!modulesStarted.other)
        return;
  modulesStarted.other = 0;
}

/*--------------------------------------------------------------------------*/

#define BaseDay 693960L

const uint DaysToMonth[12]=
       {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

bool isLeapYear(uint year)
{
  return ((year & 3) == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

uint leapYearsTo(uint year)
{
  return (year-1901) / 4 - (year-1901) / 100 + (year-1601) / 400;
}

void TTime::set(time_t t)
{
  tm tmbuf;
  UINT d;
  
  tmbuf = *localtime(&t);
  d = (DaysToMonth[tmbuf.tm_mon]) + tmbuf.tm_mday;
  if (isLeapYear(tmbuf.tm_year + 1900) && tmbuf.tm_mon > 2)
    d++;
  days  = newInt(d + BaseDay + (UINT)tmbuf.tm_year * 365
                 + leapYearsTo(tmbuf.tm_year + 1900));
  msecs = newInt(((UINT)tmbuf.tm_hour * 3600
                 + (UINT)tmbuf.tm_min * 60 + tmbuf.tm_sec) * 1000);
}

Time TTime::now()
{
  set(time(NULL));
  return this;
}

bool TTime::ofFile(const char *fname)
{
  struct stat info;

  if (stat(fname,&info))
    return false;
  set(info.st_ctime);
  return true;
}

bool TTime::after(Time other)
{
  if (days != other->days)
    return asInt(days) > asInt(other->days);
  else
    return asInt(msecs) > asInt(other->msecs);
}

bool TTime::before(Time other)
{
  if (days != other->days)
    return asInt(days) < asInt(other->days);
  else
    return asInt(msecs) < asInt(other->msecs);
}

/*--------------------------------------------------------------------------*/

LargeInt newLInt(INT value)
{
  if (!value)
    return largeZero;
  LargeInt result = (LargeInt)clsLargeInt->createVar(1);
#ifdef BIGENDIAN
  UINT x = abs(asInt(value));
  Byte *p = &result->atByte[Byte0];
  while (x) {
    *p++ = x & 0xFF;
    x>>= 8;
  };
#else
  result->at[1] = (Obj)(INT)iabs(asInt(value));
#endif
  if (asInt(value) > 0)
    result->sign = newInt(1);
  else
    result->sign = newInt(-1);
  return result;
}

UINT    TLargeInt::bitSize()
{
  uint i = byteSize();
  if (!i)
    return 0;
  Byte b = atByte[Byte0 + i];
  uint j = 8;
  while (b) { b >>= 1; j--; }
  return i * 8 + j;
}

Obj     TLargeInt::simplified()
{
  return NULL;
}

Real    TLargeInt::toReal()
{
  return NULL;
}

bool    TLargeInt::less(LargeInt other)
{
  if (asInt(sign) != asInt(other->sign))
    return asInt(sign) < asInt(other->sign);
  if (asInt(sign) == 1)
    return primLess(other);
  else
    return primGreater(other);
}

bool    TLargeInt::greater(LargeInt other)
{
  if (asInt(sign) != asInt(other->sign))
    return asInt(sign) > asInt(other->sign);
  if (asInt(sign) == 1)
    return primGreater(other);
  else
    return primLess(other);
}

LargeInt TLargeInt::add(LargeInt other)
{
  if (sign == newInt(0))
    return other;
  if (other->sign == newInt(0))
    return this;
  if (sign == other->sign) {
    if (sign == newInt(1))
      return primAdd(other);
    else
      return primAdd(other)->negate();
  } else {
    if (sign == newInt(1))
      return primSub(other);
    else
      return other->primSub(this);
  }
}

LargeInt TLargeInt::sub(LargeInt other)
{
  if (sign == newInt(0)) {
    if (other->sign == newInt(0))
      return largeZero;
    LargeInt result = (LargeInt)copyOf(other);
    return result->negate();
  }
  if (other->sign == newInt(0))
    return this;
  if (sign == other->sign) {
    if (sign == newInt(1))
      return primSub(other);
    else
      return other->primSub(this);
  } else {
    if (sign == newInt(1))
      return primAdd(other);
    else
      return primAdd(other)->negate();
  }
}

LargeInt TLargeInt::mul(LargeInt other)
{
  if (sign == newInt(0))
    return largeZero;
  if (other->sign == newInt(0))
    return largeZero;
  LargeInt tmp = primMul(other);
  if (sign != other->sign)
    tmp->sign = newInt(-1);
  return tmp;
}

LargeInt TLargeInt::div(LargeInt other)
{
  LargeInt div, rest;

  if (sign == newInt(0))
    return largeZero;
  div = primDiv(other, &rest);
  if (sign != other->sign)
    div->sign = newInt(-1);
  return div;
}

LargeInt TLargeInt::mod(LargeInt other)
{
  LargeInt tmp;

  primDiv(other, &tmp);
  if (sign != other->sign)
    tmp->sign = newInt(-1);
  return tmp;
}

LargeInt TLargeInt::operator &(LargeInt other)
{
  if (sign == newInt(0))
    return largeZero;
  if (other->sign == newInt(0))
    return largeZero;
  return primAnd(other);
}

LargeInt TLargeInt::operator |(LargeInt other)
{
  if (sign == newInt(0))
    return other;
  if (other->sign == newInt(0))
    return this;
  return primOr(other);
}

LargeInt TLargeInt::operator ^(LargeInt other)
{
  if (isEqual(this,other))
    return largeZero;
  return primXor(other);
}

LargeInt TLargeInt::shift(INT delta)
{
  uint size, i, count, shift, nshift;

  if (sign == newInt(0))
    return largeZero;
#if BITS == 16
  if (iabs(delta) > 65535L)
    return NULL;
#endif    
  count = (uint)(iabs(delta) / 8);
  shift = (uint)(iabs(delta) & 7);
  nshift = 8 - shift;
  size = byteSize();
  LargeInt result = (LargeInt)clsLargeInt->
                createVar(byteLimit(size + (int)((delta + 7) % 8)));
  if (!result)
    return NULL; 
  Byte *r = &result->atByte[Byte0];
  if (delta >= 0) {
    if (shift) {
      for (i = size; i > 1; i--)
        r[i+count] = (Byte)(r[i] << shift) | (Byte)(r[i-1] >> nshift);
      r[count+1] = (Byte)(r[1] << shift);
    } else
      heapMove(&r[count+1],&r[1],size-count+1);
    heapFillB(&r[1],count,0);
  } else {
    if (shift) {
      for (i = count+1; i < size; i++)
        r[i-count] = (Byte)(r[i] >> shift) | (Byte)(r[i+1] << nshift);
      r[size-count] = (Byte)(r[size] << shift);
    } else
      heapMove(&r[1],&r[count+1],size-count+1);
  };
  result->sign = sign;
  return result->reduce();
}

/*--------------------------------------------------------------------------*/

int  TLargeInt::signedAt(uint index)
{
  int s = (int)asInt(sign);
  if (index <= getLimit()*BytesPerPtr)
    return atByte[Byte0 + index] * s;
  else
    return s;
}

uint TLargeInt::byteSize()
{
  uint i = getLimit();
  while (i && at[i]) i--;
  if (!i)
    return 0;
  uint j = BytesPerPtr;
  UINT x = (UINT)at[i];
  while (x) { x >>= 8; j--; }
  return i * BytesPerPtr + j;
}

uint TLargeInt::getSize()
{
  uint i = getLimit();
  while (i && at[i]) i--;
  return i;
}

LargeInt TLargeInt::primAbs()
{
  if (sign == newInt(-1))
    sign = newInt(1);
  return this;
}

LargeInt TLargeInt::negate()
{
  sign = newInt(-asInt(sign));
  return this;
}

LargeInt TLargeInt::reduce()
{
  uint i = getLimit();
  while (i && at[i]) i--;
  if (!i)
    return largeZero;
  else
    return this;
}

bool TLargeInt::primLess(LargeInt other)
{
  uint i = getSize();
  if (i != other->getSize())
    return i < other->getSize();
  while (i && (at[i] == other->at[i])) i--;
  if (i)
    return at[i] < other->at[i];
  else
    return false;
}

bool TLargeInt::primGreater(LargeInt other)
{
  uint i = getSize();
  if (i != other->getSize())
    return i > other->getSize();
  while (i && (at[i] == other->at[i])) i--;
  if (i)
    return at[i] > other->at[i];
  else
    return false;
}

LargeInt TLargeInt::primAdd(LargeInt other)
{
  uint i, sum;
  Byte *a, *b;
  Byte carry;

  uint l = max(getSize(),other->getSize());
  LargeInt result = (LargeInt)clsLargeInt->createVar(l);
  heapCopyD(&result->at[1],&at[1],getSize());
  a = &result->atByte[Byte0 + 1];
  b = &other->atByte[Byte0 + 1];
  i = l*BytesPerPtr; carry = 0;
  while (i--) {
    sum = *a + *b++ + carry;
    *a++ = (Byte)(sum & 0xFF);
    carry = (Byte)(sum >> 8);
  }
  if (carry) {
    result->setLimit(l+1);
    result->atByte[Byte0 + l*BytesPerPtr + 1] = carry;
  }
  result->sign = newInt(1);
  return result;
}

LargeInt TLargeInt::primSub(LargeInt other)
{
  Byte *a, *b;

  uint s = getSize();
  LargeInt result = (LargeInt)clsLargeInt->createVar(s);
  heapCopyD(&result->at[1],&at[1],s);
  a = &result->atByte[Byte0 + 1];
  b = &other->atByte[Byte0 + 1];
  uint i = other->getSize()*BytesPerPtr;
  while (i--) {
    if (*a < *b) {
      uint j = i;
      Byte *c = a+1;
      while (j--) {
        if (*c)
          *(c++) -= 1;
        else
          *c++ = 0xFF;
      }
      *a += 256 - *b++;
    } else
      *a -= *b++;
    a++;
  }
  result->sign = newInt(1);
  return result;
}

LargeInt TLargeInt::primMul(LargeInt other)
{
  uint outer, inner, i, j;
  Byte *a, *b, *c;

  outer = getSize();
  inner = other->getSize();
  LargeInt result = (LargeInt)clsLargeInt->createVar(inner + outer);
  a = &atByte[Byte0 + 1];
  b = &other->atByte[Byte0 + 1];
  for (i = 0; i < outer; i++)
    for (j = 1; j <= inner; j++) {
      uint n = *a * *b;
      c = &result->atByte[Byte0 + i + j];
      while (n) {
        n += *c;
        *c++ = (Byte)(n & 0xFF);
        n >>= 8;
      }
    }
  result->sign = newInt(1);
  return result;
}

LargeInt TLargeInt::primDiv(LargeInt other, LargeInt *rest)
{ // this und other von root ref.
  LargeInt q, d, u, v, t, t2;
  uint n, m, qHat, r1, r2, r3, r4, j, jj;
  UINT sh;

  List tmp = newList(4);
  sysObjs->ins(tmp);
  if (byteSize() < other->byteSize()) {
    *rest = this;
    return largeZero;
  }
  d = other;
  sh = 8 - d->bitSize();
  tmp->ins(u = shift(sh)->primAbs());
  tmp->ins(v = d->shift(sh)->primAbs());
  n = v->byteSize();
  m = u->byteSize() - n + 1;
  tmp->ins(q = (LargeInt)clsLargeInt->createVar(byteLimit(m)));
  tmp->ins(t = (LargeInt)copyOf(q));
  r2 = v->atByte[Byte0 + v->byteSize()];
  j = m;
  while (j) {
    jj = j + n;
    r1 = u->atByte[Byte0 + jj];
    if (r1 != r2) {
      r4 = (r1 << 8) + u->atByte[Byte0 + jj - 1];
      qHat = r4 / r2;
      r3 = u->atByte[Byte0 + jj - 2];
      while (qHat*v->atByte[Byte0+n-1] > (((r4-qHat*r2) << 8) +r3)) qHat--;
    } else
      qHat = 255;
    t->atByte[Byte0 + j] = (Byte)qHat;
    t2 = v->mul(t);
    if (u->less(t2)) {
      t->atByte[Byte0 + j]--; qHat--;
      t2 = u->sub(v->mul(t));
    }
    tmp->at[1] = u = u->sub(t2);
    q->atByte[Byte0 + j] = (Byte)qHat;
    j--;
  }
  tmp->at[1] = u = u->shift(-sh);
  if (q->sign == newInt(-1) && u->sign != newInt(0))
    tmp->at[1] = u = d->sub(u);
  *rest = u;
  sysObjs->del(tmp);
  return q;
}

LargeInt TLargeInt::primAnd(LargeInt other)
{
  uint l = min(getSize(),other->getSize());
  LargeInt result = (LargeInt)clsLargeInt->createVar(l);
  Byte *r = &result->atByte[Byte0 + 1];
  l *= BytesPerPtr;
  for (uint i = 1; i <= l; i++)
    *r++ = signedAt(i) & other->signedAt(i);
  result->sign = sign;
  return result;
}

LargeInt TLargeInt::primOr(LargeInt other)
{
  uint l = max(getSize(),other->getSize());
  LargeInt result = (LargeInt)clsLargeInt->createVar(l);
  Byte *r = &result->atByte[Byte0 + 1];
  l *= BytesPerPtr;
  for (uint i = 1; i <= l; i++)
    *r++ = signedAt(i) | other->signedAt(i);
  result->sign = sign;
  return result;
}

LargeInt TLargeInt::primXor(LargeInt other)
{
  uint l = max(getSize(),other->getSize());
  LargeInt result = (LargeInt)clsLargeInt->createVar(l);
  Byte *r = &result->atByte[Byte0 + 1];
  l *= BytesPerPtr;
  for (uint i = 1; i <= l; i++)
    *r++ = signedAt(i) ^ other->signedAt(i);
  result->sign = sign;  
  return result;
}

/*--------------------------------------------------------------------------*/

UINT hashFor(Obj obj)
{
  uint i, s;
  UINT h;

  if (obj == NULL)
    return 0;
  h = 0;
  switch (objType(obj)) {

  case otInt:
  case otChar:
    return (UINT)obj;

  case otObj:
    switch (obj->_opt.varType) {

    case vtNone:
      if (obj->instVars())
        return (UINT)obj->ivarAt[1];
      else
        return (UINT)obj;

    case vtBytes:
      s = limit(obj) * BytesPerPtr;
      if (s > 8)
        s = 8;
      for (i = 1; i <= s; i++)
        h += obj->atByte[Byte0+i];
      return h;

    case vtWords:
      s = limit(obj) * WordsPerPtr;
      if (s > 8)
        s = 8;
      for (i = 1; i <= s; i++)
        h += obj->atWord[Word0+i];
      return h;

    case vtObjs:
      s = limit(obj);
      if (s > 8)
        s = 8;
      for (i = 1; i <= s; i++)
        h += (UINT)obj->at[i];
      return h;
    }
    break;
  }
  return h;
}

/*--------------------------------------------------------------------------*/

void TDictionary::init(uint Size)
{
  size = newInt(0);
  keys = newList(Size);
  values = newList(Size);
  keys->resizeTo(Size);
  values->resizeTo(Size);
}

Obj TDictionary::at(Obj key)
{
  uint i, j;
  List k;

  i = (uint)(hashFor(key) % size(keys));
  k = (List)keys->at[i+1];
  if (k == NULL)
    return NULL;
  j = k->indexOfEq(key,1);
  if (!j)
    return NULL;
  return ((List)values->at[i+1])->at[j];
}

void TDictionary::atPut(Obj key, Obj value)
{
  uint i, j;
  List  k, v;

  i = (uint)(hashFor(key) % size(keys));
  k = (List)keys->at[i+1];
  if (k == NULL) {
    keys->at[i+1] = k = newList(1);
    values->at[i+1] = v = newList(1);
    j = 0;
  } else {
    v = (List)values->at[i+1];
    j = k->indexOfEq(key,1);
  }
  if (!j) {
    k->ins(key);
    v->ins(value);
    size = newInt(asInt(size) + 1);
  } else
    v->at[j] = value;
}


bool TDictionary::includesKey(Obj key)
{
  uint i;
  List k;

  i = (uint)(hashFor(key) % size(keys));
  k = (List)keys->at[i+1];
  if (k == NULL)
    return false;
  return (k->indexOfEq(key,1) != 0);
}

/*
bool TDictionary::forEach(Iterator it)
{
  uint i,j,l,l2;
  List k,v;

  l = keys->getSize();
  for (i = 1; i <= l; i++) {
    k = (List)keys->at[i];
    if (k) {
      v = (List)values->at[i];
      l2 = v->getSize();
      for (j = 1; j <= l2; j++) {
        if (!it(k->at[j],v->at[j]))
          return false;
      }
    }
  }
  return true;
}
*/

/*--------------------------------------------------------------------------*/

#ifndef NOCOMPILER

bool TCompiledMethod::atIns(uint index, Byte value)
{
  uint Size;

  Size = asUInt(size);
  if (!setLimit(byteLimit(Size+1)))
    return false;
  heapMove(&atByte[Byte0+index+1], &atByte[Byte0+index], Size - index + 1);
  atByte[Byte0 + index] = value;
  size = newInt(Size+1);
  return true;
}

void TCompiledMethod::ins(Byte value)
{
  uint limit, curSize;

  curSize = asUInt(size);
  limit = getLimit();
  if (curSize == limit * BytesPerPtr)
    growLimit();
  atByte[Byte0 + curSize + 1] = value;
  size = newInt(curSize+1);
}

bool TCompiledMethod::resizeTo(uint newSize)
{
  uint Size;

  Size = asUInt(size);
  if (Size == newSize)
    return true;
  if (newSize > Size) {
    if (!setLimit(byteLimit(newSize)))
      return false;
  } else
    heapFillB(&atByte[Byte0+newSize+1], Size - newSize, 0);
  size = newInt(newSize);
  return true;
}

/*--------------------------------------------------------------------------*/

void TClass::initSupers(List supers)
{
  uint i, j, l, l2, dist, levelNr, curStart;
  List level, start, nextLevel, nextStart, tmp;
  Class c, c2;

  l = size(supers);
  compObjs->ins(start = newList(l));
  dist = 0;
  curStart = 0;
  for (i = 1; i <= l; i++) {
    c = (Class)supers->at[i];
    dist = max(dist,asUInt(c->rootDist));
    start->ins(newOInt(curStart));
    curStart += asUInt(c->instanceVars);
  }
  precedenceList = newList(0);
  precedenceInfo = newList(0);
  classStart = newList(0);
  rootDist = newInt(dist+1);

  levelNr = 1;
  compObjs->ins(level = (List)copyOf(supers));
  compObjs->ins(nextLevel = newList(0));
  compObjs->ins(nextStart = newList(0));
  while ((l = size(level))) {
   for (i = 1; i <= l; i++) {
     c = (Class)level->at[i];
     curStart = asUInt(start->at[i]);
     j = precedenceList->indexOf(c,1);
     if (!j) {
       precedenceList->ins(c);
       precedenceInfo->ins(newOInt(levelNr));
       precedenceInfo->ins(newOInt(1));  // first occurrence
       classStart->ins(newOInt(curStart));
     } else
       precedenceInfo->at[j*2+2] =      // add occurrence
           newOInt(asInt(precedenceInfo->at[j*2+2])+1);
     l2 = size(c->superClasses);
     for (j = 1; j <= l2; j++) {
       c2 = (Class)c->superClasses->at[j];
       if (!nextLevel->includes(c2)) {
         nextLevel->ins(c2);
         nextStart->ins(newOInt(curStart));
       }
       curStart += asUInt(c2->instanceVars);
     }
   }
   level->reset();
   start->reset();
   tmp = level; level = nextLevel; nextLevel = tmp;
   tmp = start; start = nextStart; nextStart = tmp;
   levelNr++;
  }
  compObjs->del(start);
  compObjs->del(level);
  compObjs->del(nextLevel);
  compObjs->del(nextStart);
  superClasses = (List)copyOf(supers);
}

void TClass::initIVars()
{
  uint i, l, ivars;

  ivars = 0;
  l = size(superClasses);
  for (i = 1; i <= l; i++)
    ivars += asUInt(((Class)superClasses->at[i])->instanceVars);
  ivars += size(instVarNames);
  instanceVars = newInt(ivars);
  if (!options)
    options = newInt(0);
}

bool TClass::ivarPos(String name, uint *pos)
{
  uint i, j, l;
  bool found = false;

  l = size(precedenceList);
  for (i = 1; i <= l; i++) {
    j = ((Class)precedenceList->at[i])->instVarNames->indexOfEq(name,1);
    if (j) {
      if (asInt(precedenceInfo->at[2*i+2]) == 1)
        *pos = j + asUInt(classStart->at[i]);
      else
        *pos = 0;
      found = true;
    }
  }
  i = instVarNames->indexOfEq(name,1);
  if (i) {
    if (found)
      return false;
    *pos = i + asUInt(instanceVars) - size(instVarNames);
    return true;
  }
  return found;
}

uint TClass::startOf(Class c)
{
  uint i;

  i = precedenceList->indexOf(c,1);
  if (!i)
    return 0;
  if (asInt(precedenceInfo->at[2*i+2]) > 1)
    return 0;
  return asUInt(classStart->at[i]);
}

#endif

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
