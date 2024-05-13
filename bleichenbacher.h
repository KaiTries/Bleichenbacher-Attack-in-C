#pragma once
#include "interval.h"
#include "rsa.h"
#include <stdio.h>
#include <string.h>

extern mpz_t B, B2, B3;
extern RSA rsa;

// sets up environment for the attack
void setup();

// trimming
void trimming(mpz_t *t, mpz_t *ul, mpz_t *uh, mpz_t *c, RSA *rsa);

// optimized step for the iterative searching of s
void findNextS_2a(mpz_t *c, mpz_t *s, mpz_t *a, mpz_t *b);

// searches the last Interval with given algorithm
int searchingWithOneIntervalLeft(Interval *interval, mpz_t *c, mpz_t *s);

// searches through each interval with the same strategy as if only one interval left
void findNextS_multipleIntervals(IntervalSet *set, mpz_t *c, mpz_t *s);

// Finds new possible intervals
void findNewIntervals(IntervalSet *priorSet, mpz_t *s);