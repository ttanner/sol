"-----------------------------------------------------------------"
"-----------------------SOL - The Language------------------------"
"-------------------------version 0.0.6---------------------------"
"-----------------------------------------------------------------"
"-----------------------" MODUL System "--------------------------"
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

#PUBLIC

"--------------------------Root Classes---------------------------"

Object ()
 @

 Boolean (Object)
  @

 Model (Object)
  dependencies	"List"
  @

"------------------------Magnitude Classes------------------------"

Magnitude (Object)
 "#DEFERRED"@

 Time (Magnitude)
  days		"Integer"
  msecs		"Integer"
  @

 Character (Magnitude)
  @

 ArithmeticValue (Magnitude)
  "#DEFERRED"@

  Number (ArithmeticValue)
   "#DEFERRED"@

   Integer (Number)
    @

    SmallInteger (Integer)
     @

    LargeInteger (Integer)
     sign	"Boolean"
     @

   Fraction (Number)
    nom		"Integer"
    denom	"Integer"
    @

   Real (Number)
    @

"----------------------Collection Classes------------------------"

Collection (Model)
 "#DEFERRED"@

 KeyedCollection (Collection)
  "#DEFERRED"@

  OrderedCollection (KeyedCollection)
   @

   IndexedCollection (OrderedCollection)
    size	"Integer"
    @

    ByteList (IndexedCollection)
     @

    String (IndexedCollection Magnitude)
     @

    List (IndexedCollection)
     @

     GenericMethod (List)
      selector	"String"
      modul	"Modul"
      @

      Methods (GenericMethod)
       genericMethod	"GenericMethod"
       @

     Context (List)
      caller	"Context"
      ip	"Integer"
      tp	"Integer"
      sp	"Integer"
      method	"CompiledMethod"
      castList	"List of (sp,class,start)"
      @

      BlockContext (Context)
       fatherContext	"Context"
       homeContext	"Context"
       @

  Dictionary (KeyedCollection)
   keys		"List of List"
   values	"List of List"
   size		"Integer"
   @

   HashTable (Dictionary)
    @

"--------------------------Stream Classes-------------------------"

Stream (Object)
 "#DEFERRED"@

 Random (Stream)
  seed 		"Integer"
  @

 PositionableStream (Stream)
  "#DEFERRED"@

  ListStream (PositionableStream)
   contents 	"List"
   position 	"Integer"
   @

  ExternalStream (PositionableStream)
   status	"Boolean"
   context
   @

"-----------------------------------------------------------------"

Association (Object)
 key		"Object"
 value          "Object"
 @

 Global (Association)
  modul		"Modul"
  @

"------------------------Runtime Classes--------------------------"

Modul (Object)
 name		"String"
 references	"Integer"
 imports	"List of Modul"
 globals	"List of Assoc"
 classes	"List of Class"
 genericMethods "List of GenericMethod"
 methods        "List of Methods"
 init		"CompiledMethod"
 options	"Integer"
 version	"Time"
 purpose	"List of Object"
 @

Class (Object)
 modul		"Modul"
 name		"String"
 superClasses	"List of Class"
 rootDist	"Integer"
 precedenceList "List of Class"
 precedenceInfo "List of 2*Integer"
 classStart	"List of Integer"
 instVars	"Integer"
 instVarNames	"List of String"
 options	"Integer"
 comment	"String/Text"
 @

Method (Object)
 paramClasses	"List of Class"
 rootDistSum	"Integer"
 comment	"String/Text"
 @

 CompiledMethod (Method ByteList)
  literals	"List of String,Integer,Real,GenericMethod,Global"
  temporaries	"Integer"
  maxStack	"Integer"
  source	"String/Text"
  @

 PrimitiveMethod (Method)
  primitiveClass "Integer"
  primitive	"Integer"
  @

Block (Object)
 fatherContext	"Context"
 homeContext	"Context"
 startIP	"Integer"
 parameters	"Integer"
 temporaries	"Integer"
 maxStack	"Integer"
 @

"------------------------Errors-------------------------------"

Error(Object)
  "#DEFERRED"@

 ArithmeticError(Error)
  value
  @

  RangeError(ArithmeticError)
   @

  DivisionError(ArithmeticError)
   @

  RaiseError(ArithmeticError)
   exponent
   @

  LogarithmicError(ArithmeticError)
   @

  ArcTanError(ArithmeticError)
   @

  TanError(ArithmeticError)
   @

 CollectionError(Error)
  collection
  @

  KeyNotFoundError(CollectionError)
   key
   @

  IndexError(CollectionError)
   index
   @

  CantInsertError(CollectionError)
   value
   @

 StreamError(Error)
  stream
  @

  StreamReadError(StreamError)
   @

  StreamSeekError(StreamError)
   position
   @

"-----------------------------------------------------------------"
@ "END OF CLASSES"
"-----------------------------------------------------------------"
"---------------------------" GLOBALS "---------------------------"
"-----------------------------------------------------------------"

#PUBLIC

Moduls
HalfPI
PI

#PRIVATE

MLog10
SinValues
MillisecondsInDay
DaysInMonth
DaysToMonth
BaseDay

"
#PUBLIC

PI			3.14159265358979323846

#PRIVATE

BaseDay			693960
			""693960 is the number of days from year 0
			 to year 1900 leap year days included""
MillisecondsInDay	86400000
DaysInMonth		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
DaysToMonth		{0, 31, 59, 90, 120, 151, 181,
			 212, 243, 273, 304, 334, 365}

"
"-----------------------------------------------------------------"
@ "END OF GLOBALS"

"-----------------------------------------------------------------"
"---------------------------" METHODS "---------------------------"
"-----------------------------------------------------------------"

"------------------------------Object-----------------------------"

#PUBLIC

#PURPOSE 'Object'

obj->Object init.
	^ obj@

obj->Object init: parameter->Object.
	^ obj@

#PURPOSE 'Object','comparing'

a->Object == b->Object.
	"Answer whether a and b are the same object"
	#PRIMITIVE Object 1 @

a->Object ## b->Object.
	"Answer whether <a> and <b> are not the same object"
	^ (a == b) not@

a->Object = b->Object.
	"Answer whether <a> has the same value as <b>"
	^ a shallowEqual: b@

a->Object # b->Object.
	"Answer whether <a> has not the same value as <b>"
	^ (a = b) not@

a->Object shallowEqual: b->Object.
	"Answer whether <a> has the same value
	 of its instance variables as <b>"
	#PRIMITIVE Object 2 @

a->Object deepEqual: b->Object.
	if: a == b then:[^ true].
	^ a deepEqual: b with: HashTable new@

#PURPOSE 'Object','copying'

obj->Object copy.
	^ obj shallowCopy@

obj->Object shallowCopy.
	#PRIMITIVE Object 3 @

obj->Object deepCopy.
	^ obj deepCopyWith: HashTable new @

#PURPOSE 'Object','misc'

obj->Object hash.
	#PRIMITIVE Object 4 @

obj->Object isPointer.
	"Answer whether <obj> is an in-pointer-object"
	^ false@

obj->Object isPrimitive.
	^ false@

#PURPOSE 'Object','access'

obj->Object instSize.
	^ obj class instVars@

obj->Object atInstVar: index->SmallInteger.
	#PRIMITIVE Object 5 @

obj->Object atInstVar: index->SmallInteger put: value->Object.
	#PRIMITIVE Object 6 @

#PURPOSE 'Object','updating'

obj->Object update.
	@

obj->Object updateAt: aspect->Object.
	@

#PURPOSE 'Object','private'
#PRIVATE

a->Object deepEqual: b->Object with: pairs->HashTable.
	| candidates |
	if: a isPrimitive then:[^ a = b].
	if: a class # b class then:[^ false].
	candidates:= pairs at: a ifAbsent:
	 [ | new | pairs at: a put: (new:= {}). new].
	if: (candidates includes: b) then:[^ true].
	candidates insert: b.
	1 to: a instSize do:[ :i
	 ifNot: ((a atInstVar: i) deepEqual: (b atInstVar: i) with: pairs)
	  then:[^ false]].
	^ true@

obj->Object deepCopyWith: copies->HashTable.
	| copy |
	^ copies at: obj ifAbsent:
	 [copy := obj shallowCopy.
	  if: copy == obj then:[^ obj].
	  copies at: obj put: copy.
	  1 to: obj instSize do:
	  [ :i copy atInstVar: i put:
	       ((copy atInstVar: i) deepCopyWith: copies)].
	  copy]@

"---------------------------Model---------------------------------"

#PUBLIC

#PURPOSE 'Model'

m->Model changed.
	if: m->dependencies ## nil then:
	 [ m->dependencies forEach:[ :element element update ] ].
	^ m@

m->Model changedAt: aspect->Object.
	if: m->dependencies ## nil then:
	 [ m->dependencies forEach:[ :element element updateAt: aspect ] ].
	^ m@

m->Model insertDependent: dependent->Object.
	if: m->dependencies == nil
	  then:[ m->dependencies := {dependent} ]
	  else:[ m->dependencies insert: dependent ifExists:[] ].
	^ m@

