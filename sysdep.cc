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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#include "sysdep.hpp"

/*--------------------------------------------------------------------------*/


char *GetDirectory(char *Result, const char *FileName)
{
  int i = strlen(FileName);
#if OS == LINUX
  while (i > 0 && FileName[i - 1] != '/')
    i--;
  if (i > 1 && FileName[i - 1] == '/' && FileName[i - 2] != '/')
    i--;
#else
  while (i > 0 && FileName[i - 1] != ':' && FileName[i - 1] != '\\')
    i--;
  if (i > 1 && FileName[i - 1] == '\\' && FileName[i - 2] != ':' &&
      FileName[i - 2] != '\\')
    i--;
#endif
  strncpy(Result,FileName,i);
  Result[i] = '\0';
  return Result;
}

/*--------------------------------------------------------------------------*/

typedef enum { ctBool, ctChar, ctStr, ctInt, ctReal } cnfTyp;

union cnfValue {
  bool    cnfBool;
  char    cnfChar;
  char    cnfStr[67];
  int     cnfInt;
  double  cnfReal;
};

struct cnfRec {
  char   name[17];
  cnfTyp typ;
  void   *value;
};

#define cnfDefs         6

static cnfRec cnfDef[cnfDefs] = {
  { "test", ctBool, &test },
  { "show", ctBool, &show },
  { "debug", ctBool, &debug },
  { "disablegc", ctBool, &disableGC }
};

bool ReadConfig(const char *fname)
{
  FILE *t;
  char l[256];
  const char *p;
  int i,vi;
  double vr;

  t = fopen(fname,READTEXT);
  if (t == NULL)
    return false;
  while (fgets(l, 256, t) != NULL) {
    if (!strlen(l))
        continue;
    if (*l == '#')
        continue;
    p = strchr(l, '=');
    if (!p)
        continue;
    do p++; while (*p == ' ' || *p == '\b' || *p == 0 );
    if (*p == 0)
        continue;
    for (i = 0; i < cnfDefs; i++) {
      if (!strnicmp(cnfDef[i].name, l, strlen(cnfDef[i].name))) {
        switch (cnfDef[i].typ) {

        case ctBool:
          *(bool *)cnfDef[i].value = !strnicmp(p, "Yes", 3) ||
                                     (*p == '1') || (*p == '+');
          break;

        case ctChar:
          *(char *)cnfDef[i].value = *p;
          break;

        case ctStr:
          strcpy((char *)(&cnfDef[i].value), p);
          break;

        case ctInt:
          if (sscanf(p, "%d", &vi) == 1)
            *(long *)cnfDef[i].value = vi;
          break;

        case ctReal:
          if (sscanf(p, "%lg", &vr) == 1)
            *(double *)cnfDef[i].value = vr;
          break;

        default:
          continue;
        }
        break;
      }
    }
  }
  fclose(t);
  return true;
}

/*--------------------------------------------------------------------------*/

int console(const char *format, ...)
{
  va_list param;
  int result;

  va_start(param,format);
  result = vprintf(format,param);
  va_end(param);
  return result;
}

#if COMP == TURBOC
extern unsigned _stklen = 61440U;
#endif

int main(int argc, char *argv[])
{
  pargc = argc;
  pargv = argv;

  SOLmain();
  return 0;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
