#include "bleichenbacher.h"
#include <gmp.h>
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
char *message;

int compareInt(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int compareDouble(const void *a, const void *b) {
  return (*(double*)a - *(double*)b);
}

double calculateMedianDouble(double *array, int size) {
    qsort(array, size, sizeof(int), compareDouble);
    if (size % 2 != 0)
        return (double)array[size / 2];
    else
        return (double)(array[(size - 1) / 2] + array[size / 2]) / 2.0;
}

double calculateMedianInt(int *array, int size) {
    qsort(array, size, sizeof(int), compareInt);
    if (size % 2 != 0)
        return (double)array[size / 2];
    else
        return (double)(array[(size - 1) / 2] + array[size / 2]) / 2.0;
}


void PMS(gmp_randstate_ptr state) {
  mpz_t a, b, f;
  mpz_inits(a,b,f,NULL);

  mpz_ui_pow_ui(a,16,95);
  mpz_ui_pow_ui(b,16,96);
  mpz_sub_ui(b,b,1);

  mpz_urandomm(f,state,b);
  mpz_add(f,f,a);

  message = mpz_get_str(NULL, 16, f);

  mpz_clears(a,b,f,NULL);
}

/**
 * Reads a line from standard input, pads the input using PKCS#1 v1.5 padding scheme, 
 * and converts the padded input to an mpz_t integer.
 */
void get_user_input(mpz_t *m){
    fgets(user_input, sizeof(user_input), stdin);
    prepareInput(pkcs_padded_input, user_input);
    mpz_set_str(*m, pkcs_padded_input, 16);
}
void set_current_c(int i, mpz_t *c, mpz_t *m, gmp_randstate_ptr state) {
  PMS(state);
  int written = snprintf(user_input, sizeof(user_input), "%s\n", message);
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
    gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_init(m); mpz_init(c);
    int iterations = 1000;
    //get_user_input(&m);
    initSetup();

    // encrypt(&c, &m, &rsa);


    // optimizedWithoutTrimmers(&c);

    // trimmersOnly(&c);

    // baseAttack(&c);


    int allCalls[iterations];
    int alls2Calls[iterations];
    double allrTimes[iterations];
    int calls, s2aCalls;
    double rTime;
    double totalCalls = 0, totals2aCalls = 0, totalrTime = 0;
    for (size_t i = 0; i < iterations; i++)
    {
      printf("Iteration no. %zu \n", i + 1);
      calls = s2aCalls = rTime = 0;
      setup(state);
      set_current_c(i,&c,&m, state);
      fullyOptimizedAttack(&c,&calls, &s2aCalls, &rTime );
      printf("Original Message: %s\n", user_input_copy);
      allCalls[i] = calls;
      alls2Calls[i] = s2aCalls;
      allrTimes[i] = rTime;
      totalCalls += calls;
      totals2aCalls += s2aCalls;
      totalrTime += rTime;
    }

    


    printf("Total Oracle Calls: Mean: %f Median: %f\n", totalCalls / iterations, calculateMedianInt(allCalls, iterations));
    printf("Total step2a Calls: Mean: %f Median: %f\n", totals2aCalls / iterations, calculateMedianInt(alls2Calls, iterations));
    printf("Average Time atk  : Mean: %f Median: %f\n", totalrTime / iterations, calculateMedianDouble(allrTimes, iterations));

    mpz_clear(c);
    mpz_clear(m);
    return 0;
}




