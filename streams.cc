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

#ifndef OTHER_HPP
#include "other.hpp"
#endif

#ifndef SYSDEP_HPP
#include "sysdep.hpp"
#endif

#ifndef NOCOMPILER
extern Modul compile(const char *, const char *);
#endif

#include "streams.hpp"

Class clsExtStream;

/*--------------------------------------------------------------------------*/

static List specialClasses;

/*--------------------------------------------------------------------------*/

bool streamsInit()
{
  if (modulesStarted.streams)
        return true;
  sysObjs->ins(specialClasses = newList(4));
  clsExtStream = createClass("ExternalStream", IVars(TExtStream),
                                vtNone + optBinary);
  specialClasses->ins(clsClass);
  specialClasses->ins(clsGlobal);
  specialClasses->ins(clsGenericMethod);
  specialClasses->ins(clsModul);
  modulesStarted.streams = 1;
  return true;
}

void streamsDone()
{
  if (!modulesStarted.streams)
        return;
  modulesStarted.streams = 0;
  sysObjs->del(specialClasses);
}

/*--------------------------------------------------------------------------*/

void TExtStream::collectObjs(Obj from)
{
  uint  i, l;
  Modul m;

  if ( (!from) || (objType(from) != otObj) ||
       (from == objTrue) || (from == objFalse) )
    return;
  i = specialClasses->indexOf(classOf(from),1);
  if (i) {
    switch (i) {
     case 1: m = ((Class)from)->modul; break;
     case 2: m = ((Global)from)->modul; break;
     case 3: m = ((GenericMethod)from)->modul; break;
     case 4:
       if (from != exc->savModul) {
         exc->impModuls->insAbsent(from);
         return;
       }
       m = exc->savModul;
       break;
    }
    if (m != exc->savModul) {
      exc->impModuls->insAbsent(m);
      exc->impLinks->insAbsent(from);
      return;
    }
  }
  if (!exc->instances->insAbsent(from))
    return;
  if (from->_opt.varType == vtReal) {
    if (!exc->sysModul)
      exc->impLinks->insAbsent(clsReal);
    return;
  }
  l = from->instVars();
  for (i = 0; i <= l; i++)
    collectObjs(from->ivarAt[i]);
  if (from->_opt.varType != vtObjs)
    return;
  l = limit(from);
  for (i = 1; i <= l; i++)
    collectObjs(from->at[i]);
}

bool TExtStream::readModuls()
{
  uint  i;
  DWord n;
  char  mn[MAX_IDLEN];
  Modul m;

  for (i = 1; i <= exc->header.moduls; i++) {
    if (!fread(&n,  sizeof(n), 1, exc->stream))
      return false;
    n = conv(n);
    if (fread(&mn[0], sizeof(char), (size_t)n, exc->stream) != n)
      return false;
    mn[(uint)n] = '\0';
    m = loadModul(mn);
    if (!m)
      return false;
    exc->impModuls->ins(m);
  }
  return true;
}

bool TExtStream::writeModuls()
{
  uint  i;
  DWord n;
  char  mn[MAX_IDLEN];

  for (i = 1; i <= exc->header.moduls; i++) {
    n = strlen(asStr(mn,((Modul)exc->impModuls->at[i])->name));
    n = conv(n);
    if (!fwrite(&n,  sizeof(n), 1, exc->stream))
      return false;
    n = conv(n);
    if (fwrite(&mn[0], sizeof(char), (size_t)n, exc->stream) != n)
      return false;
  }
  return true;
}

bool TExtStream::readLinks()
{
  uint  i;
  DWord n;
  Modul m;

  for (i = 1; i <= exc->header.moduls; i++) {
    m = (Modul)exc->impModuls->at[i];
    for (;;) {
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
      n = conv(n);
      if (!n) break;
      switch (n & 3) {
        case 1 : exc->impLinks->ins(m->classes->at[(uint)n >> 2]); break;
        case 2 : exc->impLinks->ins(m->globals->at[(uint)n >> 2]); break;
        case 3 : exc->impLinks->ins(m->genericMethods->at[(uint)n >> 2]); break;
      }
    }
  }
  return true;
}

