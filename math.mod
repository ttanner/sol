"-----------------------------------------------------------------"
"-----------------------" MODUL MATH "----------------------------"
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

"--------------------------Numbers----------------------------"

LargeReal (Real)
 mantisse	"Integer"
 exponent	"Integer"
 @

Complex (Number)
 real	"Number"
 imag	"Number"
 @

"------------------------Errors-------------------------------"

TanhError(ArithmeticError)
 number
 @

"-----------------------------------------------------------------"
@ "END OF CLASSES"
"-----------------------------------------------------------------"
"---------------------------" METHODS "---------------------------"
"-----------------------------------------------------------------"

"----------------------------Number-------------------------------"

#PUBLIC

#PURPOSE 'Number'

#PURPOSE 'Number','inverse trigonometric functions'

n->Number arcSin.
	if: n = 1 then:[^ HalfPI].
	if: n =-1 then:[^ HalfPI negated].
	if: (n > -1) & [n < 1] then:[^ (n / (1 - n sqr) sqrt) arcTan].
	^ (RangeError new init: n) raise@

n->Number arcCos.
	if: n = 0 then:[^ 0].
	if: (n > -1) & [n < 1] then:[^ ((1 - n sqr) sqrt / n) arcTan].
	^ (RangeError new init: n) raise@

a->Number arcTanDiv: b->Number.
	| angle |
	if: b = 0
	 then:[ if: a < 0
		 then:[angle:=HalfPI negated]
		 else:[angle:=HalfPI]]
	 else:[ angle:= (a / b) arcTan].
	if: b < 0 then:
	 [ if: a < 0
	    then:[angle := angle - PI]
	    else:[angle := angle + PI]].
	^ angle@

#PURPOSE 'Number','hyperbolic trigonometric functions'

n->Number sinh.
	| e |
	e := n exp.
	^ (e - 1) / e * 0.5@

n->Number cosh.
	| e |
	e := n exp.
	^ (e + 1) / e * 0.5@

n->Number tanh.
	| e1 e2 |
	e1 := n exp.
	e2 := e1 reziprocal.
	^ (e1 - e2) / (e1 + e2)@

n->Number arcSinh.
	^ (n + (n sqr + 1) sqrt) ln@

n->Number arcCosh.
	if: n > 1 then:[^ (n + (n sqr - 1) sqrt) ln].
	^ (RangeError new init: n) raise@

n->Number arcTanh.
	if: (n >= -1) & [n < 1] then:[^ ((1 + n) / (1 - n)) ln * 0.5].
	^ (RangeError new init: n) raise@


#PURPOSE 'Number','conversion'

n->Number i.
	^ Complex new initImag: n@

n->Number asComplex.
	^ Complex new initReal: n@

"------------------------Integer----------------------------------"

#PUBLIC

#PURPOSE 'Integer'

