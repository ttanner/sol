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

#include <math.h>
#include <stdlib.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

#ifndef OTHER_HPP
#include "other.hpp"
#endif

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#ifndef INTERPRETER_HPP
#include "intrprtr.hpp"
#endif

#include "primitiv.hpp"

/*--------------------------------------------------------------------------*/

typedef void (*primFunc)();

/*--------------------------------------------------------------------------*/

static void primObj_same()
{
  Obj tmp = *top--; sp--;
  if (tmp == *top)
    *top = objTrue;
  else
    *top = objFalse;
}

static void primObj_equal()
{
  Obj tmp = *top--; sp--;
  if (isEqual(tmp,*top))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primObj_shallowCopy()
{
  *top = copyOf(*top);
}

static void primObj_hash()
{
// FIXME
  *top = newOInt(asInt(*top));
}

static void primObj_atIVar()
{
// CHECK INDEX
// FIXME
  Obj index = *top--; sp--;
  *top = (*top)->ivarAt[asUInt(index)];
}

static void primObj_atIVarPut()
{
// CHECK INDEX
// FIXME
  Obj value = *top--; sp--;
  Obj index = *top--; sp--;
  (*top)->ivarAt[asUInt(index)] = value;
}

static const primFunc primObject[] = {
 primObj_same,
 primObj_equal,
 primObj_shallowCopy,
 primObj_hash,
 primObj_atIVar,
 primObj_atIVarPut
};

/*--------------------------------------------------------------------------*/

static void primBList_at()
{
  Obj index = *top--; sp--;
  *top = newOInt((*top)->atByte[Byte0+asUInt(index)]);
}

static void primBList_atPut()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj index = *top--; sp--;
  (*top)->atByte[Byte0+asUInt(index)] = (Byte)asUInt(value);
}

static void primBList_fillSpace()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj start = *top--; sp--;
  Obj count = *top--; sp--;
  ((ByteList)*top)->fillSpace(asUInt(start),asUInt(count),(Byte)asUInt(value));
}

static void primBList_moveSpace()
{
  Obj dest = *top--; sp--;
  Obj src  = *top--; sp--;
  Obj count = *top--; sp--;
  ((ByteList)*top)->moveSpace(asUInt(src),asUInt(dest),asUInt(count));
}

static void primBList_indexOf()
{
  Obj start = *top--; sp--;
  Obj value  = *top--; sp--;
  ((ByteList)*top)->indexOf((Byte)asUInt(value),asUInt(start));
}

static void primBList_limit()
{
  *top = newOInt(limit(*top)*BytesPerPtr);
}

static void primBList_setLimit()
{
// MEM ?
// FIXME
  Obj limit = *top--; sp--;
  (*top)->setLimit(byteLimit(asUInt(limit)));
}

static const primFunc primByteList[] = {
 primBList_at,
 primBList_atPut,
 primBList_fillSpace,
 primBList_moveSpace,
 primBList_indexOf,
 primBList_limit,
 primBList_setLimit
};

/*--------------------------------------------------------------------------*/

static void primString_at()
{
  Obj index = *top--; sp--;
  *top = newChar((*top)->atWord[Word0+asUInt(index)]);
}

static void primString_atPut()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj index = *top--; sp--;
  (*top)->atWord[Word0+asUInt(index)] = (Word)asUInt(value);
}

static void primString_fillSpace()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj start = *top--; sp--;
  Obj count = *top--; sp--;
  ((String)*top)->fillSpace(asUInt(start),asUInt(count),(Word)asUInt(value));
}

static void primString_moveSpace()
{
  Obj dest = *top--; sp--;
  Obj src  = *top--; sp--;
  Obj count = *top--; sp--;
  ((String)*top)->moveSpace(asUInt(src),asUInt(dest),asUInt(count));
}

static void primString_indexOf()
{
  Obj start = *top--; sp--;
  Obj value  = *top--; sp--;
  ((String)*top)->indexOf((Word)asUInt(value),asUInt(start));
}

static void primString_limit()
{
  *top = newOInt(limit(*top)*WordsPerPtr);
}

static void primString_setLimit()
{
// MEM ?
// FIXME
  Obj limit = *top--; sp--;
  (*top)->setLimit(wordLimit(asUInt(limit)));
}

static const primFunc primString[] = {
 primString_at,
 primString_atPut,
 primString_fillSpace,
 primString_moveSpace,
 primString_indexOf,
 primString_limit,
 primString_setLimit
};

