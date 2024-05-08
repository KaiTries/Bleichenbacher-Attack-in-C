#include "bleichenbacher.h"


char D[] = "78913193811721706055797481363832150942743134067034761542313338748370726201033546287491574731373544676987613046861971631833989239967097513496626766533445786686631919564361097610399703352053354810041508557673189268121081384446559379257550457322913033120589771886529032181431944443148253890788094353860064363521";
char N[] = "106256861909081308550682107497975585066045298679965014118971673273174972942084474092770696358687294491611957516687069256153520562535413990426290853022321612859805226173471895669079726027809218970486618822150156382823218043763245669403338643791786280745103610457180590091099463063822447698700065670969522057503";
int E = 65537;
int TOTAL_REQUESTS = 0;

char oracleString[RSA_BLOCK_BYTE_SIZE];

mpz_t d, n, e, B, B2, B3, s, m, c, a, b, r, r1, r2, oracle_decrypted_input, c_prime;

void encrypt(mpz_t *output) {
    mpz_powm(*output, m, e, n);
}

void decrypt(mpz_t *input, mpz_t *output) {
    mpz_powm(*output, *input, d, n);
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

int add_padding(char *pkcs_padded_input, char *user_input_hex) {
    // check that user input is not longer than RSA_BLOCK_BYTE_SIZE - 11
    if (strlen(user_input_hex) > RSA_BLOCK_BYTE_SIZE - 11) {
        printf("User input is too long\n");
        return -1;
    }

    pkcs_padded_input[0] = '0';
    pkcs_padded_input[1] = '2';

    for (int i = 2; i < RSA_BLOCK_BYTE_SIZE - strlen(user_input_hex); i++) {
        pkcs_padded_input[i] = '1'; // add randomness
    }

    pkcs_padded_input[RSA_BLOCK_BYTE_SIZE - strlen(user_input_hex) - 1] = '0';

    for (int i = RSA_BLOCK_BYTE_SIZE - strlen(user_input_hex), t = 0; i < RSA_BLOCK_BYTE_SIZE; i++, t++) {
        pkcs_padded_input[i] = user_input_hex[t];
    }

    return 0;
}

void char_to_hex_array(char *user_input_hex,char *user_input) {
    for (int i = 0; i < strlen(user_input); i++) {
        sprintf(&user_input_hex[i * 2], "%02x", (unsigned char)user_input[i]);
    }
}

void mpz_to_hex_array(char* hex_string, mpz_t number) {
        mpz_get_str(hex_string, 16, number);
        int len = strlen(hex_string);
        if (len > RSA_BLOCK_BYTE_SIZE) {
            exit(1);
        }

        if(len < RSA_BLOCK_BYTE_SIZE) {
            size_t dif = RSA_BLOCK_BYTE_SIZE - len;

            for (int i = len; i >= 0; i--) {
                hex_string[i + dif] = hex_string[i];
            }

            for (size_t i = 0; i < dif; i++)
            {
                hex_string[i] = '0';
            }
            
        }
}

int oracle(mpz_t number) {
    mpz_to_hex_array(oracleString, number);
    if(oracleString[0] != '0' || oracleString[1] != '2') return 0;
    return 1;
}