m->Model deleteDependent: dependent->Object.
	if: m->dependencies ## nil then:
	 [ m->dependencies delete: dependent ].
	^ m@

"-----------------------Collection--------------------------------"

#PUBLIC

#PURPOSE 'Collection','testing'

c->Collection hash.
	^ c size@

c->Collection size.
	| size |
	size := 0.
	c forEach:[size:= size + 1].
	^ size@

c->Collection isEmpty.
	^ c size = 0@

c->Collection includes: value->Object.
	c forEach:[ :element if: element == value then:[^ true]].
	^ false@

c->Collection includesEq: value->Object.
	c forEach:[ :element if: element = value then:[^ true]].
	^ false@

c->Collection occurrencesOf: value->Object.
	| count |
	count := 0.
	c forEach:[ :element if: element == value then:[count := count + 1]].
	^ count@

c->Collection occurrencesOfEq: value->Object.
	| count |
	count := 0.
	c forEach:[ :element if: element = value then:[count := count + 1]].
	^ count@

c->Collection emptyCopy.
	^ c class new@

#PURPOSE 'Collection','insert & delete'

c->Collection insert: value->Object.
	#DEFERRED@

c->Collection insert: value->Object ifExists: block->Block.
	^ if: (c includes: value)
	   then:[block do]
	   else:[c insert: value]@

c->Collection insert: value->Object ifExistsEq: block->Block.
	^ if: (c includesEq: value)
	   then:[block do]
	   else:[c insert: value]@

c->Collection delete: value->Object.
	^ c delete: value ifAbsent:[]@

c->Collection deleteEq: value->Object.
	^ c deleteEq: value ifAbsent:[]@

c->Collection delete: value->Object ifAbsent: block->Block.
	#DEFERRED@

c->Collection deleteEq: value->Object ifAbsent: block->Block.
	#DEFERRED@

dest->Collection insertAll: src->Collection.
	src forEach:[ :element dest insert: element].
	^ dest@

dest->Collection deleteAll: src->Collection.
	src forEach:[ :element dest delete: element].
	^ dest@

dest->Collection deleteAllEq: src->Collection.
	src forEach:[ :element dest deleteEq: element].
	^ dest@

c->Collection deleteAll.
	^ c deleteAll: c copy@

#PURPOSE 'Collection','iterators'

c->Collection forEach: block->Block.
	#DEFERRED@

c->Collection selectWith: block->Block.
	| answer |
	answer := c emptyCopy.
	c forEach:[ :element
	   if: (block doWith: element) then:[answer insert: element]].
	^ answer@

c->Collection rejectWith: block->Block.
	| answer |
	answer := c emptyCopy.
	c forEach:[ :element
	   ifNot: (block doWith: element) then:[answer insert: element]].
	^ answer@

c->Collection collectWith: block->Block.
	| answer |
	answer := c emptyCopy.
	c forEach:[ :element answer insert: (block doWith: element)].
	^ answer@

c->Collection detectWith: block->Block.
	^ c detectWith: block ifNone:[]@

c->Collection detectWith: condition->Block ifNone: block->Block.
	c forEach:[ :element
	   if: (condition doWith: element) then:[^ element]].
	^ block do@

#PURPOSE 'Collection','combining'

a->Collection + b->Collection.
	^ a copy insertAll: b@

a->Collection - b->Collection.
	^ a copy deleteAll: b@

a->Collection & b->Collection.
	| answer |
	answer:= a emptyCopy.
	a forEach:[ :element
	 if: (b includes: element)
	  then:[answer insert: element ifExists:[]]].
	^ answer@

a->Collection | b->Collection.
	| answer |
	answer:= a emptyCopy.
	a forEach:[ :element answer insert: element ifExists:[]].
	b forEach:[ :element answer insert: element ifExists:[]].
	^ answer@

a->Collection ~ b->Collection.
	| answer |
	answer:= a emptyCopy.
	a forEach:[ :element
	 ifNot: (b includes: element)
	  then:[answer insert: element ifExists:[]]].
	^ answer@

#PURPOSE 'Collection','converting'

c->Collection asList.
	^ {} insertAll: c@

"-----------------KeyedCollection-------------------------"

#PUBLIC

#PURPOSE 'KeyedCollection','access'

k->KeyedCollection at: key->Object.
	^ k at: key ifAbsent:
	 [^ (KeyNotFoundError new init: k key: key) raise]@

k->KeyedCollection at: key->Object ifAbsent: block->Block.
	#DEFERRED@

k->KeyedCollection at: key->Object put: value->Object.
	#DEFERRED@

k->KeyedCollection at: key->Object put: value->Object ifAbsent: block->Block.
	#DEFERRED@

k->KeyedCollection atAll: keys->List put: value->Object.
	keys forEach:[ :key k at: key put: value].
	^ k@

k->KeyedCollection atAllPut: value->Object.
	^ k atAll: k keys put: value@

k->KeyedCollection keys.
	#DEFERRED@

k->KeyedCollection values.
	#DEFERRED@

k->KeyedCollection keyFor: value->Object.
	^ k keyFor: value ifAbsent:[]@

k->KeyedCollection keyForEq: value->Object.
	^ k keyFor: value ifAbsent:[]@

k->KeyedCollection keyFor: value->Object ifAbsent: block->Block.
	#DEFERRED@

k->KeyedCollection keyForEq: value->Object ifAbsent: block->Block.
	#DEFERRED@

k->KeyedCollection keysFor: value->Object.
	#DEFERRED@

k->KeyedCollection keysForEq: value->Object.
	#DEFERRED@

#PURPOSE 'KeyedCollection','testing'

k->KeyedCollection autoKey.
	#DEFERRED@

k->KeyedCollection includes: value->Object.
	k keyFor: value ifAbsent:[^ false].
	^ true@

k->KeyedCollection includesEq: value->Object.
	k keyForEq: value ifAbsent:[^ false].
	^ true@

k->KeyedCollection includesKey: key->Object.
	k at: key ifAbsent:[^ false].
	^ true@

k->KeyedCollection includesEqKey: key->Object.
"	k at: key ifAbsent:[^ false].
	^ true@"
	#DEFERRED@

#PURPOSE 'KeyedCollection','insert & delete'

k->KeyedCollection delete: value->Object ifAbsent: block->Block.
	| key |
	key := k keyFor: value ifAbsent:[^ block do].
	^ k deleteAt: key@

k->KeyedCollection deleteEq: value->Object ifAbsent: block->Block.
	| key |
	key := k keyForEq: value ifAbsent:[^ block do].
	^ k deleteAt: key@

k->KeyedCollection deleteAt: key->Object.
	^ k deleteAt: key ifAbsent:[]@

k->KeyedCollection deleteAt: key->Object ifAbsent: block->Block.
	#DEFERRED@

#PURPOSE 'KeyedCollection','iterators'

k->KeyedCollection forEach: block->Block.
	^ k values forEach: block@

k->KeyedCollection forEachKey: block->Block.
	^ k keys forEach: block@

k->KeyedCollection forEachPair: block->Block.
	#DEFERRED@

"----------------OrderedCollection------------------------"

#PUBLIC

#PURPOSE 'OrderedCollection','access'

o->OrderedCollection first.
	^ o at: 1@

o->OrderedCollection last.
	^ o at: o size@

o->OrderedCollection keyFor: value->Object ifAbsent: block->Block.
	| index |
	if: (index:= o indexOf: value) = 0 then:[^ block do].
	^ index@

o->OrderedCollection keyForEq: value->Object ifAbsent: block->Block.
	| index |
	if: (index:= o indexOfEq: value) = 0 then:[^ block do].
	^ index@

o->OrderedCollection keysFor: value->Object.
	| keys |
	keys:= {}.
	1 to: o size do:[ :index
	  if: (o at: index) == value then:[keys insert: index]].
	^ keys@

o->OrderedCollection keysForEq: value->Object.
	| keys |
	keys:= {}.
	1 to: o size do:[ :index
	  if: (o at: index) = value then:[keys insert: index]].
	^ keys@

o->OrderedCollection keys.
	| keys |
	keys:= {} setLimit: o size.
	1 to: o size do:[ :index keys insert: index].
	^ keys@

o->OrderedCollection values.
	| values |
	values:= {} setLimit: o size.
	1 to: o size do:[ :index values insert: (o at: index)].
	^ values@

o->OrderedCollection copySpace: count->SmallInteger from: start->SmallInteger.
	| answer |
	if: count < 0 then:[^ o copySpace: count negated from: start + count].
	o checkIndex: start.
	o checkIndex: start + count.
	answer := o emptyCopy.
	start to: start + count do:[ :index answer insert: (o at: index)].
	^ answer@

#PURPOSE 'OrderedCollection','testing'

o->OrderedCollection autoKey.
	^ true@

o->OrderedCollection checkIndex: index->SmallInteger.
	ifNot: (o validIndex: index)
	 then:[^ (KeyNotFoundError new init: o key: index) raise]@

