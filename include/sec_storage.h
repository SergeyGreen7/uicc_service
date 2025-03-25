#ifndef _SEC_STORAGE_H_
#define _SEC_STORAGE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

int register_new_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size);
int unregister_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size);

int create_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t data_size);
int read_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t* data_size);
int delete_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size);

#endif // _SEC_STORAGE_H_