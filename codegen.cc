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

#ifndef NOCOMPILER

#include <string.h>

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#ifndef STREAMS_HPP
#include "streams.hpp"
#endif

#include "compiler.hpp"

Class   clsVariable, clsSymContext, clsModulState;
TCompileError cError;
ModulState curState;
Modul   curModul;
List    compModuls; // currently compiled moduls
char    curName[MAX_IDLEN];
bool    inList;

/*--------------------------------------------------------------------------*/

bool codegenInit()
{
  if (modulesStarted.codeGen)
        return true;
  cError.typ = cerrNone;
  compObjs->ins(compModuls = newList(4));
  clsVariable = createClass("%Variable", IVars(TVariable), vtNone);
  clsSymContext = createClass("%SymbolContext", IVars(TSymContext), vtNone);
  clsModulState = createClass("%ModulState", IVars(TModulState), vtNone);
  modulesStarted.codeGen = 1;
  return true;
}

void codegenDone()
{
  if (!modulesStarted.codeGen)
        return;
  compObjs->del(compModuls);
  modulesStarted.codeGen = 0;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void compError(uint errcode, TSrcPos where)
{
  toktyp = tokError;
  cError.typ = cerrCode;
  cError.val.code = errcode;
  cError.pos = where;
  cError.params = 0;
  strcpy(cError.modul,curName);
  *token = '\0';
}

void scanError(uint errcode)
{
  if (toktyp == tokError)
    return;
  compError(errcode,scc.srcpos);
}

void parseError(uint errcode)
{
  if (toktyp == tokError)
    return;
  compError(errcode,scc.tokpos);
}

void parseError1(uint errcode, const char *p)
{
  if (toktyp == tokError)
    return;
  strcpy(cError.param[0],p);
  compError(errcode,scc.tokpos);
  cError.params = 1;
}

void parseError2(uint errcode, const char *p1, const char *p2)
{
  if (toktyp == tokError)
    return;
  strcpy(cError.param[0],p1);
  strcpy(cError.param[1],p2);
  compError(errcode,scc.tokpos);
  cError.params = 2;
}

/*--------------------------------------------------------------------------*/

static Dictionary globals, methods;
static List     symContexts,undefGenMeths,undefPos,listStack;

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static Global findGlobal(String name)
{
  Obj g;

  g = globals->at(name);
  if (!g)
    g = curState->impGlobals->at(name);
  if (g)
    if (classOf(g) == clsGlobal)
      return (Global)g;
  return NULL;
}

void symNewGlobal(const char *name)
{
  String n;
  Global g;

  compObjs->ins(n = newStr(name));
  g = findGlobal(n);
  if (g) {
    parseError1(cerrSymbolWasDef, name);
    compObjs->del(n);
    return;
  }
  g = newInst(Global);
  globals->atPut(n,g);
  g->key = n;
  g->value = NULL;
  curModul->globals->ins(g);
  g->modul = curModul;
  compObjs->del(n);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static Class findClass(String name)
{
  Obj c;

  c = globals->at(name);
  if (!c)
    c = curState->impGlobals->at(name);
  if (c)
    if (classOf(c) == clsClass)
      return (Class)c;
  return NULL;
}

static GenericMethod findGenMeth(const char *sel, bool create)
{
  String s;
  GenericMethod gm;
  Methods ms;
  uint  i;

  compObjs->ins(s = newStr(sel));
  gm = (GenericMethod)methods->at(s);
  if (!gm) {
    gm = (GenericMethod)curState->impMethods->at(s);
    if (!gm) {
      compObjs->ins(gm = newInst(GenericMethod));
      methods->atPut(s,gm);
      curModul->genericMethods->ins(gm);
      compObjs->del(gm);
      i = undefGenMeths->indexOfEq(s,1);
      if (create && i) {
        undefGenMeths->atDel(i);
        undefPos->delSpace(2*i-1,2);
      }
      if (!create && !i) {
        undefGenMeths->ins(s);
        undefPos->ins(newOInt(scc.tokpos.line));
        undefPos->ins(newOInt(scc.tokpos.pos));
      }
    } else
    if (!create) {
      compObjs->del(s);
      return gm;
    } else {
      compObjs->ins(ms = newInst(Methods));
      methods->atPut(s,ms);
      ms->genericMethod = gm;
      curModul->methods->ins(ms);
      gm = ms;
      compObjs->del(ms);
    }
    gm->init();
    gm->selector = s;
    gm->modul = curModul;
  } else {
    i = undefGenMeths->indexOfEq(s,1);
    if (create && i) {
      undefGenMeths->atDel(i);
      undefPos->delSpace(2*i-1,2);
    }
  }
  compObjs->del(s);
  return gm;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static GenericMethod    curGenMethod;
static uint             rootDistSum, stack, maxStack;
static List             paramClasses;
static SymContext       curSymContext;
static Obj              lastLiteral;

static CompiledMethod   curMethod;
static List             literals;

/*--------------------------------------------------------------------------*/

static void newSymContext(uint start)
{
  if (curSymContext) {
    curSymContext->stack = newInt(stack);
    curSymContext->maxStack = newInt(maxStack);
  }
  symContexts->ins(curSymContext = newInst(SymContext));
  curSymContext->params = newList(1);
  curSymContext->temps = newList(0);
  curSymContext->stack = newInt(0);
  curSymContext->maxStack = newInt(0);
  curSymContext->start = newInt(start);
  stack = 0;
  maxStack = 0;
}

static void endSymContext()
{
  symContexts->delLast();
  if (size(symContexts))
    curSymContext = (SymContext)symContexts->last();
  stack = asUInt(curSymContext->stack);
  maxStack = asUInt(curSymContext->maxStack);
}

typedef struct {
  uint  context,index;
  bool  temp;
} LocalSymbol;

static bool findLocal(String name, LocalSymbol *sym)
{
  uint i, j, l;

  l = size(symContexts);
  for (i = 1; i <= l; i++) {
    j = ((SymContext)symContexts->at[i])->params->indexOfEq(name,1);
    if (j) {
      if (i != 1)
        sym->context = l - i + 1;
      else
        sym->context = 0;
      sym->index = j;
      sym->temp = false;
      return true;
    }
    j = ((SymContext)symContexts->at[i])->temps->indexOfEq(name,1);
    if (j) {
      if (i != 1)
        sym->context = l - i + 1;
      else
        sym->context = 0;
      sym->index = j;
      sym->temp = true;
      return true;
    }
  }
  return false;
}

void symNewInst(const char *name, const char *typ)
{
  String n;
  Class c;

  n = newStr(name);
  if (curSymContext->params->indexOfEq(n,1)) {
    parseError1(cerrSymbolWasDef, name);
    return;
  }
  curSymContext->params->ins(n);
  compObjs->ins(n = newStr(typ));
  c = findClass(n);
  if (!c) {
    parseError1(cerrSymbolNotDef, typ);
    compObjs->del(n);
    return;
  }
  paramClasses->ins(c);
  compObjs->del(n);
}

void symNewTemp(const char *name)
{
  String n;
  LocalSymbol sym;

  compObjs->ins(n = newStr(name));
  if (!findLocal(n,&sym))
    curSymContext->temps->ins(n);
  else
    parseError1(cerrSymbolWasDef, name);
  compObjs->del(n);
}

void symNewParam(const char *name)
{
  String n;
  LocalSymbol sym;

  compObjs->ins(n = newStr(name));
  if (!findLocal(n,&sym))
    curSymContext->params->ins(n);
  else
    parseError1(cerrSymbolWasDef, name);
  compObjs->del(n);
}

Variable varInitLocal(const char *name)
{
  LocalSymbol sym;
  Variable v;

  if (!findLocal(newStr(name),&sym)) {
    uint i = 0;
    if (!strcmp(name,"nil"))
      i = 1;
    if (!strcmp(name,"true"))
      i = 2;
    if (!strcmp(name,"false"))
      i = 3;
    if (!i) {
      parseError1(cerrSymbolNotDef, name);
      return NULL;
    }
    compObjs->ins(v = newInst(Variable));
    v->type = varSpecial;
    v->index = newInt(i);
    return v;
  }
  compObjs->ins(v = newInst(Variable));
  v->context = newInt(sym.context);
  v->index = newInt(sym.index);
  if (!sym.context) {
    if (sym.temp)
      v->type = varTemp;
    else {
      v->type = varInst;
      v->curClass = (Class)paramClasses->at[sym.index];
      v->ivar = newInt(0);
    }
  } else {
    if (sym.temp)
      v->type = varBlockTemp;
    else
      v->type = varBlockParam;
  }
  return v;
}

Variable varInitGlobal(const char *name)
{
  Global g;
  Variable v;
  String n = newStr(name);

  g = (Global)globals->at(n);
  if (!g)
    g = (Global)curState->impGlobals->at(n);
  if (g) {
    compObjs->ins(v = newInst(Variable));
    v->type = varGlobal;
    v->global = g;
    return v;
  } else {
    parseError1(cerrSymbolNotDef, name);
    return NULL;
  }
}

bool varAddCast(Variable v, const char *name)
{
  uint i;

  if (v->type != varInst && v->type != varCast) {
    parseError(cerrNotAnInst);
    return false;
  }
  Class c = findClass(newStr(name));
  if (c)
    i = v->curClass->startOf(c);
  if (c && i) {
    v->ivar = newInt(asInt(v->ivar)+i-1);
    v->curClass = c;
  } else {
    parseError1(cerrCantCast, name);
    return false;
  }
  v->type = varCast;
  return true;
}

bool varSetIVar(Variable v, const char *name)
{
  uint i;

  if (v->type != varInst && v->type != varCast) {
    parseError(cerrNotAnInst);
    return false;
  }
  if (v->curClass->ivarPos(newStr(name), &i))
    if (i) {
      v->type = varInstVar;
      v->ivar = newInt(asInt(v->ivar)+i);
      return true;
    }
  parseError1(cerrIVarNotDef, name);
  return false;
}

/*--------------------------------------------------------------------------*/

void mthNew()
{
  curSymContext = NULL;
  compObjs->ins(paramClasses = newList(1));
  newSymContext(1);
}

void mthEnd()
{
  compObjs->del(paramClasses);
  endSymContext();
}

void mthStart(const char *sel, uint params)
{
  uint i, l;
  Method  m;

  if (!strlen(sel)) {    // init code
    curGenMethod = NULL;
    return;
  }
  curGenMethod = findGenMeth(sel, true);
  rootDistSum = 0;
  l = size(paramClasses);
  for (i = 1; i <= l; i++)
    rootDistSum += asUInt(((Class)paramClasses->at[i])->rootDist);
  l = size(curGenMethod);
  for (i = 1; i <= l; i++) {
    m = (Method)curGenMethod->at[i];
    if (asInt(m->rootDistSum) == rootDistSum &&
        isEqual(m->paramClasses,paramClasses)) {
      parseError1(cerrMethodWasDef, sel);
      return;
    }
  }
  curState->nrMethods = newInt(asInt(curState->nrMethods) + 1);
}

#define primClasses     14

static const char *primClass[primClasses] = {
 "Object",
 "ByteList",
 "String",
 "List",
 "Character",
 "Time",
 "SmallInteger",
 "LargeInteger",
 "Real",
 "Class",
 "Block",
 "Boolean",
 "Error",
 "Test"
};

void mthPrim(const char *cls, uint prim)
{
  uint c, i;
  PrimitiveMethod p;

  c = 0;
  for (i = 0; i < primClasses; i++)
   if (!strcmp(cls,primClass[i])) {
     c = i + 1;
     break;
   }
  if (!c) {
    parseError1(cerrUnknownPrimClass, cls);
    return;
  }
  curGenMethod->ins(p = newInst(PrimitiveMethod));
  p->paramClasses = paramClasses;
  p->rootDistSum = newInt(rootDistSum);
  p->primitiveClass = newInt(c);
  p->primitive = newInt(prim);
  curState->nrPrimitives = newInt(asInt(curState->nrPrimitives) + 1);
}

void mthDeferred()
{
}

/*--------------------------------------------------------------------------*/

void codeStart()
{
  compObjs->ins(curMethod = newInst(CompiledMethod));
  curMethod->init();
  curMethod->literals = literals = newList(0);
}

void codeEnd()
{
  curMethod->maxStack = newInt(maxStack);
  curMethod->temporaries = newInt(size(curSymContext->temps));
  curMethod->paramClasses = paramClasses;
  curMethod->rootDistSum = newInt(rootDistSum);
  symContexts->delLast();
  if (curGenMethod)
    curGenMethod->ins(curMethod);
  else
    curModul->init = curMethod;
  compObjs->del(curMethod);
}

/*--------------------------------------------------------------------------*/

#define addBCode(b)     curMethod->ins(b)
#define addWCode(w)     curMethod->ins((Byte)(w & 255)); curMethod->ins((Byte)(w >> 8))

static void pushStack(uint params)
{
  stack += params;
  if (stack > maxStack)
    maxStack = stack;
}

static void popStack(uint params)
{
  stack -= params;
}

static uint addLiteral(Obj l)
{
  uint i = literals->indexOf(l,1);
  if (!i) {
    literals->ins(l);
    i = size(literals);
  }
  return i;
}

/*--------------------------------------------------------------------------*/

void codePushSys(uint sys)
{
  curSymContext->blocks = NULL;
  statOpcode(OPC_PUSH_SYSTEM);
  pushStack(1);
  addBCode((Byte)sys - 1 + 0x8);
  switch (sys) {

   case 1: lastLiteral = NULL; break;
   case 2: lastLiteral = objTrue; break;
   case 3: lastLiteral = objFalse; break;
   case 4:
   case 5:
   case 6:
   case 7:
   case 8: lastLiteral = newOInt(sys-6);
  }
}

#define MAX_PARAMS      16
#define MAX_LITERALS1   16
#define MAX_LITERALS    256

void codePushLit(Obj l)
{
  uint i;

  curSymContext->blocks = NULL;
  statOpcode(OPC_PUSH_LITERAL);
  if (objType(l) == otInt)
    if (asInt(l) >= -2 && asInt(l) <= 2) {
      codePushSys(asUInt(l) + 6);
      return;
    }
  if (size(literals) >= MAX_LITERALS) {
    parseError(cerrTooManyLiterals);
    return;
  }
  pushStack(1);
  lastLiteral = l;
  i = addLiteral(l) - 1;
  if (i < MAX_LITERALS1)
    addBCode(0x20+(Byte)i);
  else {
    addBCode(0x01);
    addBCode((Byte)i);
  }
}

static void codePushGlobal(Global g)
{
  statOpcode(OPC_PUSH_GLOBAL);
  uint i = addLiteral(g) - 1;
  if (i<16)
    addBCode(0x30+(Byte)i);
  else {
    addBCode(0x02);
    addBCode((Byte)i);
  }
}

static void codePushInst(uint index)
{
  statOpcode(OPC_PUSH_INST);
  addBCode(0x40+(Byte)(index - 1));
}

static void codePushTemp(uint index)
{
  statOpcode(OPC_PUSH_TEMP);
  index--;
  if (index<16)
    addBCode(0x60+(Byte)index);
  else {
    addBCode(0x03);
    addBCode((Byte)index);
  }
}

static void codePushBlockParam(uint context, uint index)
{
  statOpcode(OPC_PUSH_BLKPARAM);
  addBCode(0x70+(Byte)(context - 1));
  addBCode((Byte)(index - 1));
}

static void codePushBlockTemp(uint context, uint index)
{
  statOpcode(OPC_PUSH_BLKTEMP);
  index += size(((SymContext)symContexts->at[context])->params);
  addBCode(0x70+(Byte)(context - 1));
  addBCode((Byte)(index - 1));
}

static void codePushInstVar(uint index, uint ivar)
{
  statOpcode(OPC_PUSH_INSTVAR);
  addBCode(0x50+(Byte)(index - 1));
  addBCode((Byte)(ivar - 1));
}

static void codePushCast(Class c, uint ivar)
{
  statOpcode(OPC_PUSH_CAST);
  addBCode(0x13);
  addBCode((Byte)addLiteral(c));
  addBCode((Byte)ivar);
}

void codePushVar(Variable v)
{
  curSymContext->blocks = NULL;
  switch (v->type) {
    case varSpecial:
      codePushSys(asUInt(v->index));
      break;
    case varGlobal:
      inList = false;
      pushStack(1);
      if (classOf(v->global) == clsClass)
        codePushLit(v->global);
      else
        codePushGlobal(v->global);
      break;
    case varInst:
      inList = false;
      pushStack(1);
      codePushInst(asUInt(v->index));
      break;
    case varTemp:
      inList = false;
      pushStack(1);
      codePushTemp(asUInt(v->index));
      break;
    case varBlockParam:
      inList = false;
      pushStack(1);
      codePushBlockParam(asUInt(v->context),asUInt(v->index));
      break;
    case varBlockTemp:
      inList = false;
      pushStack(1);
      codePushBlockTemp(asUInt(v->context),asUInt(v->index));
      break;
    case varInstVar:
      inList = false;
      pushStack(1);
      codePushInstVar(asUInt(v->index),asUInt(v->ivar));
      break;
    case varCast:
      inList = false;
      codePushCast(v->curClass,asUInt(v->ivar));
      break;
  }
}

/*--------------------------------------------------------------------------*/

static void codeStoreGlobal(Global g)
{
  statOpcode(OPC_STORE_GLOBAL);
  uint i = addLiteral(g) - 1;
  if (i<16)
    addBCode(0x90+(Byte)i);
  else {
    addBCode(0x11);
    addBCode((Byte)i);
  }
}

static void codeStoreTemp(uint index)
{
  statOpcode(OPC_STORE_TEMP);
  index--;
  if (index<16)
    addBCode(0xB0+(Byte)index);
  else {
    addBCode(0x12);
    addBCode((Byte)index);
  }
}

static void codeStoreBlockTemp(uint context, uint index)
{
  statOpcode(OPC_STORE_BLKTEMP);
  index += size(((SymContext)symContexts->at[context])->params);
  addBCode(0xC0+(Byte)(context - 1));
  addBCode((Byte)(index - 1));
}

static void codeStoreInstVar(uint index, uint ivar)
{
  statOpcode(OPC_STORE_INSTVAR);
  addBCode(0xA0+(Byte)(index - 1));
  addBCode((Byte)(ivar - 1));
}

void codeStoreVar(Variable v)
{
  curSymContext->blocks = NULL;
  switch (v->type) {
    case varGlobal:
      codeStoreGlobal(v->global);
      break;
    case varTemp:
      codeStoreTemp(asUInt(v->index));
      break;
    case varBlockTemp:
      codeStoreBlockTemp(asUInt(v->context),asUInt(v->index));
      break;
    case varInstVar:
      codeStoreInstVar(asUInt(v->index),asUInt(v->ivar));
      break;
  }
}

void codePop()
{
  statOpcode(OPC_POP);
  popStack(1);
  addBCode(0x10);
}

static void codePopGlobal(Global g)
{
  statOpcode(OPC_POP_GLOBAL);
  uint i = addLiteral(g) - 1;
  if (i<16)
    addBCode(0x90+(Byte)i);
  else {
    addBCode(0x11);
    addBCode((Byte)i);
  }
}

static void codePopTemp(uint index)
{
  statOpcode(OPC_POP_TEMP);
  index--;
  if (index<16)
    addBCode(0xB0+(Byte)index);
  else {
    addBCode(0x12);
    addBCode((Byte)index);
  }
}

static void codePopBlockTemp(uint context, uint index)
{
  statOpcode(OPC_POP_BLKTEMP);
  index += size(((SymContext)symContexts->at[context])->params);
  addBCode(0xC0+(Byte)(context - 1));
  addBCode((Byte)(index - 1));
}

static void codePopInstVar(uint index, uint ivar)
{
  statOpcode(OPC_POP_INSTVAR);
  addBCode(0xA0+(Byte)(index - 1));
  addBCode((Byte)(ivar - 1));
}

void codePopVar(Variable v)
{
  curSymContext->blocks = NULL;
  switch (v->type) {
    case varGlobal:
      codePopGlobal(v->global);
      break;
    case varTemp:
      codePopTemp(asUInt(v->index));
      break;
    case varBlockTemp:
      codePopBlockTemp(asUInt(v->context),asUInt(v->index));
      break;
    case varInstVar:
      codePopInstVar(asUInt(v->index),asUInt(v->ivar));
      break;
  }
 // codePop();
}

/*--------------------------------------------------------------------------*/

#define specials        11

static const char *special[specials] = {
 "+","-","*","/","//","\\\\",
 "&","|","~",
 ":basicAt:",":basicAt:put:"
};

static void codeCallSpecial(uint sp)
{
  statOpcode(OPC_CALL_SPECIAL);
  sp--;
  if (sp < 16)
    addBCode(0xE0 + (Byte)sp);
  else {
    addBCode(0x5);
    addWCode((Word)sp);
  }
}

#define blockSpecials   10

static const char *blockSpecial[blockSpecials] = {
 ":and:",":or:",
 "if:then:", "ifNot:then:",
 "if:then:else:", "ifNot:then:else:",
 "while:do:", "whileNot:do:",
 "do:while:", "do:whileNot:"
};

static void codeBlockSpecial(uint nr)
{
  uint pos1, pos2;

  statOpcode(OPC_CALL_BLKSPEC);
  switch (nr) {

   case 0: // and:
   case 1: // or:
/*
   condition
  -pop, if false/true->jmp 1
   new block
   block
  -do block
  -jmp end
1:-push false/true
end:
*/
     if (size(curSymContext->blocks) != 1) {
       parseError1(cerrIllBlockMeth, blockSpecial[nr]);
       return;
     }
     pos1 = asUInt(curSymContext->blocks->at[1]);
     addBCode(0x18); // do
     addBCode(0x16); // jump to end
     addBCode(127 + 2);
     if (nr == 0)
       codePushSys(3); // push false
     else
       codePushSys(2); // push true
     curMethod->atIns(pos1, (Byte)(size(curMethod)  - (pos1+1) - 127));
     if (nr == 0)
       curMethod->atIns(pos1, 0x07); // if false -> end
     else
       curMethod->atIns(pos1, 0x06); // if true -> end
     break;

   case 2: // if:then:
   case 3: // ifNot:then:
/*
   condition
  -pop, if false/true->jmp 1
   new block
   block
  -do block
  -jmp end
1:-push nil
end:
*/
     if (size(curSymContext->blocks) != 1) {
       parseError1(cerrIllBlockMeth, blockSpecial[nr]);
       return;
     }
     pos1 = asUInt(curSymContext->blocks->at[1]);
     addBCode(0x18); // do
     addBCode(0x16); // jump to end
     addBCode(2 - 128);
     codePushSys(1); // push nil
     curMethod->atIns(pos1, (Byte)(size(curMethod) - 1 - pos1 - 127));
     if (nr == 2)
       curMethod->atIns(pos1, 0x07); // if false -> end
     else
       curMethod->atIns(pos1, 0x06); // if true -> end
     break;

   case 4: // if:then:else:
   case 5: // ifNot:then:else:
/*
   condition
  -pop, if false/true->jmp 1
   new block
   block1
  -do block
  -jmp end
1: new block
   block2
  -do block
end:
*/
     if (size(curSymContext->blocks) != 2) {
       parseError1(cerrIllBlockMeth, blockSpecial[nr]);
       return;
     }
     pos1 = asUInt(curSymContext->blocks->at[1]);
     pos2 = asUInt(curSymContext->blocks->at[2]);
     addBCode(0x18); // do 2
     curMethod->atIns(pos2, (Byte)(size(curMethod) - pos2 - 127));
     curMethod->atIns(pos2, 0x16); // jump to end
     curMethod->atIns(pos2, 0x18); // do 1
     curMethod->atIns(pos1, (Byte)(pos2 + 2 - pos1 - 127));
     if (nr == 4)
       curMethod->atIns(pos1, 0x07); // if false -> 1
     else
       curMethod->atIns(pos1, 0x06); // if true -> 1
     break;

   case 6: // while:do:
   case 7: // whileNot:do:
/*
start:
   new block
   condition
  -do block
  -pop, if false/true->jmp stop
   new block
   action
  -do block
  -pop
  -jmp start
stop:
  -push nil
*/
     if (size(curSymContext->blocks) != 2) {
       parseError1(cerrIllBlockMeth, blockSpecial[nr]);
       return;
     }
     pos1 = asUInt(curSymContext->blocks->at[1]);
     pos2 = asUInt(curSymContext->blocks->at[2]);
     addBCode(0x18); // do action
     addBCode(0x10); // pop ret value
     addBCode(0x16); // jump to start
     addBCode((Byte)(126 - (size(curMethod) + 3 - pos1)));
     codePushSys(1); // push nil
     curMethod->atIns(pos2, (Byte)(size(curMethod) - (pos2+1) - 127));
     if (nr == 6)
       curMethod->atIns(pos2, 0x07); // if false -> end
     else
       curMethod->atIns(pos2, 0x06); // if true -> end
     curMethod->atIns(pos2, 0x18); // do condition
     break;

   case 8: // do:while:
   case 9: // do:whileNot:
/*
start:
   new block
   action
  -do block
  -pop
   new block
   condition
  -do block
  -pop, if true/false->jmp start
  -push nil
*/
     if (size(curSymContext->blocks) != 2) {
       parseError1(cerrIllBlockMeth, blockSpecial[nr]);
       return;
     }
     pos1 = asUInt(curSymContext->blocks->at[1]);
     pos2 = asUInt(curSymContext->blocks->at[2]);
     curMethod->atIns(pos2, 0x10); // pop ret value
     curMethod->atIns(pos2, 0x18); // do action
     addBCode(0x18); // do condition
     if (nr == 8)
       addBCode(0x06); // if true -> start
     else
       addBCode(0x07); // if false -> start
     addBCode((Byte)(127 - (size(curMethod) + 1 - pos1)));
     codePushSys(1); // push nil
     break;

   default:
     break;
  }
}

static void codeCallMethod(uint p, uint m)
{
  statOpcode(OPC_CALL);
  p--; m--;
  if (p < 16 && m < 256) {
    addBCode(0xD0 + (Byte)p);
    addBCode((Byte)m);
    return;
  }
  addBCode(0x4);
  addBCode((Byte)p);
  addWCode((Word)m);
}

void codeCall(const char *sel, uint params)
{
  uint i;

  popStack(params - 1);
/*
  for (i = 0; i < specials; i++)
    if (!strcmp(special[i],sel)) {
      codeCallSpecial(i + 1);
      curSymContext->blocks = NULL;
      return;
    }
*/
  if (curSymContext->blocks && (params == 2 || params == 3))
    for (i = 0; i < blockSpecials; i++)
      if (!strcmp(blockSpecial[i],sel)) {
        codeBlockSpecial(i);
        curSymContext->blocks = NULL;
        return;
    }
  curSymContext->blocks = NULL;
  i = addLiteral(findGenMeth(sel, false));
  if (i)
    codeCallMethod(params, i);
}

void codeReturn()
{
  statOpcode(OPC_RETURN);
  addBCode(0x14);
}

/*--------------------------------------------------------------------------*/

void codePushBlock()
{
  statOpcode(OPC_PUSH_BLK);
  if (!curSymContext->blocks)
    curSymContext->blocks = newList(2);
  curSymContext->blocks->ins(newOInt(size(curMethod)+1));
  pushStack(1);
  newSymContext(size(curMethod)+1);
  addBCode(0);
  addWCode(0);  // empty parameters
}

void codeReturnBlock()
{
  statOpcode(OPC_RETURN_BLK);
  addBCode(0x15);
}

void codeEndBlock()
{
  uint start;

  statOpcode(OPC_END_BLK);
  start = asUInt(curSymContext->start);
  curMethod->atByte[Byte0 + start] = 0x80 + (Byte)size(curSymContext->temps);
  curMethod->atByte[Byte0 + start + 1] =
        (Byte)((size(curSymContext->params) << 5) + maxStack);
  curMethod->atByte[Byte0 + start + 2] = (Byte)(size(curMethod) - (start + 2));
  endSymContext();
}

/*--------------------------------------------------------------------------*/

static  List    curList;

void codeNewList(uint *start)
{
  statOpcode(OPC_NEW_LIST);
  listStack->ins(curList = newList(0));
  codePushLit(curList);
  *start = size(curMethod);
}

void codeEndList()
{
  statOpcode(OPC_END_LIST);
  listStack->delLast();
  if (size(listStack))
    curList = (List)listStack->last();
}

void codePopList(uint *start)
{
  statOpcode(OPC_POP_LIST);
  if (inList) {// is literal, store in current list
    curList->ins(lastLiteral);
    curMethod->resizeTo(*start); // remove push code
    if (stack == maxStack)
      maxStack--;
    popStack(1);
  } else {
    curList->ins(NULL);
    curMethod->ins(0x17);  // store to list
    curMethod->ins((Byte)size(curList)-1);
  }
  *start = size(curMethod);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static Class curClass;
static List curSupers;

void clsNew(const char *name)
{
  uint i, l;
  String n;

  compObjs->ins(n = newStr(name));
  curClass = findClass(n);
  if (curClass) {
    parseError1(cerrSymbolWasDef, name);
    return;
  }
  curClass = NULL;
  l = size(classes);
  for (i = 1; i <= l; i++)
    if (isEqual(((Class)(classes->at[i]))->name,n)) {
      curClass = (Class)classes->at[i];
      break;
    }
  if (!curClass) {
    curClass = newInst(Class);
    curClass->name = n;
  }
  compObjs->del(n);
  compObjs->ins(curClass);
  compObjs->ins(curSupers = newList(1));

  curState->nrClasses = newInt(asInt(curState->nrClasses) + 1);
}

void clsAddSuper(const char *super)
{
  Class s, c;
  uint i, l;

  s = findClass(newStr(super));
  if (!s) {
    parseError1(cerrSymbolNotDef, super);
    return;
  }
  l = size(curSupers);
  for (i = 1; i <= l; i++) {
    c = (Class)curSupers->at[i];
    if (s == c) {
      parseError1(cerrDoubleSuperclass, super);
      return;
    }
  }
  curSupers->ins(s);
}

void clsStart()
{
  curClass->initSupers(curSupers);
  compObjs->del(curSupers);
  curClass->instVarNames = newList(0);
}

void clsAddIVar(const char *name)
{
  String n;
  uint pos;

  compObjs->ins(n = newStr(name));
  if (curClass->ivarPos(n,&pos)) {
    parseError1(cerrDoubleInstVar, name);
    compObjs->del(n);
    return;
  }
  curClass->instVarNames->ins(n);
  compObjs->del(n);
}

void clsEnd()
{
  curClass->initIVars();
  curModul->classes->ins(curClass);
  curClass->modul = curModul;
  globals->atPut(curClass->name,curClass);
  compObjs->del(curClass);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void purpScope(bool isPublic)
{
}


void purpName(List purpose)
{
/*
  uint i,l;
  String s;

  l = purpose->getSize();
  for (i = 1; i <= l; i++) {
    s = (String)(purpose->at[i]);
  }
*/
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static bool compiling(const char *name)
{
  uint i,l;
  String n;

  n = newStr(name);
  l = size(compModuls);
  for (i = 1; i <= l; i++)
   if (isEqual(((ModulState)(compModuls->at[i]))->modul->name,n))
     return true;
  return false;
}

static bool addSymbols(Modul from)
{
  uint i, l;
  List src;
  Dictionary dest;
  Global g;
  Class  c;
  GenericMethod m;
  char n[MAX_IDLEN];

  if (!from)
    return true;
  // globals
  src = from->globals;
  dest = curState->impGlobals;
  l = size(src);
  for (i = 1; i <= l; i++) {
    g = (Global)src->at[i];
    if (!dest->includesKey(g->key))
      dest->atPut(g->key,g);
    else {
      parseError1(cerrSymbolConflict, asStr(n, g->key));
      return false;
    }
  }
  // classes
  src = from->classes;
  l = size(src);
  for (i = 1; i <= l; i++) {
    c = (Class)src->at[i];
    if (!dest->includesKey(c->name))
      dest->atPut(c->name,c);
    else {
      parseError1(cerrSymbolConflict, asStr(n, c->name));
      return false;
    }
  }
  // generic methods
  src = from->genericMethods;
  dest = curState->impMethods;
  l = size(src);
  for (i = 1; i <= l; i++) {
    m = (GenericMethod)src->at[i];
    if (!dest->includesKey(m->selector))
      dest->atPut(m->selector,m);
    else {
      parseError1(cerrSymbolConflict, asStr(n, m->selector));
      return false;
    }
  }
  curModul->imports->ins(from);
  return true;
}

bool modNew(const char *name)
{
  Modul m;

  if (compiling(name)) {
    parseError1(cerrCircularDep, name);
    return false;
  }
  compModuls->ins(curState = newInst(ModulState));
  m = newInst(Modul);
  m->references = newInt(1);
  curState->modul = m;
  curModul = m;
  curState->impGlobals = newInst(Dictionary);
  curState->impGlobals->init(31);
  curState->impMethods = newInst(Dictionary);
  curState->impMethods->init(31);
  curState->nrClasses = newInt(0);
  curState->nrMethods = newInt(0);
  curState->nrPrimitives = newInt(0);

  m->name = newStr(name);
  m->imports = newList(0);
  m->globals = newList(0);
  m->classes = newList(0);
  m->genericMethods = newList(0);
  m->methods = newList(0);
  m->purpose = newList(0);

  return addSymbols(findModul(SYSMODUL));
}

bool modImport(const char *name)
{
  TScanContext save;
  Modul m;

  if (compiling(name)) {
    parseError1(cerrCircularDep, name);
    return false;
  }
  scannerSave();
  save = scc;
  m = loadModul(name);
  scc = save;
  if (!scannerRestore())
    return false;
  curState = (ModulState)compModuls->last();
  curModul = curState->modul;
  asStr(curName,curModul->name);
  if (!m) {
    parseError1(cerrCantLoadModul, name);
    return false;
  }
  return addSymbols(m);
}

void modStart()
{
  compObjs->ins(globals = newInst(Dictionary));
  compObjs->ins(methods = newInst(Dictionary));
  globals->init(31);
  methods->init(31);
  compObjs->ins(symContexts = newList(4));
  compObjs->ins(undefGenMeths = newList(0));
  compObjs->ins(undefPos = newList(0));
  compObjs->ins(listStack = newList(0));
}

Modul modEnd(bool abort)
{
  char fname[255];
  uint  l;
  char n[MAX_SELLEN];
  ExtStream st;

  if (undefGenMeths)
    l = size(undefGenMeths);
  else
    l = 0;
  if (l) {
    scc.tokpos.line = asUInt(undefPos->at[1]);
    scc.tokpos.pos = asUInt(undefPos->at[2]);
    parseError1(cerrMethNotDef, asStr(n, (GenericMethod)undefGenMeths->at[1]));
    abort = true;
  }
  compObjs->del(globals);
  compObjs->del(methods);
  compObjs->del(symContexts);
  compObjs->del(undefGenMeths);
  compObjs->del(listStack);
  if (!abort) {
    strcat(asStr(fname,curModul->name),".sol");
    sysObjs->ins(st = newInst(ExtStream));
    if (st->open(fname,openCreate + openWrite)) {
      st->put(curModul);
      st->close();
    } else
      console(" WARNING: can't save modul as %s\n",fname);
    sysObjs->del(st);
    moduls->insAbsent(curModul);
    curModul->doInit();
  } else
    curModul = NULL;
  compModuls->del(curState);
  return curModul;
}

#else

void codegenDummy() {} // dummy for ansi compatibility

#endif // NOCOMPILER

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
