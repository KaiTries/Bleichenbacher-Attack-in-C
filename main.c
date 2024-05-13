#include "interval.h"
#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>

clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;
char user_input[RSA_BLOCK_BYTE_SIZE];
char user_input_hex[RSA_BLOCK_BYTE_SIZE * 2];
char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE * 2];
char encrypted_input[RSA_BLOCK_BYTE_SIZE * 2];
char decrypted_input_char[RSA_BLOCK_BYTE_SIZE * 2];



void print(char *string) {
    printf("%s\n",string);
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

int main() {
    mpz_t m, c, s;
    mpz_init(m); mpz_init(c);
    mpz_init_set_ui(s,1);

    setup();
    get_user_input(&m);
    print("this is the original input with padding displayed in hex format");
    print(pkcs_padded_input);
    print("===============================================================");

    encrypt(&c, &m, &rsa);

    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t a, b, t, ul, uh;
    mpz_init(t);
    mpz_init(ul);
    mpz_init(uh);
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);

    trimming(&t,&ul,&uh,&c, &rsa);

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
    printf("\n\n----------- Starting the Attack -----------\n\n");
    start_time = clock();
    start_time_2 = clock();
    printf("\n\n----------- Step 2a. -----------\n\n");
    findNextS_2a(&c, &s, &a, &b);
    end_time_2 = clock();
    while(1) {
        if(set.size > 1) {
            printf("\n\n----------- Step 2b. -----------\n\n");
            findNextS_multipleIntervals(&set, &c, &s);
            times2b++;
        } else { 
            if (mpz_cmp(set.intervals[0].lower, set.intervals[0].upper) == 0) {
                if (mpz_cmp(m, set.intervals[0].lower) == 0) {
                        mpz_to_hex_array(decrypted_input_char, &set.intervals[0].lower);
                        printf("Decrypted message: ");
                        print(decrypted_input_char);
                        print("===============================================================");
                        break;
                }
                else {
                    printf("remaining interval is not the same as original message");
                    break;
                }
            }
            if (j % 100 == 0) printf("\n\n----------- Step 2c. -----------\n\n");
            searchingWithOneIntervalLeft(&set.intervals[0], &c, &s);
            times2c++;
        }
        if (j % 100 == 0) printf("\n\n----------- Step 3. -----------\n\n");
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
    printf("Amount of iterations: %d\n", j);
    printf("Number of oracle calls: %d\n", oracleCalls);
    printf("Times step 2b called: %d\n", times2b);
    printf("Times step 2c called: %d\n", times2c);
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Time for first s: %f seconds\n", cpu_time_used_2);

    mpz_clear(m);
    mpz_clear(c);
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(s);
    mpz_clear(t);
    mpz_clear(uh);
    mpz_clear(ul);
    return 0;
}

