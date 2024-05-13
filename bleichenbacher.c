#include "bleichenbacher.h"

#define TRIMMER_LIMIT 500

mpz_t B, B2, B3;
RSA rsa;

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

/*
https://stackoverflow.com/questions/19738919/gcd-function-for-c
*/
int gcd(int a, int b)
{
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
    if (lower_bound < num < upper_bound) return 1;
    return 0;
}

void trimming(mpz_t *t_prime, mpz_t *ul, mpz_t *uh, mpz_t *c, RSA *rsa) {
    int counter = 0;
    int us[500];
    int ts[500];
    int idx = 0;
    
    int t = 1, u;

    for (t = 3; t < 10000; t++) {
        if (counter >= TRIMMER_LIMIT) break;
        for (u = t-1; u < t+2; u++) {
            if (counter >= TRIMMER_LIMIT || !in_range(u, t)) break;
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
    printf("Conducted %d trimmings and found %d candidates.\n", counter, idx);

    int denom = (idx > 0) ? lcm(ts,idx) : 1;
    printf("lcm of ts: %d\n",denom);

    // 2t / 3 < u < 3t / 2
    double min_u = (2 * denom) / 3.0;
    double max_u = (3 * denom) / 2.0;

    // max bound for u_lower is u_a / t_a
    double max_u_lower = (us[0] / (double) ts[0]) * denom;

    // min bound for u_upper is u_b / t_b
    double min_u_upper = (us[idx - 1] / (double) ts[idx - 1]) * denom;

    int u_lower = 1;
    int u_upper = 1;

    for (int i = min_u + 1; i <= max_u_lower; i++) {
        if (test2(i, denom, c, rsa) && u_lower == 1) {
            u_lower = i;
            break;
        }
    }

    for (int i = max_u - 1; i >= min_u_upper ; i--) {
        if (test2(i, denom, c, rsa) && u_upper == 1) {
            u_upper = i;
            break;
        }
    }
    printf("lower u: %d, upper u: %d\n", u_lower, u_upper);

    mpz_set_ui(*t_prime, denom);
    mpz_set_ui(*ul, u_lower);
    mpz_set_ui(*uh, u_upper);
}


void findNextS_iterative(mpz_t *c, mpz_t *s) {
    mpz_t c_prime;
    mpz_init(c_prime);
    
    mpz_add_ui(*s,*s,1);

    while(1) {
        mpz_powm(c_prime, *s, rsa.E, rsa.N);
        mpz_mul(c_prime, c_prime, *c);
        mpz_mod(c_prime, c_prime, rsa.N);
        if (oracle(&c_prime, &rsa)) {
            return;
        }
        mpz_add_ui(*s,*s,1);
    }
    mpz_clear(c_prime);
}

void findNextS_multipleIntervals(IntervalSet *set, mpz_t *c, mpz_t *s) {
    for (size_t j = 0; j < set->size; j++) {
        int result = searchingWithOneIntervalLeft(&set->intervals[j], c, s);
        if(result) break;
    }
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
    mpz_fdiv_q(r, r, rsa.N);

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




