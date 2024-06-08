#include "bleichenbacher.h"
#include "gmp.h"
#include <stdio.h>
#include <stdlib.h>

char user_input[RSA_BLOCK_BYTE_SIZE];
char user_input_hex[RSA_BLOCK_BYTE_SIZE * 2];
char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE * 2];
char encrypted_input[RSA_BLOCK_BYTE_SIZE * 2];

void runXTimes(int version, int x, mpz_ptr c, mpz_ptr m);

/**
 * Reads a line from standard input, pads the input using PKCS#1 v1.5 padding
 * scheme, and converts the padded input to an mpz_t integer.
 */
void get_user_input(mpz_ptr m) {
  fgets(user_input, sizeof(user_input), stdin);
  prepareInput(pkcs_padded_input, user_input);
  mpz_set_str(m, pkcs_padded_input, 16);
}

void set_current_c(int i, mpz_ptr c, mpz_ptr m) {
  snprintf(user_input, sizeof(user_input), "%s%d%s%d", "MessageNumber: ", i,
           " adding randomness: ", rand());
  prepareInput(pkcs_padded_input, user_input);
  mpz_set_str(m, pkcs_padded_input, 16);
  encrypt(c, m, &rsa);
}

/**
 * Runs 3 different bleichenbacher attacks on some message given by the user
 */
int main() {
  mpz_t m, c;
  mpz_init(m);
  mpz_init(c);
  setup();
  // get_user_input(&m);

  encrypt(c, m, &rsa);

  runXTimes(3, 100, c, m);

  // optimizedWithoutTrimmers(&c);

  // trimmersOnly(&c);

  // baseAttack(&c);

  mpz_clear(c);
  mpz_clear(m);
  return 0;
}

void runXTimes(int version, const int x, mpz_ptr c, mpz_ptr m) {
  int calls;
  double time;

  int *callsArr = malloc(x * sizeof(int));
  double *timeArr = malloc(x * sizeof(double));

  for (int i = 0; i < x; i++) {
    printf("iteration: %d\n", i);
    set_current_c(i, c, m);
    switch (version) {
    case 0:
      // baseAttack(c);
      break;
    case 1:
      // trimmersOnly(c);
      break;
    case 2:
      // optimizedWithoutTrimmers(c);
      break;
    case 3:
      fullyOptimizedAttack(c, &calls, &time);
      break;
    }
    callsArr[i] = calls;
    timeArr[i] = time;
  }

  int avgCalls = 0;
  int totCalls = 0;
  double avgTime = 0.0;
  double totTime = 0.0;

  for (int i = 0; i < x; i++) {
    totCalls += callsArr[i];
    totTime += timeArr[i];
  }
  avgCalls = totCalls / x;
  avgTime = totTime / x;

  printf("Average number of oracle calls: %d\n", avgCalls);
  printf("Average time spent decrypting: %f\n", avgTime);

  free(callsArr);
  free(timeArr);
}
