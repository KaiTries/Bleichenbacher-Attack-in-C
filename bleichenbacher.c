#include "bleichenbacher.h"

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




