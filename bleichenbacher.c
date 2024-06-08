#include "bleichenbacher.h"
#include "gmp.h"
#include "interval.h"
#include "rsa.h"
#include <stdio.h>
#include <time.h>

#define TRIMMER_LIMIT 500

mpz_t B, B2, B3;
clock_t start_time, end_time, intervalStart, intervalEnd, searchingOneStart,
    searchingOneEnd, searchingMultipleStart, searchingMultipleEnd;
double cpu_time_used, cpu_time_used_2;
RSA rsa;
char decrypted_input_char[RSA_BLOCK_BYTE_SIZE * 2];
char depadded_output_char[RSA_BLOCK_BYTE_SIZE];

void print(char *string) { printf("%s\n", string); }

void setup() {
  generate(&rsa);
  mpz_inits(B, B2, B3, NULL);
  mpz_setbit(B, 8 * (RSA_BLOCK_BYTE_SIZE - 2));
  mpz_mul_ui(B2, B, 2);
  mpz_init(B3);
  mpz_mul_ui(B3, B, 3);

  // assert that B is of correct length
  if (mpz_sizeinbase(B, 2) / 8 != RSA_BLOCK_BYTE_SIZE - 2 ||
      mpz_sizeinbase(B2, 2) / 8 != RSA_BLOCK_BYTE_SIZE - 2 ||
      mpz_sizeinbase(B3, 2) / 8 != RSA_BLOCK_BYTE_SIZE - 2) {
    printf("Something wrong with the B values");
  }
}

int gcd(int a, int b) {
  // https://stackoverflow.com/questions/19738919/gcd-function-for-c
  int temp;
  while (b != 0) {
    temp = a % b;

    a = b;
    b = temp;
  }
  return a;
}

int test1(int u, int t) {
  if (gcd(u, t) == 1)
    return 1;
  return 0;
}

int test2(int u_int, int t_int, mpz_ptr c, RSA *rsa) {
  mpz_t u, t, t_inv, a, b, c_prime;
  mpz_inits(u, t, t_inv, a, b, c_prime, NULL);

  mpz_set_ui(u, u_int);
  mpz_set_ui(t, t_int);

  mpz_invert(t_inv, t, rsa->N);
  mpz_powm(a, u, rsa->E, rsa->N);
  mpz_powm(b, t_inv, rsa->E, rsa->N);

  mpz_mul(c_prime, a, b);
  mpz_mul(c_prime, c_prime, c);
  mpz_mod(c_prime, c_prime, rsa->N);
  if (oracle(c_prime, rsa, B2, B3)) {
    mpz_clears(u, t, t_inv, a, b, c_prime, NULL);
    return 1;
  }
  mpz_clears(u, t, t_inv, a, b, c_prime, NULL);
  return 0;
}

int lcm(double *a, int length) {
  double lcm = a[0];
  for (int i = 1; i < length; i++) {
    lcm = (lcm * a[i]) / gcd(lcm, a[i]);
  }
  return lcm;
}

int in_range(double u, double t) {
  double lower_bound = 2 / 3.0;
  double upper_bound = 3 / 2.0;
  double num = u / t;
  if (lower_bound < num < upper_bound)
    return 1;
  return 0;
}

