"-----------------------------------------------------------------"
"-----------------------" MODUL Test "----------------------------"
"-----------------------------------------------------------------"
"
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

"
"-----------------------------------------------------------------"

"-----------------------------------------------------------------"
"---------------------------" CLASSES "---------------------------"
"-----------------------------------------------------------------"

Test (Object)
 @

"-----------------------------------------------------------------"
@ "END OF CLASSES"
"-----------------------------------------------------------------"
"---------------------------" METHODS "---------------------------"
"-----------------------------------------------------------------"

t->Test testPrint.
        '->Print test:' print.
	'Hello World!' print.
	3 print.
	nil print.
	$$ print.
	10 asCharacter print.
	@

t->Test testAdd.
	| x |
        '->Add test:' print.
	x := 1.
	x print.
	x := x + 2.
	x print@

t->Test testReal.
        '->Real test:' print.
        (2 * 3.1415) print.
	@

t->Test testBlock.
        '->Block test:' print.
	[5 print] do@

t->Test testIf.
        '->If test:' print.
	if: 3 # 4 then:['3 # 4' print] else: ['3 = 4' print]@

t->Test testAndOr.
        '->And & Or test:' print.
	if: (3 = 3 and: [3 = 4]) then:['true' print] else:['false' print].
	if: (3 = 4  or: [3 = 3]) then:['true' print] else:['false' print]@

t->Test testWhileDo.
	| c |
        '->WhileDo test:' print.
	c := 5.
	while:[c > 0] do:[c print. c:=c-1]@

t->Test testDoWhile.
	| c |
        '->DoWhile test:' print.
	c := 5.
	do:[c print. c:=c-1] while:[c > 0]@

t->Test testList.
        '->List test:' print.
	({15, 'hello', 3.14, true} at: 2) print@

t->Test testToDo.
	| l |
        '->ToDo test:' print.
	l:= {15, 'hello', 3.14, true}.
	1 to: l size do:[:i (l at: i) print]@

t->Test testListVar.
	| x l |
        '->ListVar test:' print.
	x:= 10.
	l:= {15, 'hello', 314, x + 3, true}.
	1 to: l size do:[:i (l at: i) print]@

t->Test testDictionary.
	| d |
        '->Dictionary test:' print.
	d := Dictionary new.
	d at: 'abc' put: 123.
	d at: 'def' put: 456.
	d at: 'ghi' put: 789.
	(d at: 'def') print@

t->Test testAll.
	t testPrint.
	t testAdd.
	t testReal.
	t testBlock.
	t testIf.
	t testAndOr.
	t testWhileDo.
	t testDoWhile.
	t testList.
	t testToDo.
	t testListVar.
	t testDictionary.
	@

"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
@ "END OF METHODS"
"-----------------------------------------------------------------"
"----------------------------" INIT "-----------------------------"
"-----------------------------------------------------------------"

Test new testAll@

"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------" END "-----------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"