o->OrderedCollection validIndex: index->SmallInteger.
	if: index > o size then:[^ false].
	if: index < 1 then:[^ false].
	^ true@

o->OrderedCollection includes: value->Object.
	^ (o indexOf: value) # 0@

o->OrderedCollection includesEq: value->Object.
	^ (o indexOfEq: value) # 0@

o->OrderedCollection includesKey: index->SmallInteger.
	^ o validIndex: index@

o->OrderedCollection indexOf: value->Object.
	^ o indexOf: value startingAt: 1@

o->OrderedCollection indexOfEq: value->Object.
	^ o indexOfEq: value startingAt: 1@

o->OrderedCollection indexOf: value->Object startingAt: start->SmallInteger.
	if: start < 1 then:[^ 0].
	start to: o size do:[ :index
	  if: (o at: index) == value then:[^ index]].
	^ 0@

o->OrderedCollection indexOfEq: value->Object startingAt: start->SmallInteger.
	if: start < 1 then:[^ 0].
	start to: o size do:[ :index
	  if: (o at: index) = value then:[^ index]].
	^ 0@

o->OrderedCollection indexOfCollection: other->OrderedCollection.
	^ o indexOfCollection: other startingAt: 1@

o->OrderedCollection indexOfEqCollection: other->OrderedCollection.
	^ o indexOfEqCollection: other startingAt: 1@

o->OrderedCollection indexOfCollection: other->OrderedCollection startingAt: start->SmallInteger.
	| first index subIndex limit subSize |
	first := other first.
	subSize := other size.
	limit := o size - subSize - start + 2.
	index := start - 1.
	while:[ (index:= o indexOf: first startingAt: index + 1) <= limit ]
	 do:[ subIndex:= 2.
	      while:[ subIndex < subSize & [(o at: index + subIndex - 1)
					    = (other at: subIndex)]]
	       do:[ subIndex := subIndex + 1 ].
	      if: subIndex > index then:[^ index].
	    ].
	^ 0@

o->OrderedCollection indexOfEqCollection: other->OrderedCollection startingAt: start->SmallInteger.
	| first index subIndex limit subSize |
	first := other first.
	subSize := other size.
	limit := o size - subSize - start + 2.
	index := start - 1.
	while:[ (index:= o indexOf: first startingAt: index + 1) <= limit ]
	 do:[ subIndex:= 2.
	      while:[ subIndex < subSize & [(o at: index + subIndex - 1)
					    == (other at: subIndex)]]
	       do:[ subIndex := subIndex + 1 ].
	      if: subIndex > index then:[^ index].
	    ].
	^ 0@

#PURPOSE 'OrderedCollection','insert & delete'

o->OrderedCollection deleteFirst.
	^ o deleteAt: 1@

o->OrderedCollection deleteLast.
	^ o deleteAt: o size@

o->OrderedCollection deleteAt: index->SmallInteger ifAbsent: block->Block.
	ifNot: (o validIndex: index) then:[^ block do].
	^ o deleteSpace: 1 at: index@

o->OrderedCollection deleteSpace: count->SmallInteger at: start->SmallInteger.
	#DEFERRED@

#PURPOSE 'OrderedCollection','iterators'

o->OrderedCollection forEach: block->Block.
	| index |
	index:= 1.
	while:[index <= o size] do:
	[ block doWith: (o at: index).
	  index:= index + 1].
	^ o@

o->OrderedCollection forEachReversed: block->Block.
	| count |
	count:= 1.
	while:[count <= o size] do:
	[ block doWith: (o at: o size - count + 1).
	  count:= count + 1].
	^ o@

o->OrderedCollection forEachKey: block->Block.
	| index |
	index:= 1.
	while:[index <= o size] do:
	[ block doWith: index.
	  index:= index + 1].
	^ o@

o->OrderedCollection forEachPair: block->Block.
	| index |
	index:= 1.
	while:[index <= o size] do:
	[ block doWith: index and: (o at: index).
	  index:= index + 1].
	^ o@

o->OrderedCollection findFirst: condition->Block.
	^ o findFirst: condition ifNone:[0]@

o->OrderedCollection findFirst: condition->Block ifNone: block->Block.
	1 to: o size do:[ :index
	   if: (condition doWith:(o at: index)) then:[^ index]].
	^ block do@

o->OrderedCollection findLast: condition->Block.
	^ o findLast: condition ifNone:[0]@

o->OrderedCollection findLast: condition->Block ifNone: block->Block.
	o size downTo: 1 do:[ :index
	   if: (condition doWith:(o at: index)) then:[^ index]].
	^ block do@

"----------------IndexedCollection------------------------"

#PUBLIC

#PURPOSE 'IndexedCollection'

i->IndexedCollection init.
	^ i init: 0@

i->IndexedCollection init: size->SmallInteger.
	^ i setLimit: size@

#PURPOSE 'IndexedCollection','access'

i->IndexedCollection at: index->SmallInteger.
	i checkIndex: index.
	^ i basicAt: index@

i->IndexedCollection at: index->SmallInteger ifAbsent: block->Block.
	ifNot: (i validIndex: index) then:[^ block do].
	^ i basicAt: index@

i->IndexedCollection at: index->SmallInteger put: value->Object.
	i checkIndex: index.
	i basicAt: index put: value@

i->IndexedCollection at: index->SmallInteger put: value->Object ifAbsent: block->Block.
	ifNot: (i validIndex: index) then:[^ block do].
	^ i basicAt: index put: value@

i->IndexedCollection atAllPut: value->Object.
	^ i basicFillSpace: i size at: 1 with: value@

i->IndexedCollection fillSpace: count->SmallInteger at: start->SmallInteger with: value->Object.
	i checkIndex: start.
	i checkIndex: start + count.
	^ i basicFillSpace: count at: start with: value@

dest->IndexedCollection replaceAllFrom: start->SmallInteger with: src->IndexedCollection.
	| stop |
	stop := start + src size.
	if: stop < start then:[^ nil].
	if: stop > dest size then:[ dest resizeAbout: src size ].
	start to: stop do:[ :index
	   dest at: index put: (src at: index - start + 1)].
	^ dest@

i->IndexedCollection reversed.
	| answer |
	answer:= i emptyCopy.
	1 to: i size do:[ :index answer insert:(i at: i size - index + 1)].
	^ answer@

a->IndexedCollection deepEqual: b->IndexedCollection with: pairs->HashTable.
	| candidates |
	if: a size # b size then:[^ false].
	candidates:= pairs at: a ifAbsent:
	 [ | new | pairs at: a put: (new:= {}). new].
	if: (candidates includes: b) then:[^ true].
	candidates insert: b.
	1 to: a instSize do:[ :i
	 ifNot: ((a atInstVar: i) deepEqual: (b atInstVar: i) with: pairs)
	  then:[^ false]].
	1 to: a size do:[ :i
	 ifNot: ((a at: i) deepEqual: (b at: i) with: pairs) then:[^ false]].
	^ true@

i->IndexedCollection deepCopyWith: copies->HashTable.
	| copy |
	^ copies at: i ifAbsent:
	 [copy := i shallowCopy.
	  if: copy == i then:[^ i].
	  copies at: i put: copy.
	  1 to: i instSize do:[ :j
	   copy atInstVar: j put: ((copy atInstVar: j) deepCopyWith: copies)].
	  1 to: i size do:[ :j
	   copy at: j put: ((copy at: j) deepCopyWith: copies)].
	  ^ copy]@

#PURPOSE 'IndexedCollection','access','private'

i->IndexedCollection basicAt: index->SmallInteger.
	#DEFERRED@

i->IndexedCollection basicAt: index->SmallInteger put: value->Object.
	#DEFERRED@

i->IndexedCollection basicFillSpace: count->SmallInteger at: start->SmallInteger with: value->Object.
	#DEFERRED@

i->IndexedCollection basicMoveSpace: count->SmallInteger from: srcPos->SmallInteger to: destPos->SmallInteger.
	#DEFERRED@

#PURPOSE 'IndexedCollection','insert & delete'

i->IndexedCollection insert: value->Object.
	i resizeAbout: 1.
	i at: i size put: value.
	^ i@

i->IndexedCollection insert: value->Object at: index->SmallInteger.
	i insertSpace: 1 at: index.
	i at: index put: value.
	^ i@

i->IndexedCollection insertFirst: value->Object.
	^ i insert: value at: 1@

i->IndexedCollection insertLast: value->Object.
	^ i insert: value@

dest->IndexedCollection insertAllFirst: src->OrderedCollection.
	^ dest insertAll: src at: 1@

dest->IndexedCollection insertAllLast: src->OrderedCollection.
	^ dest insertAll: src at: dest size@

dest->IndexedCollection insertAll: src->OrderedCollection at: start->SmallInteger.
	dest insertSpace: src size at: start.
	^ dest replaceAllFrom: start with: src@

i->IndexedCollection insertSpace: count->SmallInteger at: start->SmallInteger.
	if: count < 0 then:[^ i insertSpace: count negated at: start + count].
	i checkIndex: start.
	i resizeAbout: count.
	i basicMoveSpace: i size - start + 1 from: start to: start + count.
	^ i@