void trimming(mpz_ptr t_prime, mpz_ptr ul, mpz_ptr uh, mpz_ptr c, RSA *rsa) {
  int counter = 0;
  double us[500] = {0};
  double ts[500] = {0};
  int idx = 0;

  double t, u;

  for (t = 3; t < 4097; t++) {
    if (counter >= TRIMMER_LIMIT)
      break;
    for (u = t - 1; u < t + 2; u++) {
      if (counter >= TRIMMER_LIMIT || !in_range(u, t))
        break;
      if (test1(u, t) == 1) {
        counter++;
        if (test2(u, t, c, rsa) == 1) {
          us[idx] = u;
          ts[idx++] = t;
          break;
        }
      }
    }
  }

  if (idx == 0) {
    return;
  }

  double denom = lcm(ts, idx);

  // 2t / 3 < u < 3t / 2
  double min_u = (2 * denom) / 3.0;
  double max_u = (3 * denom) / 2.0;

  // max bound for u_lower is u_a / t_a
  double max_u_lower = (us[0] / (double)ts[0]) * denom;

  // min bound for u_upper is u_b / t_b
  double min_u_upper = (us[idx - 1] / (double)ts[idx - 1]) * denom;

  double u_lower = 1;
  double u_upper = 1;

  printf("max_u_lower: %f - min_u: %f\n", max_u_lower, min_u);
  // binary search for upper and lower u
  while (max_u_lower - min_u > 1) {
    u = (max_u_lower + min_u) / 2;
    counter++;
    if (test2(u, denom, c, rsa)) {
      max_u_lower = u;
    } else {
      min_u = u;
    }
  }
  u_lower = max_u_lower;

  printf("min_u_upper: %f - max_u: %f\n", min_u_upper, max_u);

  while (min_u_upper + 1 < max_u) {
    u = (min_u_upper + max_u) / 2;
    printf("min_u_upper: %f - max_u: %f\n", min_u_upper, max_u);

    counter++;
    if (test2(u, denom, c, rsa)) {
      min_u_upper = u;
    } else {
      max_u = u;
    }
  }
  u_upper = min_u_upper;

  printf("t: %f - ul: %f - uh: %f\n",denom, u_lower, u_upper);

  mpz_set_ui(t_prime, denom);
  mpz_set_ui(ul, u_lower);
  mpz_set_ui(uh, u_upper);
}

void findNextS_iteratively(mpz_ptr c, mpz_ptr s, mpz_ptr a, mpz_ptr b) {
  mpz_t c_prime;
  mpz_add_ui(s, s, 1);
  mpz_init(c_prime);

  while (1) {
    mpz_powm(c_prime, s, rsa.E, rsa.N);
    mpz_mul(c_prime, c_prime, c);
    mpz_mod(c_prime, c_prime, rsa.N);
    if (oracle(c_prime, &rsa, B2, B3)) {
      mpz_clear(c_prime);
      return;
    }
    mpz_add_ui(s, s, 1);
  }
}

void findNextS_2a(mpz_srcptr c, mpz_ptr s, mpz_ptr a, mpz_ptr b) {
  mpz_t r, c_prime, comparison;
  mpz_init2(r, 1024);
  mpz_init2(comparison, 1024);
  mpz_init2(c_prime, 1024);

  mpz_add(s, rsa.N, B2);
  mpz_cdiv_q(s, s, b);

  while (1) {
    // r = ((s * a) - 3B) / n
    mpz_mul(r, s, a);
    mpz_sub(r, r, B3);
    mpz_fdiv_q(r, r, rsa.N);

    // compare = (2B + (r + 1) * n) / b
    mpz_add_ui(comparison, r, 1);
    mpz_mul(comparison, comparison, rsa.N);
    mpz_add(comparison, comparison, B2);
    mpz_cdiv_q(comparison, comparison, b);

    if (mpz_cmp(s, comparison) >= 0) {
      mpz_powm(c_prime, s, rsa.E, rsa.N);
      mpz_mul(c_prime, c_prime, c);
      mpz_mod(c_prime, c_prime, rsa.N);
      if (oracle(c_prime, &rsa, B2, B3)) {
        mpz_clears(r, c_prime, comparison, NULL);
        return;
      }
      mpz_add_ui(s, s, 1);
    } else {
      mpz_set(s, comparison);
    }
  }
  mpz_clears(r, c_prime, comparison, NULL);
}

