#include <apdu_commands.h>
#include <str_utils.h>
#include <uicc_common.h>

#include <string.h>

// tmp
static const char crypto_aid[] = "F04D4552499301";
static const char apdu_select_crypto_aid[] = "00A4040007F04D4552499301";
static const char apdu_get_challange_4_bytes[] = "0084000004";


static void get_apdu_select_aid(char* str, size_t* size, const char* aid, size_t aid_size) {
    // char cmd[] = "00A40400";
    char cmd[] = "00A4040C";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);
    
    append_symbol_asbyte_hex(str, size, aid_size / 2);
    memcpy(str + *size, aid, aid_size);
    *size += aid_size;
}

static void get_apdu_get_challenge(char* str, size_t* size, int num_bytes) {
    char cmd[] = "00840000";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);
    append_symbol_asbyte_hex(str, size, num_bytes);
}

void get_apdu_select_crypto_app(char* str, size_t* size) {
    get_apdu_select_aid(str, size, crypto_aid, strlen(crypto_aid));
}

void get_apdu_select_fid_no_rsp(char* str, size_t* size, uint16_t fid) {
    // AT+CGLA=1,14,"00A4000C02DF04"
    char cmd[] = "00A4000C";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, 2);
    append_symbol_hex(str, size, 2, fid);
}

void get_apdu_select_fid_fcp(char* str, size_t* size, uint16_t fid) {
    char cmd[] = "00A40004";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, 2);
    append_symbol_hex(str, size, 2, fid);
}

void get_apdu_set_lcs_use(char* str, size_t* size) {
    char cmd[] = "00440000";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);
}

void get_apdu_get_challenge_4_bytes(char* str, size_t* size) {
    get_apdu_get_challenge(str, size, 4);
}

void get_apdu_create_file(char* str, size_t* size, const uint8_t* fcp, size_t fcp_size) {
    char cmd[] = "00E00000";
    
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, fcp_size);

    for (size_t i = 0; i < fcp_size; i++) {
        append_symbol_asbyte_hex(str, size, fcp[i]);
    }
}

void get_apdu_delete_current_file(char* str, size_t* size) {
    char cmd[] = "00E40000";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);
}

void get_apdu_append_record(char* str, size_t* size, const uint8_t* record, size_t record_size) {
    char cmd[] = "00E20000";
    
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, record_size);

    for (size_t i = 0; i < record_size; i++) {
        append_symbol_asbyte_hex(str, size, record[i]);
    }
}

void get_apdu_change_refrence_data_pin(char* str, size_t* size, const uint8_t* passwd, size_t passwd_size) {
    char cmd[] = "00240100";
    
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, passwd_size);

    for (size_t i = 0; i < passwd_size; i++) {
        append_symbol_asbyte_hex(str, size, passwd[i]);
    }
}

void get_apdu_update_binary(char* str, size_t* size, uint16_t offset, const uint8_t* data, size_t data_size) {
    char cmd[] = "00D6";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, offset / 256);
    append_symbol_asbyte_hex(str, size, offset % 256);

    append_symbol_asbyte_hex(str, size, data_size);
    for (size_t i = 0; i < data_size; i++) {
        append_symbol_asbyte_hex(str, size, data[i]);
    }
}

void get_apdu_read_binary(char* str, size_t* size, uint16_t offset, uint8_t data_chunk_size) {
    char cmd[] = "00B0";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);

    append_symbol_asbyte_hex(str, size, offset / 256);
    append_symbol_asbyte_hex(str, size, offset % 256);
    append_symbol_asbyte_hex(str, size, data_chunk_size);
}

void get_apdu_verify_pin(char* str, size_t* size, uint8_t key_id, const uint8_t* passwd, size_t passwd_size) {
    // printf("size = %zu\n", *size);
    char cmd[] = "002000";
    memcpy(str + *size, cmd, strlen(cmd));
    *size += strlen(cmd);
    // printf("size = %zu\n", *size);

    append_symbol_asbyte_hex(str, size, key_id);
    // printf("size = %zu\n", *size);

    // printf("size = %zu, passwd_size = %zu, passwd = %p \n", *size, passwd_size, passwd);
    // for (size_t i = 0; i < passwd_size; i++) {
    //     printf("i %zu, passwd[i] = %d \n", i, passwd[i]);
    // }

    append_symbol_asbyte_hex(str, size, passwd_size);
    for (size_t i = 0; i < passwd_size; i++) {
        append_symbol_asbyte_hex(str, size, passwd[i]);
    }
}