i->IndexedCollection deleteAll.
	^ i resizeTo: 0@

i->IndexedCollection deleteSpace: count->SmallInteger at: start->SmallInteger.
	if: count < 0 then:[^ i deleteSpace: count negated at: start + count].
	i checkIndex: start.
	i checkIndex: start + count - 1.
	i basicMoveSpace: i size - start - count + 1 from: start + count to: start.
	i resizeAbout: count negated.
	^ i@

i->IndexedCollection pack.
	| index |
	index:= 1.
	while:[index <= i size] do:
	[ if: (i at: index) == nil
	   then:[i deleteAt: index]
	   else:[index:= index + 1]].
	^ i@

#PURPOSE 'IndexedCollection','order'

i->IndexedCollection sort.
	^ i sortBy: [ :a :b a < b]@

i->IndexedCollection sortBy: sortBlock->Block.
	^ i basicSortFrom: 1 to: i size by: sortBlock@

i->IndexedCollection sortFrom: start->SmallInteger to: stop->SmallInteger by: sortBlock->Block.
	i checkIndex: start.
	i checkIndex: stop.
	^ i basicSortFrom: start to: stop by: sortBlock@

#PURPOSE 'IndexedCollection','order','private'

i->IndexedCollection basicSortFrom: start->SmallInteger to: stop->SmallInteger by: sortBlock->Block.
	| a b x y z |
	a:= start. b:= stop.
	x:= i at: (a + b) // 2.
	do:[
	  while:[ sortBlock doWith: (i at: a) and: x ] do:[ a:=a + 1].
	  while:[ sortBlock doWith: x and: (i at: b) ] do:[ b:=b - 1].
	  if: a <= b then:
	   [ y:=i at: a.
	     i at: a put: (i at: b).
	     i at: b put: y.
	     a:= a + 1. b:= b - 1].
	] whileNot:[ a > b ].
	if: start < b then:[i basicSortFrom: start to: b by: sortBlock].
	if: a < stop  then:[i basicSortFrom: a to: stop  by: sortBlock].
	^ i@

#PURPOSE 'IndexedCollection','size'

i->IndexedCollection size.
	^ i->size@

i->IndexedCollection resizeTo: size->SmallInteger.
	if: i->size = size then:[^ i].
	if: i limit < size then:[
	   if: i limit + i growSize < size
	    then:[i setLimit: size]
	    else:[i grow]].
	if: i->size > size then:
	 [ i basicFillSpace: i->size - size at: i->size with: nil].
	i->size := size.
	i changed.
	^ i@

i->IndexedCollection resizeAbout: count->SmallInteger.
	^ i resizeTo: i size + count@

i->IndexedCollection growSize.
	if: i limit < 4   then:[^ 1].
	if: i limit < 32  then:[^ 4].
	if: i limit < 256 then:[^ 16].
	^ 64@

i->IndexedCollection grow.
	^ i setLimit: i limit + i growSize@

i->IndexedCollection adjustLimit.
	^ i setLimit: i size@

i->IndexedCollection limit.
	#DEFERRED@

i->IndexedCollection setLimit: limit->SmallInteger.
	#DEFERRED@

"--------------------ByteList------------------------------"

#PUBLIC

#PURPOSE 'ByteList'

l->ByteList hash.
	| value |
	value := 0.
	1 to: (16 min: l size) do:[ :index value:=value + (l at: index)].
	^ value@

l->ByteList basicAt: index->SmallInteger.
	#PRIMITIVE ByteList 1 @

l->ByteList basicAt: index->SmallInteger put: value->SmallInteger.
	#PRIMITIVE ByteList 2 @

l->ByteList basicFillSpace: count->SmallInteger at: start->SmallInteger with: value->SmallInteger.
	#PRIMITIVE ByteList 3 @

l->ByteList basicMoveSpace: count->SmallInteger from: srcPos->SmallInteger to: destPos->SmallInteger.
	#PRIMITIVE ByteList 4 @

l->ByteList indexOf: value->SmallInteger startingAt: start->SmallInteger.
	#PRIMITIVE ByteList 5 @

l->ByteList limit.
	#PRIMITIVE ByteList 6 @

l->ByteList setLimit: limit->SmallInteger.
	#PRIMITIVE ByteList 7 @

l->ByteList asString.
	| str |
	str:= String new: l size.
	1 to: l size do:[ :index str insert: (l at: index) asCharacter].
	^ str@

"------------------------String------------------------------"

#PUBLIC

#PURPOSE 'String'

s->String hash.
	| value |
	value := 0.
	1 to: (8 min: s size) do:
	[ :index value:=value + (s at: index) asASCII].
	^ value@

s->String length.
	^ s size@

s->String asLowercase.
	| answer |
	answer:= s emptyCopy.
	1 to: s size do:
	[ :index answer insert: (s at: index) asLowercase].
	^ answer@

s->String asUppercase.
	| answer |
	answer:= s emptyCopy.
	1 to: s size do:
	[ :index answer insert: (s at: index) asUppercase].
	^ answer@

