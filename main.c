#include "interval.h"
#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>

clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;
char user_input[RSA_BLOCK_BYTE_SIZE];
char user_input_hex[RSA_BLOCK_BYTE_SIZE];
char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE];
char decrypted_input_char[RSA_BLOCK_BYTE_SIZE];


void print(char *string) {
    printf("%s\n", string);
}

void get_user_input(){
    fgets(user_input, sizeof(user_input), stdin);
    char_to_hex_array(user_input_hex, user_input);
}

int main() {
    setup();
    get_user_input();

    add_padding(pkcs_padded_input, user_input_hex);

    // Original Message
    mpz_set_str(m, pkcs_padded_input, 16);
    gmp_printf("Original Message: %Zd\n", m);

    encrypt(&c);
    printf("%d, %d\n", oracle(m), oracle(c));
    decrypt(&c, &c_prime);

    gmp_printf("%Zd\n", m);
    gmp_printf("%Zd\n", c_prime);


}