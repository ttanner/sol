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
#include <stdio.h>

#ifndef SOL_HPP
#include "sol.hpp"
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

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#ifndef PRIMITIV_HPP
#include "primitiv.hpp"
#endif

#include "intrprtr.hpp"

/*--------------------------------------------------------------------------*/

static Obj      sysconst[8];
static INT      newParams[32];

/*--------------------------------------------------------------------------*/

bool interpreterInit()
{
  int i;

  if (modulesStarted.interpreter)
    return true;
  sysconst[0] = NULL;
  sysconst[1] = objTrue;
  sysconst[2] = objFalse;
  for (i = -2; i <= 2; i++)
    sysconst[5 + i] = newOInt(i);
  modulesStarted.interpreter = 1;
  return true;
}


void interpreterDone()
{
  if (!modulesStarted.interpreter)
    return;
  modulesStarted.interpreter = 0;
}

/*--------------------------------------------------------------------------*/

static Context curContext, homeContext;

static CompiledMethod mth;
static List     castList;
static Byte     *bcode;
static uint     ip,lastCast;
static Obj      *stack, *params, *tmps, *literals;
uint sp;
Obj  *top;

static bool     stop;

/*--------------------------------------------------------------------------*/

static Context newContext(CompiledMethod mth, uint params)
{
  Context c = (Context)clsContext->
        createVar(asUInt(mth->maxStack)+asUInt(mth->temporaries)+params*2);
  c->init();
  c->ip = newInt(1);
  c->tp = newInt(params*2+1);
  c->sp = newInt(params*2+asInt(mth->temporaries));
  c->method = mth;
  return c;
}

static BlockContext newBlockContext(Block b, uint params)
{
  BlockContext c = (BlockContext)clsBlockContext->
        createVar(asUInt(mth->maxStack)+asUInt(mth->temporaries)+params);
  c->init();
  c->ip = b->startIP;
  c->caller = curContext;
  c->tp = newInt(params+1);
  c->sp = newInt(params+asInt(b->temporaries));
  c->method = b->homeContext->method;
  c->fatherContext = b->fatherContext;
  c->homeContext = b->homeContext;
  return c;
}

static void loadContext()
{
  uint i;

  root->at[ROOT_CURCONTEXT] = curContext;
  if (curContext->iClass == clsBlockContext)
    homeContext = ((BlockContext)curContext)->homeContext;
  else
    homeContext = curContext;
  ip = asUInt(curContext->ip);
  sp = asUInt(curContext->sp);
  stack = curContext->at;
  top = &stack[sp];
  params = &homeContext->at[1];
  tmps = &homeContext->at[asUInt(homeContext->tp)];
  mth = curContext->method;
  bcode = &mth->atByte[Byte0 + ip];
  literals = mth->literals->at;
  if (curContext->castList) {
    i = size(castList);
    if (i)
      lastCast = asUInt(castList->at[i]);
    else
      lastCast = 0;
  } else
    lastCast = 0;
}

static void saveContext()
{
  curContext->ip = newInt(ip);
  curContext->sp = newInt(sp);
}

void doBlock(Block b, uint n)
{
  BlockContext c;

  uint bp = asUInt(b->params);
  c = newBlockContext(b, bp);
  if (n != 2) {
    if (n > bp)
      n = bp;
    heapCopyD(&c->at[1],&top[1-n],n);
    // fill
  } else {
    List params = (List)*top;
    n = size(params);
    if (n > bp)
      n = bp;
    heapCopyD(&c->at[1],params->at[1],n);
   // fill
  }
  saveContext();
  curContext = c;
  loadContext();
}

/*--------------------------------------------------------------------------*/

Method TGenericMethod::findMethod(uint params)
{
  uint i, j, l;
  uint rootDistSum, bestSum, bestCount, sum;
  Method mth,best;
  List  list;
  Class p1, p2;

  rootDistSum = 0;
  for (i = 0; i < params; i++)
    rootDistSum += asUInt(((Class)newParams[2*i])->rootDist);
  bestCount = bestSum = 0; best = NULL;
  l = asUInt(size);
  for (i = 1; i <= l; i++) {
    mth = (Method)at[i];
    if (rootDistSum < asInt(mth->rootDistSum))
      continue;
    list = mth->paramClasses;
    sum = 0;
    for (j = 0; j < params; j++) {
      p1 = (Class)newParams[2*j];
      p2 = (Class)list->at[j+1];
      if (p1 != p2) {
       uint dist = p1->distTo(p2);
       if (!dist)
         goto next;
       sum += dist;
      }
    }
    if (bestCount) {
      if (sum > bestSum)
        continue;
      if (sum == bestSum) {
        bestCount++;
        continue;
      }
    }
    bestCount = 1; bestSum = sum; best = mth;
    if (!sum)
      break;
next: ;
  }
  if (bestCount == 1) {
    if (bestSum) {
      list = best->paramClasses;
      for (i = 0; i < params; i++) {
        p1 = (Class)newParams[2*i];
        p2 = (Class)list->at[i+1];
        if (p1 != p2) {
          j = p1->precedenceList->indexOf(p2,1);
          newParams[2*i+1] += asInt(p1->classStart->at[j]);
        }
      }
    }
    return best;
  }
  // error
#ifdef DEBUG
  selector->show();
  fflush(stdout);
#endif
  assert(false);
  return NULL;
}