a->String matches: b->String.
	if: a length # b length then:[^ false].
	1 to: a length do:
	[ :index | ca cb |
	  ca:= (a at: index) asUppercase.
	  cb:= (b at: index) asUppercase.
	  if: ca # cb then:[^ false]].
	^ true@

a->String < b->String.
	1 to: (a length min: b length) do:
	[ :index | ca cb |
	  ca:= a at: index.
	  cb:= b at: index.
	  if: ca # cb then:[^ ca < cb]].
	^ a length < b length@

a->String > b->String.
	1 to: (a length min: b length) do:
	[ :index | ca cb |
	  ca:= a at: index.
	  cb:= b at: index.
	  if: ca # cb then:[^ ca > cb]].
	^ a length > b length@

s->String basicAt: index->SmallInteger.
	#PRIMITIVE String 1 @

s->String basicAt: index->SmallInteger put: value->Character.
	#PRIMITIVE String 2 @

s->String basicFillSpace: count->SmallInteger at: start->SmallInteger with: value->Character.
	#PRIMITIVE String 3 @

s->String basicMoveSpace: count->SmallInteger from: srcPos->SmallInteger to: destPos->SmallInteger.
	#PRIMITIVE String 4 @

s->String indexOf: value->Character startingAt: start->SmallInteger.
	#PRIMITIVE String 5 @

s->String limit.
	#PRIMITIVE String 6 @

s->String setLimit: limit->SmallInteger.
	#PRIMITIVE String 7 @

"------------------------List------------------------------"

#PUBLIC

#PURPOSE 'List'

l->List basicAt: index->SmallInteger.
	#PRIMITIVE List 1 @

l->List basicAt: index->SmallInteger put: value->Object.
	#PRIMITIVE List 2 @

l->List basicFillSpace: count->SmallInteger at: start->SmallInteger with: value->Object.
	#PRIMITIVE List 3 @

l->List basicMoveSpace: count->SmallInteger from: srcPos->SmallInteger to: destPos->SmallInteger.
	#PRIMITIVE List 4 @

l->List indexOf: value->Object startingAt: start->SmallInteger.
	#PRIMITIVE List 5 @

l->List limit.
	#PRIMITIVE List 6 @

l->List setLimit: limit->SmallInteger.
	#PRIMITIVE List 7 @

#PURPOSE 'List','converting'

l->List asList.
	^ l@

"-----------------------Dictionary-------------------------"

#PUBLIC

#PURPOSE 'Dictionary'

d->Dictionary init.
	^ d init: 17@

d->Dictionary init: size->SmallInteger.
	d->keys:= {} resizeTo: size.
	d->values:= {} resizeTo: size.
	d->size:=0.
	^ d@

#PURPOSE 'Dictionary','access'

d->Dictionary at: key->Object ifAbsent: block->Block.
	| i j k |
	i:= key hash \\ d->keys size + 1.
	if: (k:= d->keys at: i) == nil then:[^ block do].
	j:= d findKey: key in: k.
	if: j = 0 then:[^ block do].
	^ (d->values at: i) at: j@

d->Dictionary at: key->Object put: value->Object.
	^ d insertKey: key value: value@

d->Dictionary at: key->Object put: value->Object ifAbsent: block->Block.
	| i j k |
	i:= key hash \\ d->keys size + 1.
	if: (k:= d->keys at: i) == nil then:[^ block do].
	j:= d findKey: key in: k.
	if: j = 0 then:[^ block do].
	(d->values at: i) at: j put: value.
	^ d@

d->Dictionary keys.
	| keys |
	keys:= {}.
	d->keys forEach:[ :l if: l ## nil then:[keys insertAll: l]].
	^ keys@

d->Dictionary values.
	| values |
	values:= {}.
	d->values forEach:[ :l if: l ## nil then:[values insertAll: l]].
	^ values@

d->Dictionary keyFor: value->Object ifAbsent: block->Block.
	1 to: d->values size do:
	 [ :i | l |
	  if: (l:=d->values at: i) ## nil then:
	   [ | j |
	    if: (j:=l indexOf: value) # 0 then:
	     [^ (d->keys at: i) at: j]
	   ]
	 ].
	 ^ block do@

d->Dictionary keyForEq: value->Object ifAbsent: block->Block.
	1 to: d->values size do:
	 [ :i | l |
	  if: (l:=d->values at: i) ## nil then:
	   [ | j |
	    if: (j:=l indexOfEq: value) # 0 then:
	     [^ (d->keys at: i) at: j]
	   ]
	 ].
	 ^ block do@

d->Dictionary keysFor: value->Object.
	| keys |
	keys:= {}.
	1 to: d->values size do:
	 [ :i | l |
	  if: (l:=d->values at: i) ## nil then:
	   [ | j |
	    j:= 0.
	    while:[(j:=l indexOf: value startingAt: j + 1) # 0]
	     do:[keys insert: ((d->keys at: i) at: j)]
	   ]
	 ].
	 ^ keys@

d->Dictionary keysForEq: value->Object.
	| keys |
	keys:= {}.
	1 to: d->values size do:
	 [ :i | l |
	  if: (l:=d->values at: i) ## nil then:
	   [ | j |
	    j:= 0.
	    while:[(j:=l indexOfEq: value startingAt: j + 1) # 0]
	     do:[keys insert: ((d->keys at: i) at: j)]
	   ]
	 ].
	 ^ keys@

#PURPOSE 'Dictionary','testing'

d->Dictionary autoKey.
	^ false@

#PURPOSE 'Dictionary','insert & delete'

d->Dictionary insert: value->Object.
	^ (CantInsertError new init: d value: value) raise@

d->Dictionary insertKey: key->Object value: value->Object.
	| i j k v |
	i:= key hash \\ d->keys size + 1.
	if: (k:= d->keys at: i) == nil
	 then:[ d->keys   at: i put: (k:= {}).
		d->values at: i put: (v:= {})]
	 else:[ v:= d->values at: i].
	j:= d findKey: key in: k.
	if: j = 0
	 then:[ k insert: key. v insert: value.
		d->size:= d->size + 1. d checkSize]
	 else:[ v at: j put: value].
	^ d@

d->Dictionary deleteAt: key->Object ifAbsent: block->Block.
	| i j |
	i:= key hash \\ d->keys size + 1.
	if: (d->keys at: i) == nil then:[^ block do].
	j:= d findKey: key in: (d->keys at: i).
	if: j = 0 then:[^ block do].
	(d->keys at: i) deleteAt: j.
	(d->values at: i) deleteAt: j.
	d->size:= d->size - 1.
	^ d@

d->Dictionary deleteAll.
	d->keys atAllPut: nil.
	d->values atAllPut: nil.
	d->size := 0.
	^ d@

#PURPOSE 'Dictionary','iterators'

d->Dictionary forEach: block->Block.
	d->values forEach:[ :l if: l ## nil then:[l forEach: block]].
	^ d@

d->Dictionary forEachKey: block->Block.
	d->keys forEach:[ :l if: l ## nil then:[l forEach: block]].
	^ d@

d->Dictionary forEachPair: block->Block.
	1 to: d->keys size do:
	[ :i | k v |
	  if: (k:=d->keys at: i) ## nil then:
	  [ v:=d->values at: i.
	    1 to: k size do:
	     [ :j block doWith: (k at: j) and: (v at: j)]]
	].
	^ d@

#PURPOSE 'Dictionary','private'

d->Dictionary checkSize.
	if: d->size > d->keys size * 3 then:[d grow]@

d->Dictionary growTo: size->SmallInteger.
	| oldKeys oldValues k v |
	oldKeys:=d->keys.
	oldValues:=d->values.
	d->keys:={} resizeTo: size.
	d->values:={} resizeTo: size.
	d->size:=0.
	1 to: oldKeys size do:[ :i
	 if: (k:=oldKeys at: i) ## nil then:
	 [ v:=oldValues at: i.
	   1 to: k size do:
	    [ :j d insertKey: (k at: j) value: (v at: j)]
	 ]
	].
	^ d@

d->Dictionary findKey: key->Object in: keys->List.
	^ keys indexOfEq: key@

"-----------------------HashTable--------------------------"

#PUBLIC

#PURPOSE 'HashTable','private'

h->HashTable findKey: key->Object in: keys->List.
	^ keys indexOf: key@

"--------------------------Stream---------------------------------"

#PUBLIC

#PURPOSE 'Stream'

s->Stream put: obj->Object.
	#DEFERRED@

s->Stream get.
	#DEFERRED@

s->Stream getNext: count->SmallInteger.
	| answer c |
	answer:= {} setLimit: count.
	c:= count.
	whileNot:[s atEnd | (c = 0)] do:[answer insert:(s get). c:=c - 1].
	if: c # 0 then:[^ (StreamReadError new init: s) raise].
	^ answer@

s->Stream getAllUpTo: obj->Object.
	| answer tmp |
	answer:= {}.
	whileNot:[s atEnd | [(tmp:= s get) = obj]] do:[answer insert: tmp].
	if: tmp # obj then:[^ (StreamReadError new init: s) raise].
	^ answer@

s->Stream atEnd.
	#DEFERRED@

s->Stream truncate.
	#DEFERRED@

s->Stream close.
	#DEFERRED@

s->Stream seekEnd.
	#DEFERRED@

s->Stream skipAbout: count->SmallInteger.
	s getNext: count.
	^ s@

s->Stream skipTo: obj->Object.
	s getAllUpTo: obj.
	^ s@

"---------------------PositionableStream---------------------------"

#PUBLIC

#PURPOSE 'PositionableStream'

s->PositionableStream peek.
	^ s peekNext: 1@

s->PositionableStream peekNext: count->SmallInteger.
	| old answer |
	old:= s position.
	answer:= s getNext: count.
	s seekTo: old.
	^ answer@
"
s->PositionableStream skipBackTo: obj->Object.
s->PositionableStream positionOf: obj->Object.
s->PositionableStream positionOfList: List->l.
"
s->PositionableStream reset.
	^ s seekTo: 1@

s->PositionableStream position.
	#DEFERRED@

s->PositionableStream seekTo: position->SmallInteger.
	#DEFERRED@

"------------------------ListStream------------------------------"

#PUBLIC

#PURPOSE 'ListStream'

s->ListStream put: obj->Object.
	if: s atEnd
	 then:[s->contents insert: obj]
	 else:[s->contents at: s->position put: obj].
	s->position:= s->position + 1@

s->ListStream get.
	if: s atEnd then:[^ (StreamReadError new init: s) raise].
	s->position:= s->position + 1.
	^ s->contents at: s->position - 1@

s->ListStream getNext: count->SmallInteger.
	| answer c |
	answer:= s peekNext: count.
	s->position := s->position + count.
	^ answer@

s->ListStream atEnd.
	^ s->position = s->contents size@

s->ListStream truncate.
	s->contents resizeTo: s->position.
	^ s@

s->ListStream close.
	^ s@

s->ListStream seekEnd.
	s->position := s->contents size.
	^ s@

s->ListStream skipAbout: count->SmallInteger.
	ifNot: (s checkPosition: s->position + count) then:[].
	s->position := s->position + count.
	^ s@

s->ListStream skipTo: obj->Object.
	s->position := s->contents indexOf: obj startingAt: s->position.
	^ s@

s->ListStream peek.
	^ s->contents at: s->position@

s->ListStream peekNext: count->SmallInteger.
	| pos |
	pos:= s->position.
	^ s->contents copySpace: count from: pos@

"
s->ListStream skipBackTo: obj->Object.
s->ListStream positionOf: obj->Object.
s->ListStream positionOfList: List->l.
"

s->ListStream position.
	^ s->position@

s->ListStream seekTo: position->SmallInteger.
	ifNot: (s checkPosition: position)
	 then:[^ (StreamSeekError new init:s position: position) raise].
	^ s->position:= position@

s->ListStream checkPosition: position->SmallInteger.
	^ (position between: 1 and: s->contents size)@


"----------------------ExternalStream-----------------------------"

#PUBLIC

#PURPOSE 'ExternalStream'

"-----------------------------------------------------------------"


"-------------------------Magnitude-------------------------------"

#PUBLIC

#PURPOSE 'Magnitude'

a->Magnitude <  b->Magnitude.
	#DEFERRED@

a->Magnitude <= b->Magnitude.
	^ (a > b) not@

a->Magnitude >  b->Magnitude.
	#DEFERRED@

a->Magnitude >= b->Magnitude.
	^ (a < b) not@

a->Magnitude min: b->Magnitude.
	^ if: a < b then:[a] else:[b]@

a->Magnitude max: b->Magnitude.
	^ if: a > b then:[a] else:[b]@

x->Magnitude between: low->Magnitude and: high->Magnitude.
	^ (x >= low) & [x <= high]@

"--------------------------Character------------------------------"

#PUBLIC

#PURPOSE 'Character'

c->Integer asCharacter.
	#PRIMITIVE Character 1 @

c->Character asASCII.
	#PRIMITIVE Character 2 @

c->Character isPrimitive.
	^ true@

c->Character shallowCopy.
	^ c@

a->Character = b->Character.
	^ a asASCII = b asASCII@

a->Character < b->Character.
	^ a asASCII < b asASCII@

a->Character > b->Character.
	^ a asASCII > b asASCII@

c->Character asLowercase.
	ifNot: c isUppercase then:[^ c].
	^ ((c asASCII - $A asASCII) + $a asASCII) asCharacter@

c->Character asUppercase.
	ifNot: c isLowercase then:[^ c].
	^ ((c asASCII - $a asASCII) + $A asASCII) asCharacter@

c->Character isAlphanumeric.
	^ c isDigit | c isLetter@

c->Character isDigit.
	^ c between: $0 and: $9@

c->Character isLetter.
	^ c isLowercase | c isUppercase@

c->Character isLowercase.
	^ c between: $a and: $z@

c->Character isUppercase.
	^ c between: $A and: $Z@

c->Character isSeparator.
	^ c = $  | c = 9 asCharacter@

c->Character isVowel.
	^ { $a, $e, $i, $o, $u } includes: c asLowercase@

"------------------------------Time-------------------------------"

#PUBLIC

#PURPOSE 'Time','instance initialisation'

time->Time init.
	time->msecs := 0.
	time->days  := 0.
	^ time@

time->Time now.
	#PRIMITIVE Time 1 @

time->Time setMSecs: msecs->Integer.
	time->msecs := msecs \\ MillisecondsInDay.
	time->days  := msecs // MillisecondsInDay.
	^ time@

time->Time setDays: days->Integer.
	time->msecs := 0.
	time->days  := days.
	^ time@

time->Time setHour: hour->Integer min: min->Integer
     sec: sec->Integer msec: msec->Integer.
	ifNot: (validHour: hour min: min sec: sec msec: msec) then:[^ nil].
	time->msecs := absoluteHour: hour min: min sec: sec msec: msec.
	^ time@

time->Time setYear: year->Integer
     month: month->Integer day: day->Integer.
	ifNot: (validYear: year month: month day: day) then:[^ nil].
	time->days := absoluteYear: year month: month day: day.
	^ time@

validHour: hour->Integer min: min->Integer
     sec: sec->Integer msec: msec->Integer.
	ifNot: (hour between: 0 and: 23) then:[^ false].
	ifNot: (min  between: 0 and: 59) then:[^ false].
	ifNot: (sec  between: 0 and: 59) then:[^ false].
	ifNot: (msec between: 0 and: 999) then:[^ false].
	^ true@

validYear: year->Integer month: month->Integer day: day->Integer.
	if: year < 1900 then:[^ false].
	ifNot: (month between: 1 and: 12) then:[^ false].
	if: day < 1 then:[^ false].
	^ day > (year daysInMonth: month)@

#PURPOSE 'Time'

time->Time hour.
	^ time getTime at: 1@

time->Time min.
	^ time getTime at: 2@

time->Time sec.
	^ time getTime at: 3@

time->Time msec.
	^ time getTime at: 4@

time->Time year.
	^ time getDate at: 1@

time->Time month.
	^ time getDate at: 2@

time->Time day.
	^ time getDate at: 3@

time->Time weekDay.
	^ time getDate at: 4@

time->Time getTime.
	| values tmp |
	values := {} setLimit: 4.
	values insert: time->msecs // 3600000.
	tmp := time->msecs \\ 3600000.
	values insert: tmp // 60000.
	tmp := tmp \\ 60000.
	values insert: tmp // 1000.
	values insert: tmp \\ 1000.
	^ values@

time->Time getDate.
	| values days tmp year month day |
	if: time->days < BaseDay then:[^ {0, 0, 0, time->days}].
	days := time->days - BaseDay.
	year := 1900 + days * 400 // 146097.
	tmp := days - (year - 1900) * 365 - (leapYearsTo: year).
	if: year isLeapYear & [tmp = 60]
	 then:[ month := 2. day := 29 ]
	 else:[ month := 1.
		while:[(DaysToMonth at: month+1) < tmp]
		 do:[month := month + 1].
		day := tmp - (DaysToMonth at: month)].
	^ {year, month, day, (days - 1) // 7}@

#PURPOSE 'Time','comparing'

a->Time < b->Time.
	^ a before: b@

a->Time > b->Time.
	^ a after: b@

a->Time before: b->Time.
	if: a->days # b->days
	 then:[^ a->days < b->days]
	 else:[^ a->msecs < b->msecs]@

a->Time after: b->Time.
	if: a->days # b->days
	 then:[^ a->days > b->days]
	 else:[^ a->msecs > b->msecs]@

#PURPOSE 'Time','calculating'

time->Time addMSecs: msecs->Integer.
	if: (time->msecs := time->msecs + msecs) > MillisecondsInDay
	 then: [time->msecs := time->msecs - MillisecondsInDay. time addDays: 1].
	^ time@

time->Time subMSecs: msecs->Integer.
	if: (time->msecs := time->msecs - msecs) < 0
	 then: [time->msecs := time->msecs + MillisecondsInDay. time subDays: 1].
	^ time@

a->Time addTime: b->Time.
	if: (a->msecs := a->msecs + b->msecs) > MillisecondsInDay
	 then: [a->msecs := a->msecs - MillisecondsInDay. a addDays: 1].
	^ a@

a->Time subTime: b->Time.
	if: (a->msecs := a->msecs - b->msecs) < 0
	 then: [a->msecs := a->msecs + MillisecondsInDay. a subDays: 1].
	^ a@

time->Time addDays: days->Integer.
	time->days := time->days + days.
	^ time@

time->Time subDays: days->Integer.
	time->days := time->days - days.
	^ time@

daysBetween: a->Time and: b->Time.
	^ (b->days - a->days) abs@

msecsBetween: a->Time and: b->Time.
	^ (b->days - a->days) abs * MillisecondsInDay +
	  (b->msecs - a->msecs) abs@

year->Integer isLeapYear.
	^ (year & 3 = 0) & [(year \\ 100 # 0) | [year \\ 400 = 0]]@

year->Integer daysInYear.
	^ if: year isLeapYear then:[^ 366] else:[^ 365]@

year->Integer daysInMonth: month->Integer.
	if: (month = 2) & [year isLeapYear] then:[^ 29].
	^ DaysInMonth at: month@

#PURPOSE 'Time','stopping'

timeToRun: block->Block.
	| start |
	start := Time new now.
	block do.
	^ Time new now subTime: start@

#PURPOSE 'Time','private'
#PRIVATE

leapYearsTo: year->Integer.
	^ (year-1901) // 4 - (year-1901) // 100 + (year-1601) // 400@

absoluteHour: hour->Integer min: min->Integer
 sec: sec->Integer msec: msec->Integer.
	^ hour * 3600000 + min * 60000 + sec * 1000 + msec@

absoluteYear: year->Integer month: month->Integer day: day->Integer.
	| days |
	days := (DaysToMonth at: month) + day.
	if: year isLeapYear & [month > 2] then:[days := days+1].
	^ BaseDay + days + (year - 1900) * 365 + (leapYearsTo: year)@

"-----------------------ArithmecticValue--------------------------"

#PUBLIC

#PURPOSE 'ArithmeticValue'

a->ArithmeticValue + b->ArithmeticValue.
	#DEFERRED@

a->ArithmeticValue - b->ArithmeticValue.
	#DEFERRED@

a->ArithmeticValue * b->ArithmeticValue.
	#DEFERRED@

a->ArithmeticValue / b->ArithmeticValue.
	#DEFERRED@

"----------------------------Number-------------------------------"

#PUBLIC

#PURPOSE 'Number','comparing'

a->Number = b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) = b]
	 else:[^ a = (b withGenerality: ga)]@

a->Number < b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) < b]
	 else:[^ a < (b withGenerality: ga)]@

a->Number > b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) > b]
	 else:[^ a > (b withGenerality: ga)]@

#PURPOSE 'Number','calculating'

a->Number + b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) + b]
	 else:[^ a + (b withGenerality: ga)]@

a->Number - b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) - b]
	 else:[^ a - (b withGenerality: ga)]@

a->Number * b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) * b]
	 else:[^ a * (b withGenerality: ga)]@

