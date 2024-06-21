#include "bleichenbacher.h"
#include "interval.h"
#include <time.h>
#include <stdio.h>
#include <math.h>

#define TRIMMER_LIMIT 500

mpz_t B, B2, B3;
clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;
RSA rsa;
char decrypted_input_char[RSA_BLOCK_BYTE_SIZE * 2];
char depadded_output_char[RSA_BLOCK_BYTE_SIZE];

void print(char *string) {
    printf("%s\n",string);
}

void setup() {
    generate(&rsa);
    mpz_init(B);  mpz_setbit(B, 8 * (RSA_BLOCK_BYTE_SIZE - 2));
    mpz_init(B2); mpz_mul_ui(B2, B, 2);
    mpz_init(B3); mpz_mul_ui(B3, B, 3);

    // assert that B is of correct length
    if (mpz_sizeinbase(B,2) / 8 != RSA_BLOCK_BYTE_SIZE - 2 
    || mpz_sizeinbase(B2,2) / 8 != RSA_BLOCK_BYTE_SIZE - 2 
    || mpz_sizeinbase(B3,2) / 8 != RSA_BLOCK_BYTE_SIZE - 2) {
        printf("Something wrong with the B values");
    }  
}

int gcd(int a, int b) {
    // https://stackoverflow.com/questions/19738919/gcd-function-for-c
    int temp;
    while (b != 0)
    {
        temp = a % b;

        a = b;
        b = temp;
    }
    return a;
}

int test1(int u, int t) {
    if (gcd(u,t) == 1) return 1;
    return 0;
}

int test2(int u_int, int t_int,mpz_t *c, RSA *rsa) {
    mpz_t u, t, t_inv, a, b, c_prime;
    mpz_init(u);
    mpz_init(t);
    mpz_init(t_inv);
    mpz_init(a);
    mpz_init(b);
    mpz_init(c_prime);

    mpz_set_ui(u, u_int);
    mpz_set_ui(t, t_int);

    mpz_invert(t_inv, t, rsa->N);
    mpz_powm(a, u, rsa->E, rsa->N);
    mpz_powm(b, t_inv, rsa->E, rsa->N);

    mpz_mul(c_prime, a, b);
    mpz_mul(c_prime, c_prime, *c);
    mpz_mod(c_prime, c_prime, rsa->N);
    if (oracle(&c_prime, rsa)) {
        mpz_clear(u);
        mpz_clear(t);
        mpz_clear(t_inv);
        mpz_clear(a);
        mpz_clear(b);
        mpz_clear(c_prime);
        return 1;
    }
    mpz_clear(u);
    mpz_clear(t);
    mpz_clear(t_inv);
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(c_prime);
    return 0;
}

int lcm(int *a, int length) {
    int lcm = a[0];
    for(int i = 1; i < length; i++) {
        lcm = (lcm * a[i]) / gcd(lcm, a[i]);
    }
    return lcm;
}

int in_range(int u, int t) {
    double lower_bound = 2 / 3.0;
    double upper_bound = 3 / 2.0;
    double num = u / (double) t;
    if (lower_bound < num  && num < upper_bound) return 1;
    return 0;
}

void trimming(mpz_t *t_prime, mpz_t *ul, mpz_t *uh, mpz_t *c, RSA *rsa) {
    int counter = 0;
    int ts[TRIMMER_LIMIT];
    ts[0] = 1;
    int idx = 1;
    
    int t = 1, u;

    for (t = 3; t < 10000; t++) {
        if (counter >= TRIMMER_LIMIT) break;
        for (u = t-1; u < t+2; u++) {
            if (counter >= TRIMMER_LIMIT || !in_range(u, t)) break;
            if (test1(u, t) == 1) {
                counter++;
                if (test2(u, t, c, rsa) == 1) {
                    ts[idx++] = t;
                    break;
                }
            }
        }
    }

    if (idx == 1) {
        mpz_set_ui(*t_prime, 1);
        mpz_set_ui(*ul, 1);
        mpz_set_ui(*uh, 1);
        return;
    }

    int denom = lcm(ts,idx);

    // 2t / 3 < u < 3t / 2
    int min_u = floor((2 * denom) / 3.0);
    int max_u = ceil((3 * denom) / 2.0);

    // max bound for u_lower is u_a / t_a
    int max_u_lower = denom;

    // min bound for u_upper is u_b / t_b
    int min_u_upper = denom;

    int u_lower = 1;
    int u_upper = 1;

    // binary search for upper and lower u
    while(max_u_lower - min_u != 1) {
        u =  ceil((max_u_lower + min_u) / 2.0);
        counter++;
        if(test2(u, denom, c, rsa)) {
            max_u_lower = u;
        } else {
            min_u = u;
        }
    }
    u_lower = max_u_lower;

    while(min_u_upper + 1 !=  max_u) {
        u = floor((min_u_upper + max_u) / 2.0);
        counter++;
        if(test2(u, denom, c, rsa)) {
            min_u_upper = u;
        } else {
            max_u = u;
        }
    }
    u_upper = min_u_upper;

    mpz_set_ui(*t_prime, denom);
    mpz_set_ui(*ul, u_lower);
    mpz_set_ui(*uh, u_upper);
}

