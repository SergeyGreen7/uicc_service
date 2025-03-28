#ifndef _SEC_STORAGE_H_
#define _SEC_STORAGE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

typedef enum {
    SEC_STORAGE_OK = 0,

    SEC_STORAGE_WRONG_USER_ID,
    SEC_STORAGE_WRONG_FILE_ID,
    SEC_STORAGE_WRONG_PASSWORD_SIZE,
    
    SEC_STORAGE_AID_SELECT_ERROR,
    SEC_STORAGE_USER_IS_NOT_REGISTERED,
    SEC_STORAGE_USER_IS_ALREADY_REGISTERED,

    SEC_STORAGE_FILE_IS_NOT_CREATED,
    SEC_STORAGE_FILE_IS_ALREADY_CREATED,

    SEC_STORAGE_PASSWORD_VERIF_ERROR,

    SEC_STORAGE_UNKNOWN_ERROR,
} sec_storage_status_t;

sec_storage_status_t select_user(int fd, int user_id);

sec_storage_status_t register_new_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size);
sec_storage_status_t unregister_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size);
sec_storage_status_t is_user_registered(int fd, int user_id, bool* reg_flag);
sec_storage_status_t get_list_of_registered_users(int fd, uint8_t* user_ids, size_t* size);

sec_storage_status_t create_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t data_size);
sec_storage_status_t read_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t* data_size);
sec_storage_status_t delete_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size);

sec_storage_status_t get_list_of_user_bf_files(int fd, int user_id, uint8_t* file_ids, uint16_t* file_sizes, size_t* num_files);

#endif // _SEC_STORAGE_H_