a->Number / b->Number.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) / b]
	 else:[^ a / (b withGenerality: ga)]@

a->Number // b->Number.
	^ (a / b) int@

a->Number \\ b->Number.
	^ (a / b) frac * b@

n->Number negated.
	^ 0 - n@

n->Number reziprocal.
	^ 1 / n@

n->Number abs.
	if: n < 0 then:[^ n negated].
	^ n@

n->Number sign.
	if: n > 0 then:[^ 1].
	if: n < 0 then:[^ -1].
	^ 0@

n->Number sqr.
	^ n * n@

n->Number sqrt.
	^ n asReal sqrt@

n->Number exp.
	^ n asReal exp@

a->Number raisedTo: b->Integer.
	| result |
	if: (a = 0) & [b <= 0] then:
	 [^ (RaiseError new init: a exponent: b) raise].
	if: b = 0 then:[^ 1].
	if: b = 1 then:[^ a].
	result := 1.
	1 to: b abs do:[result := result * a].
	if: b < 0 then:[^ result reziprocal].
	^ result@

a->Number raisedTo: b->Number.
	if: (a = 0) & [b <= 0] then:
	 [^ (RaiseError new init: a exponent: b) raise].
	if: b = 0 then:[^ 1].
	if: b = 1 then:[^ a].
	^ (b * a ln) exp@