void findNextS_iteratively(mpz_t *c, mpz_t *s, mpz_t *a, mpz_t *b) {
    mpz_t c_prime;
    mpz_add_ui(*s,*s,1);
    mpz_init(c_prime);

    while(1) {
        mpz_powm(c_prime, *s, rsa.E, rsa.N);
        mpz_mul(c_prime, c_prime, *c);
        mpz_mod(c_prime, c_prime, rsa.N);
        if (oracle(&c_prime, &rsa)) {
            mpz_clear(c_prime);
            return;
        }
        mpz_add_ui(*s,*s,1);
    }
}

void findNextS_2a(mpz_t *c, mpz_t *s, mpz_t *a, mpz_t *b) {
    mpz_t r, c_prime, comparison;
    mpz_init(r);
    mpz_init(c_prime);
    mpz_init(comparison);

    mpz_add(*s, rsa.N, B2);
    mpz_cdiv_q(*s, *s, *b);

    int found = 0;
    while(1) {
        // r = ((s * a) - 3B) / n
        mpz_mul(r, *s, *a);
        mpz_sub(r, r, B3);
        mpz_fdiv_q(r, r, rsa.N);

        // compare = (2B + (r + 1) * n) / b
        mpz_add_ui(comparison, r, 1);
        mpz_mul(comparison, comparison, rsa.N);
        mpz_add(comparison, comparison, B2);
        mpz_cdiv_q(comparison, comparison, *b);

        if (mpz_cmp(*s,comparison) >= 0) {
            mpz_powm(c_prime, *s, rsa.E, rsa.N);
            mpz_mul(c_prime, c_prime, *c);
            mpz_mod(c_prime, c_prime, rsa.N);
            if (oracle(&c_prime, &rsa)) {
                mpz_clear(r);
                mpz_clear(comparison);
                mpz_clear(c_prime);
                return;
            }
            mpz_add_ui(*s,*s,1);
        } else {
            mpz_set(*s, comparison);
        }
    }
    mpz_clear(r);
    mpz_clear(comparison);
    mpz_clear(c_prime);
}

int searchingWithOneIntervalLeft(Interval *interval, mpz_t *c, mpz_t *s) {
    mpz_t a, b, r, r1, r2, c_prime;
    mpz_init(r1); mpz_init(r2);
    mpz_init(r); mpz_init(c_prime);
    

    mpz_init_set(a, interval[0].lower);
    mpz_init_set(b, interval[0].upper);

    mpz_mul(r, b, *s);
    mpz_sub(r, r, B2);
    mpz_mul_ui(r,r,2);
    mpz_cdiv_q(r, r, rsa.N);

    while(1) {
        // left bound
        // low_bound = ceil((B2 + r * N), b)
        mpz_mul(r1, r, rsa.N);
        mpz_add(r1, r1, B2);
        mpz_cdiv_q(r1, r1, b);

        // right bound
        // high_bound = ceil((B3 - 1 + r * N),a) + 1
        mpz_mul(r2, r, rsa.N);
        mpz_add(r2, r2, B3);
        mpz_sub_ui(r2,r2,1);
        mpz_cdiv_q(r2, r2, a);
        mpz_add_ui(r2,r2,1);

        for(mpz_set(*s, r1); mpz_cmp(*s,r2) <= 0; mpz_add_ui(*s,*s,1)) {
            mpz_powm(c_prime, *s, rsa.E, rsa.N);
            mpz_mul(c_prime, c_prime, *c);
            mpz_mod(c_prime, c_prime, rsa.N);
            if(oracle(&c_prime, &rsa)) {
                mpz_clear(a);
                mpz_clear(b);
                mpz_clear(r);
                mpz_clear(r1);
                mpz_clear(r2);
                mpz_clear(c_prime);
                return 1;
            }
        }
        mpz_add_ui(r,r,1);
    }    
    
    // free gmp structs
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(r);
    mpz_clear(r1);
    mpz_clear(r2);
    mpz_clear(c_prime);
    return 0;
}

void findNextS_multipleIntervals(IntervalSet *set, mpz_t *c, mpz_t *s) {
    for (size_t j = 0; j < set->size; j++) {
        int result = searchingWithOneIntervalLeft(&set->intervals[j], c, s);
        if(result) break;
    }
}

