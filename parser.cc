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

#include <string.h>

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#ifndef NOCOMPILER

#include "compiler.hpp"

/*--------------------------------------------------------------------------*/

static bool check(uint tok)
{
  if (toktyp == tokError)
    return false;
  if ((toktyp & tok) == 0) {
    compError(0, scc.tokpos);
    cError.typ = cerrExpTok;
    cError.val.tok = tok;
    return false;
  } else
    return true;
}

static bool checkSym(const char *sym)
{
  if (toktyp == tokError)
    return false;
  if (strcmp(token, sym)) {
    compError(0, scc.tokpos);
    cError.typ = cerrExpSym;
    strcpy(cError.val.sym, sym);
    return false;
  } else {
    scan();
    return true;
  }
}

/*--------------------------------------------------------------------------*/
/*
operator priority:
0. user
1. * / // \\ & << >>    multiplicative
2. + - | ~              additive
3. = == # ## < > <= >=  relational
*/

#define selectors       19

static const char *sel[] = {
  "*", "/", "//", "\\\\", "&", "<<", ">>",
  "+", "-", "|", "~",
  "=", "==", "#","##", "<", ">", ">=", "<="
};


static uint getPrior()
{
  uint i;

  for (i = 1; i <= selectors; i++) {
    if (!strcmp(token, sel[i - 1])) {
      if (i <= 7)
        return 1;
      else if (i <= 11)
        return 2;
      else
        return 3;
    }
  }
  return 0;
}

/*--------------------------------------------------------------------------*/

static bool varCached;
static Variable varCache;

static bool parseList();
static bool parseExpr();
static bool parseExpressions(bool *);
static bool parseBlock();

/*--------------------------------------------------------------------------*/

static Variable parseVariable()
{
  Variable v;

  if (varCached) {
    varCached = false;
    return varCache;
  }
  if (toktyp == tokLocal) {
    v = varInitLocal(token);
    if (!v)
      return NULL;
    scan();
    if (strcmp(token, "->"))
      return v;
    while (!strcmp(token, "->")) {
      scan();
      if (toktyp == tokLocal) {
         if (!varSetIVar(v,token)) {
           compObjs->del(v);
           return NULL;
         }
         scan();
         return v;
      }
      if (!check(tokGlobal)) {
        compObjs->del(v);
        return NULL;
      }
      if (!varAddCast(v,token)) {
        compObjs->del(v);
        return NULL;
      }
      scan();
    }
    return v;
  } else
  if (toktyp == tokGlobal) {
    v = varInitGlobal(token);
    if (!v)
      return NULL;
    scan();
    return v;
  }
  return NULL;
}

/*--------------------------------------------------------------------------*/

static Obj parseLiteral()
{
  Obj l;

  switch (toktyp) {

  case tokNumber:
    l = asNumber(tokval.tokNum);
    if (!l) {
      parseError(cerrNumber);
      return NULL;
    }
    scan();
    return l;

  case tokCharacter:
    l = newChar(tokval.tokChar);
    scan();
    return l;

  case tokString:
    l = newStr(tokval.tokStr);
    scan();
    return l;

  default:
    return NULL;
  }
}

/*--------------------------------------------------------------------------*/

static bool parsePrimary()
{
  Obj l;
  bool endexpr;
  Variable v;

  if ((v = parseVariable())) {
    codePushVar(v);
    compObjs->del(v);
    return true;
  } else
  if ((l = parseLiteral())) {
    codePushLit(l);
    return true;
  } else
  if (parseList())
    return true;
  else
  if (parseBlock())
    return true;
  else
  if (!strcmp(token, "(")) {
    inList = false;
    scan();
    if (!parseExpressions(&endexpr)) {
      parseError(cerrExpExprs);
      return false;
    }
    if (!endexpr) {
      parseError(cerrExpNoStop);
      return false;
    }
    if (!checkSym(")"))
      return false;
    return true;
  }
  return false;
}


