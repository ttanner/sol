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

#include <ctype.h>
#include <string.h>

#ifndef NOCOMPILER

#include <math.h>
#include <stdlib.h>

#ifndef SYSDEP_HPP
#include "sysdep.hpp"
#endif

#include "compiler.hpp"

/*--------------------------------------------------------------------------*/
TScanContext scc;
uint    toktyp;
char    token[MAX_IDLEN];
TTokVal tokval;
uint    tabSize = 8;

static char next();
/*--------------------------------------------------------------------------*/

bool initScanner(const char *src)
{

  strcpy(scc.name, src);
  scc.source = fopen(src,READBINARY);
  if (!scc.source)
    return false;
#if COMP != TURBOC
  char *buf = (char*)malloc(SRCBUFSIZE);
  if (!buf) {
    fclose(scc.source);
    return false;
  }
  if (setvbuf(scc.source,buf,_IOFBF,SRCBUFSIZE)) {
    fclose(scc.source);
    return false;
  }
#endif
  scc.srcpos.line = 1;
  scc.srcpos.pos = 0;
  scc.cacheused = false;
  scc.cur = next();
  return true;
}

void scannerSave()
{
  if (scc.source) {
    scc.fpos = ftell(scc.source);
    fclose(scc.source);
  }
}

bool scannerRestore()
{
  scc.source = fopen(scc.name, READBINARY);
  if (!scc.source)
    return false;
  return !fseek(scc.source, scc.fpos, SEEK_SET);
}

void doneScanner()
{
  if (scc.source)
    fclose(scc.source);
}

/*--------------------------------------------------------------------------*/

const char WhiteSpace[5]        = {' ','\n','\t','\r'};
const char Symbol[12]           = {'+','*','/','\\','&',
                                   '|','~','<','>','=','#'};
const char Terminal[12]         = {'[',']','{','}','(',')',
                                   '.',',','@',':','^'};
//const char WhiteSpace[5]      = " \n\t\r";
//const char Symbol[12]         = "+*/\\&|~<>=#";
//const char Terminal[12]       = "[]{}().,@:^";

static char cur;
/*--------------------------------------------------------------------------*/

static char next()
{
  int tmp;
    
  if (scc.cacheused) {
    cur = scc.cache;
    scc.cacheused = false;
  } else
  if ((tmp = fgetc(scc.source)) != EOF)
    cur = tmp;
  else
    cur = 0;
  if (cur == '\r') scc.srcpos.pos = 0;
  else if (cur == '\n') scc.srcpos.line++;
  else if (cur == '\t') scc.srcpos.pos += tabSize;
  else scc.srcpos.pos++;
  return cur;
}

/*--------------------------------------------------------------------------*/

char *scanSet(char *Result, const char *s)
{
  char tmp[MAX_IDLEN];
  char *p;
  uint len;

  len = 1;
  p = tmp;
  while ((strchr(s,cur) != NULL) && len < MAX_IDLEN) {
    *p++ = cur;
    len++;
    next();
  }
  *p = '\0';
  if (len >= MAX_IDLEN) {
    scanError(cerrIDTooLong);
    return NULL;
  }
  return strcpy(Result, tmp);
}

static bool scanExp()
{
  char *p;
  uint len;

  next();
  if (cur == '-') {
    tokval.tokNum.numExpS = true;
    next();
  } else
    tokval.tokNum.numExpS = false;
  if (isdigit(cur)) {
   len = 1;
   p = tokval.tokNum.numExp;
   while (isdigit(cur) && len < MAX_NREXPLEN) {
     *p++ = cur;
     len++;
     next();
   }
   *p = '\0';
   if (len >= MAX_NREXPLEN) {
     scanError(cerrNumTooLong);
     return false;
   }
   return true;
  } else {
    scanError(cerrExpectDigit);
    return false;
  }
}