void findNewIntervals(IntervalSet *priorSet, mpz_t *s) {
    mpz_t a, b, r, r1, r2, aa, bb;
    mpz_init(a); mpz_init(b);
    mpz_init(r); mpz_init(r1);
    mpz_init(r2); mpz_init(aa);
    mpz_init(bb); 
    
    Interval interval;
    IntervalSet set;
    init_interval_set(&set);

    // loop through all intervals in priorSet
    for (size_t j = 0; j < priorSet->size; j++) {
        mpz_set(a, priorSet->intervals[j].lower); 
        mpz_set(b, priorSet->intervals[j].upper); 

        // r1 = (as - 3B + 1) / n
        // r1 = ceil((a * s - B3 + 1), N)
        mpz_mul(r1, a, *s);
        mpz_sub(r1, r1, B3);
        mpz_add_ui(r1, r1, 1);
        mpz_cdiv_q(r1, r1, rsa.N);

        // r2 = (bs - 2B) / n
        // r2 = floor((b * s - B2), N) + 1
        mpz_mul(r2, b, *s);
        mpz_sub(r2, r2, B2);
        mpz_fdiv_q(r2, r2, rsa.N);
        mpz_add_ui(r2 ,r2, 1);
        
        for (mpz_set(r, r1); mpz_cmp(r, r2) <= 0; mpz_add_ui(r, r, 1)) {
            // aa = ceil(B2 + r*N, s)
            mpz_mul(aa, r, rsa.N);
            mpz_add(aa, aa, B2);
            mpz_cdiv_q(aa, aa, *s);

            // bb = floor(B3 - 1 + r*N, s)
            mpz_mul(bb, r, rsa.N);
            mpz_add(bb, bb, B3);
            mpz_sub_ui(bb, bb, 1);
            mpz_fdiv_q(bb, bb, *s);


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
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(r);
    mpz_clear(r1);
    mpz_clear(r2);
    mpz_clear(aa);
    mpz_clear(bb);
}

void baseAttack(mpz_t *c) {
    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t s, a, b;
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);
    mpz_init_set_ui(s,1);

    mpz_div(s,rsa.N,B3);

    Interval interval;
    set_interval(&interval,a,b);
    IntervalSet set;
    init_interval_set(&set);
    add_interval(&set, &interval);

    int times2b = 0;
    int times2c = 0;
    int j = 0;
    printf("\n-------- BASE BLEICHENBACHER ATTACK --------\n");
    printf("----------- Starting the Attack ------------\n");
    start_time = clock();
    start_time_2 = clock();
    findNextS_iteratively(c, &s, &a, &b);
    end_time_2 = clock();
    findNewIntervals(&set, &s);
    while(1) {
        if(set.size > 1) {
            findNextS_iteratively(c, &s, &a, &b);
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                mpz_to_hex_array(decrypted_input_char, &set.intervals[0].lower);
                prepareOutput(depadded_output_char, decrypted_input_char);
                printf("Decrypted message: ");
                printf("%s",depadded_output_char);
                print("===============================================================");
                break;
            }
            searchingWithOneIntervalLeft(&set.intervals[0], c, &s);
            times2c++;
        }
        findNewIntervals(&set, &s);

        if (set.size == 0) {
            printf("No solution found\n");
            break;
        }
        j++;
    }


    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    cpu_time_used_2 = ((double) (end_time_2 - start_time_2)) / CLOCKS_PER_SEC;
    printf("Number of oracle calls: %d\n", oracleCalls);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(s);
    free_interval_set(&set);
}

void trimmersOnly(mpz_t *c) {
    // ListOfIntervals = [(2B, 3B - 1)]
    oracleCalls = 0;
    mpz_t s, a, b, t, ul, uh;
    mpz_init(t);
    mpz_init(ul);
    mpz_init(uh);
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);
    mpz_init_set_ui(s,1);

    trimming(&t,&ul,&uh,c, &rsa);

    mpz_mul(a, a, t);
    mpz_div(a, a, ul);

    mpz_mul(b, b, t);
    mpz_div(b, b, uh);

    mpz_add(s, rsa.N, B2);
    mpz_cdiv_q(s, s, b);

    Interval interval;
    set_interval(&interval,a,b);
    IntervalSet set;
    init_interval_set(&set);
    add_interval(&set, &interval);

    int times2b = 0;
    int times2c = 0;
    int j = 0;
    printf("\n------- BLEICHENBACHER WITH TRIMMERS -------\n");
    printf("----------- Starting the Attack ------------\n");
    start_time = clock();
    start_time_2 = clock();
    findNextS_iteratively(c, &s, &a, &b);
    end_time_2 = clock();
    findNewIntervals(&set, &s);
    while(1) {
        if(set.size > 1) {
            findNextS_iteratively(c, &s, &a, &b);
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                mpz_to_hex_array(decrypted_input_char, &set.intervals[0].lower);
                prepareOutput(depadded_output_char, decrypted_input_char);
                printf("Decrypted message: ");
                printf("%s",depadded_output_char);
                print("===============================================================");
                break;
            }
            searchingWithOneIntervalLeft(&set.intervals[0], c, &s);
            times2c++;
        }
        findNewIntervals(&set, &s);

        if (set.size == 0) {
            printf("No solution found\n");
            break;
        }
        j++;
    }


    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    cpu_time_used_2 = ((double) (end_time_2 - start_time_2)) / CLOCKS_PER_SEC;
    printf("Number of oracle calls: %d\n", oracleCalls);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(s);
    mpz_clear(t);
    mpz_clear(uh);
    mpz_clear(ul);
    free_interval_set(&set);
}