/*--------------------------------------------------------------------------*/

static bool parseList()
{
  uint start;
  bool wasInList = inList;

  if (strcmp(token, "{"))
    return false;
  scan();
  codeNewList(&start);
  inList = true;
  if (parseExpr()) {
    codePopList(&start);
    while (!strcmp(token, ",")) {
      scan();
      inList = true;
      if (!parseExpr()) {
        parseError(cerrExpExpr);
        return false;
      }
      codePopList(&start);
    }
  }
  codeEndList();
  inList = wasInList;
  return checkSym("}");
}

/*--------------------------------------------------------------------------*/

static bool parseUnaryExpr()
{
  if (!parsePrimary())
    return false;
  while (toktyp == tokLocal) {
    inList = false;
    codeCall(token,1);
    scan();
  }
  return true;
}

/*--------------------------------------------------------------------------*/

static bool parseBinaryExpr(int prior)
{
  char sel[MAX_IDLEN];

  scc.binsel = (prior == 2);
  if (prior == 0) {
    if (parseUnaryExpr()) {
      while (toktyp == tokBinSel) {
        if (getPrior() > 0)
          break;
        inList = false;
        strcpy(sel, token);
        scan();
        if (!parseUnaryExpr()) {
          parseError(cerrExpUnary);
          return false;
        }
        codeCall(sel,2);
      }
    scc.binsel = false;
    return true;
    }
  } else
  if (parseBinaryExpr(prior - 1)) {
    while (toktyp == tokBinSel) {
      if (getPrior() > prior)
        break;
      inList = false;
      strcpy(sel, token);
      scan();
      if (!parseBinaryExpr(prior - 1)) {
        parseError(cerrExpUnary);
        return false;
      }
      codeCall(sel,2);
    }
    scc.binsel = false;
    return true;
  };
  scc.binsel = false;
  return false;
}

/*--------------------------------------------------------------------------*/

static bool parseKeywordExpr()
{
  char sel[MAX_SELLEN];
  uint params;
  bool keyw;

  if (toktyp == tokKeyword && !varCached) {
    inList = false;
    params = 0;
    strcat(strcpy(sel,token),":");
    scan();
    keyw = true;
    while (parseBinaryExpr(3)) {
      params++;
      keyw = false;
      if (toktyp == tokKeyword) {
        strcat(strcat(sel,token),":");
        scan();
        keyw = true;
      } else
        break;
    }
    if (keyw) {
      parseError(cerrExpBinary);
      return false;
    }
    codeCall(sel, params);
    return true;
  } else
  if (parseBinaryExpr(3)) {
    if (toktyp == tokKeyword) {
      inList = false;
      params = 1;
      strcpy(sel,":");
      while (toktyp == tokKeyword) {
        strcat(strcat(sel,token),":");
        scan();
        if (!parseBinaryExpr(3)) {
          parseError(cerrExpBinary);
          return false;
        }
        params++;
      }
      codeCall(sel, params);
    }
    return true;
  }
  return false;
}

/*--------------------------------------------------------------------------*/

static bool parseExpr()
{
  List dest;
  uint i,j;
  Variable v;

  dest = NULL;
  scc.binsel = true;
  while ((v = parseVariable())) {
    if (!strcmp(token, ":=")) {
      inList = false;
      scan();
      if (v->type == varSpecial ||
          v->type == varInst ||
          v->type == varBlockParam ||
          v->type == varCast) {
        parseError(cerrCantStore);
        return false;
      }
      if (!dest)
        compObjs->ins(dest = newList(1));
      dest->ins(v);
      compObjs->del(v);
    } else {
      varCache = v;
      varCached = true;
      break;
    }
  }
  scc.binsel = false;
  if (!parseKeywordExpr()) {
    if (dest)
      parseError(cerrExpKeyw);
    return false;
  }
  if (dest) {
     j = size(dest);
     for (i=1; i<=j; i++) {
       v = (Variable)dest->at[i];
       if (i != j)
         codeStoreVar(v);
       else
         codePopVar(v);
     }
     compObjs->del(dest);
  }
  return true;
}

