======================================================================
GOALS
======================================================================
1) Simple (small) psuedo random number generator suitable for games
	[Easy to copy and paste the code]
2) State stored locally
	[Easy to reproduce results]
3) Pass most, if not all, tests of Bigcrush
4) Speed comparable with the glibc generator

======================================================================
Description
======================================================================

Four of the generators are Multiple with Carry (MWC) generators with
a lag. This type of generator was invented by George Marsaglia. For a
MWC, X2 = (A*X1+C1) mod B, C2 = (A*X1+C1)/B where X2 and C2 are the 
new state, X1 and C1 are the old state, A is the multiplier, and B is 
the base. A version with lag L, uses the Lth previous X, but still uses 
the previous C. To find an A that will give the maximum period for a 
given base B and lag L, chose multiplier A such that A*B^L-1 and 
A*B^L/2-1 are both prime. This will give a period of A*B^L/2-1. There 
appears to be a slight bias in the high bits according to Raymond 
Couture and Pierre L'Ecuyer in "Distribution properties of Multiply-
with-carry random number generators" from 1997. It is not clear if this 
bias is seen in lag 2 or higher generators, but, quite opposite from 
an LCG, it seems like the lower bits should be preferred. If this bias 
exists then either the tests of BigCrush do not test for it or the bias 
is very small for lag 2 and lag 3 generators.

The MWC32 lag 2 generator is the simplest generator that will pass all
of the tests of BigCrush. It has a period of ~2^95.

The MWC32 lag 3 generator has a longer period without sacrificing much
speed. It has a period of ~2^127.

The MWC60 lag 2 generator can produce a finer-grained double. A 60 bit
generator is used instead of 64 bit to make the calculation easier. A 
double has a max of 53 bits of percision (52 bit significand with the 
leading 1 assumed). The lower 50 bits are used to give some margin when 
coverting the double to an open interval. So there will be 2^50 unique 
values instead of 2^32 as in the mwc32 generators. This generator is up 
to twice as slow as the MWC32 ones. It has a period of ~2^179.

The MWC60 lag 3 generator has longer period without sacrificing much
speed over the MWC60 lag2. It has a period of ~2^239.

The LCG96 is a 96 bit Linear Congruential Generator (LCG). Please read
Knuth's TAoCP II for the theory behind LCGs and how to pick values. It
is included here to give an idea of how big an LCG is needed to get
comparable quality random numbers.

======================================================================
BigCrush Test Results
======================================================================

BigCrush is a suite of tests in the TestU01 library by Pierre L'Ecuyer 
of the Université de Montréal.
[See http://www.iro.umontreal.ca/~simardr/testu01/tu01.html.]
It uses 357,109,737,005 random numbers in running 254 tests and takes 
~4 hours (depending on the generator) to run on i7-2600 @ 3.4Ghz.

Results from the included generators:

The MWC32 lag2 and lag3 generators pass all of the tests in BigCrush.

The MWC60 lag2 and lag3 generators pass all of the tests in BigCrush 
(using only the bottom 32 bits.

The LCG96 generator pass all of the tests in BigCrush (using only the 
top 32 bits).


Results from other generators:

The KISS generator by George Marsaglia passes all of the tests.

The Mersenne Twister fails 2 out of the 254 tests (Two different 
implementations fail the same two tests.)

A LCG64 (using only the top 32 bits) fails 7 out of the 254 tests.

A MWC32 generator with no lag fails 7 out of the 254 tests.

Donald Knuth's generator from TAoCP II p186 fails 50 out of the 254 
tests.

The glibc generator has not been tested on BigCrush, but it fails 8
out of 15 tests on SmallCrush compared with Knuth's generator which 
fails 5 out of 15 tests on SmallCrush.

======================================================================
SPEED
======================================================================

Time to generate 1,000,000,000 random numbers.

			 [AMD E-350]	[i3-2125@3.3Ghz]	[i7-2600@3.4Ghz]	
			 -O0	-O1		-O0		-O1			-O0		-O1

MWC32L2		 47.8	28.3	10.2	 3.4		11.1	3.0
MWC32L3		 49.0	28.9	10.7	 3.4		 9.2	3.0
MWC60L2		109.5	67.2	15.3	 5.0		13.8	4.5
MWC60L3		113.8	68.5	15.5	 5.1		13.5	4.5
LCG96		 85.0	42.7	14.9	 5.4		13.4	4.8

glibc		 64.4	52.2	10.7	10.0		10.5	9.6
KISS		 65.4	33.9	19.5	 9.1		11.7	4.0
Knuth2		 72.2	40.2	24.1	 8.8		21.5	7.8
LCG64		 42.7	29.5	10.3	 3.4		 9.0	3.0
MWC32		 46.5	27.6	 9.1	 3.4		 7.9	3.0
Twister		 41.2	35.2	13.2	 6.8		11.7	6.1

Multiple trials were done in all cases.

Strange that the MWC32 L2 with no optimizations does worse on the i7 
than on the i3.

Strange that KISS has such a large difference between the I3 and I7.

Obviously, glibc is not being recompiled, so its optimization level is
whatever the library is compiled at. Only the test program is being
optimized here.

======================================================================
BUILDING
======================================================================

If in doubt, just use the MWC32 Lag 2 generator. It should be good 
enough in most cases.

To compile with the MWC32 Lag 2 generator include rnd_mwc_lag2.c:
gcc -lm -o rnd_test rnd.c rnd_mwc32_l2.c rnd_test.c

To compile with the MWC32 Lag 3 generator include rnd_mwc_lag5.c:
gcc -lm -o rnd_test rnd.c rnd_mwc32_l3.c rnd_test.c

To compile with the MWC60 Lag 2 generator include rnd_mwc_lag2.c:
gcc -lm -o rnd_test rnd.c rnd_mwc60_l2.c rnd_test.c

To compile with the MWC60 Lag 3 generator include rnd_mwc_lag5.c:
gcc -lm -o rnd_test rnd.c rnd_mwc60_l3.c rnd_test.c

To compile with the LCG96 generator include rnd_lcg96.c:
gcc -lm -o rnd_test rnd.c rnd_lcg96.c rnd_test.c
