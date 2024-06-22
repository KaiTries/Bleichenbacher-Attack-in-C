#include "rsa.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char oracleString[RSA_BLOCK_BYTE_SIZE * 2];

int oracleCalls = 0;

int prepareInput(char *output, char *input) {
    sprintf(&output[0],"%02x", 0);
    sprintf(&output[2],"%02x", 2);

    srand(1); 
    int i;
    for (i = 2; i < RSA_BLOCK_BYTE_SIZE - strlen(input) - 1; i++) {
        sprintf(&output[i * 2], "%02x", rand() % 256);
    }
    sprintf(&output[i++ * 2], "%02x", 0);

    for (int t = 0;t < strlen(input); i++, t++) {
        sprintf(&output[i * 2], "%02x", (unsigned char)input[t]);
    }
    return 0;
}

int prepareOutput(char *output, char *input) {
    int i = 4;
    // Skip the padding bytes
    while (input[i * 2] != '0' || input[i * 2 + 1] != '0') {
        i++;
    }
    // Skip the '00' byte
    i++;

    // Copy the rest of the input to the output
    int j = 0;
    while (input[i * 2] != '\0' && input[i * 2 + 1] != '\0') {
        sscanf(&input[i * 2], "%02hhx", &output[j]);
        i++;
        j++;
    }

    // Null-terminate the output string
    output[j] = '\0';
    return 0;
}

void encrypt(mpz_t *output, mpz_t *input, RSA *rsa) {
    mpz_powm(*output,*input,rsa->E,rsa->N);

}

void decrypt(mpz_t *output, mpz_t *input, RSA *rsa) {
    mpz_powm(*output,*input,rsa->D,rsa->N);
}

void generate(RSA *rsa, gmp_randstate_ptr state) {
    mpz_t p,q,e,d,n, x,y,phi,u,t,gcd;
    mpz_inits(p,q,e,d,n,x,y,phi,u,t,gcd, NULL);
    mpz_set_ui(e, RSA_PUBLIC_EXPONENT);

    mpz_setbit(u, 511);
    mpz_setbit(t, 512);

    mpz_urandomb(x, state, 512);
    mpz_add(x,x, u);
    mpz_mod(x,x,t);
    mpz_urandomb(y,state, 512);
    mpz_add(y,y,u);
    mpz_mod(y,y,t);
    mpz_nextprime(p,x);
    mpz_nextprime(q,y);

    mpz_mul(n,p,q);
    mpz_sub_ui(p,p,1);
    mpz_sub_ui(q,q,1);
    mpz_mul(phi, p, q);
    mpz_gcd(gcd, e, phi);

    if (mpz_cmp_ui(gcd, 1) != 0) {
        printf("e and phi are not coprime!\n");
        exit(1);
    }

    int res = mpz_invert(d, e, phi);

    if (res == 0) {
        printf("Failed to compute inverse of e!\n");
        exit(1);
    }


    mpz_set(rsa->E,e);
    mpz_set(rsa->D,d);
    mpz_set(rsa->N,n);    

    
    // assert that both d and e are smaller than n
    if (mpz_cmp(rsa->D,rsa->N) >= 0 || mpz_cmp(rsa->E,rsa->N) >= 0) {
        printf("Incorrect RSA keys");
        exit(1);
    }

    // assert that e and d work as expected
    mpz_t test;
    mpz_init_set_ui(test, 46);
    mpz_powm(test,test,rsa->E,rsa->N);
    mpz_powm(test,test,rsa->D,rsa->N);
    if(!(mpz_cmp_ui(test,46) == 0)) {
        printf("Error with d and e\n");
        exit(1);
    }
    mpz_clears(p,q,e,d,n,x,y,phi,u,t,gcd, test, NULL);
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

int oracle(mpz_t *number, RSA *rsa) {
    mpz_t decrypted_input;
    mpz_init(decrypted_input);
    decrypt(&decrypted_input,number,rsa);
    mpz_to_hex_array(oracleString, &decrypted_input);
    oracleCalls++;
    mpz_clear(decrypted_input);
    if(oracleString[0] != '0' || oracleString[1] != '0') return 0;
    if(oracleString[2] != '0' || oracleString[3] != '2') return 0;
    return 1;
}
