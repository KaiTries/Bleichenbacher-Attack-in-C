#pragma once
#include "interval.h"
#include "rsa.h"
#include <stdio.h>
#include <string.h>

extern mpz_t B, B2, B3;
extern RSA rsa;

// sets up environment for the attack
void setup();

// iteratively searches for the next bigger s that results in a valid PKCS message
void findNextS(mpz_t *c, mpz_t *s);

// searches the last Interval with given algorithm
void searchingWithOneIntervalLeft(IntervalSet *set, mpz_t *c, mpz_t *s);

// Finds new possible intervals
void findNewIntervals(IntervalSet *priorSet, mpz_t *s);