void optimizedWithoutTrimmers(mpz_t *c) {
    oracleCalls = 0;

    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t s, a, b;
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);
    mpz_init_set_ui(s,1);

    mpz_div(s,rsa.N,B3);

    Interval interval;
    set_interval(&interval,a,b);
    IntervalSet set;
    init_interval_set(&set);
    add_interval(&set, &interval);

    int times2b = 0;
    int times2c = 0;
    int j = 0;
    printf("\n-------- OPTIMIZED WITHOUT TRIMMERS --------\n");
    printf("----------- Starting the Attack ------------\n");
    start_time = clock();
    start_time_2 = clock();
    findNextS_2a(c, &s, &a, &b);
    end_time_2 = clock();
    findNewIntervals(&set, &s);
    while(1) {
        if(set.size > 1) {
            findNextS_multipleIntervals(&set, c, &s);
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                mpz_to_hex_array(decrypted_input_char, &set.intervals[0].lower);
                prepareOutput(depadded_output_char, decrypted_input_char);
                printf("Decrypted message: ");
                printf("%s",depadded_output_char);
                print("===============================================================");
                break;
            }
            searchingWithOneIntervalLeft(&set.intervals[0], c, &s);
            times2c++;
        }
        findNewIntervals(&set, &s);

        if (set.size == 0) {
            printf("No solution found\n");
            break;
        }
        j++;
    }


    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    cpu_time_used_2 = ((double) (end_time_2 - start_time_2)) / CLOCKS_PER_SEC;
    printf("Number of oracle calls: %d\n", oracleCalls);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(s);
    free_interval_set(&set);
}

void fullyOptimizedAttack(mpz_t *c, int *calls, int *s2aCalls, double *rTime) {
    oracleCalls = 0;
    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t s, a, b, t, ul, uh;
    mpz_init(t);
    mpz_init(ul);
    mpz_init(uh);
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);
    mpz_init_set_ui(s,1);

    trimming(&t,&ul,&uh,c, &rsa);

    mpz_mul(a, a, t);
    mpz_div(a, a, ul);

    mpz_mul(b, b, t);
    mpz_div(b, b, uh);

    Interval interval;
    set_interval(&interval,a,b);
    IntervalSet set;
    init_interval_set(&set);
    add_interval(&set, &interval);

    int times2b = 0;
    int times2c = 0;
    int j = 0;
    printf("\n---- OPTIMIZED BLEICHENBACHER ALGORITHM ----\n");
    printf("----------- Starting the Attack ------------\n");
    start_time = clock();
    start_time_2 = clock();
    findNextS_2a(c, &s, &a, &b);
    end_time_2 = clock();
    findNewIntervals(&set, &s);
    while(1) {
        if(set.size > 1) {
            findNextS_multipleIntervals(&set, c, &s);
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                mpz_to_hex_array(decrypted_input_char, &set.intervals[0].lower);
                prepareOutput(depadded_output_char, decrypted_input_char);
                printf("Decrypted message: ");
                printf("%s",depadded_output_char);
                print("===============================================================");
                break;
            }
            searchingWithOneIntervalLeft(&set.intervals[0], c, &s);
            times2c++;
        }
        findNewIntervals(&set, &s);

        if (set.size == 0) {
            printf("No solution found\n");
            break;
        }
        j++;
    }


    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    cpu_time_used_2 = ((double) (end_time_2 - start_time_2)) / CLOCKS_PER_SEC;
    printf("Number of oracle calls: %d\n", oracleCalls);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);

    *calls += oracleCalls;
    *s2aCalls += oracleCalls;
    *rTime += cpu_time_used;

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(s);
    mpz_clear(t);
    mpz_clear(uh);
    mpz_clear(ul);
    free_interval_set(&set);
}


