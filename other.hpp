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

#ifndef OTHER_HPP
#define OTHER_HPP

#include <time.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

/*--------------------------------------------------------------------------*/

class TTime;
class TLargeInt;
class TDictionary;

typedef TTime *Time;
typedef TLargeInt *LargeInt;
typedef TDictionary *Dictionary;

class TTime : public TObj {
 private:
  void set(time_t);  
 public:
  INT   msecs;
  INT   days;

  Time now();
  bool ofFile(const char *);
  bool after(Time);
  bool before(Time);
};

LargeInt newLInt(INT value);

class TLargeInt : public TObj {
 public:
  INT   sign;

  UINT  bitSize();
  Obj   simplified();
  Real  toReal();
  bool  less(LargeInt other);
  bool  greater(LargeInt other);
  LargeInt add(LargeInt other);
  LargeInt sub(LargeInt other);
  LargeInt mul(LargeInt other);
  LargeInt div(LargeInt other);
  LargeInt mod(LargeInt other);
  LargeInt operator &(LargeInt other);
  LargeInt operator |(LargeInt other);
  LargeInt operator ^(LargeInt other);
  LargeInt shift(INT delta);
 private:
  uint     byteSize();
  uint     getSize();
  int      signedAt(uint index);
  LargeInt reduce();
  LargeInt primAbs();
  LargeInt negate();
  bool     primLess(LargeInt other);
  bool     primGreater(LargeInt other);
  LargeInt primAdd(LargeInt other);
  LargeInt primSub(LargeInt other);
  LargeInt primMul(LargeInt other);
  LargeInt primDiv(LargeInt other, LargeInt *rest);
  LargeInt primAnd(LargeInt other);
  LargeInt primOr (LargeInt other);
  LargeInt primXor(LargeInt other);
};

//typedef bool (*Iterator)(Obj key,Obj value);

class TDictionary : public TModel {
 public:
  List  keys, values;
  INT   size;

  void  init(uint Size);
  Obj   at(Obj key);
  void  atPut(Obj key,Obj value);
  bool  includesKey(Obj key);
//  bool  forEach(Iterator it);
};

extern UINT hashFor(Obj);

extern Obj      objNil, objTrue, objFalse;
extern Class    clsBoolean, clsTime, clsLargeInt, clsDictionary;

#endif /*OTHER_HPP*/
