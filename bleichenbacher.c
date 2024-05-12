#include "bleichenbacher.h"

char P[] = "13085544939016433034116036005625821953227372297195833776806214472190928979740509797654163083677193114899060771182653327749002407615798680323156649288891299";
char Q[] = "10515460639626849709984902074446790599067546798868400707778516190055079520038452945216601553837352784786191461977224566643334966876410080204134569391652071";

int E = 65537;
int TOTAL_REQUESTS = 0;

char oracleString[RSA_BLOCK_BYTE_SIZE * 2];

mpz_t d, n, e, p, q, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime, aa, bb;


void encrypt() {
    mpz_powm(c, m, e, n);
}

void decrypt(mpz_t output, mpz_t input) {
    mpz_powm(output, input, d, n);
}

void setup() {
    // initialize p, q, and e
    mpz_init_set_str(p, P, 10);
    mpz_init_set_str(q, Q, 10);
    mpz_init_set_ui(e, E);

    // get n from p * q
    mpz_init(n);
    mpz_mul(n, p, q);
    
    // assert that n is of correct length
    size_t sizeOfN = mpz_sizeinbase(n,2);
    printf("Bytesize of Modulus: %zu\n", sizeOfN / 8);
    if (sizeOfN / 8 != RSA_BLOCK_BYTE_SIZE) {
        printf("Incorrect length of Modulus\n");
        exit(1);
    }  
    
    // get phi from p-1 * q-1
    mpz_t phi, p1,q1;
    mpz_init(p1);
    mpz_init(q1);
    mpz_init(phi);

    mpz_sub_ui(p1, p, 1);
    mpz_sub_ui(q1, q, 1);
    mpz_mul(phi, p1, q1);

    // get d from e^-1 mod phi
    mpz_init(d);
    mpz_invert(d, e, phi);

    // assert that both d and e are smaller than n
    if (mpz_cmp(d,n) >= 0 || mpz_cmp(e,n) >= 0) {
        printf("Incorrect RSA keys");
        exit(1);
    }

    // assert that e and d work as expected
    mpz_t test;
    mpz_init_set_ui(test, 46);
    mpz_powm(test,test,e,n);
    mpz_powm(test,test,d,n);
    if(!(mpz_cmp_ui(test,46) == 0)) {
        printf("Error with d and e\n");
        exit(1);
    }

    
    mpz_init(B);  mpz_setbit(B, 8 * (RSA_BLOCK_BYTE_SIZE - 2));
    mpz_init(B2); mpz_mul_ui(B2, B, 2);
    mpz_init(B3); mpz_mul_ui(B3, B, 3);

    // assert that B is of correct length
    size_t sizeOfB = mpz_sizeinbase(B,2) / 8;
    size_t sizeOfB2 = mpz_sizeinbase(B2,2) / 8;
    size_t sizeOfB3 = mpz_sizeinbase(B3,2) / 8;

    if (sizeOfB != RSA_BLOCK_BYTE_SIZE - 2 || sizeOfB2 != RSA_BLOCK_BYTE_SIZE - 2 || sizeOfB3 != RSA_BLOCK_BYTE_SIZE - 2) {
        printf("Something wrong with the B values");
    }  

    mpz_init_set_ui(s,1);
    mpz_cdiv_q(s, n, B3);
    gmp_printf("Minimal possible value for s: %Zd (n / B3)\n", s);

    // initialize globally used number structs
    mpz_init(m);
    mpz_init(c);
    mpz_init(a);
    mpz_init(b);
    mpz_init(r);
    mpz_init(r1);
    mpz_init(r2);
    mpz_init(aa);
    mpz_init(bb);
    mpz_init(c_prime);
    mpz_init(oracle_decrypted_input);
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


void mpz_to_hex_array(char* hex_string, mpz_t number) {
    mpz_get_str(hex_string, 16, number);
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


int oracle(mpz_t number) {
    mpz_to_hex_array(oracleString, number);
    if(oracleString[0] != '0' || oracleString[1] != '0') return 0;
    if(oracleString[2] != '0' || oracleString[3] != '2') return 0;
    return 1;
}

void findNextS() {
    mpz_add_ui(s,s,1);

    mpz_powm(c_prime, s, e, n);
    mpz_mul(c_prime, c_prime, c);
    mpz_mod(c_prime, c_prime, n);

    if(oracle(c_prime)) return;
    findNextS();
}

void searchingWithOneIntervalLeft(IntervalSet *set) {
    mpz_set(a, set->intervals[0].lower);
    mpz_set(b, set->intervals[0].upper);




    mpz_mul(r, b, s);
    mpz_sub(r, r, B2);
    mpz_mul_ui(r,r,2);
    mpz_fdiv_q(r, r, n);

    while(1) {
        // left bound
        // low_bound = ceil((B2 + r * N), b)
        mpz_mul(r1, r, n);
        mpz_add(r1, r1, B2);
        mpz_cdiv_q(r1, r1, b);

        // right bound
        // high_bound = ceil((B3 - 1 + r * N),a) + 1
        mpz_mul(r2, r, n);
        mpz_add(r2, r2, B3);
        mpz_sub_ui(r2,r2,1);
        mpz_cdiv_q(r2, r2, a);
        mpz_add_ui(r2,r2,1);

        for(mpz_set(s, r1); mpz_cmp(s,r2) <= 0; mpz_add_ui(s,s,1)) {
            mpz_powm(c_prime, s, e, n);
            mpz_mul(c_prime, c_prime, c);
            mpz_mod(c_prime, c_prime, n);
            if(oracle(c_prime)) {
                return;
            }
            TOTAL_REQUESTS++;
        }
        mpz_add_ui(r,r,1);
    }
}

void findNewIntervals(IntervalSet *priorSet) {
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
        mpz_cdiv_q(r1, r1, n);

        // r2 = (bs - 2B) / n
        // r2 = floor((b * s - B2), N) + 1
        mpz_mul(r2, b, s);
        mpz_sub(r2, r2, B2);
        mpz_fdiv_q(r2, r2, n);
        mpz_add_ui(r2 ,r2, 1);
        
        for (mpz_set(r, r1); mpz_cmp(r, r2) <= 0; mpz_add_ui(r, r, 1)) {
            // aa = ceil(B2 + r*N, s)
            mpz_mul(aa, r, n);
            mpz_add(aa, aa, B2);
            mpz_cdiv_q(aa, aa, s);

            // bb = floor(B3 - 1 + r*N, s)
            mpz_mul(bb, r, n);
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




