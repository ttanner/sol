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

#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#ifndef SOL_HPP
#include "sol.hpp"
#endif

/*--------------------------------------------------------------------------*/

#define otObj           0
#define otInt           1
#define otChar          2
#define otLink          3

#define objType(obj)    ((uint)(obj) & 3)
#define newChar(c)      ((Obj)((UINT)(c << 2) | otChar))
#define newInt(i)       (((i) << 2) | otInt)
#define newOInt(i)      ((Obj)(((INT)(i) << 2) | otInt))
#define asChar(c)       (((INT)(c)) >> 2)
#define asInt(i)        (((INT)(i)) >> 2)
#define asUInt(i)       (((uint)(i)) >> 2)
#define asReal(r)       ((Real)(r))->value

#define Byte0           (sizeof(UINT)/sizeof(Byte)-1)
#define Word0           (sizeof(UINT)/sizeof(Word)-1)

#define newInst(CLASS)  ((CLASS)cls##CLASS->create())
#define size(LIST)      asUInt((LIST)->size)
#define limit(LIST)     (LIST)->getLimit()

#define vtNone          0
#define vtBytes         1
#define vtWords         2
#define vtReal          3
#define vtObjs          4

#define optBinary       16

#define ROOT_CURCONTEXT 1
#define ROOT_TMP        2

/*--------------------------------------------------------------------------*/

class TObj;
class TClass;
class TReal;
class TIndexedCollection;
class TByteList;
class TString; 
class TList;
class TAssociation;
class TModul;
class TGlobal;
class TGenericMethod;
class TMethods;
class TMethod;
class TCompiledMethod;
class TPrimitiveMethod;
class TContext;
class TBlockContext;
class TBlock;
class TError;
class TInstCache;

typedef TObj            *Obj;
typedef TClass          *Class;
typedef TReal           *Real;
typedef TIndexedCollection      *IndexedCollection;
typedef TByteList       *ByteList;
typedef TString         *String;
typedef TList           *List;
typedef TAssociation    *Association;
typedef TModul          *Modul;
typedef TGlobal         *Global;
typedef TGenericMethod  *GenericMethod;
typedef TMethods        *Methods;
typedef TMethod         *Method;
typedef TCompiledMethod *CompiledMethod;
typedef TPrimitiveMethod *PrimitiveMethod;
typedef TContext        *Context;
typedef TBlockContext   *BlockContext;
typedef TBlock          *Block;
typedef TError          *Error;
typedef TInstCache      *InstCache;

/*--------------------------------------------------------------------------*/

typedef struct {
  unsigned short varType:3;
  bool  marked:1;
  bool  binary:1;
#if COMP == TURBOC
  uint  fill1: sizeof(INT)*4-5;
  uint  fill2;
#endif
} TObjOptions;

class THeapObj {
 public:
  Obj _next;
  TObjOptions _opt;
  uint  size4(uint size);
};

class TReal : public THeapObj {
 public:
  double value;
};

class TObj : public THeapObj {
 public:
  union {
    Byte *atByte;
    Word *atWord;
    Obj  *at;
  };
  union {
    Class iClass;
    Obj   ivarAt[1];
  };
  uint  instVars();
  uint  getLimit();
  bool  setLimit(uint alimit);
  bool  growLimit();
};

#define sizeReal        (sizeof(TReal) / ptrSize)
#define sizeObj         (sizeof(TObj) / ptrSize)

/*--------------------------------------------------------------------------*/

class TModel : public TObj {
 public:
  List   dependencies;
};

class TIndexedCollection : public TModel {
 public:
  INT   size;

  IndexedCollection init();
  void  moveSpace(uint from,uint to,uint count);
  bool  insSpace(uint index,uint count);
  bool  delSpace(uint index,uint count);
  bool  setData(void *data, uint count);
  bool  resizeTo(uint newSize);
  void  atDel(uint index);
  void  delLast();
  void  reset();
  void  adjustLimit();
};

class TByteList : public TIndexedCollection {
 public:
  void  fillSpace(uint index,uint count, Byte value);
  bool  setData(void *data, uint count);
  uint  indexOf(Byte value, uint start);
  bool  atIns(uint index, Byte value);
  void  ins(Byte value);
};

class TString : public TIndexedCollection {
 public:
  void  fillSpace(uint index,uint count, Word value);
  bool  setData(void *data, uint count);
  uint  indexOf(Word value, uint start);
  char  *getStr(char *s);
  bool  setStr(const char *s);
  void  show();
};

class TList : public TIndexedCollection {
 public:
  void  fillSpace(uint index,uint count, Obj value);
  bool  setData(void *data, uint count);
  Obj   last();
  bool  atIns(uint index, Obj value);
  bool  del(Obj value);
  void  ins(Obj value);
  bool  insAbsent(Obj value);
  void  insAll(List from);
  void  delAll(List from);
  uint  indexOf(Obj value, uint start);
  uint  indexOfEq(Obj value, uint start);
  bool  includes(Obj value);
};

class TAssociation : public TObj {
 public:
  Obj key,value;
};

/*--------------------------------------------------------------------------*/

#define IVars(CLASS)    (sizeof(CLASS) / ptrSize - sizeObj)

class TClass : public TObj {
 public:
  String name;
  Modul modul;
  List  superClasses;
  INT   rootDist;
  List  precedenceList;
  List  precedenceInfo;
  List  classStart;
  INT   instanceVars;
  List  instVarNames;
  INT   options;
  String comment;