a->Number root: b->Number.
	^ a raisedTo: b reziprocal@

n->Number ln.
	^ n asReal ln@

n->Number lg.
	^ n ln * MLog10@

n->Number log: base->Number.
	^ n ln / base ln@

#PURPOSE 'Number','trigonometric functions'

n->Number asRadian.
	^ n * PI / 180.0@

n->Number asDegree.
	^ n * 180.0 / PI@

n->Number sin.
	^ n asReal sin@

n->Number cos.
	^ n asReal cos@

n->Number tan.
	| cos |
	if: (cos:= n cos) = 0 then:[^ (TanError new init: n) raise].
	^ n sin / cos@

n->Number arcTan.
	^ n asReal arcTan@

#PURPOSE 'Number','misc math functions'

a->Number hypot: b->Number.
	^ (a sqr + b sqr) sqrt@

#PURPOSE 'Number','testing'

n->Number isPositive.
	^ n > 0@

n->Number nearZero: eta->Number.
	^ n abs < eta@

n->Number isNegative.
	^ n < 0@

#PURPOSE 'Number','loop'

start->Number to: stop->Number do: block->Block.
	| count |
	count := start.
	while:[count <= stop] do:[block doWith: count. count := count + 1]@

start->Number downTo: stop->Number do: block->Block.
	| count |
	count := start.
	while:[count >= stop] do:[block doWith: count. count := count - 1]@

start->Number to: stop->Number by: step->Number do: block->Block.
	| count |
	count := start.
	while:[count <= stop] do:[block doWith: count. count := count + step]@

start->Number downTo: stop->Number by: step->Number do: block->Block.
	| count |
	count := start.
	while:[count >= stop] do:[block doWith: count. count := count - step]@

#PURPOSE 'Number','conversion'

n->Number simplified.
	^ n@

n->Number rounded.
	#DEFERRED@

n->Number roundTo: precision->Number.
	^ (n / precision) rounded * precision@

n->Number ceiling.
	| floor |
	floor := n floor.
	if: n frac = 0 then:[^ floor].
	^ floor + 1@

n->Number floor.
	^ n int@

n->Number asLargeReal.
	#DEFERRED@

n->Number asComplex.
	#DEFERRED@

#PURPOSE 'Number','conversion','private'
#PRIVATE

n->Number generality.
	#DEFERRED@

n->Number withGenerality: generality->Integer.
	if: generality = 2 then:[^ n asLargeInteger].
	if: generality = 3 then:[^ n asFraction].
	if: generality = 4 then:[^ n asReal].
	if: generality = 5 then:[^ n asLargeReal].
	^ n asComplex@

"------------------------Integer----------------------------------"

#PUBLIC

#PURPOSE 'Integer','calculating'

a->Integer / b->Integer.
	if: b = 1 then:[^ a].
	^ (Fraction new
	  initNom: a
	    denom: b) simplified@

a->Integer & b->Integer.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) & b]
	 else:[^ a & (b withGenerality: ga)]@

a->Integer | b->Integer.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) | b]
	 else:[^ a | (b withGenerality: ga)]@

a->Integer ~ b->Integer.
	| ga gb |
	if: (ga := a generality) < (gb := b generality)
	 then:[^ (a withGenerality: gb) ~ b]
	 else:[^ a ~ (b withGenerality: ga)]@

#PURPOSE 'Integer','bit operations'

i->Integer bitAt: bit->Integer.
	^ i abs >> (bit - 1) & 1@

"
i->Integer bitInvert.
	^ i negated - 1@
"

i->Integer highBit.
	if: i = 0 then:[^ 0].
	i bitSize downTo: 1 do:
	[ :bit  if: (i bitAt: bit) = 1 then:[^ bit]]@

#PURPOSE 'Integer','testing'

i->Integer isPrimitive.
	^ true@

i->Integer isEven.
	^ i & 1 = 0@

i->Integer isOdd.
	^ i & 1 = 1@

#PURPOSE 'Integer','misc'

i->Integer hash.
	^ i@

a->Integer gcd: b->Integer.
	| u v r |
	u := a abs.
	v := b abs.
	if: u < v then:
	[r := u.
	 u := v.
	 v := r].
	whileNot:[v = 0] do:
	[r := u \\ v.
	 u := v.
	 v := r].
	^ u@

a->Integer lcm: b->Integer.
	| tmp |
	if: (tmp := a gcd: b) = 0 then:[^ 0].
	^ (a * b) abs // tmp@

#PURPOSE 'Integer','conversion'

i->Integer int.
	^ i@

i->Integer frac.
	^ 0@

i->Integer rounded.
	^ i@

i->Integer ceiling.
	^ i@

i->Integer floor.
	^ i@

#PURPOSE 'Integer','conversion','private'
#PRIVATE

i->Integer asFraction.
	^ Fraction new initNom: i denom: 1@

"--------------------SmallInteger-------------------------------"

#PUBLIC

#PURPOSE 'SmallInteger','comparing'

a->SmallInteger = b->SmallInteger.
	#PRIMITIVE SmallInteger 1 @

a->SmallInteger < b->SmallInteger.
	#PRIMITIVE SmallInteger 2 @

a->SmallInteger > b->SmallInteger.
	#PRIMITIVE SmallInteger 3 @

#PURPOSE 'SmallInteger','calculating'

a->SmallInteger + b->SmallInteger.
	#PRIMITIVE SmallInteger 4 @

a->SmallInteger - b->SmallInteger.
	#PRIMITIVE SmallInteger 5 @

a->SmallInteger * b->SmallInteger.
	#PRIMITIVE SmallInteger 6 @

a->SmallInteger // b->SmallInteger.
	#PRIMITIVE SmallInteger 7 @

a->SmallInteger \\ b->SmallInteger.
	#PRIMITIVE SmallInteger 8 @

#PURPOSE 'SmallInteger','bit operations'

a->SmallInteger & b->SmallInteger.
	#PRIMITIVE SmallInteger 9 @

a->SmallInteger | b->SmallInteger.
	#PRIMITIVE SmallInteger 10 @

a->SmallInteger ~ b->SmallInteger.
	#PRIMITIVE SmallInteger 11 @

i->SmallInteger << shift->SmallInteger.
	#PRIMITIVE SmallInteger 12 @

i->SmallInteger >> shift->SmallInteger.
	#PRIMITIVE SmallInteger 13 @

i->SmallInteger bitAt: bit->SmallInteger.
	^ i abs >> (bit - 1) & 1@

i->SmallInteger bitSize.
	#PRIMITIVE SmallInteger 14 @

#PURPOSE 'SmallInteger','testing'

i->SmallInteger isPrimitive.
	^ true@

#PURPOSE 'SmallInteger','conversion','private'
#PRIVATE

i->SmallInteger generality.
	^ 1@

i->SmallInteger asLargeInteger.
	#PRIMITIVE SmallInteger 15 @

i->SmallInteger asReal.
	#PRIMITIVE SmallInteger 16 @

"-----------------------LargeInteger------------------------------"

#PUBLIC

#PURPOSE 'LargeInteger','comparing'

a->LargeInteger = b->LargeInteger.
	#PRIMITIVE LargeInteger 1 @

a->LargeInteger < b->LargeInteger.
	#PRIMITIVE LargeInteger 2 @

a->LargeInteger > b->LargeInteger.
	#PRIMITIVE LargeInteger 3 @

#PURPOSE 'LargeInteger','calculating'

a->LargeInteger + b->LargeInteger.
	#PRIMITIVE LargeInteger 4 @

a->LargeInteger - b->LargeInteger.
	#PRIMITIVE LargeInteger 5 @

a->LargeInteger * b->LargeInteger.
	#PRIMITIVE LargeInteger 6 @

a->LargeInteger // b->LargeInteger.
	#PRIMITIVE LargeInteger 7 @

a->LargeInteger \\ b->LargeInteger.
	#PRIMITIVE LargeInteger 8 @

#PURPOSE 'LargeInteger','bit operations'

a->LargeInteger & b->LargeInteger.
	#PRIMITIVE LargeInteger 9 @

a->LargeInteger | b->LargeInteger.
	#PRIMITIVE LargeInteger 10 @

a->LargeInteger ~ b->LargeInteger.
	#PRIMITIVE LargeInteger 11 @

i->LargeInteger << shift->Integer.
	#PRIMITIVE LargeInteger 12 @

i->LargeInteger >> shift->Integer.
	#PRIMITIVE LargeInteger 13 @