/*--------------------------------------------------------------------------*/

static bool parseExpressions(bool *endexpr)
{
  if (!parseExpr())
    return false;
  *endexpr = true;
  while (!strcmp(token, ".")) {
    scan();
    codePop();
    if (!parseExpr()) {
      *endexpr = false;
      break;
    }
  }
  return true;
}

/*--------------------------------------------------------------------------*/

static bool parseTemporaries()
{
  if (strcmp(token, "|"))
    return false;
  scan();
  if (toktyp == tokLocal) {
    while (toktyp == tokLocal) {
      symNewTemp(token);
      scan();
    }
  }
  return checkSym("|");
}

/*--------------------------------------------------------------------------*/

static bool parseStatements(bool block)
{
  bool endexpr;

  if (parseTemporaries()) {
    if (toktyp == tokError)
      return false;
  }
  if (!parseExpressions(&endexpr))
    endexpr = false;
  if (toktyp == tokError)
    return false;
  if (!strcmp(token, "^")) {
    if (endexpr) {
      checkSym(".");
      return false;
    }
    scan();
    if (!parseExpr()) {
      parseError(cerrExpExpr);
      return false;
    }
    if (!strcmp(token, "."))
      scan();
    codeReturn();
    return true;
  }
  if (!endexpr)
    codePushSys(1);
  if (!block)
    codeReturn();
  else
    codeReturnBlock();
  return true;
}

/*--------------------------------------------------------------------------*/

static bool parseBlock()
{
  if (strcmp(token, "["))
    return false;
  inList = false;
  scan();
  codePushBlock();
  while (!strcmp(token, ":")) {
    scan();
    if (!check(tokLocal))
      return false;
    symNewParam(token);
    scan();
  }
  parseStatements(true);
  if (toktyp == tokError)
    return false;
  codeEndBlock();
  return checkSym("]");
}


/*--------------------------------------------------------------------------*/

static bool parseInstance()
{
  char name[MAX_IDLEN];

  if (toktyp != tokLocal)
    return false;
  strcpy(name, token);
  scan();
  if (!checkSym("->"))
    return false;
  if (!check(tokGlobal))
    return false;
  symNewInst(name, token);
  scan();
  return true;
}

/*--------------------------------------------------------------------------*/

static bool parseSyntax(char *sel, uint *params)
{
  mthNew();
  *params = 0;
  if (toktyp == tokKeyword) {
    strcat(strcpy(sel,token),":");
    scan();
    while (parseInstance()) {
      (*params)++;
      if (toktyp == tokKeyword) {
        strcat(strcat(sel,token),":");
        scan();
      } else
        break;
    }
    if (*params == 0) {
      parseError(cerrExpInst);
      return false;
    }
    return true;
  } else
  if (parseInstance()) {
    if (toktyp == tokLocal) {
      strcpy(sel, token);
      scan();
      *params = 1;
      return true;
    } else
    if (toktyp == tokBinSel) {
      strcpy(sel, token);
      scan();
      if (!parseInstance()) {
       parseError(cerrExpInst);
       return false;
      }
      *params = 2;
      return true;
    } else
    if (toktyp == tokKeyword) {
      strcpy(sel,":");
      while (toktyp == tokKeyword) {
        strcat(strcat(sel,token),":");
        scan();
        if (!parseInstance()) {
          parseError(cerrExpInst);
          return false;
        }
        (*params)++;
      }
      return true;
    }
    return false;
  }
  mthEnd();
  return false;
}


/*--------------------------------------------------------------------------*/

