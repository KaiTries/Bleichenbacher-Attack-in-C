#pragma once
#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>

#define RSA_BLOCK_BYTE_SIZE 128
#define RSA_PUBLIC_KEY_SIZE 1024
#define RSA_PUBLIC_EXPONENT 65537

extern int oracleCalls;

typedef struct RSA {
    mpz_t E;
    mpz_t D;
    mpz_t N;
} RSA;

// encrypts input with the given RSA key
void encrypt(mpz_t *output, mpz_t *input, RSA *rsa);

// decrypts input with the given RSA key
void decrypt(mpz_t *output, mpz_t *input, RSA *rsa);

// creates an RSA key
void generate(RSA *rsa);

// turns an mpz integer into its hex string representations
void mpz_to_hex_array(char *hex_string, mpz_t *number);

// returns 1 if the given number is PKCS conforming
int oracle(mpz_t *number, RSA *rsa);