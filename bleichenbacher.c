#include "bleichenbacher.h"


char D[] = "78913193811721706055797481363832150942743134067034761542313338748370726201033546287491574731373544676987613046861971631833989239967097513496626766533445786686631919564361097610399703352053354810041508557673189268121081384446559379257550457322913033120589771886529032181431944443148253890788094353860064363521";
char N[] = "106256861909081308550682107497975585066045298679965014118971673273174972942084474092770696358687294491611957516687069256153520562535413990426290853022321612859805226173471895669079726027809218970486618822150156382823218043763245669403338643791786280745103610457180590091099463063822447698700065670969522057503";
int E = 65537;
int TOTAL_REQUESTS = 0;

mpz_t d, n, e, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime;

// Encrypt function
void encrypt(mpz_t *output) {
    mpz_powm(*output, m, e, n);
}

// Decrypt function
void decrypt(mpz_t input, mpz_t *output) {
    mpz_powm(*output, input, d, n);
}

void setup() {
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
}

int add_padding(char *pkcs_padded_input, char *user_input) {
    // check that user input is not longer than RSA_BLOCK_BYTE_SIZE - 11
    if (strlen(user_input) > RSA_BLOCK_BYTE_SIZE - 11) {
        printf("User input is too long\n");
        return -1;
    }

    // add 0x00 byte at the end
    pkcs_padded_input[RSA_BLOCK_BYTE_SIZE - 1] = 0x00;
    pkcs_padded_input[RSA_BLOCK_BYTE_SIZE - 2] = 0x02;


    // add the input
    for (int i = 0; i < strlen(user_input); i++) {
        pkcs_padded_input[i] = user_input[i];
    }

    pkcs_padded_input[strlen(user_input)] = 0x00;

    // add padding
    for (int i = strlen(user_input) + 1; i < RSA_BLOCK_BYTE_SIZE - 2; i++) {
        pkcs_padded_input[i] = 0x01; // 1 + rand() % 255;
    }

    return 0;
}

void print_hex(char *input, int length) {
    for (int i = length - 1; i >= 0; i--) {
        printf("%02x ", input[i]);
    }
    printf("\n");
}