int searchingWithOneIntervalLeft(Interval *interval, mpz_srcptr c, mpz_ptr s) {
  mpz_t a, b, r, r1, r2, c_prime;
  mpz_init2(r, 1024);
  mpz_inits(r1, r2, c_prime, NULL);

  mpz_init_set(a, interval[0].lower);
  mpz_init_set(b, interval[0].upper);

  mpz_mul(r, b, s);
  mpz_sub(r, r, B2);
  mpz_mul_ui(r, r, 2);
  mpz_cdiv_q(r, r, rsa.N);

  while (1) {
    // left bound
    // low_bound = ceil((B2 + r * N), b)
    mpz_mul(r1, r, rsa.N);
    mpz_add(r1, r1, B2);
    mpz_cdiv_q(r1, r1, b);

    // right bound
    // high_bound = ceil((B3 - 1 + r * N),a) + 1
    mpz_mul(r2, r, rsa.N);
    mpz_add(r2, r2, B3);
    mpz_sub_ui(r2, r2, 1);
    mpz_cdiv_q(r2, r2, a);
    mpz_add_ui(r2, r2, 1);

    for (mpz_set(s, r1); mpz_cmp(s, r2) <= 0; mpz_add_ui(s, s, 1)) {
      mpz_powm(c_prime, s, rsa.E, rsa.N);
      mpz_mul(c_prime, c_prime, c);
      mpz_mod(c_prime, c_prime, rsa.N);
      if (oracle(c_prime, &rsa, B2, B3)) {
        mpz_clears(a, b, r, r1, r2, c_prime, NULL);
        return 1;
      }
    }
    mpz_add_ui(r, r, 1);
  }

  // free gmp structs
  mpz_clears(a, b, r, r1, r2, c_prime, NULL);
  return 0;
}

void findNextS_multipleIntervals(IntervalSet *set, mpz_ptr c, mpz_ptr s) {
  for (size_t j = 0; j < set->size; j++) {
    int result = searchingWithOneIntervalLeft(&set->intervals[j], c, s);
    if (result)
      break;
  }
}

void findNewIntervals(IntervalSet *priorSet, mpz_ptr s) {
  mpz_t a, b, r, r1, r2, aa, bb;
  mpz_init2(r, 1024);
  mpz_inits(a, b, r1, r2, aa, bb, NULL);

  Interval interval;
  IntervalSet set;
  init_interval_set(&set);

  // loop through all intervals in priorSet
  for (size_t j = 0; j < priorSet->size; j++) {
    mpz_set(a, priorSet->intervals[j].lower);
    mpz_set(b, priorSet->intervals[j].upper);

    // r1 = (as - 3B + 1) / n
    // r1 = ceil((a * s - B3 + 1), N)
    mpz_mul(r1, a, s);
    mpz_sub(r1, r1, B3);
    mpz_add_ui(r1, r1, 1);
    mpz_cdiv_q(r1, r1, rsa.N);

    // r2 = (bs - 2B) / n
    // r2 = floor((b * s - B2), N) + 1
    mpz_mul(r2, b, s);
    mpz_sub(r2, r2, B2);
    mpz_fdiv_q(r2, r2, rsa.N);
    mpz_add_ui(r2, r2, 1);

    for (mpz_set(r, r1); mpz_cmp(r, r2) <= 0; mpz_add_ui(r, r, 1)) {
      // a = max(a, min((2B + r * n) / s, b))
      // b = min(b, max((B3 - 1 + r * n) / s, a))
      mpz_mul(aa, r, rsa.N);
      mpz_add(aa, aa, B2);
      mpz_cdiv_q(aa, aa, s);
      if (mpz_cmp(aa, b) > 0) {
        mpz_set(aa, b);
      }

      mpz_mul(bb, r, rsa.N);
      mpz_add(bb, bb, B3);
      mpz_sub_ui(bb, bb, 1);
      mpz_fdiv_q(bb, bb, s);
      if (mpz_cmp(bb, a) < 0) {
        mpz_set(bb, a);
      }

      if (mpz_cmp(aa, a) > 0) {
        mpz_set(a, aa);
      }
      if (mpz_cmp(bb, b) < 0) {
        mpz_set(b, bb);
      }

      if (mpz_cmp(a, b) <= 0) {
        set_interval(&interval, a, b);
        add_interval(&set, &interval);
      }
    }
  }
  free_interval_set(priorSet);
  *priorSet = set;

  // free gmp structs
  mpz_clears(a, b, r, r1, r2, aa, bb, NULL);
}

