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

#ifndef SYSDEP_HPP
#define SYSDEP_HPP

#ifndef SOL_HPP
#include "sol.hpp"
#endif

#ifndef _LINUX_
#define PATHDIVIDER  "\\"
#define CPATHDIVIDER '\\'
#define MASKALL      "*.*"
#define READBINARY   "rb"
#define READTEXT     "rt"
#define WRITEBINARY  "wb"
#define WRITETEXT    "wt"
#define UPDATEBINARY "r+b"
#define CREATEBINARY "w+b"
#define APPENDTEXT   "at"
#else
#define PATHDIVIDER  "/"
#define CPATHDIVIDER '/'
#define MASKALL      "*"
#define READBINARY   "r"
#define READTEXT     "r"
#define WRITEBINARY  "w"
#define WRITETEXT    "w"
#define UPDATEBINARY "r+"
#define CREATEBINARY "w+"
#define APPENDTEXT   "a"
#endif

typedef struct {
 char fname[256];
} PathInfo;

extern char *GetDirectory(char *, const char *);
extern bool ReadConfig(const char *);
extern bool getModulPath(char *,PathInfo *p);

#endif // SYSDEP_HPP