  Obj   create();
  Obj   createVar(uint aLimit);
  bool  inheritsFrom(Class c);
  uint  distTo(Class c);
#ifndef NOCOMPILER
  void  initSupers(List superClasses);
  void  initIVars();
  bool  ivarPos(String name, uint *pos);
  uint  startOf(Class c);
#endif
};

class TModul : public TObj {
 public:
  String name;
  INT   references;
  List  imports;
  List  globals;
  List  classes;
  List  genericMethods;
  List  methods;
  CompiledMethod init;
  INT   options;
  INT   version;
  List  purpose;

  bool  doInit();
  void  doEnd();
};

class TGlobal : public TAssociation {
 public:
  Modul modul;
};

class TGenericMethod : public TList {
 public:
  String selector;
  Modul modul;

  Method findMethod(uint params);
};

class TMethods : public TGenericMethod {
 public:
  GenericMethod genericMethod;
};

class TMethod : public TObj {
 public:
  List  paramClasses;
  INT   rootDistSum;
  String comment;
};

class TCompiledMethod : public TMethod /*TByteList*/{
 public:
  List  literals;
  INT   temporaries;
  INT   maxStack;
  String source;

  List  dependencies;
  INT   size;

#ifndef NOCOMPILER
  CompiledMethod init();
  bool  resizeTo(uint newSize);
  bool  atIns(uint index, Byte value);
  void  ins(Byte value);
#endif
};

class TPrimitiveMethod : public TMethod {
 public:
  INT   primitiveClass;
  INT   primitive;
};

class TContext : public TList {
 public:
  Context caller;
  INT   ip;
  INT   tp;
  INT   sp;
  CompiledMethod method;
  List  castList;
};

class TBlockContext : public TContext {
 public:
  Context fatherContext;
  Context homeContext;
};

class TBlock : public TObj {
 public:
  Context fatherContext;
  Context homeContext;
  INT   startIP;
  INT   params;
  INT   temporaries;
  INT   maxStack;
};

class TError : public TObj {
 public:
};

class TInstCache : public TObj {
 public:
  Class type;
  List cache;

  void init(Class cls, uint maxSize);
  Obj  create(uint limit);
  void free(Obj inst);
};

/*--------------------------------------------------------------------------*/

inline uint byteLimit(uint bytes)
{
  uint limit;

  limit = bytes / BytesPerPtr;
  if ((bytes & (BytesPerPtr - 1)) != 0)
    limit++;
  return limit;
}

inline uint wordLimit(uint words)
{
  uint limit;

  limit = words / WordsPerPtr;
  if ((words & (WordsPerPtr - 1)) != 0)
    limit++;
  return limit;
}

/*--------------------------------------------------------------------------*/

inline uint TObj::instVars()
{
  return (uint)asInt(iClass->instanceVars);
}


inline uint TObj::getLimit()
{
  if (at)
    return ((uint)at[0]);
  else
    return 0;
}

/*--------------------------------------------------------------------------*/

inline IndexedCollection TIndexedCollection::init()
{
  size = newInt(0);
  return this;
}

inline void TIndexedCollection::atDel(uint index)
{
  delSpace(index, 1);
}

inline void TIndexedCollection::delLast()
{
  delSpace(asUInt(size), 1);
}

inline void TIndexedCollection::reset()
{
  resizeTo(0);
}

inline void TIndexedCollection::adjustLimit()
{
  setLimit(asUInt(size));
}

/*--------------------------------------------------------------------------*/

inline Obj TList::last()
{
  return (at[asUInt(size)]);
}

inline bool TList::insAbsent(Obj value)
{
  if (indexOf(value,1) == 0) {
    ins(value);
    return true;
  } else
    return false;
}

inline bool TList::includes(Obj value)
{
  return (indexOf(value, 1) != 0);
}

/*--------------------------------------------------------------------------*/

#ifndef NOCOMPILER

inline CompiledMethod TCompiledMethod::init()
{
  size = newInt(0);
  return this;
}

#endif

/*--------------------------------------------------------------------------*/

inline Obj TClass::create()
{
  return createVar(0);
}

inline bool TClass::inheritsFrom(Class c)
{
  return precedenceList->includes(c);
}

inline uint TClass::distTo(Class c)
{
  uint dist;

  dist = precedenceList->indexOf(c,1);
  if (dist)
    dist = asUInt(precedenceInfo->at[2*dist-1]);
  return dist;
}

/*--------------------------------------------------------------------------*/

extern Class    clsObject,
                clsSmallInteger, clsReal, clsCharacter,
                clsByteList, clsString, clsList,
                clsModul, clsClass,
                clsAssociation, clsGlobal,
                clsGenericMethod, clsMethods,
                clsCompiledMethod, clsPrimitiveMethod,
                clsContext, clsBlockContext,
                clsBlock, clsError;
extern Obj      objectList;
extern List     root, classes, moduls;
extern List     sysObjs, compObjs;
extern UINT     objCount, realCount;

//define newList(LIMIT)  (List)clsList->createVar(LIMIT)

inline List newList(uint limit)
{
  List l;

  l = (List)clsList->createVar(limit);
  l->size = newInt(0);
  return l;
}

/*--------------------------------------------------------------------------*/

extern bool objectsInit();
extern void objectsDone();

extern Class classOf(Obj);
extern Obj  copyOf(Obj);
extern bool isEqual(Obj,Obj);

extern Obj newReal(double);
extern String newStr(const char *);
extern char *asStr(char *,Obj);

extern Class createClass(const char *, uint, uchar);

/*--------------------------------------------------------------------------*/

#endif /*OBJECTS_HPP*/