void scanNumber(bool neg)
{
  char tmp[MAX_NRINTLEN];
  char *p;
  uint len;

  toktyp = tokNumber;
  *tokval.tokNum.numRad = '\0';
  *tokval.tokNum.numInt = '\0';
  *tokval.tokNum.numFrac = '\0';
  *tokval.tokNum.numExp = '\0';
  len = 1;
  p = tmp;
  while (isdigit(cur) && len < MAX_NRINTLEN) {
    *p++ = cur; len++; next();
  }
  *p = '\0';
  if (len >= MAX_NRINTLEN) {
     scanError(cerrNumTooLong);
     return;
  }
  switch (cur) {

  case 'r':
    if (len >= MAX_NRRADLEN) {
       scanError(cerrNumTooLong);
       return;
    }
    strcpy(tokval.tokNum.numRad, tmp);
    if (next() == '-') {
      neg = !neg; next();
    }
    len = 1;
    p = tokval.tokNum.numInt;
    while ((isdigit(cur) || isupper(cur)) && len < MAX_NRINTLEN) {
      *p++ = cur; len++; next();
    }
    *p = '\0';
    if (len >= MAX_NRINTLEN) {
      scanError(cerrNumTooLong);
      return;
    }
    if (cur == '.') {
      next();
      if (isupper(cur) || isdigit(cur)) {
        len = 1;
        p = tokval.tokNum.numFrac;
        while ((isdigit(cur) || isupper(cur)) && len < MAX_NRFRCLEN) {
          *p++ = cur; len++; next();
        }
        *p = '\0';
        if (len >= MAX_NRFRCLEN) {
          scanError(cerrNumTooLong);
          return;
        }
        if (cur == 'e')
          if (!scanExp())
            return;
      } else {
        scc.cache = cur;
        scc.cacheused = true;
        cur = 'r';
      }
    } else
    if (cur == 'e')
      if (!scanExp())
        return;
    break;

  case '.':
    strcpy(tokval.tokNum.numInt, tmp);
    if (isdigit(next())) {
      len = 1;
      p = tokval.tokNum.numFrac;
      while (isdigit(cur) && len < MAX_NRFRCLEN) {
        *p++ = cur; len++; next();
      }
      *p = '\0';
      if (len >= MAX_NRFRCLEN) {
        scanError(cerrNumTooLong);
        return;
      }
      if (cur == 'e')
        if (!scanExp())
          return;
    } else {
      scc.cache = cur;
      scc.cacheused = true;
      cur = '.';
    }
    break;

  case 'e':
    strcpy(tokval.tokNum.numInt, tmp);
    if (!scanExp())
      return;
    break;

  default:
    strcpy(tokval.tokNum.numInt, tmp);
    break;
  }
  tokval.tokNum.numIntS = neg;
}

/*--------------------------------------------------------------------------*/

