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
#include <stdlib.h>
#include <string.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef DEBUG_HPP
#include "debug.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

#ifndef STREAMS_HPP
#include "streams.hpp"
#endif

#ifndef SYSDEP_HPP
#include "sysdep.hpp"
#endif

extern bool mainInit();
extern void mainDone();

extern bool heapInit();
extern void heapDone();

extern bool otherInit();
extern void otherDone();

extern bool streamsInit();
extern void streamsDone();

extern bool interpreterInit();
extern void interpreterDone();

extern bool debugInit();
extern void debugDone();

#ifndef NOCOMPILER
extern bool codegenInit();
extern void codegenDone();
#endif

/*--------------------------------------------------------------------------*/

#define Version       "0.1.0"

#if   OS == DOS && BITS == 16
#define Target        "DOS16"
#elif OS == DOS && BITS == 32
#define Target        "DOS32"
#elif OS == WIN && BITS == 32
#define Target        "Win32"
#elif OS == LINUX
#define Target        "Linux"
#else
#define Target        "Unknown"
#endif

/*--------------------------------------------------------------------------*/

int     pargc;
char    **pargv;
TModules modulesStarted = {0,0,0,0,0,0,0};
bool    test = true, show = false, debug = false, disableGC = false;

/*--------------------------------------------------------------------------*/

Word swapw(Word value)
{
  return (((Byte)value) << 8) | ((Byte)(value >> 8));
}

DWord swap(DWord value)
{
  return ((Byte)value << 24) | ((Byte)(value >> 8) << 16) |
         ((Byte)(value >> 16) << 8) | (Byte)(value >> 24);
}

int stricmp(char *s1,char *s2)
{
  uint i, l;
  int l1 = strlen(s1), l2 = strlen(s2);
  l = min(l1,l2);
  for(i = 0; i<l; i++)
   if (toupper(s1[i]) != toupper(s2[i]))
     return toupper(s1[i]) - toupper(s2[i]);
  return l1 - l2;
}

int strnicmp(char *s1,char *s2, size_t size)
{
  uint i, l;
  uint l1 = strlen(s1), l2 = strlen(s2);
  l = min(min(l1,l2),size);
  for(i = 0; i<l; i++)
   if (toupper(s1[i]) != toupper(s2[i]))
     return toupper(s1[i]) - toupper(s2[i]);
  return 0;
} 

/*--------------------------------------------------------------------------*/

void error(uint code)
{
  logfile("ERROR: %d",code);
  exit(EXIT_FAILURE);
}

void fatalError(uint code)
{
  logfile("FATAL ERROR: %d",code);
  exit(EXIT_FAILURE);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static bool modulsInit()
{
  console(" initializing.....");
  if (!debugInit()) {
    console("general error\n");
    return false;
  }
  if (!heapInit()) {
    console("heap error\n");
    return false;
  }
  if (!objectsInit()) {
    console("object world error\n");
    return false;
  }
  if (!otherInit()) {
    console("object world error\n");
    return false;
  }
  if (!streamsInit()) {
    console("object world error\n");
    return false;
  }
  if (!interpreterInit()) {
    console("interpreter error\n");
    return false;
  }
#ifndef NOCOMPILER
  if (!codegenInit()) {
    console("compiler error\n");
    return false;
  }
#endif
  console("ok\n");
  return true;
}


static void modulesDone()
{
#ifndef NOCOMPILER
  codegenDone();
#endif
  interpreterDone();
  streamsDone();
  otherDone();
  objectsDone();
  debugDone();
  heapDone();
}

/*--------------------------------------------------------------------------*/

static bool initSystem()
{
  char p[256];
  bool cfg;

  console(" reading config...");
  cfg = ReadConfig(strcat(GetDirectory(p, pargv[0]),PATHDIVIDER "sol.cfg"));
  if (!cfg)
    cfg = ReadConfig("sol.cfg");
  if (!cfg)
    console("not found. setting defaults\n");
  else
    console("ok\n");
#if COMP == TURBOC
  atexit((atexit_t)modulesDone);
#else
  atexit(modulesDone);
#endif
  if (!modulsInit())
    return false;
  console("\n");
  return true;
}

static void runSystem()
{
  uint i;

  if (!loadModul(SYSMODUL))
    return;
  for (i = 1; i < pargc; i++) {
    if (!loadModul(pargv[i]))
      return;
  }
}

static void doneSystem()
{
  console(" going done\n");
}

/*--------------------------------------------------------------------------*/

void SOLmain()
{
#ifndef NOCOMPILER
  console("\nSOL (R) - Virtual Machine and Compiler v" Version " for " Target "\n");
#else
  console("\nSOL (R) - Virtual Machine v" Version " for " Target "\n");
#endif
  console("Copyright (c) 1995-97 by Thomas Tanner. All Rights Reserved\n");
  console(" compiled on " __DATE__ " " __TIME__ "\n\n");

  if (initSystem())
    runSystem();
  doneSystem();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