/*--------------------------------------------------------------------------*/

static void primList_at()
{
  Obj index = *top--; sp--;
  *top = (*top)->at[asUInt(index)];
}

static void primList_atPut()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj index = *top--; sp--;
  (*top)->at[asUInt(index)] = value;
}

static void primList_fillSpace()
{
// CHECK VALUE
// FIXME
  Obj value = *top--; sp--;
  Obj start = *top--; sp--;
  Obj count = *top--; sp--;
  ((List)*top)->fillSpace(asUInt(start),asUInt(count),value);
}

static void primList_moveSpace()
{
  Obj dest = *top--; sp--;
  Obj src  = *top--; sp--;
  Obj count = *top--; sp--;
  ((List)*top)->moveSpace(asUInt(src),asUInt(dest),asUInt(count));
}

static void primList_indexOf()
{
  Obj start = *top--; sp--;
  Obj value  = *top--; sp--;
  ((List)*top)->indexOf(value,asUInt(start));
}

static void primList_limit()
{
  *top = newOInt(limit(*top));
}

static void primList_setLimit()
{
  Obj   limit;

// MEM ?
// FIXME
  limit = *top--; sp--;
  (*top)->setLimit(asUInt(limit));
}

static const primFunc primList[] = {
 primList_at,
 primList_atPut,
 primList_fillSpace,
 primList_moveSpace,
 primList_indexOf,
 primList_limit,
 primList_setLimit
};

/*--------------------------------------------------------------------------*/

static void primChar_asChar()
{
  *top = newChar(asUInt(*top));
}

static void primChar_asASCII()
{
  *top = newOInt(asChar(*top));
}

static const primFunc primCharacter[] = {
 primChar_asChar,
 primChar_asASCII
};

/*--------------------------------------------------------------------------*/

static void primTime_now()
{
  ((Time)*top)->now();
}

static const primFunc primTime[] = {
 primTime_now
};

/*--------------------------------------------------------------------------*/

