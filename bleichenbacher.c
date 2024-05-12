#include "bleichenbacher.h"


int TOTAL_REQUESTS = 0;

char oracleString[RSA_BLOCK_BYTE_SIZE * 2];

mpz_t B, B2, B3, s;
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

    mpz_init_set_ui(s,1);
    mpz_cdiv_q(s, rsa.N, B3);
    gmp_printf("Minimal possible value for s: %Zd (n / B3)\n", s);
}

int inputToPaddedMessage(char *pkcs_padded_input, char *user_input) {
    sprintf(&pkcs_padded_input[0],"%02x", 0);
    sprintf(&pkcs_padded_input[2],"%02x", 2);

    int i;
    for (i = 2; i < RSA_BLOCK_BYTE_SIZE - strlen(user_input) - 1; i++) {
        sprintf(&pkcs_padded_input[i * 2], "%02x", 1);
    }
    sprintf(&pkcs_padded_input[i++ * 2], "%02x", 0);

    for (int t = 0;t < strlen(user_input); i++, t++) {
        sprintf(&pkcs_padded_input[i * 2], "%02x", (unsigned char)user_input[t]);
    }
    return 0;
}


void mpz_to_hex_array(char* hex_string, mpz_t *number) {
    mpz_get_str(hex_string, 16, *number);
    int len = strlen(hex_string);
    if (len < RSA_BLOCK_BYTE_SIZE * 2) {
        int dif = RSA_BLOCK_BYTE_SIZE * 2 - len;
        for (int i = len; i >= 0; i--){
            hex_string[i+dif] = hex_string[i];
        }
        for (int i = 0; i < dif;  i++) {
            hex_string[i] = '0';
        }
    }
}


int oracle(mpz_t *number) {
    mpz_to_hex_array(oracleString, number);
    if(oracleString[0] != '0' || oracleString[1] != '0') return 0;
    if(oracleString[2] != '0' || oracleString[3] != '2') return 0;
    return 1;
}

void findNextS(mpz_t *c) {
    static mpz_t c_prime;

    mpz_add_ui(s,s,1);

    mpz_powm(c_prime, s, rsa.E, rsa.N);
    mpz_mul(c_prime, c_prime, *c);
    mpz_mod(c_prime, c_prime, rsa.N);

    if(oracle(&c_prime)) return;
    findNextS(c);
}

void searchingWithOneIntervalLeft(IntervalSet *set, mpz_t *c) {
    static mpz_t a;
    static mpz_t b;
    static mpz_t r;
    static mpz_t r1;
    static mpz_t r2;
    static mpz_t c_prime;

    mpz_set(a, set->intervals[0].lower);
    mpz_set(b, set->intervals[0].upper);

    mpz_mul(r, b, s);
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

        for(mpz_set(s, r1); mpz_cmp(s,r2) <= 0; mpz_add_ui(s,s,1)) {
            mpz_powm(c_prime, s, rsa.E, rsa.N);
            mpz_mul(c_prime, c_prime, *c);
            mpz_mod(c_prime, c_prime, rsa.N);
            if(oracle(&c_prime)) {
                return;
            }
            TOTAL_REQUESTS++;
        }
        mpz_add_ui(r,r,1);
    }
}

void findNewIntervals(IntervalSet *priorSet) {
    static mpz_t a;
    static mpz_t b;
    static mpz_t r;
    static mpz_t r1;
    static mpz_t r2;
    static mpz_t aa;
    static mpz_t bb;
    
    Interval interval;
    IntervalSet set;
    init_interval_set(&set);
    int i = 0;

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
        mpz_add_ui(r2 ,r2, 1);
        
        for (mpz_set(r, r1); mpz_cmp(r, r2) <= 0; mpz_add_ui(r, r, 1)) {
            // aa = ceil(B2 + r*N, s)
            mpz_mul(aa, r, rsa.N);
            mpz_add(aa, aa, B2);
            mpz_cdiv_q(aa, aa, s);

            // bb = floor(B3 - 1 + r*N, s)
            mpz_mul(bb, r, rsa.N);
            mpz_add(bb, bb, B3);
            mpz_sub_ui(bb, bb, 1);
            mpz_fdiv_q(bb, bb, s);


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
}




