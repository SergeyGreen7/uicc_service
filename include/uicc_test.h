#ifndef _UICC_TEST_H_
#define _UICC_TEST_H_

#include <stdbool.h>
#include <stdint.h>

bool is_connection_established();
int open_connection_se();
int close_connection_se();
int get_serial_port_fd();

bool is_se_exists();

bool is_crypto_lchannel_opened();
int open_crypto_lchannel();
int close_crypto_lchannel();
int close_crypto_lchannel_force();

int select_crypto_aid();

// bool check_se_exist();

int get_random_int(int* rand_num);

int create_storage_efarr_file();

int check_if_file_exists(int);

// int register_new_user(int user_id, char* passwd, size_t size);
// int unregister_user(int user_id, char* passwd, size_t size);

void run_interactive_mode(bool read_output, bool print_flag);

// void register_user();


#endif // _UICC_TEST_H_