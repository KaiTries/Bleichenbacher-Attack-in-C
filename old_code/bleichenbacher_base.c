#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <time.h>

#define RSA_BLOCK_BYTE_SIZE 128
#define RSA_PUBLIC_KEY_SIZE 1024
#define RSA_PUBLIC_EXPONENT 65537


// Interval data structure
typedef struct {
    mpz_t lower;
    mpz_t upper;
} Interval;

// Interval set data structure
typedef struct {
    Interval *intervals;
    size_t size;
    size_t capacity;
} IntervalSet;

clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;

char D[] = "24357291003289494007393344103445516671928248099168871872692522079509677349727146191792416925622300072048103420599784521644782500410631433363101935846288701319285120334029723261348076960664796636943978140440609308656808142410028538634692033148933528774544858453409874201026962088104032733734200512141149586253";
char N[] = "137600532754295626994443374925222724431356055139662990769817241576142205367560380999181073015645951195743173336423418515217145998570084669280028581118715877265516304557540650607184862226831048734484434861756785443401572963112718409813687770038980819028043154648071701858129200920089257597152760630085547230229";
int E = 65537;
int64_t TOTAL_REQUESTS = 0;

mpz_t d, n, e, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime;


/*** function declarations ***/ 
void charArrayToMpz(char *input, mpz_t *output);
void mpzToCharArray(mpz_t *input, char *output);
int add_pkcs_padding(char *input, char *output);
void print_hex(char *input, int length);
int oracle(mpz_t input);
void encrypt(mpz_t *output);
void decrypt(mpz_t input, mpz_t *output);
void findNextS();
void findNewIntervals(IntervalSet *priorSet);
void searchingWithOneIntervalLeft(IntervalSet *set);

void init_interval(Interval *interval, mpz_t a, mpz_t b) {
    mpz_init_set(interval->lower, a);
    mpz_init_set(interval->upper, b);
}

void init_interval_set(IntervalSet *set) {
    set->size = 0;
    set->capacity = 10; 
    set->intervals = malloc(set->capacity * sizeof(Interval));
}

void ensure_capacity(IntervalSet *set) {
    if (set->size >= set->capacity) {
        size_t new_capacity = set->capacity * 2;
        Interval *new_intervals = realloc(set->intervals, new_capacity * sizeof(Interval));
        if (new_intervals == NULL) {
            // Handle realloc failure; for now, we just exit.
            exit(EXIT_FAILURE);
        }
        set->intervals = new_intervals;
        set->capacity = new_capacity;
    }
}

void mpz_set_min(mpz_t rop, const mpz_t op1, const mpz_t op2) {
    if (mpz_cmp(op1, op2) < 0) {
        mpz_set(rop, op1);
    } else {
        mpz_set(rop, op2);
    }
}

void mpz_set_max(mpz_t rop, const mpz_t op1, const mpz_t op2) {
    if (mpz_cmp(op1, op2) > 0) {
        mpz_set(rop, op1);
    } else {
        mpz_set(rop, op2);
    }
}

void add_interval(IntervalSet *set, Interval interval) {
    ensure_capacity(set);

    // First, find the position to insert the new interval
    int i = 0;
    while (i < set->size && mpz_cmp(set->intervals[i].lower, interval.lower) < 0) {
        i++;
    }

    // Check for overlaps and adjust intervals as necessary
    int start = i;
    while (i < set->size && mpz_cmp(set->intervals[i].lower, interval.upper) <= 0) {
        if (mpz_cmp(set->intervals[i].upper, interval.lower) >= 0) {
            // Merge intervals
            mpz_set_min(interval.lower, interval.lower, set->intervals[i].lower);
            mpz_set_max(interval.upper, interval.upper, set->intervals[i].upper);
        }
        i++;
    }
    int merged_count = i - start;

    // If we merged intervals, remove the old ones and insert the new merged interval
    if (merged_count > 0) {
        for (int j = start + 1; j < set->size - merged_count + 1; j++) {
            set->intervals[j] = set->intervals[j + merged_count - 1];
        }
        set->size -= (merged_count - 1);
    }

    // Insert the new or merged interval
    set->intervals[start] = interval;
    if (merged_count == 0) { // No merge happened, we need to shift elements to make space
        for (int j = set->size; j > start; j--) {
            set->intervals[j] = set->intervals[j - 1];
        }
        set->intervals[start] = interval;
        set->size++;
    }
}

