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

#ifndef STREAMS_HPP
#define STREAMS_HPP

#include <stdio.h>

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef OBJECTS_HPP
#include "objects.hpp"
#endif

/*--------------------------------------------------------------------------*/

#define openRead        1
#define openWrite       2
#define openCreate      4

#define SYSMODUL "System"

typedef struct {
  DWord moduls;
  DWord links;
  DWord instances;
} THeader;

typedef struct {
  List  impModuls, impLinks, instances;
  bool  sysModul;
  Modul savModul;
  FILE  *stream;
  THeader header;
  uint  linkStart,instStart;
} ExtStreamContext;

class   TExtStream : public TObj {
 public:
  Obj  status;
  ExtStreamContext *exc;

  bool open(char *fname, uint mode);
  void close();
  Obj  get();
  Obj  debug();
  void put(Obj obj);
 private:
  bool readModuls();
  bool readLinks();
  bool readObj(Obj *obj);
  bool readInstances();

  void collectObjs(Obj from);
  bool writeModuls();
  bool writeLinks();
  bool writeObj(Obj obj);
  bool writeInstances();
#ifdef DEBUG
  bool debugModuls();
  bool debugLinks();
  bool debugObj(Obj *obj, uint nr);
  bool debugInstances();
#endif
};

typedef TExtStream  *ExtStream;

extern Class clsExtStream;

extern bool startModul(const char *);
extern Modul findModul(const char *);
extern Modul loadModul(const char *);

/*--------------------------------------------------------------------------*/

#endif /*STREAMS_HPP*/