static void primSInt_compare()
{
  Obj tmp = *top--; sp--;
  if (asInt(*top) == asInt(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primSInt_less()
{
  Obj tmp = *top--; sp--;
  if (asInt(*top) < asInt(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primSInt_greater()
{
  Obj tmp = *top--; sp--;
  if (asInt(*top) > asInt(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

#define MAX_SINT        (((UINT)-1)>>3)
#define MAX_ADDINT      (MAX_SINT>>1)
#define MAX_MULINT      (MAX_SINT>>(ptrBits/2-3))

static void primSInt_add()
{
  INT b = asInt(*top--); sp--;
  INT a = asInt(*top);
  if (iabs(a) < MAX_ADDINT && iabs(b) < MAX_ADDINT)
    *top = newOInt(a + b);
  else {
    *top = newLInt(b);
    *top = newLInt(a)->add((LargeInt)*top);
  }
}

static void primSInt_sub()
{
  INT b = asInt(*top--); sp--;
  INT a = asInt(*top);
  if (iabs(a) < MAX_ADDINT && iabs(b) < MAX_ADDINT)
    *top = newOInt(a - b);
  else {
    *top = newLInt(b);
    *top = newLInt(a)->sub((LargeInt)*top);
  }
}

static void primSInt_mul()
{
  INT b = asInt(*top--); sp--;
  INT a = asInt(*top);
  if (iabs(a) < MAX_MULINT && iabs(b) < MAX_MULINT)
    *top = newOInt(a * b);
  else {
    *top = newLInt(b);
    *top = newLInt(a)->mul((LargeInt)*top);
  }
}

static void primSInt_div()
{
  Obj tmp = *top--; sp--;
// check zero
// FIXME
  *top = newOInt(asInt(*top) / asInt(tmp));
}

static void primSInt_mod()
{
  Obj tmp = *top--; sp--;
// check zero
// FIXME
  *top = newOInt(asInt(*top) % asInt(tmp));
}

static void primSInt_and()
{
  Obj tmp = *top--; sp--;
  *top = newOInt(asInt(*top) & asInt(tmp));
}

static void primSInt_or()
{
  Obj tmp = *top--; sp--;
  *top = newOInt(asInt(*top) | asInt(tmp));
}

static void primSInt_xor()
{
  Obj tmp = *top--; sp--;
  *top = newOInt(asInt(*top) ^ asInt(tmp));
}

static void primSInt_shl()
{
  INT sh = asInt(*top--); sp--;
  INT x = asInt(*top);
  if (!sh || !x)
    return;
// FIXME
  if (sh < ptrBits && (iabs(x) << sh) )
    *top = newOInt(x << sh);
  else
    *top = newLInt(x)->shift(sh);
}

static void primSInt_shr()
{
  INT shift = asInt(*top--); sp--;
  if (shift > sizeof(UINT)*8 - 2)
    shift = sizeof(UINT)*8 - 2;
  *top = newOInt(asInt(*top) >> shift);
}

static void primSInt_size()
{
  *top = newOInt(sizeof(UINT)*8 - 2);
}

static void primSInt_asLInt()
{
  *top = newLInt(asInt(*top));
}

static void primSInt_asReal()
{
  *top = newReal( (double)asInt(*top) );
}

static const primFunc primSmallInteger[] = {
 primSInt_compare,
 primSInt_less,
 primSInt_greater,
 primSInt_add,
 primSInt_sub,
 primSInt_mul,
 primSInt_div,
 primSInt_mod,
 primSInt_and,
 primSInt_or,
 primSInt_xor,
 primSInt_shl,
 primSInt_shr,
 primSInt_size,
 primSInt_asLInt,
 primSInt_asReal
};

/*--------------------------------------------------------------------------*/

static void primLInt_compare()
{
  Obj tmp = *top--; sp--;
  if (isEqual(*top,tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primLInt_less()
{
  Obj tmp = *top--; sp--;
  if (((LargeInt)*top)->less((LargeInt)tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primLInt_greater()
{
  Obj tmp = *top--; sp--;
  if (((LargeInt)*top)->greater((LargeInt)tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primLInt_add()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->add((LargeInt)tmp);
}

static void primLInt_sub()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->sub((LargeInt)tmp);
}

static void primLInt_mul()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->mul((LargeInt)tmp);
}

static void primLInt_div()
{
  Obj tmp = *top--; sp--;
// FIXME
  *top = ((LargeInt)*top)->div((LargeInt)tmp);
}

static void primLInt_mod()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->mod((LargeInt)tmp);
}

static void primLInt_and()
{
  Obj tmp = *top--; sp--;
  *top = *((LargeInt)top) & ((LargeInt)tmp);
}

static void primLInt_or()
{
  Obj tmp = *top--; sp--;
  *top = *((LargeInt)*top) | ((LargeInt)tmp);
}

static void primLInt_xor()
{
  Obj tmp = *top--; sp--;
  *top = *((LargeInt)*top) ^ ((LargeInt)tmp);
}

static void primLInt_shl()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->shift(asUInt(tmp));
}

static void primLInt_shr()
{
  Obj tmp = *top--; sp--;
  *top = ((LargeInt)*top)->shift(-asUInt(tmp));
}

static void primLInt_size()
{
  *top = newOInt(((LargeInt)*top)->bitSize());
}

static void primLInt_simplified()
{
  *top = ((LargeInt)*top)->simplified();
}

static void primLInt_asReal()
{
  *top = (Obj)((LargeInt)*top)->toReal();
}

static const primFunc primLargeInteger[] = {
 primLInt_compare,
 primLInt_less,
 primLInt_greater,
 primLInt_add,
 primLInt_sub,
 primLInt_mul,
 primLInt_div,
 primLInt_mod,
 primLInt_and,
 primLInt_or,
 primLInt_xor,
 primLInt_shl,
 primLInt_shr,
 primLInt_size,
 primLInt_simplified,
 primLInt_asReal
};

/*--------------------------------------------------------------------------*/

static void primReal_compare()
{
  Obj tmp = *top--; sp--;
  if (asReal(*top) == asReal(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primReal_less()
{
  Obj tmp = *top--; sp--;
  if (asReal(*top) < asReal(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primReal_greater()
{
  Obj tmp = *top--; sp--;
  if (asReal(*top) > asReal(tmp))
    *top = objTrue;
  else
    *top = objFalse;
}

static void primReal_add()
{
  Obj tmp = *top--; sp--;
  *top = newReal(asReal(*top) + asReal(tmp));
}

static void primReal_sub()
{
  Obj tmp = *top--; sp--;
  *top = newReal(asReal(*top) - asReal(tmp));
}

static void primReal_mul()
{
  Obj tmp = *top--; sp--;
  *top = newReal(asReal(*top) * asReal(tmp));
}

static void primReal_div()
{
  Obj tmp = *top--; sp--;
  *top = newReal(asReal(*top) / asReal(tmp));
}

static void primReal_sqrt()
{
  *top = newReal(sqrt(asReal(*top)));
}

static void primReal_exp()
{
  *top = newReal(exp(asReal(*top)));
}

static void primReal_ln()
{
  *top = newReal(log(asReal(*top)));
}

static void primReal_sin()
{
  *top = newReal(sin(asReal(*top)));
}

static void primReal_cos()
{
  *top = newReal(sin(asReal(*top)));
}

static void primReal_arcTan()
{
  *top = newReal(atan(asReal(*top)));
}

static void primReal_int()
{
  INT integer = (INT)asReal(*top);
  // check range
// FIXME
  *top = (Obj)integer;
}

static void primReal_frac()
{
  double tmp;

  *top = newReal(modf(asReal(*top),&tmp));
}

static const primFunc primReal[] = {
 primReal_compare,
 primReal_less,
 primReal_greater,
 primReal_add,
 primReal_sub,
 primReal_mul,
 primReal_div,
 primReal_sqrt,
 primReal_exp,
 primReal_ln,
 primReal_sin,
 primReal_cos,
 primReal_arcTan,
 primReal_int,
 primReal_frac
};

/*--------------------------------------------------------------------------*/

static void primClass_create()
{
// Boolean etc. ???
// FIXME
  *top = ((Class)*top)->create();
}

static void primClass_class()
{
  *top = classOf(*top);
}

static const primFunc primClass[] = {
 primClass_create,
 primClass_class
};

/*--------------------------------------------------------------------------*/

static void primBlock_do()
{
  doBlock((Block)*top, 0);
}

static void primBlock_doWith()
{
  doBlock((Block)top[-1], 1);
}

static void primBlock_doWithAnd()
{
  doBlock((Block)top[-2], 2);
}

static void primBlock_doWithParams()
{
  doBlock((Block)top[-1], 3);
}

static const primFunc primBlock[] = {
 primBlock_do,
 primBlock_doWith,
 primBlock_doWithAnd,
 primBlock_doWithParams
};

/*--------------------------------------------------------------------------*/

static void primBool_and()
{
  Obj tmp = *top--; sp--;
  if (tmp == objTrue && tmp == *top )
    *top = objTrue;
  else
    *top = objFalse;
}

static void primBool_or()
{
  Obj tmp = *top--; sp--;
  if (tmp == objTrue || *top == objTrue)
    *top = objTrue;
  else
    *top = objFalse;
}

static void primBool_xor()
{
  Obj tmp = *top--; sp--;
  if (tmp != *top )
    *top = objTrue;
  else
    *top = objFalse;
}

static void primBool_not()
{
  if (*top == objFalse)
    *top = objTrue;
  else
    *top = objFalse;
}

static const primFunc primBoolean[] = {
 primBool_and,
 primBool_or,
 primBool_xor,
 primBool_not
};

/*--------------------------------------------------------------------------*/

static void primError_raise()
{
// FIXME
  notSupported();
}

static void primError_try()
{
// FIXME
  notSupported();
}

/*
Error:
  1: error->Error raise.
  2: try: action->Block ifError: handle->Block.
*/

static const primFunc primError[] = {
 primError_raise,
 primError_try
};

/*--------------------------------------------------------------------------*/

static void primTest_printObj()
{
  console("%p\n",*top);
}

static void primTest_printInt()
{
  console("%d\n",asInt(*top));
}

static void primTest_printReal()
{
  console("%f\n",asReal(*top));
}

static void primTest_printChar()
{
  console("%c",asChar(*top));
}

static void primTest_printStr()
{
  char s[256];
  console("%s\n",asStr(s,*top));
}

static const primFunc primTest[] = {
  primTest_printObj,
  primTest_printInt,
  primTest_printReal,
  primTest_printChar,
  primTest_printStr
};

/*--------------------------------------------------------------------------*/

#define primClasses     14

static const primFunc *prims[] = {
 primObject,
 primByteList,
 primString,
 primList,
 primCharacter,
 primTime,
 primSmallInteger,
 primLargeInteger,
 primReal,
 primClass,
 primBlock,
 primBoolean,
 primError,
 primTest
};

void callPrimitive(uint primitiveClass, uint primitive)
{
    prims[primitiveClass-1][primitive-1]();
}

/*--------------------------------------------------------------------------*/

#define specials        1

void dummy()
{
}

static const primFunc special[] = {
  dummy
};

void callSpecial(uint nr)
{
        (*special[nr])();
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