bool TExtStream::writeLinks()
{
  uint  i, j;
  DWord n;
  Modul m;
  Obj   link;
  List  newLinks;

  sysObjs->ins(newLinks = newList((uint)exc->header.links));
  for (i = 1; i <= exc->header.moduls; i++) {
    m = (Modul)exc->impModuls->at[i];
    for (j = 1; j <= exc->header.links; j++) {
      link = exc->impLinks->at[j];
      n = specialClasses->indexOf(link->iClass,1);
      switch (n) {
        case 1 :
          if (((Class)link)->modul != m) continue;
          n = (m->classes->indexOf(link,1) << 2) + 1;
          break;
        case 2 :
          if (((Global)link)->modul != m) continue;
          n = (m->globals->indexOf(link,1) << 2) + 2;
          break;
        case 3 :
          if (((GenericMethod)link)->modul != m) continue;
          n = (m->genericMethods->indexOf(link,1) << 2) + 3;
          break;
      }
      n = conv(n);
      if (!fwrite(&n, sizeof(n), 1, exc->stream)) {
        sysObjs->del(newLinks);
        return false;
      }
      newLinks->ins(link);
    }
    n = 0;
    if (!fwrite(&n, sizeof(n), 1, exc->stream)) {
      sysObjs->del(newLinks);
      return false;
    }
  }
  sysObjs->del(exc->impLinks);
  exc->impLinks = newLinks;
  return true;
}

bool TExtStream::readObj(Obj *obj)
{
  Obj   tmp;
  DWord n;

  if (!fread(&n, sizeof(n), 1, exc->stream))
    return false;
  tmp = (Obj)(uint)conv(n);
  if ((objType(tmp) == otObj) && (tmp)) {
    n = (DWord)(uint)tmp >> 2;
    if (n <= exc->linkStart) tmp = exc->impModuls->at[(uint)n];
    else if (n > exc->instStart + 2)
      tmp = exc->instances->at[(uint)n - exc->instStart - 2];
    else if (n == exc->instStart + 1) tmp = objTrue;
    else if (n == exc->instStart + 2) tmp = objFalse;
    else if (n <= exc->instStart) {
     if (!exc->sysModul)
       tmp = exc->impLinks->at[(uint)n - exc->linkStart];
     else
       tmp = classes->at[(uint)n];
    }
  };
  *obj = tmp;
  return true;
}

bool TExtStream::writeObj(Obj obj)
{
  DWord n;

  n = 0;
  if ((!obj) || objType(obj) != otObj)
    n = (DWord)(uint)obj;
  else if (exc->sysModul && (n = classes->indexOf(obj,1)))
    n = n << 2;
  else if ((n = exc->instances->indexOf(obj,1)))
    n = (n + 2 + exc->instStart) << 2;
  else if (obj == objTrue)
    n = (1 + exc->instStart) << 2;
  else if (obj == objFalse)
    n = (2 + exc->instStart) << 2;
  else if ((n = exc->impLinks->indexOf(obj,1)))
    n = (n + exc->linkStart) << 2;
  else if ((n = exc->impModuls->indexOf(obj,1)))
    n = n << 2;
  n = conv(n);
  return (fwrite(&n, sizeof(n), 1, exc->stream) == 1);
}

bool TExtStream::readInstances()
{
  uint  i, j, l;
  DWord n;
  Obj   inst;

// read class of each instance
  for (i = 1; i <= exc->header.instances; i++) {
    if (!readObj(&inst))
      return false;
    if (inst != clsReal)
      exc->instances->ins(((Class)inst)->create());
    else
      exc->instances->ins(newReal(0));
  }
  if (exc->sysModul) {
    j = size(classes);
    for (i = 1; i <= j; i++) {
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
     n = conv(n);
     if (n)
       exc->instances->at[(uint)n] = classes->at[i];
    }
  }
// read each instance
  for (i = 1; i <= exc->header.instances; i++) {
    inst = exc->instances->at[i];
    if (inst->_opt.varType != vtReal) {
      l = inst->instVars();
      for (j = 1; j <= l; j++)
        if (!readObj(&inst->ivarAt[j]))
          return false;
      if (inst->_opt.varType == vtNone)
        continue;
      if (!fread(&n, sizeof(n), 1, exc->stream))
        return false;
      n = conv(n);
      switch (inst->_opt.varType) {
        case vtBytes:
          inst->setLimit(byteLimit((uint)n));
          if (fread(&inst->atByte[Byte0+1], 1, (size_t)n, exc->stream) != n)
            return false;
          break;
        case vtWords:
          inst->setLimit(wordLimit((uint)n));
          if (fread(&inst->atWord[Word0+1], sizeof(Word), (size_t)n, exc->stream) != n)
            return false;
#ifdef BIGENDIAN
          for (j = 1; j <= n; j++)
            inst->atWord[Word0+j] = convw(inst->atWord[Word0+j]);
#endif
          break;
        case vtObjs:
          inst->setLimit((uint)n);
          for (j = 1; j <= n; j++)
            if (!readObj(&inst->at[j]))
              return false;
          break;
      }
    } else {    // write real
// ??? PORTABLE ???
      if (!fread(&((Real)inst)->value, sizeof(double), 1, exc->stream))
        return false;
    };
  }
  return true;
}