static bool parseMethod()
{
  char sel[MAX_SELLEN];
  uint params;
  Obj prim;

  inList = false;
  if (!parseSyntax(sel, &params))
    return false;
  mthStart(sel, params);
  if (!checkSym("."))
    return false;
  if (!strcmp(token, "#")) {
    scan();
    if (!stricmp(token, "DEFERRED")) {
      scan();
      mthDeferred();
      mthEnd();
      return true;
    } else
    if (!stricmp(token, "PRIMITIVE")) {
      scan();
      if (!check(tokGlobal))
        return false;
      strcpy(sel,token);
      scan();
      if (!check(tokNumber))
        return false;
      prim = asNumber(tokval.tokNum);
      if (!prim) {
        parseError(cerrNumber);
        return false;
      }
      if (objType(prim) != otInt) {
        parseError(cerrNumber);
        return false;
      }
      scan();
      mthPrim(sel,asUInt(prim));
      mthEnd();
      return true;
    }
    parseError1(cerrUnknownCompInstr, token);
    return false;
  } else {
    codeStart();
    if (!parseStatements(false))
      return false;
    codeEnd();
    mthEnd();
    return true;
  }
}

/*--------------------------------------------------------------------------*/

static bool parseClass()
{
  if (toktyp != tokGlobal)
    return false;
  clsNew(token);
  scan();
  if (!checkSym("("))
    return false;
  while (toktyp == tokGlobal) {
    clsAddSuper(token);
    scan();
  }
  if (!checkSym(")"))
    return false;
  clsStart();
  while (toktyp == tokLocal) {
    clsAddIVar(token);
    scan();
  }
  clsEnd();
  return true;
}


/*--------------------------------------------------------------------------*/

static bool parsePurpose()
{
  List purpose;

  if (strcmp(token, "#"))
    return false;
  scan();
  if (!check(tokGlobal))
    return false;
  if (!stricmp(token, "PUBLIC")) {
    purpScope(true);
    scan();
    return true;
  }
  if (!stricmp(token, "PRIVATE")) {
    purpScope(false);
    scan();
    return true;
  }
  if (stricmp(token, "PURPOSE")) {
    parseError1(cerrUnknownCompInstr, token);
    return false;
  }
  scan();
  if (!check(tokString))
    return false;
  compObjs->ins(purpose = newList(1));
  purpose->ins(newStr(tokval.tokStr));
  scan();
  while (!strcmp(token, ",")) {
    scan();
    if (!check(tokString))
      return false;
    purpose->ins(newStr(tokval.tokStr));
    scan();
  }
  purpName(purpose);
  compObjs->del(purpose);
  return true;
}


/*--------------------------------------------------------------------------*/

static bool parseClasses()
{
  if (stricmp(token, "CLASSES"))
    return false;
  scan();
  for (;;) {
    if (parsePurpose()) {
    } else
    if (parseClass()) {
      if (!checkSym("@"))
        return false;
    } else
      break;
  }
  return (checkSym("@"));
};


/*--------------------------------------------------------------------------*/

static bool parseMethods()
{
  if (stricmp(token, "METHODS"))
    return false;
  scan();
  for (;;) {
    if (parsePurpose()) {
    } else
    if (parseMethod()) {
      if (!checkSym("@"))
        return false;
    } else
      break;
  }
  return checkSym("@");
};


/*--------------------------------------------------------------------------*/

static bool parseGlobals()
{
  if (stricmp(token, "GLOBALS"))
    return false;
  scan();
  for (;;) {
    if (parsePurpose()) {
    } else
    if (toktyp == tokGlobal) {
      symNewGlobal(token);
      scan();
    } else
      break;
  }
  return (checkSym("@"));
};

/*--------------------------------------------------------------------------*/

static bool parseInit()
{
  if (stricmp(token, "INIT"))
    return false;
  scan();
  mthNew();
  mthStart("", 0);
  codeStart();
  if (!parseStatements(false)) {
    parseError(cerrExpStatm);
    return false;
  }
  codeEnd();
  mthEnd();
  return (checkSym("@"));
};

