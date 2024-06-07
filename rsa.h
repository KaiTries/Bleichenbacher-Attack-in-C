#pragma once
#include <gmp.h>

#define RSA_BLOCK_BYTE_SIZE 128
#define RSA_PUBLIC_KEY_SIZE 1024
#define RSA_PUBLIC_EXPONENT 65537

extern int oracleCalls;

typedef struct RSA {
  mpz_t E;
  mpz_t D;
  mpz_t N;
} RSA;

// creates PKCS conform output string
int prepareInput(char *output, char *input);

// stripts PKCS padding away
int prepareOutput(char *output, char *input);

// encrypts input with the given RSA key
void encrypt(mpz_ptr output, mpz_ptr input, RSA *rsa);

// decrypts input with the given RSA key
void decrypt(mpz_ptr output, mpz_ptr input, RSA *rsa);

// creates an RSA key
void generate(RSA *rsa);

// turns an mpz integer into its hex string representations
void mpz_to_hex_array(char *hex_string, mpz_ptr number);

// returns 1 if the given number is PKCS conforming
int oracle(mpz_ptr number, RSA *rsa, mpz_ptr B2, mpz_ptr B3);
