#include "interval.h"
#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>


void print(char *string) {
    printf("%s\n",string);
}


void interval_tests() {
    // initialize variables
    mpz_t a, b, c, d;
    mpz_init_set_ui(a, 1);
    mpz_init_set_ui(b, 10);
    mpz_init_set_ui(c, 20);
    mpz_init_set_ui(d, 30);

    // set intervals with mpz_t structs
    Interval intervalA;
    set_interval(&intervalA,a,b);
    Interval intervalB;
    set_interval(&intervalB,c,d);
    // set intervals with ints
    Interval intervalC;
    set_interval_ui(&intervalC,19,22);
    Interval intervalD;
    set_interval_ui(&intervalD,5,11);

    // init set
    IntervalSet set;
    init_interval_set(&set);

    // add intervalA
    add_interval(&set,&intervalA);
    // change intervalA after -> should not affect the set
    set_interval_ui(&intervalA, 100,200);

    add_interval(&set,&intervalB);
    add_interval(&set,&intervalC);
    add_interval(&set,&intervalD);
    
    // print interval should be -> [[1, 11], [19, 30]]
    // -> Intervals get correctly merged and sorted
    printf("print interval should be -> [[1, 11], [19, 30]]\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");
    free_interval_set(&set);
    init_interval_set(&set);

    Interval testInterval;
    for (size_t i = 0; i < 5; i++)
    {
        set_interval_ui(&testInterval,i*10,i*10 + 2);
        add_interval(&set, &testInterval);
    }
    printf("print interval should be -> [(0,2),(10,12),(20,22),(30,32),(40,42)]\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");

    set_interval_ui(&testInterval, 25,27);
    add_interval(&set, &testInterval);
    printf("Adding interval (25,27) should be placed correctly in middle\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");
    
    
    set_interval_ui(&testInterval, 22,32);
    add_interval(&set, &testInterval);
    printf("adding interval (22,32) should correctly fuse the intervals together\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");

    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(c);
    mpz_clear(d);
}

void rsa_tests() {
    setup();
    mpz_t decrypted_input;
    mpz_init(decrypted_input);
    
    char pkcs_padded_input[300];
    char pkcs_decrypted_input[300];
    char message[] = "testMessage";

    inputToPaddedMessage(pkcs_padded_input, message);
    mpz_set_str(m, pkcs_padded_input, 16);
    encrypt(c,m,rsa);
    decrypt(decrypted_input,c,rsa);
    mpz_to_hex_array(pkcs_decrypted_input,decrypted_input);

    for(int i = 0; i < strlen(pkcs_decrypted_input); i++) {
        if (pkcs_padded_input[i] != pkcs_decrypted_input[i]) {
            printf("failed RSA encryption and decryption\n");
        }
    }
}

void oracle_tests() {
    setup();
    mpz_t decrypted_input;
    mpz_init(decrypted_input);
    
    char pkcs_padded_input[300];
    char pkcs_decrypted_input[300];
    char message[] = "testMessage";

    inputToPaddedMessage(pkcs_padded_input, message);
    mpz_set_str(m, pkcs_padded_input, 16);

    if (!oracle(m)) {
        printf("oracle should be correct for original input");
    }

    encrypt(c,m,rsa);
    for(int i = 0; i < 5; i++){    
        findNextS();

        mpz_t validInput;
        mpz_init(validInput);

        mpz_powm(validInput,s,rsa.E,rsa.N);
        mpz_mul(validInput,validInput,c);
        mpz_mod(validInput,validInput,rsa.N);

        if (!oracle(validInput)) {
            printf("oracle should be correct for valid input");
        }

        mpz_mul(validInput,validInput,validInput);
        mpz_mod(validInput,validInput,rsa.N);
        if (oracle(validInput)) {
            printf("oracle should not be correct for invalid input");
        }
    }
}


int main() {
    interval_tests();
    rsa_tests();
    oracle_tests();

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
    
    return 0;   
}