void free_interval_set(IntervalSet *set) {
    for (size_t i = 0; i < set->size; i++) {
        mpz_clear(set->intervals[i].lower);
        mpz_clear(set->intervals[i].upper);
    }
    free(set->intervals);
}

void charArrayToMpz(char *input, mpz_t *output) {
    mpz_import(*output, RSA_BLOCK_BYTE_SIZE, -1, 1, 0, 0, input);
}

void mpzToCharArray(mpz_t *input, char *output) {
    mpz_export(output, NULL, -1, 1, 0, 0, *input);
}

int add_pkcs_padding(char *input, char *output) {
    // check that user input is not longer than RSA_BLOCK_BYTE_SIZE - 11
    if (strlen(input) > RSA_BLOCK_BYTE_SIZE - 11) {
        printf("User input is too long\n");
        return -1;
    }

    // add 0x00 byte at the end
    output[RSA_BLOCK_BYTE_SIZE - 1] = 0x00;
    output[RSA_BLOCK_BYTE_SIZE - 2] = 0x02;


    // add the input
    for (int i = 0; i < strlen(input); i++) {
        output[i] = input[i];
    }

    output[strlen(input)] = 0x00;

    // add padding
    for (int i = strlen(input) + 1; i < RSA_BLOCK_BYTE_SIZE - 2; i++) {
        output[i] = 0x01; // 1 + rand() % 255;
    }

    return 0;

}

void print_hex(char *input, int length) {
    for (int i = length - 1; i >= 0; i--) {
        printf("%02x ", input[i]);
    }
    printf("\n");
}

void print_string(char *input, int length) {
    int test = 0;
    for (int i = 0; i < length; i++) {
        printf("%c", input[i]);
        if (input[i] == 0){
            printf("\n");
            return;
        }
    }
}
 
int oracle(mpz_t input) {
    decrypt(input, &oracle_decrypted_input);
    char input_char[RSA_BLOCK_BYTE_SIZE];
    mpzToCharArray(&oracle_decrypted_input, input_char);

    // must start with 00 02
    if (input_char[RSA_BLOCK_BYTE_SIZE - 1] != 0x00 || input_char[RSA_BLOCK_BYTE_SIZE - 2] != 0x02) {
        return 0;
    }

    int i;
    // go to first index that is null
    for(i = RSA_BLOCK_BYTE_SIZE - 3; input_char[i] != 0x00; i--);
    // need atleast 8 non null padding
    if (i > RSA_BLOCK_BYTE_SIZE - 11) return 0;

    return 1;
}

// Encrypt function
void encrypt(mpz_t *output) {
    mpz_powm(*output, m, e, n);
}

// Decrypt function
void decrypt(mpz_t input, mpz_t *output) {
    mpz_powm(*output, input, d, n);
}

// Find s that satisfies the oracle
void findNextS() {
    mpz_powm(c_prime, s, e, n);
    mpz_mul(c_prime, c_prime, c);
    mpz_mod(c_prime, c_prime, n);
    if (oracle(c_prime) == 0) {
        mpz_add_ui(s, s, 1);
        TOTAL_REQUESTS++;
        findNextS();
    }
}

// Find new intervals
void findNewIntervals(IntervalSet *priorSet) {
    IntervalSet set;
    init_interval_set(&set);

    mpz_t aa, bb;
    mpz_init(aa);
    mpz_init(bb);

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
                Interval interval;
                init_interval(&interval, a, b);
                add_interval(&set, interval);

            }
        }


    }
    free_interval_set(priorSet);
    *priorSet = set;
    if (set.size > 1) printf("size of set: %zu", set.size);
}