void baseAttack(mpz_ptr c) {
  // ListOfIntervals = [(2B, 3B - 1)]
  mpz_t s, a, b;
  mpz_init_set(a, B2);
  mpz_init_set(b, B3);
  mpz_sub_ui(b, b, 1);
  mpz_init2(s, 1024);
  mpz_set_ui(s, 1);

  mpz_div(s, rsa.N, B3);

  Interval interval;
  set_interval(&interval, a, b);
  IntervalSet set;
  init_interval_set(&set);
  add_interval(&set, &interval);

  int times2b = 0;
  int times2c = 0;
  int j = 0;
  printf("\n-------- BASE BLEICHENBACHER ATTACK --------\n");
  printf("----------- Starting the Attack ------------\n");
  start_time = clock();
  intervalStart = clock();
  findNextS_iteratively(c, s, a, b);
  intervalEnd = clock();
  findNewIntervals(&set, s);
  while (1) {
    if (set.size > 1) {
      findNextS_iteratively(c, s, a, b);
      times2b++;
    } else {
      if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
        /*
        mpz_to_hex_array(decrypted_input_char, set.intervals[0].lower);
        prepareOutput(depadded_output_char, decrypted_input_char);
        printf("Decrypted message: ");
        printf("%s", depadded_output_char);
        print(
            "===============================================================");
        */
        break;
      }
      searchingWithOneIntervalLeft(&set.intervals[0], c, s);
      times2c++;
    }
    findNewIntervals(&set, s);

    if (set.size == 0) {
      printf("No solution found\n");
      break;
    }
    j++;
  }

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  cpu_time_used_2 = ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;
  printf("Number of oracle calls: %d\n", oracleCalls);
  printf("Execution time: %f seconds\n", cpu_time_used);
  printf("Time for first s: %f seconds\n", cpu_time_used_2);

  mpz_clears(a, b, s);
  free_interval_set(&set);
}

void trimmersOnly(mpz_ptr c) {
  // ListOfIntervals = [(2B, 3B - 1)]
  oracleCalls = 0;
  mpz_t s, a, b, t, ul, uh;
  mpz_init_set_ui(t, 1);
  mpz_init_set_ui(ul, 1);
  mpz_init_set_ui(uh, 1);
  mpz_init_set(a, B2);
  mpz_init_set(b, B3);
  mpz_sub_ui(b, b, 1);
  mpz_init2(s, 1024);

  trimming(t, ul, uh, c, &rsa);

  mpz_mul(a, a, t);
  mpz_div(a, a, ul);

  mpz_mul(b, b, t);
  mpz_div(b, b, uh);

  mpz_add(s, rsa.N, B2);
  mpz_cdiv_q(s, s, b);

  Interval interval;
  set_interval(&interval, a, b);
  IntervalSet set;
  init_interval_set(&set);
  add_interval(&set, &interval);

  int times2b = 0;
  int times2c = 0;
  int j = 0;
  printf("\n------- BLEICHENBACHER WITH TRIMMERS -------\n");
  printf("----------- Starting the Attack ------------\n");
  start_time = clock();
  intervalStart = clock();
  findNextS_iteratively(c, s, a, b);
  intervalEnd = clock();
  findNewIntervals(&set, s);
  while (1) {
    if (set.size > 1) {
      findNextS_iteratively(c, s, a, b);
      times2b++;
    } else {
      if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
        break;
      }
      searchingWithOneIntervalLeft(&set.intervals[0], c, s);
      times2c++;
    }
    findNewIntervals(&set, s);

    if (set.size == 0) {
      printf("No solution found\n");
      break;
    }
    j++;
  }

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  cpu_time_used_2 = ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;
  printf("Number of oracle calls: %d\n", oracleCalls);
  printf("Execution time: %f seconds\n", cpu_time_used);
  printf("Time for first s: %f seconds\n", cpu_time_used_2);

  mpz_clears(a, b, s, t, uh, ul);
  free_interval_set(&set);
}

