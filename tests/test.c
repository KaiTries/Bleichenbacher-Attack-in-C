#include "../interval.h"
#include "../bleichenbacher.h"
#include <time.h>
#include <stdio.h>

int inputToPaddedMessage(char *pkcs_padded_input, char *user_input) {
    sprintf(&pkcs_padded_input[0],"%02x", 0);
    sprintf(&pkcs_padded_input[2],"%02x", 2);

    int i;
    for (i = 2; i < RSA_BLOCK_BYTE_SIZE - strlen(user_input) - 1; i++) {
        sprintf(&pkcs_padded_input[i * 2], "%02x", 1); // add randomness here
    }
    sprintf(&pkcs_padded_input[i++ * 2], "%02x", 0);

    for (int t = 0;t < strlen(user_input); i++, t++) {
        sprintf(&pkcs_padded_input[i * 2], "%02x", (unsigned char)user_input[t]);
    }
    return 0;
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
    gmp_randstate_t state;
    gmp_randinit_default(state);
    setup(state);
    mpz_t decrypted_input;
    mpz_t c, m;
    mpz_init(c);
    mpz_init(m);
    mpz_init(decrypted_input);
    
    char pkcs_padded_input[300];
    char pkcs_decrypted_input[300];
    char message[] = "testMessage";

    inputToPaddedMessage(pkcs_padded_input, message);
    mpz_set_str(m, pkcs_padded_input, 16);
    encrypt(&c,&m,&rsa);
    decrypt(&decrypted_input,&c,&rsa);
    mpz_to_hex_array(pkcs_decrypted_input,&decrypted_input);

    for(int i = 0; i < strlen(pkcs_decrypted_input); i++) {
        if (pkcs_padded_input[i] != pkcs_decrypted_input[i]) {
            printf("failed RSA encryption and decryption\n");
        }
    }
}

void oracle_tests() {
     gmp_randstate_t state;
    gmp_randinit_default(state);   
    setup(state);
    mpz_t c, m, s, a, b;
    mpz_init(c);
    mpz_init(m);
    mpz_init_set_ui(s,1);
    mpz_init_set(a, B2);
    mpz_init_set(b, B3);
    mpz_sub_ui(b,b,1);
    mpz_t decrypted_input;
    mpz_init(decrypted_input);
    
    char pkcs_padded_input[300];
    char pkcs_decrypted_input[300];
    char message[] = "testMessage";

    inputToPaddedMessage(pkcs_padded_input, message);
    mpz_set_str(m, pkcs_padded_input, 16);

    if (!oracle(&m, &rsa)) {
        printf("oracle should be correct for original input");
    }

    encrypt(&c,&m,&rsa);
    for(int i = 0; i < 5; i++){    
        findNextS_iteratively(&c, &s, &a, &b);
        mpz_t validInput;
        mpz_init(validInput);

        mpz_powm(validInput,s,rsa.E,rsa.N);
        mpz_mul(validInput,validInput,c);
        mpz_mod(validInput,validInput,rsa.N);

        if (!oracle(&validInput, &rsa)) {
            printf("oracle should be correct for valid input");
        }

        mpz_mul(validInput,validInput,validInput);
        mpz_mod(validInput,validInput,rsa.N);
        if (oracle(&validInput, &rsa)) {
            printf("oracle should not be correct for invalid input");
        }
    }
    mpz_clear(a);
    mpz_clear(b);
}


int main() {
    interval_tests();
    rsa_tests();
    // oracle_tests();

    mpz_clear(B);
    mpz_clear(B2);
    mpz_clear(B3);
    return 0;   
}