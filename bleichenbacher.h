#pragma once
#include "rsa.h"

extern mpz_t B, B2, B3;
extern RSA rsa;

// sets up environment for the attack
void setup();

// base S search function
void findNextS_iteratively(mpz_ptr c, mpz_ptr s, mpz_ptr a, mpz_ptr b);

// base bleichenbacher attack as outlined in original paper
void baseAttack(mpz_ptr c);

// base bleichenbacher but with trimmed initial interval
void trimmersOnly(mpz_ptr c);

// All optimizations except trimmers
void optimizedWithoutTrimmers(mpz_ptr c);

// optimized bleichenbacher attack
void fullyOptimizedAttack(mpz_ptr c, int *calls, double *time);