i->Integer intSqrt.
	| guess newGuess condition |
	if: i <= 0 then:[^ 0].
	guess := i // 2 max: 1.
	newGuess := (guess * guess + i) // 2 // guess.
	condition := if: guess * guess < i
		      then:[[newGuess <= guess]]
		      else:[[newGuess >= guess]].
	whileNot: [condition do] do:
	[guess := newGuess.
	 newGuess := (guess * guess + i) // 2 // guess].
	^ guess@

#PURPOSE 'Integer','statistical functions'

n->Integer factorial.
	| x t s |
	x := if: n isOdd then:[n] else:[1].
	t := n >> 1.
	s := t * (t + 1).
	1 to: t do:
	[:i
	 x := x * s.
	 s := s - (i + i)].
	^ x@

n->Integer combination: k->Integer.
	^ (n permutation: k) // k factorial@

n->Integer permutation: k->Integer.
	^ n factorial // (n - k) factorial@

#PURPOSE 'Integer','trigonometric functions'

degree->Integer intSin.
	| q r |
	r := degree \\ 360.
	q := r // 90.
	r := r \\ 90 + 1.
	if: q = 0 then:[^ SinValues at: r].
	if: q = 1 then:[^ SinValues at: 92 - r].
	if: q = 2 then:[^ 0 - (SinValues at: r)].
	if: q = 3 then:[^ 0 - (SinValues at: 92 - r)]@

degree->Integer intCos.
	| q r |
	r := degree \\ 360.
	q := r // 90.
	r := r \\ 90 + 1.
	if: q = 0 then:[^ SinValues at: 92 - r].
	if: q = 1 then:[^ 0 - (SinValues at: r)].
	if: q = 2 then:[^ 0 - (SinValues at: 92 - r)].
	if: q = 3 then:[^ SinValues at: r]@

degree->Integer intTan.
	^ degree intSin / degree intCos@

#PURPOSE 'Integer','conversion','private'
#PRIVATE

i->Integer asLargeReal.
	^ LargeReal new initMantisse: i@

#PURPOSE 'Integer','testing'

i->Integer isPrim.
	| k n |
	if: i isEven & [i # 2] then:[^ false].
	if: (n := i intSqrt) sqr = i then:[^ false].
	k := 3.
	while:[k <= n] do:
	[ if: (i \\ k) = 0 then:[^ false].
	  k := k + 2].
	^ true@

"------------------------Fraction---------------------------------"

#PURPOSE 'Fraction','conversion','private'
#PRIVATE

f->Fraction asLargeReal.
	^ LargeReal new initFraction: f@

"-------------------------LargeReal---------------------------------"

#PUBLIC

#PURPOSE 'LargeReal','instance initialisation'

r->LargeReal initMantisse: mantisse->Number.
	r->mantisse := mantisse.
	r->exponent := 0.
	^ r@

r->LargeReal initFraction: f->Fraction.
	r->exponent := 0.
	^ r@

#PURPOSE 'LargeReal','comparing'

a->LargeReal < b->LargeReal.
	@

#PURPOSE 'LargeReal','conversion','private'
#PRIVATE

r->LargeReal generality.
	^ 5@

"--------------------------Complex--------------------------------"

#PUBLIC

#PURPOSE 'Complex','instance initialisation'

c->Complex initReal: real->Number.
	^ c initReal: real imag: 0@

c->Complex initImag: imag->Number.
	^ c initReal: 0 imag: imag@

c->Complex initReal: real->Number imag: imag->Number.
	c->real := real.
	c->imag := imag.
	^ c@

#PURPOSE 'Complex','comparing'

"
a->Complex < b->Complex.
	^ x absSqr < y absSqr@

a->Complex > b->Complex.
	^ x absSqr > y absSqr@
"

#PURPOSE 'Complex','calculating'

a->Complex + b->Complex.
	^ Complex new
	  initReal: a->real + b->real
	      imag: a->imag + b->imag@

a->Complex - b->Complex.
	^ Complex new
	  initReal: a->real - b->real
	      imag: a->imag - b->imag@

a->Complex * b->Complex.
	| tmp tmp2 |
	tmp :=a->real * b->real.
	tmp2:=a->imag * b->imag.
	^ Complex new
	  initReal: tmp - tmp2
	      imag: tmp + tmp2@

a->Complex / b->Complex.
	| tmp |
	if: (tmp:= b absSqr) = 0 then:[^ DivisionError new].
	tmp:=tmp reziprocal.
	^ Complex new
	  initReal: (a->real * b->real + a->imag * b->imag) * tmp
	      imag: (a->imag * b->real - a->imag * b->real) * tmp@

c->Complex i.
	^ Complex new
	  initReal: c->imag negated
	      imag: c->real@

c->Complex negated.
	^ Complex new
	  initReal: c->real negated
	      imag: c->imag negated@

c->Complex reziprocal.
	| tmp |
	if: (tmp:= c absSqr) = 0 then:[^ DivisionError new].
	tmp:=tmp reziprocal.
	^ Complex new
	  initReal: c->real * tmp
	      imag: c->imag * tmp@

c->Complex abs.
	^ c->real hypot: c->imag@

c->Complex absSqr.
	^ c->real sqr + c->imag sqr@

c->Complex sqr.
	^ Complex new
	  initReal: c->real sqr - c->imag sqr
	      imag: 2 * c->real * c->imag@

c->Complex sqrt.
	| s d answer |
	s:= ((c->real abs + (c->real hypot: c->imag)) * 0.5) sqrt.
	d:= c->imag / s * 0.5.
	answer:= Complex new.
	if: c->real > 0  then:[^ answer initReal: s imag: d].
	if: c->imag >= 0 then:[^ answer initReal: d imag: s].
	^ answer initReal: d negated imag: s negated@

c->Complex exp.
	| r |
	r:= c->real exp.
	^ Complex new
	  initReal: r * c->imag cos
	      imag: r * c->imag sin@

a->Complex power: b->Real.
	| h arg lr li |
	h:= a->real hypot: a->imag.
	if: h = 0 then:[^ RaiseError new].
	arg:= a->imag arcTanDiv: a->real.
	lr:= h power: b.
	li:= b * arg.
	^ Complex new
	  initReal: lr * li cos
	      imag: lr * li sin@

a->Complex power: b->Complex.
	| h arg lr li |
	h:= a->real hypot: a->imag.
	if: h = 0 then:[^ RaiseError new].
	arg:= a->imag arcTanDiv: a->real.
	lr:= h power: b->real.
	li:= b->real * arg.
	ifNot: b->imag = 0 then:
	[lr:= lr / (b->imag * arg) exp.
	 li:= li + b->imag * h ln].
	^ Complex new
	  initReal: lr * li cos
	      imag: lr * li sin@

c->Complex ln.
	| h |
	h:= c abs.
	if: h = 0 then:[^ LogarithmicError new].
	^ Complex new
	  initReal: (c->real arcTanDiv: c->imag)
	      imag: h ln@

c->Complex lg.
	| h |
	h:= c abs.
	if: h = 0 then:[^ LogarithmicError new].
	^ Complex new
	  initReal: (c->real arcTanDiv: c->imag)
	      imag: h lg@

#PURPOSE 'Complex','trigonometric functions'

c->Complex sin.
	^ Complex new
	  initReal: c->real sin * c->imag cosh
	      imag: c->real cos * c->imag sinh@

c->Complex cos.
	^ Complex new
	  initReal: c->real cos * c->imag cosh
	      imag: c->real sin * c->imag sinh@

c->Complex tan.
	| tmp |
	tmp:= (2 * c->real) cos + (2 * c->imag) cosh.
	if: tmp = 0 then:[^ TanError new].
	tmp:= tmp reziprocal.
	^ Complex new
	  initReal: (2 * c->real) sin  * tmp
	      imag: (2 * c->real) sinh * tmp@

c->Complex arcSin.
	| tmp phi rp |
	tmp:= Complex new.
	tmp initReal: 1 - c->real sqr + c->imag sqr
		imag: -2 * c->real * c->imag.
	phi:= tmp arg * 0.5.
	rp:= tmp abs sqrt.
	tmp initReal: c->real - rp * phi sin
		imag: c->imag - rp * phi cos.
	^ tmp initReal: tmp arg
		 imag: tmp abs lg negated@

c->Complex arcCos.
	| tmp phi rp |
	tmp:= Complex new.
	tmp initReal: 1 - c->real sqr + c->imag sqr
		imag: -2 * c->real * c->imag.
	phi:= tmp arg * 0.5.
	rp:= tmp abs sqrt.
	tmp initReal: c->real - rp * phi cos
		imag: c->imag - rp * phi sin.
	^ tmp initReal: tmp arg
		 imag: tmp abs lg negated@

c->Complex arcTan.
	| tmp opb a2 den |
	opb:= c->imag + 1.
	a2 := c->real sqr.
	den:= opb sqr + a2.
	if: den = 0 then:[^ (ArcTanError new init: c) raise].
	tmp:= Complex new
	       initReal: ((1 - c->imag) * opb - a2) / den
		   imag: 2 * c->real / den.
	^ tmp initReal: tmp arg * 0.5
		 imag: tmp abs lg negated * 0.5@

c->Complex sinh.
	^ Complex new
	  initReal: c->real sinh * c->imag cos
	      imag: c->real cosh * c->imag sin@

c->Complex cosh.
	^ Complex new
	  initReal:  c->real cosh * c->imag cos
	      imag: (c->real sinh * c->imag sin) negated@

c->Complex tanh.
	| tmp |
	tmp:= (2 * c->real) cosh + (2 * c->imag) cos.
	if: tmp = 0 then:[^ (TanhError new init: c) raise].
	tmp:= tmp reziprocal.
	^ Complex new
	  initReal: (2 * c->real) sinh * tmp
	      imag: (2 * c->real) sin  * tmp@

"
c->Complex arcSinh.
c->Complex arcCosh.
c->Complex arcTanh.
"

#PURPOSE 'Complex','misc'

c->Complex arg.
	if: c absSqr = 0 then:[^ 0].
	^ c->imag arcTanDiv: c->real@

#PURPOSE 'Complex','conversion'

c->Complex simplified.
	if: c->imag = 0 then:[^ c->real simplified].
	^ c@

c->Complex int.
	^ Complex new
	  initReal: c->real int
	      imag: c->imag int@

c->Complex frac.
	^ Complex new
	  initReal: c->real frac
	      imag: c->imag frac@

#PURPOSE 'Complex','conversion','private'
#PRIVATE

c->Complex generality.
	^ 6@

"-----------------------------------------------------------------"
@ "END OF METHODS"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------" END "-----------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"
"-----------------------------------------------------------------"