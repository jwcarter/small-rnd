
To compile with the MWC Lag 2 generator include rnd_mwc_lag2.c:
gcc -lm -o rnd_test rnd.c rnd_mwc_l2.c rnd_test.c

To compile with the MWC Lag 5 generator include rnd_mwc_lag5.c:
gcc -lm -o rnd_test rnd.c rnd_mwc_l5.c rnd_test.c

To compile with the LCG96 generator include rnd_lcg96.c:
gcc -lm -o rnd_test rnd.c rnd_lcg96.c rnd_test.c