void scan()
{
  char *p;
  uint len;

  if (toktyp == tokError)
    return;
  cur = scc.cur;
  do {
    *token = '\0';
    while (cur != 0 && strchr(WhiteSpace,cur))
      next();
    scc.tokpos = scc.srcpos;
    if (cur == '\0')
      toktyp = tokEnd;
    /*--------------------------------------------------------------------------*/
    if (strchr(Terminal,cur)) {
      toktyp = tokTerminal;
      if (cur == ':') {
        next();
        if (cur == '=') {
          strcpy(token, ":=");
          next();
        } else
          strcpy(token, ":");
      } else {
        token[0] = cur;
        token[1]= '\0';
        next();
      }
    } else
    /*--------------------------------------------------------------------------*/
    if (strchr(Symbol,cur)) {
      if (scanSet(token, Symbol))
        toktyp = tokBinSel;
    } else
    /*--------------------------------------------------------------------------*/
    if (islower(cur)) {
      len = 1;
      p = token;
      while ((isdigit(cur) || isalpha(cur)) && len < MAX_IDLEN) {
        *p++ = cur; len++; next();
      }
      *p = '\0';
      if (len >= MAX_IDLEN) {
        scanError(cerrIDTooLong);
        return;
      }
      if (cur == ':') {
        if (next() == '=') {
          cur = ':';
          scc.cache = '=';
          scc.cacheused = true;
          toktyp = tokLocal;
        } else
          toktyp = tokKeyword;
      } else
        toktyp = tokLocal;
    } else
    /*--------------------------------------------------------------------------*/
    if (isupper(cur)) {
      len = 1;
      p = token;
      while ((isdigit(cur) || isalpha(cur)) && len < MAX_IDLEN) {
        *p++ = cur; len++; next();
      }
      *p = '\0';
      if (len >= MAX_IDLEN) {
        scanError(cerrIDTooLong);
        return;
      }
      toktyp = tokGlobal;
    } else
    /*--------------------------------------------------------------------------*/
    if (isdigit(cur))
      scanNumber(false);
    else
    /*--------------------------------------------------------------------------*/
    if (cur == '-') {
      next();
      if (cur == '>') {
        next();
        strcpy(token, "->");
        toktyp = tokTerminal;
      } else {
        if (isdigit(cur) && !scc.binsel)
          scanNumber(true);
        else {
          strcpy(token, "-");
          toktyp = tokBinSel;
        }
      }
    } else
    /*--------------------------------------------------------------------------*/
    if (cur == '\'') {
      toktyp = tokString;
      len = 1;
      p = tokval.tokStr;
      *p = '\0';
      for (;;) {
        next();
        while (cur != '\'' && cur != '\0' && len < MAX_STRLEN) {
          *p++ = cur; len++; next();
        }
        if (len >= MAX_STRLEN)
          break;
        if (cur == '\0')
          scanError(cerrEOF);
        else {
          if ((next() == '\'') && len < MAX_STRLEN) {
            *p++ = '\''; len++;
          } else
           break;
        }
      }
      *p = '\0';
      if (len >= MAX_STRLEN)
        scanError(cerrStrTooLong);
    } else
    /*--------------------------------------------------------------------------*/
    if (cur == '"') {
      toktyp = tokComment;
      for (;;) {
        next();
        while (cur != '"' && cur != 0)
          next();
        if (!cur)
          scanError(cerrEOF);
        else if (next() != '"')
          break;
      };
    } else
    /*--------------------------------------------------------------------------*/
    if (cur == '$') {
      next();
      if (cur != '\0') {
        toktyp = tokCharacter;
        tokval.tokChar = cur;
        next();
      } else
        scanError(cerrEOF);
    } else
      scanError(cerrChar);
    /*--------------------------------------------------------------------------*/
  } while (toktyp == tokComment);
  scc.cur = cur;
}

/*--------------------------------------------------------------------------*/

static int toNum(char c)
{
  if (isdigit(c))
    return (c - '0');
  else
    return (c + 10 - 'A');
}


Obj asNumber(TNumTok tok)
{
  int i, l, rad;
  int intpart, expn;
  double frac;

  if (*tok.numRad != '\0') {
    if (sscanf(tok.numRad, "%d", &rad) != 1)
      return NULL;
    if (rad < 2 || rad > 36)
      return NULL;
  } else
    rad = 10;
  intpart = 0;
  l = strlen(tok.numInt);
  for (i = 0; i < l; i++)
    intpart = intpart * rad + toNum(tok.numInt[i]);
  if (tok.numIntS)
    intpart = -intpart;
  if (*tok.numFrac == '\0' && *tok.numExp == '\0')
    return (newOInt(intpart));
  frac = 0.0;
  for (i = strlen(tok.numFrac) - 1; i >= 0; i--)
    frac = toNum(tok.numFrac[i]) + frac / rad;
  frac /= rad;
  if (tok.numIntS)
    frac = -frac;
  frac = (double)intpart + frac;
  if (*tok.numExp != '\0') {
    sscanf(tok.numExp, "%d", &expn);
    if (tok.numExpS)
      expn = -expn;
    frac *= exp(expn * log(rad));
  }
  return (newReal(frac));
}

#else

void scannerDummy() {} // dummy for ansi compatibility

#endif // NOCOMPILER

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
