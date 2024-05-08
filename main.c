#include "interval.h"
#include "bleichenbacher.h"
#include <time.h>
#include <stdio.h>




clock_t start_time, end_time, start_time_2, end_time_2;
double cpu_time_used, cpu_time_used_2;

int main() {
    setup();

    char user_input[100];
    char pkcs_padded_input[RSA_BLOCK_BYTE_SIZE];
    char decrypted_input_char[RSA_BLOCK_BYTE_SIZE];
    fgets(user_input, sizeof(user_input), stdin);

    add_padding(pkcs_padded_input, user_input);
    print_hex(pkcs_padded_input, RSA_BLOCK_BYTE_SIZE);
}