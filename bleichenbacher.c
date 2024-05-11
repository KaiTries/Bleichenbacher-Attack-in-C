#include "bleichenbacher.h"


char D[] = "19727681193088754998276840795242220172847954323582026599409554750710680516732825740654119125160405694628074721074488313095978066471740218523267221333124930912045508998326372609804295096626247353229652064503029230942667254095587233294723190967327901760323309838110506410300878964667549764418213042321270124233";
char N[] = "107249526532680027898968835769206916919779044587689363520987473222507330487359535509352883044847574285262557693493051893436011160709949290863489330942265354587959300152933510320421511548593135293237968069030863205339705974034914072788956288197084503166100466275582392396991364327049906650762893346005191867791";
int E = 65537;
int TOTAL_REQUESTS = 0;

char oracleString[RSA_BLOCK_BYTE_SIZE * 2];

mpz_t d, n, e, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime, aa, bb;


void encrypt() {
    mpz_powm(c, m, e, n);
}

void decrypt(mpz_t *input, mpz_t *output) {
    mpz_powm(*output, *input, d, n);
}

void setup() {
    mpz_init_set_str(n, N, 10);
    mpz_init_set_str(d, D, 10);
    mpz_init_set_ui(e, E);
    

    mpz_init(B);  mpz_setbit(B, 8 * (RSA_BLOCK_BYTE_SIZE - 2));
    mpz_init(B2); mpz_mul_ui(B2, B, 2);
    mpz_init(B3); mpz_mul_ui(B3, B, 3);

    mpz_init_set_ui(s,1);
    mpz_cdiv_q(s, n, B3);

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
    mpz_fdiv_q(r, r, n);
    mpz_mul_ui(r,r,2);

    while(1) {
        // left bound
        mpz_mul(r1, r, n);
        mpz_add(r1, r1, B2);
        mpz_fdiv_q(r1, r1, b);

        // right bound
        mpz_mul(r2, r, n);
        mpz_add(r2, r2, B3);
        mpz_cdiv_q(r2, r2, a);



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
        mpz_mul(r1, a, s);
        mpz_sub(r1, r1, B3);
        mpz_add_ui(r1, r1, 1);
        mpz_cdiv_q(r1, r1, n);

        // r2 = (bs - 2B) / n
        mpz_mul(r2, b, s);
        mpz_sub(r2, r2, B2);
        mpz_cdiv_q(r2, r2, n);
        
        for (mpz_set(r, r1); mpz_cmp(r, r2) <= 0; mpz_add_ui(r, r, 1)) {
            mpz_mul(aa, r, n);
            mpz_add(aa, aa, B2);
            mpz_cdiv_q(aa, aa, s);

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