bool TExtStream::writeInstances()
{
  uint  i, j, l;
  DWord n;
  Obj   inst;

// write class of each instance
  for (i = 1; i <= exc->header.instances; i++)
    if (!writeObj(classOf(exc->instances->at[i])))
      return false;
  if (exc->sysModul) {
    j = size(classes);
    for (i = 1; i <= j; i++) {
      n = exc->instances->indexOf(classes->at[i],1);
      n = conv(n);
      if (!fwrite(&n, sizeof(n), 1, exc->stream))
        return false;
    }
  }
// write each instance
  for (i = 1; i <= exc->header.instances; i++) {
    inst = exc->instances->at[i];
    if (inst->_opt.varType != vtReal) {
      l = inst->instVars();
      for (j = 1; j <= l; j++)
        if (!writeObj(inst->ivarAt[j]))
          return false;
      if (inst->_opt.varType == vtNone)
        continue;
      n = limit(inst);
      switch (inst->_opt.varType) {
        case vtBytes:
          n = conv(n * BytesPerPtr);
          if (!fwrite(&n, sizeof(n), 1, exc->stream))
            return false;
          if (fwrite(&inst->atByte[Byte0+1], 1, (size_t)n, exc->stream) != n)
            return false;
          break;
        case vtWords:
          n = conv(n * WordsPerPtr);
          if (!fwrite(&n, sizeof(n), 1, exc->stream))
            return false;
          n = conv(n);
#ifdef BIGENDIAN
          for (j = 1; j <= n; j++)
            inst->atWord[Word0+j] = convw(inst->atWord[Word0+j]);
#endif
          if (fwrite(&inst->atWord[Word0+1], sizeof(Word), (size_t)n, exc->stream) != n)
            return false;
#ifdef BIGENDIAN
          for (j = 1; j <= n; j++)
            inst->atWord[Word0+j] = convw(inst->atWord[Word0+j]);
#endif
          break;
        case vtObjs:
          n = conv(n);
          if (!fwrite(&n, sizeof(n), 1, exc->stream))
            return false;
          n = conv(n);
          for (j = 1; j <= n; j++)
            if (!writeObj(inst->at[j]))
              return false;
          break;
      }
    } else {    // write real
// ??? PORTABLE ???
      if (!fwrite(&((Real)inst)->value, sizeof(double), 1, exc->stream))
        return false;
    };
  }
  return true;
}

/*--------------------------------------------------------------------------*/

bool TExtStream::open(char *fname, uint mode)
{
#define SOLSign "SOL\0" // 0x004c4f53L
  char  signature[4];
  const char    *openmode;

  status = objFalse;
  if (mode & openCreate) {
    if (mode & openWrite)
      openmode = CREATEBINARY;
    else
      return false;
  } else
  if (mode & openRead) {
    if (mode & openWrite)
      openmode = UPDATEBINARY;
    else
      openmode = READBINARY;
  } else
    openmode = WRITEBINARY;
  exc = new ExtStreamContext;
  exc->stream = fopen(fname,openmode);
  if (exc->stream) {
    if (mode & openRead) {
      if (!fread(&signature[0], sizeof(signature), 1, exc->stream))
        goto error;
      if (strncmp(signature,SOLSign,4))
        goto error;
    } else
    if (mode & openWrite) {
      strncpy(signature,SOLSign,4);
      if (!fwrite(&signature[0], sizeof(signature), 1, exc->stream))
        goto error;
    }
    sysObjs->ins(exc->impModuls = newList(4));
    sysObjs->ins(exc->impLinks = newList(16));
    sysObjs->ins(exc->instances = newList(16));
    status = objTrue;
    return true;
  error:
    fclose(exc->stream);
  }
  delete exc;
  return false;
}