void searchingWithOneIntervalLeft(IntervalSet *set) {

    mpz_set(a, set->intervals[0].lower);
    mpz_set(b, set->intervals[0].upper);

    mpz_mul(r, b, s);
    mpz_sub(r, r, B2);
    mpz_div(r, r, n);
    mpz_mul_ui(r,r,2);

    while(1) {
        // left bound
        mpz_mul(r1, r, n);
        mpz_add(r1, r1, B2);
        mpz_div(r1, r1, b);

        // right bound
        mpz_mul(r2, r, n);
        mpz_add(r2, r2, B3);
        mpz_div(r2, r2, a);



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

int main() {
    // setup data structures that hold large numbers
    mpz_init(d);
    mpz_init(n);
    mpz_init(e);
    mpz_init(B);
    mpz_init(B2);
    mpz_init(B3);
    mpz_init(s);
    mpz_init(m);
    mpz_init(c);
    mpz_init(a);
    mpz_init(b);
    mpz_init(r);
    mpz_init(r1);
    mpz_init(r2);
    mpz_init(c_prime);
    mpz_init(oracle_decrypted_input);


    mpz_set_ui(e, E);
    mpz_set_str(d,D, 10);
    mpz_set_str(n,N, 10);
    mpz_ui_pow_ui(B, 2, 8 * (RSA_BLOCK_BYTE_SIZE - 2));
    mpz_mul_ui(B2, B, 2);
    mpz_mul_ui(B3, B, 3);
    mpz_set_ui(s, 1);

    char user_input[100];
    char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE];
    char decrypted_input_char[RSA_BLOCK_BYTE_SIZE];

    // read user input (just one word)
    fgets(user_input, sizeof(user_input), stdin);

    // add pkcs padding
    add_pkcs_padding(user_input, pkcs_padded_input);
    // print pkcs padded input
    printf("The original padded message is: ");
    print_hex(pkcs_padded_input, RSA_BLOCK_BYTE_SIZE);

    charArrayToMpz(pkcs_padded_input, &m);
    // print m
    gmp_printf("Integer value of original m: %Zd\n", m);

    // encrypt input
    encrypt(&c);

    // print encrypted input
    gmp_printf("Encrypted input c: %Zd\n", c);

    // SETUP FOR THE ATTACK
    // Initial s value
    mpz_cdiv_q(s, n, B3);


    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t a, b;
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);
    Interval interval;
    init_interval(&interval, a, b);
    IntervalSet set;
    init_interval_set(&set);
    add_interval(&set, interval);

    int times2b = 0;
    int times2c = 0;

    int j = 0;
    printf("\n\n----------- Starting the Attack -----------\n\n");
    start_time = clock();
    start_time_2 = clock();
    printf("\n\n----------- Step 2a. -----------\n\n");
    findNextS();
    end_time_2 = clock();
    while(1) {
        if(set.size > 1) {
            printf("\n\n----------- Step 2b. -----------\n\n");
            mpz_add_ui(s, s, 1);
            findNextS();
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                mpzToCharArray(&set.intervals[0].lower,decrypted_input_char);
                gmp_printf("Decrypted message: %Zd\n", set.intervals[0].lower);
                printf("Decrypted message without padding: ");
                print_string(decrypted_input_char, RSA_BLOCK_BYTE_SIZE);
                break;
            }
            printf("\n\n----------- Step 2c. -----------\n\n");
            searchingWithOneIntervalLeft(&set);
            times2c++;
        }
        printf("\n\n----------- Step 3. -----------\n\n");
        findNewIntervals(&set);

        if (set.size == 0) {
            printf("No solution found\n");
            break;
        }
        j++;
    }


    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    cpu_time_used_2 = ((double) (end_time_2 - start_time_2)) / CLOCKS_PER_SEC;
    printf("Amount of iterations: %d\n", j);
    printf("Oracle queries made: %lld\n", TOTAL_REQUESTS);
    printf("Times step 2b called: %d\n", times2b);
    printf("Times step 2c called: %d\n", times2c);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);





    mpz_clear(d);
    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(B);
    mpz_clear(B2);
    mpz_clear(B3);
    mpz_clear(s);
    mpz_clear(m);
    mpz_clear(c);
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(r);
    mpz_clear(r1);
    mpz_clear(r2);
    mpz_clear(c_prime);
    mpz_clear(oracle_decrypted_input);
    free_interval_set(&set);
    return 0;
}

