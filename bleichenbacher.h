#pragma once
#include "interval.h"
#include <stdio.h>
#include <string.h>

#define RSA_BLOCK_BYTE_SIZE 128
#define RSA_PUBLIC_KEY_SIZE 1024
#define RSA_PUBLIC_EXPONENT 65537


void setup();

void encrypt(mpz_t *output);

void decrypt(mpz_t input, mpz_t *output);

int add_padding(char *pkcs_padded_input, char *user_input);

void print_hex(char *input, int length);