static void callMethod(uint p, uint m)
{
  uint i, pp;
  CompiledMethod mth;
  Context c;

  p++;
// get classes of params and set default instvar start
  for (i = 0; i < p; i++) {
    newParams[2*i] = (INT)classOf(top[i-p+1]);
    newParams[2*i+1] = 1;
  }
// casting
  if ((castList) && lastCast > sp - p) {
    i = size(castList);
    while ((i) && (pp = asUInt(castList->at[i])) > sp - p) {
      newParams[2*(sp-pp)] = (INT)castList->at[i-1];
      newParams[2*(sp-pp)+1] = (INT)castList->at[i-2];
      i -= 3;
//      testObjs();
    }
    castList->resizeTo(i);
    if (i)
      lastCast = asUInt(castList->at[i]);
    else
      lastCast = 0;
  }
  mth = (CompiledMethod)((GenericMethod)literals[m+1])->findMethod(p);
  if (!mth) {
    stop = true;
//  call  error
    assert(false);
  }
  if (mth->iClass == clsPrimitiveMethod) {
    callPrimitive(asUInt(((PrimitiveMethod)mth)->primitiveClass),
                  asUInt(((PrimitiveMethod)mth)->primitive));
    return;
  }
  c = newContext(mth, p);
  c->caller = curContext;
  for (i = 0; i < p; i++) {
    c->at[2*i+1] = top[i-p+1];
    c->at[2*i+2] = newOInt(newParams[2*i+1]);
  }
  top -= p - 1; sp -= p - 1;
  saveContext();
  curContext = c;
  loadContext();
}

/*--------------------------------------------------------------------------*/

static Block newBlock(uint temps)
{
  Byte t = *bcode++;
  Block b = newInst(Block);
  b->fatherContext = curContext;
  b->homeContext = homeContext;
  b->startIP = newInt(ip + 2);
  b->params = newInt(t >> 5);
  b->temporaries = newInt(temps);
  b->maxStack = newInt(t & 0x1F);
  t = *bcode;
  bcode += t + 1; ip += t + 2;
  return b;
}

static void doReturn(bool block)
{
  Obj  tmp = *top;
  saveContext();
  Context save = curContext;
  if (!block && save->iClass == clsBlockContext)
    curContext = ((BlockContext)curContext)->homeContext;
  curContext = curContext->caller;
  save->caller = NULL;
  if (curContext) {
    loadContext();
    *top = tmp;
  } else
    stop = true;
}

