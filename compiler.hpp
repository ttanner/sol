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

#ifndef COMPILER_HPP
#define COMPILER_HPP

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

#ifndef OTHER_HPP
#include "other.hpp"
#endif

#include <stdio.h>

/*--------------------------------------------------------------------------*/

#define MAX_SELLEN      256
#define MAX_NRRADLEN    3
#define MAX_NRINTLEN    11
#define MAX_NRFRCLEN    21
#define MAX_NREXPLEN    5

#define SRCBUFSIZE      16384

#define tokEnd          0
#define tokError        1
#define tokComment      2
#define tokTerminal     4
#define tokCharacter    8
#define tokString       16
#define tokNumber       32
#define tokLocal        64
#define tokGlobal       128
#define tokBinSel       256
#define tokKeyword      512

typedef struct TSrcPos {
  Word line, pos;
} TSrcPos;

typedef struct TScanContext {
  FILE  *source;
  char  name[256];
  long  fpos;
  TSrcPos srcpos, tokpos;
  char  cur, cache;
  bool  cacheused, binsel;
} TScanContext;

typedef struct TNumTok {
  bool numIntS, numExpS;
  char numRad[MAX_NRRADLEN];
  char numInt[MAX_NRINTLEN];
  char numFrac[MAX_NRFRCLEN];
  char numExp[MAX_NREXPLEN];
} TNumTok;

extern TScanContext scc;
extern uint     toktyp;
extern char     token[MAX_IDLEN];
extern union    TTokVal {
  char tokChar;
  char tokStr[MAX_STRLEN];
  TNumTok tokNum;
} tokval;
extern uint     tabSize;

extern bool initScanner(const char *);
extern void scannerSave();
extern bool scannerRestore();
extern void doneScanner();

extern void scan();

extern Obj asNumber(TNumTok);

/*--------------------------------------------------------------------------*/

typedef enum {
 OPC_PUSH_SYSTEM,
 OPC_PUSH_LITERAL,
 OPC_PUSH_BLK,
 OPC_PUSH_INST,
 OPC_PUSH_TEMP,
 OPC_PUSH_BLKPARAM,
 OPC_PUSH_BLKTEMP,
 OPC_PUSH_INSTVAR,
 OPC_PUSH_GLOBAL,
 OPC_PUSH_CAST,
 OPC_STORE_TEMP,
 OPC_STORE_BLKTEMP,
 OPC_STORE_INSTVAR,
 OPC_STORE_GLOBAL,
 OPC_POP_TEMP,
 OPC_POP_BLKTEMP,
 OPC_POP_INSTVAR,
 OPC_POP_GLOBAL,
 OPC_POP_LIST,
 OPC_POP,
 OPC_RETURN,
 OPC_RETURN_BLK,
 OPC_CALL,
 OPC_CALL_SPECIAL,
 OPC_CALL_BLKSPEC,
 OPC_NEW_LIST,
 OPC_END_LIST,
 OPC_END_BLK,
 OPC_LAST
} Opcode;

#define varSpecial      newInt(1)
#define varGlobal       newInt(2)
#define varInst         newInt(3)
#define varTemp         newInt(4)
#define varBlockParam   newInt(5)
#define varBlockTemp    newInt(6)
#define varInstVar      newInt(7)
#define varCast         newInt(8)

class TVariable;
class TSymContext;
class TModulState;

typedef TVariable *Variable;
typedef TSymContext *SymContext;
typedef TModulState *ModulState;

class TVariable : public TObj {
 public:
  INT    type;
  INT    context;
  INT    index;
  Class  curClass;
  INT    ivar;
  Global global;
};

class TSymContext : public TObj {
 public:
  List  params;
  List  temps;
  INT   stack;
  INT   maxStack;
  INT   start;
  List  blocks;
};

class TModulState : public TObj {
 public:
  Modul  modul;
  Dictionary impGlobals; // globals, classes
  Dictionary impMethods; // generic methods
  INT   nrClasses;
  INT   nrMethods;
  INT   nrPrimitives;
};

struct TCodeStart {
  uint start;
  uint size;
};

typedef enum { cerrNone, cerrCode, cerrExpTok, cerrExpSym } cerrTyp;

enum  { cerrNoError,
// Scanner
        cerrEOF,cerrChar,cerrExpectDigit,cerrNumber,
        cerrIDTooLong,cerrStrTooLong,cerrNumTooLong,cerrExpNoStop,
// Parser
        cerrExpUnary,cerrExpBinary,cerrExpKeyw,
        cerrExpExpr,cerrExpExprs,
        cerrExpStatm,cerrExpInst,
// Codegen
        cerrTooManyParams,cerrTooManyLiterals,
        cerrSymbolNotDef,cerrSymbolWasDef,cerrSymbolConflict,
        cerrNotAnInst,cerrCantCast,cerrIVarNotDef,
        cerrCantStore,cerrIllBlockMeth,
        cerrMethodWasDef,cerrMethNotDef,
        cerrUnknownCompInstr,cerrUnknownPrimClass,
// Class
        cerrDoubleSuperclass,cerrDoubleInstVar,
// Modul
        cerrCantLoadModul,cerrCircularDep,
        cerrLast };

struct TCompileError {
  cerrTyp typ;
  char modul[32];
  TSrcPos pos;
  union {
    uint code;
    uint tok;
    char sym[9];
  } val;
  uint params;
  char param[2][MAX_IDLEN];
};

extern char     curName[MAX_IDLEN];
extern Class    clsVariable, clsSymContext, clsModulState;
extern TCompileError cError;
extern ModulState curState;
extern Modul    curModul;
extern bool     inList;

/*--------------------------------------------------------------------------*/
// errors
extern void compError(uint errcode, TSrcPos where);
extern void scanError(uint);
extern void parseError(uint);
extern void parseError1(uint, const char *);
extern void parseError2(uint, const char *, const char *);
// modul
extern bool modNew(const char *);
extern bool modImport(const char *);
extern void modStart();
extern Modul modEnd(bool);
// purpose
extern void purpScope(bool);
extern void purpName(List);
// symbol
extern void symNewGlobal(const char *);
extern void symNewInst(const char *, const char *);
extern void symNewTemp(const char *);
extern void symNewParam(const char *);
extern Variable varInitLocal(const char *);
extern Variable varInitGlobal(const char *);
extern bool varAddCast(Variable, const char *);
extern bool varSetIVar(Variable, const char *);
// class
extern void clsNew(const char *);
extern void clsAddSuper(const char *);
extern void clsStart();
extern void clsAddIVar(const char *);
extern void clsEnd();
// method
extern void mthNew();
extern void mthStart(const char *, uint);
extern void mthPrim(const char *, uint);
extern void mthDeferred();
extern void mthEnd();
// code
extern void codeStart();
extern void codeEnd();
extern void codePushSys(uint);
extern void codePushLit(Obj);
extern void codePop();
extern void codePushVar(Variable);
extern void codeStoreVar(Variable);
extern void codePopVar(Variable);
extern void codeCall(const char *, uint);
extern void codeReturn();
extern void codePushBlock();
extern void codeReturnBlock();
extern void codeEndBlock();
extern void codeNewList(uint *);
extern void codeEndList();
extern void codePopList(uint *);

#endif // COMPILER_HPP

#endif // NOCOMPILER