i->LargeInteger bitSize.
	#PRIMITIVE LargeInteger 14 @

#PURPOSE 'LargeInteger','testing'

i->LargeInteger sign.
	if: i = 0 then:[^ 0].
	^ i->sign@

i->LargeInteger isPositive.
	^ i->sign = 1 & [i # 0]@

i->LargeInteger isNegative.
	^ i->sign = -1 & [i # 0]@

#PURPOSE 'LargeInteger','conversion'

i->LargeInteger simplified.
	#PRIMITIVE LargeInteger 15 @

#PURPOSE 'LargeInteger','conversion','private'
#PRIVATE

i->LargeInteger generality.
	^ 2@

i->LargeInteger asReal.
	#PRIMITIVE LargeInteger 16 @

"------------------------Fraction---------------------------------"

#PUBLIC

#PURPOSE 'Fraction','instance initialisation'

f->Fraction initNom: nom->Integer denom: denom->Integer.
	if: denom = 0 then:[^ DivisionError new raise].
	f->nom := nom.
	f->denom := denom.
	^ f@

#PURPOSE 'Fraction','comparing'

a->Fraction = b->Fraction.
	^ a->nom * b->denom = b->nom * a->denom@

a->Fraction < b->Fraction.
	^ a->nom * b->denom < b->nom * a->denom@

a->Fraction > b->Fraction.
	^ a->nom * b->denom > b->nom * a->denom@

#PURPOSE 'Fraction','calculating'

a->Fraction + b->Fraction.
	^ Fraction new
	  initNom: a->nom * b->denom + b->nom * a->denom
	    denom: a->denom * b->denom@

a->Fraction - b->Fraction.
	^ Fraction new
	  initNom: a->nom * b->denom - b->nom * a->denom
	    denom: a->denom * b->denom@

a->Fraction * b->Fraction.
	^ Fraction new
	  initNom: a->nom * b->nom
	    denom: a->denom * b->denom@

a->Fraction / b->Fraction.
	^ Fraction new
	  initNom: a->nom * b->denom
	    denom: a->denom * b->nom@

f->Fraction negated.
	^ Fraction new
	  initNom: f->nom negated
	    denom: f->denom@

f->Fraction reziprocal.
	if: f->nom = 1 then:[^ f->denom].
	^ Fraction new
	  initNom: f->denom
	    denom: f->nom@

#PURPOSE 'Fraction','conversion'

f->Fraction simplified.
	| lcm |
	if: f->denom = 1 then:[^ f->nom].
	if: f->nom = 0 then:[^ 0].
	if: f->nom = f->denom then:[^ 1].
	lcm := f->nom lcm: f->denom.
	^ Fraction new
	  initNom: f->nom // lcm
	    denom: f->denom // lcm@

f->Fraction int.
	^ f->nom // f->denom@

f->Fraction frac.
	^ f->nom \\ f->denom / f->denom@

f->Fraction rounded.
	if: f->nom \\ f->denom * 2 >= f->denom then:[^ f int + 1].
	^ f int@

f->Fraction ceiling.
	if: f->nom \\ f->denom # 0 then:[^ f int + 1].
	^ f int@

f->Fraction floor.
	^ f int@

#PURPOSE 'Fraction','conversion','private'
#PRIVATE

f->Fraction generality.
	^ 3@

f->Fraction asReal.
	^ f->nom asReal / f->denom asReal@

"--------------------------Real-----------------------------------"

#PUBLIC

#PURPOSE 'Real','comparing'

a->Real = b->Real.
	#PRIMITIVE Real 1 @

a->Real < b->Real.
	#PRIMITIVE Real 2 @

a->Real > b->Real.
	#PRIMITIVE Real 3 @

#PURPOSE 'Real','calculating'

a->Real + b->Real.
	#PRIMITIVE Real 4 @

a->Real - b->Real.
	#PRIMITIVE Real 5 @

a->Real * b->Real.
	#PRIMITIVE Real 6 @

a->Real / b->Real.
	#PRIMITIVE Real 7 @

a->Real \\ b->Real.
	^ a - b * (a / b) floor@

r->Real sqrt.
	if: r < 0 then:[^ r asComplex sqrt].
	^ r basicSqrt@

r->Real exp.
	#PRIMITIVE Real 9 @

r->Real ln.
	if: r = 0 then:[^ (LogarithmicError new init: r) raise].
	if: r < 0 then:[^ r asComplex ln].
	^ r basicLn@

#PURPOSE 'Real','calculating','private'
#PRIVATE

r->Real basicSqrt.
	#PRIMITIVE Real 8 @

r->Real basicLn.
	#PRIMITIVE Real 10 @

#PUBLIC
#PURPOSE 'Real','trigonometric functions'

r->Real sin.
	#PRIMITIVE Real 11 @

r->Real cos.
	#PRIMITIVE Real 12 @

r->Real arcTan.
	#PRIMITIVE Real 13 @

#PURPOSE 'Real','misc'

r->Real int.
	#PRIMITIVE Real 14 @

r->Real frac.
	#PRIMITIVE Real 15 @

#PURPOSE 'Real','conversion'

r->Real simplified.
	if: r frac = 0 then:[^ r floor].
	^ r@

r->Real rounded.
	^ (r + 0.5) floor@

#PURPOSE 'Real','conversion','private'
#PRIVATE

r->Real generality.
	^ 4@

"--------------------------Class----------------------------------"

#PUBLIC

#PURPOSE 'Class'

a->Class = b->Class.
	"Answer whether <a> has the same value as <b>"
	^ a == b@

class->Class new.
	^ class create init@

class->Class new: obj->Object.
	^ class create init: obj@

class->Class create.
	#PRIMITIVE Class 1 @

class->Class instanceVars.
	^ class->instVars @

class->Class inheritsFrom: other->Class.
	if: class == other then:[^ true].
	^ class->precedenceList includes: other@

#PURPOSE 'Object','class testing'

obj->Object class.
	"Answer the class of object"
	#PRIMITIVE Class 2 @

obj->Object is: class->Class.
	"Answer whether <class> is a superclass or the class of <obj>"
	^ obj class inheritsFrom: class@

obj->Object isKindOf: class->Class.
	"Answer whether <obj> is a instance of <class>"
	^ obj class == class@

obj->Object instVars.
	^ obj class instanceVars @

"--------------------------Block----------------------------------"

#PUBLIC

#PURPOSE 'Block'

block->Block do.
	#PRIMITIVE Block 1 @

block->Block doWith: obj->Object.
	#PRIMITIVE Block 2 @

block->Block doWith: a->Object and: b->Object.
	#PRIMITIVE Block 3 @

block->Block doWithParams: params->List.
	#PRIMITIVE Block 4 @

i->Integer timesRepeat: action->Block.
	^ 1 to: i do: action@

"--------------------------Boolean-------------------------------"

#PUBLIC

#PURPOSE 'Boolean'

a->Boolean & b->Boolean.
"	if: a then:[^ b].
	^ false"
	#PRIMITIVE Boolean 1 @

a->Boolean | b->Boolean.
"	ifNot: a then:[^ b].
	^ true"
	#PRIMITIVE Boolean 2 @

a->Boolean ~ b->Boolean.
"	^ a # b"
	#PRIMITIVE Boolean 3 @

b->Boolean not.
"	if: b then:[^ false].
	^ true"
	#PRIMITIVE Boolean 4 @

"--------------------------Error------------------------------"

#PUBLIC

#PURPOSE 'Error'

error->Error raise.
	#PRIMITIVE Error 1@

try: action->Block ifError: handle->Block.
	#PRIMITIVE Error 2 @

"--------------------------Errors------------------------------"

#PUBLIC

#PURPOSE 'Errors'

e->ArithmeticError init: a->ArithmeticValue.
	e->value := a.
	^ e@

e->RaiseError init: n->Number exponent: exponent->Number.
	e->value := n.
	e->exponent:= exponent.
	^ e@

e->CollectionError init: c->Collection.
	e->collection := c.
	^ e@

e->KeyNotFoundError init: c->Collection key: key->Object.
	e->collection := c.
	e->key := key.
	^ e@

e->CantInsertError init: c->Collection value: value->Object.
	e->collection := c.
	e->value := value.
	^ e@

e->StreamError init: s->Stream.
	e->stream := s.
	^ e@

e->StreamSeekError init: s->Stream position: position->Integer.
	e->stream := s.
	e->position := position.
	^ e@

"----------------------------Test---------------------------------"

obj->Object print.
 #PRIMITIVE Test 1 @

i->SmallInteger print.
 #PRIMITIVE Test 2 @

r->Real print.
 #PRIMITIVE Test 3 @

c->Character print.
 #PRIMITIVE Test 4 @

s->String print.
 #PRIMITIVE Test 5 @

"-----------------------------------------------------------------"
@ "END OF METHODS"
"-----------------------------------------------------------------"
"---------------------------- INIT -----------------------------"
"-----------------------------------------------------------------"
"
MLog10 := 10 ln reziprocal.
HalfPI := PI / 2.
@
"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------" END "-----------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"