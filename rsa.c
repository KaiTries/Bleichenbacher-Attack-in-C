#include "rsa.h"
#include <string.h>

char P[] = "13085544939016433034116036005625821953227372297195833776806214472190928979740509797654163083677193114899060771182653327749002407615798680323156649288891299";
char Q[] = "10515460639626849709984902074446790599067546798868400707778516190055079520038452945216601553837352784786191461977224566643334966876410080204134569391652071";

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

void generate(RSA *rsa){
    mpz_t p,q,e,d,n;
    // initialize p, q, and e
    mpz_init_set_str(p, P, 10);
    mpz_init_set_str(q, Q, 10);
    mpz_init_set_ui(e, RSA_PUBLIC_EXPONENT);

    // get n from p * q
    mpz_init(n);
    mpz_mul(n, p, q);

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

    mpz_init_set(rsa->E,e);
    mpz_init_set(rsa->D,d);
    mpz_init_set(rsa->N,n);    

    // assert that n is of correct length
    size_t sizeOfN = mpz_sizeinbase(rsa->N,2);
    printf("Bytesize of Modulus: %zu\n", sizeOfN / 8);
    if (sizeOfN / 8 != RSA_BLOCK_BYTE_SIZE) {
        printf("Incorrect length of Modulus\n");
        exit(1);
    }  
    
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
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(n);
    mpz_clear(phi);
    mpz_clear(p1);
    mpz_clear(q1);
    mpz_clear(test);
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