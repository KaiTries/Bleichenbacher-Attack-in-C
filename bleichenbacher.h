#pragma once
#include "rsa.h"

extern mpz_t B, B2, B3;
extern RSA rsa;

void initSetup();
// sets up environment for the attack
void setup(gmp_randstate_ptr state);

// base S search function
void findNextS_iteratively(mpz_t *c, mpz_t *s, mpz_t *a, mpz_t *b);

// base bleichenbacher attack as outlined in original paper
void baseAttack(mpz_t *c, int *calls, int *s2aCalls, double *rTime);

// base bleichenbacher but with trimmed initial interval
void trimmersOnly(mpz_t *c, int *calls, int *s2aCalls, double *rTime);

// All optimizations except trimmers
void optimizedWithoutTrimmers(mpz_t *c, int *calls, int *s2aCalls, double *rTime);

// optimized bleichenbacher attack
void fullyOptimizedAttack(mpz_t *c, int *calls, int *s2aCalls, double *rTime);
