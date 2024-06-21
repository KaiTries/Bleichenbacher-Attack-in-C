#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char user_input[RSA_BLOCK_BYTE_SIZE];
char user_input_copy[sizeof(user_input)];
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
void set_current_c(int i, mpz_t *c, mpz_t *m) {
  int written = snprintf(user_input, sizeof(user_input), "%s%d\n", "Number: ", i);
  strcpy(user_input_copy, user_input);
  prepareInput(pkcs_padded_input, user_input_copy);
  mpz_set_str(*m, pkcs_padded_input, 16);
  encrypt(c, m, &rsa);
}
/**
 * Runs 3 different bleichenbacher attacks on some message given by the user
 */
int main() {
    mpz_t m, c;
    mpz_init(m); mpz_init(c);
    setup();
    int iterations = 100;
    //get_user_input(&m);

    // encrypt(&c, &m, &rsa);


    // optimizedWithoutTrimmers(&c);

    // trimmersOnly(&c);

    // baseAttack(&c);

    int calls;
    int s2aCalls;
    double rTime;

    for (size_t i = 0; i < iterations; i++)
    {
      set_current_c(i,&c,&m);
      fullyOptimizedAttack(&c, &calls, &s2aCalls, &rTime);
      printf("Original Message: %s", user_input_copy);
    }

    printf("Average num of Calls: %d\n", calls / iterations);
    printf("Average num Calls 2a: %d\n", s2aCalls / iterations);
    printf("average time to fins: %f\n", rTime / iterations);

    mpz_clear(c);
    mpz_clear(m);
    return 0;
}




