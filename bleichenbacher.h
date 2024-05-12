#pragma once
#include "interval.h"
#include <stdio.h>
#include <string.h>

#define RSA_BLOCK_BYTE_SIZE 128
#define RSA_PUBLIC_KEY_SIZE 1024
#define RSA_PUBLIC_EXPONENT 65537

extern mpz_t d, n, e, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime;


void setup();

void encrypt();

void decrypt(mpz_t output, mpz_t input);

int inputToPaddedMessage(char *pkcs_padded_input, char *user_input);

void mpz_to_hex_array(char *hex_string, mpz_t number);

int oracle(mpz_t number);

void findNextS();

void searchingWithOneIntervalLeft(IntervalSet *set);

void findNewIntervals(IntervalSet *priorSet);