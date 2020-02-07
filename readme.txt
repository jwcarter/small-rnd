======================================================================
GOALS
======================================================================
1) Simple (small) psuedo random number generators suitable for games
	[Easy to copy and paste the code]
2) State stored locally
	[Easy to reproduce results]
3) Pass most, if not all, tests of Bigcrush
4) Speed comparable with the glibc generator [Not too slow]

======================================================================
Description
======================================================================

The generators are all based on Multiple with Carry (MWC) generators.

This type of generator was invented by George Marsaglia. For a MWC,
X2 = (A*X1+C1) mod B, C2 = (A*X1+C1)/B where X2 and C2 are the new state,
X1 and C1 are the old state, A is the multiplier, and B is the base. A
version with lag L, uses the Lth previous X, but still uses the previous
C. To find an A that will give the maximum period for a given base B and
lag L, chose multiplier A such that A*B^L-1 and A*B^L/2-1 are both prime.
This will give a period of A*B^L/2-1.

There appears to be a slight bias in the high bits according to Raymond 
Couture and Pierre L'Ecuyer in "Distribution properties of Multiply-
with-carry random number generators" from 1997. It is not clear if this 
bias is seen in lag 2 or higher generators, but, quite opposite from 
an LCG, it seems like the lower bits should be preferred. If this bias 
exists then either the tests of BigCrush do not test for it or the bias 
is very small for lag 2 and lag 3 generators.

The rand_63.c generator uses a MWC32 generator with no lag. The new state
X2 and C2 (where X2 is the lower 32-bits and C2 is the upper of a 64-bit
number) are multipled with constants K1 and K2. The product of X2 and K1
has its upper and lower 32 bits swapped and then is added to the product of
C2 and K2 to make the new random number. This generator has a period of
~2^63.

The rand_95.c generator uses a MWC32 generator with lag two. The full
64-bit output is mixed by multiplying the upper and lower 32-bits with
32-bit numbers, rotating the second product by 32-bits and adding it to
the first product to make the new random number. This generator has a
period of ~2^95.

The rand_126.c generator uses two MWC32 generators with no lag. The new
64 bits of state of one generator has its upper and lower 32 bits swapped
and then added to the new 64 bits of state of the other generator to make
the new random number. This generator has a period of ~2^126.

The rand_127.c generator uses a MWC32 generator with lag three. The full
64-bit output is mixed by multiplying the upper and lower 32-bits with
32-bit numbers, rotating the second product by 32-bits and adding it to
the first product to make the new random number. This generator has a
period of ~2^127.

The rand_255.c generator uses a MWC32 generator with lag seven. The new
state X2 and C2 (where X2 is the lower 32-bits and C2 is the upper of a
64-bit number) are multipled with constants K1 and K2. The product of X2
and K1 has its upper and lower 32bits swapped and then is added to the
product of C2 and K2 to make the new random number. This generator has a
period of ~2^255.

The rand_287.c generator uses a MWC32 generator with lag eight. The full
64-bit output is mixed by multiplying the upper and lower 32-bits with
32-bit numbers, rotating the second product by 32-bits and adding it to
the first product to make the new random number. This generator has a
period of ~2^287.

======================================================================
BigCrush Test Results
======================================================================

BigCrush is a suite of tests in the TestU01 library by Pierre L'Ecuyer 
of the Université de Montréal.
[See http://www.iro.umontreal.ca/~simardr/testu01/tu01.html.]
It uses 357,109,737,005 random numbers in running 254 tests and takes 
~3-4 hours to run depending on the generator and the computer.

Since BigCrush only tests 32-bit numbers each generator was tested using
bits 1-32, 17-48, 33-64, and 49-16. This is not sufficient to fully test
a generator producing 64-bits, but it will hopefully find any major flaws.
The rand_63.c generator, since its period is < 2^64, does not produce
every number from 0 to 2^64-1 and a good 64-bit test would catch that.

======================================================================
BUILDING
======================================================================

To build the test program, use one of the following depending on
which generator you want to use.

To use rnd_63.c:
gcc -lm -o rnd_test rnd.c rnd_63.c rnd_test.c

To use rnd_95.c:
gcc -lm -o rnd_test rnd.c rnd_95.c rnd_test.c

To use rnd_126.c:
gcc -lm -o rnd_test rnd.c rnd_126.c rnd_test.c

To use rnd_127.c:
gcc -lm -o rnd_test rnd.c rnd_127.c rnd_test.c

To use rnd_255.c:
gcc -lm -o rnd_test rnd.c rnd_255.c rnd_test.c

To use rnd_287.c:
gcc -lm -o rnd_test rnd.c rnd_287.c rnd_test.c

======================================================================
EXAMPLE GENERATORS
======================================================================

In the extras directory, there are examples of random number generators
in examples_64bit.c.

To build the examples:
gcc -O1 -o examples_64bit -lm examples_64bit.c

Running examples_64bit will run each generator 1,000,000,000 times and
print the time it took. (Because, of course, this is the most important
characterictic of a random number generator.)
