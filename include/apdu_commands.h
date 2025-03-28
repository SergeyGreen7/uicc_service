#ifndef _APDU_COMMANDS_H_
#define _APDU_COMMANDS_H_

#include <stdint.h>

void get_apdu_select_crypto_app(char* str, size_t* size);

void get_apdu_select_first_file_cur_dir_fcp(char* str, size_t* size);
void get_apdu_select_next_file_cur_dir_fcp(char* str, size_t* size);

void get_apdu_select_fid_no_rsp(char* str, size_t* size, uint16_t fid);
void get_apdu_select_fid_fcp(char* str, size_t* size, uint16_t fid);

void get_apdu_set_lcs_use(char* str, size_t* size);

void get_apdu_get_challenge_4_bytes(char* str, size_t* size);

void get_apdu_create_file(char* str, size_t* size, const uint8_t* fcp, size_t fcp_size);
void get_apdu_delete_current_file(char* str, size_t* size);

void get_apdu_append_record(char* str, size_t* size, const uint8_t* record, size_t record_size);

void get_apdu_change_refrence_data_pin(char* str, size_t* size, const uint8_t* passwd, size_t passwd_size);

void get_apdu_update_binary(char* str, size_t* size, uint16_t offset, const uint8_t* data, size_t data_size);
void get_apdu_read_binary(char* str, size_t* size, uint16_t offset, uint8_t data_chunk_size);

void get_apdu_verify_pin(char* str, size_t* size, uint8_t key_id, const uint8_t* passwd, size_t passwd_size);

#endif // _APDU_COMMANDS_H_