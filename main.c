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

void get_user_input(){
    fgets(user_input, sizeof(user_input), stdin);
    inputToPaddedMessage(pkcs_padded_input, user_input);
    mpz_set_str(m, pkcs_padded_input, 16);

}

int main() {
    setup();
    get_user_input();
    print("this is the original input with padding displayed in hex format");
    print(pkcs_padded_input);
    print("===============================================================");

    
    encrypt();
    mpz_to_hex_array(encrypted_input, c);

    print("this is the original input encrypted with RSA displayed as hex");
    print(encrypted_input);
    print("===============================================================");

    // ListOfIntervals = [(2B, 3B - 1)]
    mpz_t a, b;
    mpz_init_set(a,B2);
    mpz_init_set(b,B3);
    mpz_sub_ui(b, b, 1);

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
                gmp_printf("Decrypted message: %Zd\n", set.intervals[0].lower);
                printf("Decrypted message without padding: ");
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