void TExtStream::close()
{
  sysObjs->del(exc->impModuls);
  sysObjs->del(exc->impLinks);
  sysObjs->del(exc->instances);
  fclose(exc->stream);
  delete exc;
}

Obj  TExtStream::get()
{
  Obj result;

  if (status != objTrue)
    return NULL;
  if (fread(&exc->header, sizeof(exc->header), 1, exc->stream)) {
    exc->header.moduls = conv(exc->header.moduls);
    exc->header.links = conv(exc->header.links);
    exc->header.instances = conv(exc->header.instances);
    exc->impModuls->setLimit((uint)exc->header.moduls);
    exc->impLinks->setLimit((uint)exc->header.links);
    exc->instances->setLimit((uint)exc->header.instances);
    exc->linkStart = (uint)exc->header.moduls;
    exc->sysModul = (uint)exc->header.moduls == 0;
    if (!exc->sysModul)
      exc->instStart = exc->linkStart + (uint)exc->header.links;
    else {
      exc->instStart = size(classes);
    }
    if (readModuls()) // read imported moduls
     if (readLinks()) // read imported links
      if (readInstances()) { // read classes + instances
        sysObjs->ins(result = exc->instances->at[1]);
        exc->impModuls->resizeTo(0);
        exc->impLinks->resizeTo(0);
        exc->instances->resizeTo(0);
        sysObjs->del(result);
        return result;
      }
  }
  return NULL;
}

void TExtStream::put(Obj obj)
{
  if (status != objTrue)
    return;
  exc->savModul = NULL;
  if (classOf(obj) == clsModul)
    exc->savModul = (Modul)obj;
  exc->sysModul = (findModul(SYSMODUL) == NULL);
// collect subobjects
  collectObjs(obj);
// write header
  exc->header.moduls = conv(size(exc->impModuls));
  exc->header.links = conv(size(exc->impLinks));
  exc->header.instances = conv(size(exc->instances));
  if (fwrite(&exc->header, sizeof(exc->header), 1, exc->stream) == 1) {
    exc->header.moduls = conv(exc->header.moduls);
    exc->header.links = conv(exc->header.links);
    exc->header.instances = conv(exc->header.instances);
    exc->linkStart = size(exc->impModuls);
    if (!exc->sysModul)
      exc->instStart = exc->linkStart + size(exc->impLinks);
    else
      exc->instStart = size(classes);
    if (writeModuls()) // write imported moduls
     if (writeLinks()) // write imported links
      if (writeInstances()) // write classes + instances
        return;
  }
  status = objFalse;
}

/*--------------------------------------------------------------------------*/

Modul findModul(const char *name)
{
  uint i, l;
  String n;

  n = newStr(name);
  l = size(moduls);
  for (i = 1; i <= l; i++) {
    if (isEqual(((Modul)(moduls->at[i]))->name,n))
      return ((Modul)(moduls->at[i]));
  }
  return NULL;
}

Modul loadModul(const char *name)
{
  Modul m;
  char src[256],str[256];
  TTime a,b;
  bool srcExists,strExists;
  ExtStream st;

  m = findModul(name);
  if (m) {
    m->references = newInt(asInt(m->references) + 1);
    return m;
  }
  strcat(strcpy(src,name),".mod");
  strcat(strcpy(str,name),".sol");
  srcExists = a.ofFile(src);
  strExists = b.ofFile(str);
  if (strExists) {
     if (srcExists && !a.before(&b))
       goto recompile;
     console(" loading %s",name);
     sysObjs->ins(st = newInst(ExtStream));
     if (st->open(str, openRead)) {
       if (debug)
         m = (Modul)st->debug();
       else
         m = (Modul)st->get();
       console("\n");
       st->close();
       if (m) {
        sysObjs->del(st);
        moduls->ins(m);
        m->doInit();
        return m;
       }
     }
     sysObjs->del(st);
  recompile: ;
#ifndef NOCOMPILER
     console("\r recompiling %s\n",name);
     return compile(src, name);
#endif
  } else
  if (srcExists) {
#ifndef NOCOMPILER
    console(" compiling %s\n",name);
    return compile(src, name);
#endif
  }
  console(" ERROR: modul %s not found\n",name);
  return NULL;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