/*--------------------------------------------------------------------------*/

static bool parseModul()
{
  if (stricmp(token, "MODUL")) {
    checkSym("MODUL");
    return false;
  }
  scan();
  if (!check(tokGlobal))
    return false;
  if (!modNew(token))
    return false;
  scan();
  if (!stricmp(token, "IMPORT")) {
    scan();
    if (!check(tokGlobal))
      return false;
    while (toktyp == tokGlobal) {
      if (!modImport(token))
        return false;
      scan();
    }
    if (!checkSym("@"))
      return false;
  }
  modStart();
  for (;;) {
    if (parseClasses()) {
    } else
    if (parseMethods()) {
    } else
    if (parseGlobals()) {
    } else
    if (parseInit()) {
    } else
      break;
  };
  if (stricmp(token, "END")) {
    checkSym("END");
    return false;
  }
  scan();
  return true;
};

/*--------------------------------------------------------------------------*/

static const char* errs[] = {
  "no error",
// Scanner
  "unexpected end of file",
  "unknown character",
  "digit expected",
  "in number",
  "identifier too long",
  "string too long",
  "number too long",
  "no '.' expected",
// Parser
  "unary expression expected",
  "binary expression expected",
  "keyword expression expected",
  "expression expected",
  "expressions expected",
  "statements expected",
  "instance expected",
// Codegen
  "too many parameters",
  "too many literals",
  "'%s' is not defined",
  "'%s' was already defined",
  "symbol conflict in '%s'",
  "not an instance",
  "can't cast instance to '%s'",
  "instance variable '%s' is not defined",
  "can't store to",
  "illegal use of block method %s",
  "method '%s' was already defined",
  "method '%s' is not defined",
  "unknown compiler instruction '%s'",
  "unknown primitive class '%s'",
// Class
  "superclass already used",
  "instance variable '%s' already defined",
// Modul
  "can't find module '%s'",
  "circular dependency in '%s'"
};

static const char *tokens[] = {
  "end",
  "",
  "",
  "terminal",
  "character",
  "string",
  "number",
  "local",
  "global",
  "binary selector",
  "keyword"
};

static uint tokIndex(uint tok)
{
  uint i = 0;

  while (tok) { tok >>= 1; i++; }
  return i;
}

void showError(const char *msg)
{
  console("%s\n", msg);
}

static void makeErrorMsg(char *msg)
{
  sprintf(msg, " ERROR \"%s\"(%u|%u): ", cError.modul,
                cError.pos.line, cError.pos.pos);
  msg = strchr(msg,0);
  switch (cError.typ) {
    case cerrCode:
      {
        const char *fmt = errs[cError.val.code];
        switch (cError.params) {
          case 0:
            strcpy(msg, fmt);
                break;
          case 1:
            sprintf(msg, fmt, cError.param[0]);
                break;
          case 2:
            sprintf(msg, fmt, cError.param[0], cError.param[1]);
                break;
        };
      };
      break;

    case cerrExpTok:
      sprintf(msg, "%s expected", tokens[tokIndex(cError.val.tok)]);
      break;

    case cerrExpSym:
      sprintf(msg, "'%s' expected", cError.val.sym);
      break;
    case cerrNone: ;
    }
}

Modul compile(const char *source, const char *name)
{
  Modul m;
  char  msg[256];

  strcpy(curName, name);
  varCached = false;
  if (!initScanner(source)) {
    parseError1(cerrCantLoadModul, source);
    return NULL;
  }
  scan();
  m = modEnd(!parseModul());
  doneScanner();
  if (!m) {
    if (cError.typ != cerrNone) {
      makeErrorMsg(msg);
      showError(msg);
    }
    cError.typ = cerrNone;
  }
  return m;
};

#else

void parserDummy() {} // dummy for ansi compatibility

#endif // NOCOMPILER

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