void optimizedWithoutTrimmers(mpz_ptr c) {
  oracleCalls = 0;

  mpz_t s, a, b;
  mpz_init_set(a, B2);
  mpz_init_set(b, B3);
  mpz_sub_ui(b, b, 1);
  mpz_init2(s, 1024);
  mpz_div(s, rsa.N, B3);

  Interval interval;
  set_interval(&interval, a, b);
  IntervalSet set;
  init_interval_set(&set);
  add_interval(&set, &interval);

  int times2b = 0;
  int times2c = 0;
  int j = 0;
  printf("\n-------- OPTIMIZED WITHOUT TRIMMERS --------\n");
  printf("----------- Starting the Attack ------------\n");
  start_time = clock();
  intervalStart = clock();
  findNextS_2a(c, s, a, b);
  intervalEnd = clock();
  findNewIntervals(&set, s);
  while (1) {
    if (set.size > 1) {
      findNextS_multipleIntervals(&set, c, s);
      times2b++;
    } else {
      if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
        break;
      }
      searchingWithOneIntervalLeft(&set.intervals[0], c, s);
      times2c++;
    }
    findNewIntervals(&set, s);

    if (set.size == 0) {
      printf("No solution found\n");
      break;
    }
    j++;
  }

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  cpu_time_used_2 = ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;
  printf("Number of oracle calls: %d\n", oracleCalls);
  printf("Execution time: %f seconds\n", cpu_time_used);
  printf("Time for first s: %f seconds\n", cpu_time_used_2);

  mpz_clears(a, b, s);
  free_interval_set(&set);
}

void fullyOptimizedAttack(mpz_ptr c, int *calls, double *time) {
  oracleCalls = 0;
  mpz_t s, a, b, t, ul, uh;
  mpz_init_set_ui(t, 1);
  mpz_init_set_ui(ul, 1);
  mpz_init_set_ui(uh, 1);
  mpz_init_set(a, B2);
  mpz_init_set(b, B3);
  mpz_sub_ui(b, b, 1);
  mpz_init2(s, 1024);
  mpz_set_ui(s, 1);

  trimming(t, ul, uh, c, &rsa);

  mpz_mul(a, a, t);
  mpz_div(a, a, ul);

  mpz_mul(b, b, t);
  mpz_div(b, b, uh);

  Interval interval;
  set_interval(&interval, a, b);
  IntervalSet set;
  init_interval_set(&set);
  add_interval(&set, &interval);

  int times2b = 0;
  int times2c = 0;
  double totalTimeInterval = 0.0;
  double totalTimeMultipleIntervals = 0.0;
  double totalTimeOneInterval = 0.0;
  int j = 0;
  start_time = clock();
  intervalStart = clock();
  findNextS_2a(c, s, a, b);
  intervalEnd = clock();
  double timeForFirstS =
      ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;

  intervalStart = clock();
  findNewIntervals(&set, s);
  intervalEnd = clock();

  totalTimeInterval += ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;

  int found = 0;
  while (found == 0) {
    switch (set.size) {
    case 0:
      printf("failed");
      break;
    case 1:
      if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
        found = 1;
        break;
      }
      searchingOneStart = clock();
      searchingWithOneIntervalLeft(&set.intervals[0], c, s);
      searchingOneEnd = clock();
      totalTimeOneInterval +=
          ((double)(searchingOneEnd - searchingOneStart)) / CLOCKS_PER_SEC;
      times2c++;
      break;
    default:
      searchingMultipleStart = clock();
      findNextS_multipleIntervals(&set, c, s);
      searchingMultipleEnd = clock();
      totalTimeMultipleIntervals +=
          ((double)(searchingMultipleEnd - searchingMultipleStart)) /
          CLOCKS_PER_SEC;
      times2b++;
      break;
    }
    if (found == 0) {
      intervalStart = clock();
      findNewIntervals(&set, s);
      intervalEnd = clock();

      totalTimeInterval +=
          ((double)(intervalEnd - intervalStart)) / CLOCKS_PER_SEC;
    }
  }

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  printf("Number of oracle calls: %d\n", oracleCalls);
  printf("Execution time: %f seconds\n", cpu_time_used);

  printf("Time taken to find first s: %f seconds\n", timeForFirstS);
  printf("Total time spent creating Intervals: %f seconds\n",
         totalTimeInterval);
  printf("Total time spent searching Multiple Intervals: %f seconds\n",
         totalTimeMultipleIntervals);
  printf("Total time spent searching one interval: %f seconds\n",
         totalTimeOneInterval);

  *calls = oracleCalls;
  *time = cpu_time_used;

  mpz_clears(a, b, s, t, uh, ul, NULL);
  free_interval_set(&set);
}
