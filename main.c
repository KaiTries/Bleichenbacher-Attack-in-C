#include "interval.h"
#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>

clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;
char user_input[RSA_BLOCK_BYTE_SIZE];
char user_input_hex[RSA_BLOCK_BYTE_SIZE * 2];
char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE * 2];
char encrypted_input[RSA_BLOCK_BYTE_SIZE * 2];
char decrypted_input_char[RSA_BLOCK_BYTE_SIZE * 2];


/**
 * Reads a line from standard input, pads the input using PKCS#1 v1.5 padding scheme, 
 * and converts the padded input to an mpz_t integer.
 */
void get_user_input(mpz_t *m){
    fgets(user_input, sizeof(user_input), stdin);
    prepareInput(pkcs_padded_input, user_input);
    mpz_set_str(*m, pkcs_padded_input, 16);
}

int main() {
    mpz_t m, c;
    mpz_init(m); mpz_init(c);
    setup();
    get_user_input(&m);

    encrypt(&c, &m, &rsa);

    fullyOptimizedAttack(&c);

    optimizedWithoutTrimmers(&c);

    trimmersOnly(&c);

    baseAttack(&c);

    mpz_clear(c);
    return 0;
}

