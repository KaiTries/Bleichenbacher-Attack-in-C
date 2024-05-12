#pragma once
#include "interval.h"
#include "rsa.h"
#include <stdio.h>
#include <string.h>

extern mpz_t B, B2, B3, s;
extern RSA rsa;

// sets up environment for the attack
void setup();

// turns user input into valid pkcs string
int inputToPaddedMessage(char *pkcs_padded_input, char *user_input);

// turns an mpz integer into its hex string representations
void mpz_to_hex_array(char *hex_string, mpz_t *number);

// returns 1 if the given number is PKCS conforming
int oracle(mpz_t *number);

// iteratively searches for the next bigger s that results in a valid PKCS message
void findNextS(mpz_t *c);

// searches the last Interval with given algorithm
void searchingWithOneIntervalLeft(IntervalSet *set, mpz_t *c);

// Finds new possible intervals
void findNewIntervals(IntervalSet *priorSet);