static void mainLoop()
{
  uint  i;
  Byte  opcode, arg, arg2;
  Obj   obj;

  stop = false;
  do {
    opcode = *bcode++; ip++;
    arg = opcode & 0xF; opcode >>= 4;
    switch (opcode) {
/*--------------------------------------------------------------------------*/
    case 0: // extended
       if (arg & 8) { // push system
         *++top = sysconst[arg & 7]; sp++;
       } else
       switch (arg) {
    /*----------------------------------------------------------------------*/
        case 0: // nop
         assert(false /*nop*/);
         break;
    /*----------------------------------------------------------------------*/
        case 1: // push literal
         arg = *bcode++; ip++;
         *++top = copyOf(literals[arg + 1]); sp++;
         break;
    /*----------------------------------------------------------------------*/
        case 2: // push global
         arg = *bcode++; ip++;
         *++top = ((Global)literals[arg + 1])->value; sp++;
         break;
    /*----------------------------------------------------------------------*/
        case 3: // push temporary
         arg = *bcode++; ip++;
         *++top = tmps[arg]; sp++;
         break;
    /*----------------------------------------------------------------------*/
        case 4: // call method
         arg = *bcode++; ip++;
         i = *(Word*)bcode;
         bcode += 2; ip += 2;
         callMethod(arg,i);
         break;
    /*----------------------------------------------------------------------*/
        case 5: // call special
         i = *(Word*)bcode;
         bcode += 2; ip += 2;
         callSpecial(i);
         break;
    /*----------------------------------------------------------------------*/
        case 6: // jump true and pop
         arg = *bcode++; ip++; sp--;
         if (*top-- == objTrue) {
           ip += arg - 128;  bcode += arg - 128;
         }
         break;
    /*----------------------------------------------------------------------*/
        case 7: // jump false and pop
         arg = *bcode++; ip++; sp--;
         if (*top-- == objFalse) {
           ip += arg - 128;  bcode += arg - 128;
         }
         break;
    /*----------------------------------------------------------------------*/
        default: // reserved
         break;
       }
       break;
/*--------------------------------------------------------------------------*/
    case 1: // extended 2
       switch (arg) {
    /*----------------------------------------------------------------------*/
        case 0: // pop
         top--; sp--;
         break;
    /*----------------------------------------------------------------------*/
        case 1: // pop global
         arg = *bcode++; ip++;
         ((Global)literals[arg + 1])->value = *top;
         break;
    /*----------------------------------------------------------------------*/
        case 2: // pop temporary
         arg = *bcode++; ip++;
         tmps[arg] = *top;
         break;
    /*----------------------------------------------------------------------*/
        case 3: // cast
         arg = *bcode++; ip++;
         i = *bcode++; ip++;
         castList->ins(literals[arg]);
         castList->ins(newOInt(i));
         castList->ins(newOInt(sp));
        break;
    /*----------------------------------------------------------------------*/
        case 4: // return
        case 5: // return block
         doReturn(arg == 5);
         break;
    /*----------------------------------------------------------------------*/
        case 6: // jump
         arg = *bcode;
         ip += arg - 127;  bcode += arg - 127;
         break;
    /*----------------------------------------------------------------------*/
        case 7: // store to list
         arg = *bcode++; ip++;
         obj = *top--; sp--;
         ((List)*top)->at[arg+1] = obj;
         break;
    /*----------------------------------------------------------------------*/
        case 8: // do block
         doBlock((Block)*top, 0);
         break;
    /*----------------------------------------------------------------------*/
        default: // reserved
         break;
        }
        break;
/*--------------------------------------------------------------------------*/
    case 2: // push literal
        *++top = copyOf(literals[arg + 1]); sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 3: // push global
        *++top = ((Global)literals[arg + 1])->value; sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 4: // push parameter
        *++top = params[arg * 2]; sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 5: // push instvar
        arg2 = *bcode++; ip++;
        *++top = params[arg * 2]->ivarAt[asUInt(params[arg*2+1])+arg2];
        sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 6: // push temporary
        *++top = tmps[arg]; sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 7: // push blocktemp
        arg2 = *bcode++; ip++;
        obj = curContext;
        for (i = 1; i <= arg; i++)
          obj = ((BlockContext)obj)->fatherContext;
        *++top = obj->at[arg2 + 1]; sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 8: // push block
        *++top = newBlock(arg); sp++;
        break;
/*--------------------------------------------------------------------------*/
    case 9: // pop global
        ((Global)literals[arg + 1])->value = *top;
        break;
/*--------------------------------------------------------------------------*/
    case 10: // pop instvar
        arg2 = *bcode++; ip++;
        params[arg * 2]->ivarAt[asUInt(params[arg*2+1])+arg2] = *top;
        break;
/*--------------------------------------------------------------------------*/
    case 11: // pop temporary
        tmps[arg] = *top;
        break;
/*--------------------------------------------------------------------------*/
    case 12: // pop blocktemp
        arg2 = *bcode++; ip++;
        obj = curContext;
        for (i = 1; i <= arg; i++)
          obj = ((BlockContext)obj)->fatherContext;
        obj->at[arg2 + 1] = *top;
        break;
/*--------------------------------------------------------------------------*/
    case 13: // call method
        arg2 = *bcode++; ip++;
        callMethod(arg,arg2);
        break;
/*--------------------------------------------------------------------------*/
    case 14: // call special
        callSpecial(arg);
        break;
/*--------------------------------------------------------------------------*/
    case 15: // reserved
        break;
   }
#ifdef DEBUG
   bytecodes++;
#endif
  } while (!stop);
}

bool execute(CompiledMethod start)
{
  curContext = newContext(start, 0);
  curContext->caller = NULL;
  loadContext();
  mainLoop();
  return true;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
