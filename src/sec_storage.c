#include "sec_storage.h"

#include "at_commands.h"
#include "str_utils.h"
#include "fcp_parser.h"
#include "utils.h"

#include <stdint.h>

#define PASSWORD_SIZE_MIN   8
#define PASSWORD_SIZE_MAX   64

static uint8_t user_key_id      = 0xC0;
static uint16_t user_kf_fid     = 0xCD00;
static uint16_t user_df_fid     = 0xDF00;
static uint16_t user_bf_fid     = 0xBF00;
static uint16_t user_efarr_fid  = 0xEF00;
static uint8_t user_efarr_sfi   = 0xF8;

static bool is_user_id_ok(int user_id) {
    return (user_id >= 1 && user_id <= 15);
}

static bool is_file_id_ok(int file_id) {
    return (file_id >= 1 && file_id <= 15);
}

static bool is_password_size_ok(size_t passwd_size) {
    return (passwd_size >= PASSWORD_SIZE_MIN && passwd_size <= PASSWORD_SIZE_MAX);
}

static int configure_user(int user_id) {
    if (!is_user_id_ok(user_id)) {
        printf("configure_user(), user id is not OK\n");
        return 1;
    }

    user_df_fid = 0xDF00 + user_id;
    user_kf_fid = 0xCD00 + user_id;
    user_efarr_fid = 0xEF00 + user_id;
    // user_efarr_sfi = 0xF000 + user_id;
    user_key_id = 0xC0 + user_id;

    dprint("user is confgiured:\n");
    dprint(" - user_df_fid     = 0x%04X \n", user_df_fid);
    dprint(" - user_kf_fid     = 0x%04X \n", user_kf_fid);
    dprint(" - user_efarr_fid  = 0x%04X \n", user_efarr_fid);
    dprint(" - user_efarr_sfi  = 0x%02X \n", user_efarr_sfi);
    dprint(" - user_key_id     = 0x%02X \n", user_key_id);

    return 0;
}

static void configure_user_(int user_id) {
    user_df_fid = 0xDF00 + user_id;
    user_kf_fid = 0xCD00 + user_id;
    user_efarr_fid = 0xEF00 + user_id;
    // user_efarr_sfi = 0xF000 + user_id;
    user_key_id = 0xC0 + user_id;

    dprint("user is confgiured:\n");
    dprint(" - user_df_fid     = 0x%04X \n", user_df_fid);
    dprint(" - user_kf_fid     = 0x%04X \n", user_kf_fid);
    dprint(" - user_efarr_fid  = 0x%04X \n", user_efarr_fid);
    dprint(" - user_efarr_sfi  = 0x%02X \n", user_efarr_sfi);
    dprint(" - user_key_id     = 0x%02X \n", user_key_id);
}

static int configure_bf_fid(int file_id) {
    if (!is_file_id_ok(file_id)) {
        wprint("file id is not OK\n");
        return 1;
    }
    user_bf_fid = 0xBF00 + file_id;
    return 0;
}

static int configure_bf_fid_(int file_id) {
    user_bf_fid = 0xBF00 + file_id;
    return 0;
}

static int set_file_lcs_to_use(int fd, uint16_t fid) {
    int ret;
    ret = do_select_fid_no_rsp(fd, fid);
    if (ret != 0) {
        printf("set_file_lcs_to_use, Error during 'do_select_fid_no_rsp' function for fid = 0x%04X\n", fid);
        return ret;
    }

    ret = do_set_lcs_to_use_cur_file(fd);
    if (ret != 0) {
        printf("set_file_lcs_to_use, Error during 'do_set_lcs_to_use_cur_file' function\n");
        return ret;
    }

    return 0;
}

static int create_user_df_file(int fd) {
    struct DirFcp df_fcp = {
        .fid = user_df_fid,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x01,
    };
    int ret = do_create_df(fd, &df_fcp);
    if (ret != 0) {
        printf("create_user_df_file, Error during user DF creation\n");
        return 1;
    }
    return ret;
}

static int delete_user_df_file(int fd) {
    int ret;

    ret = do_select_fid_no_rsp(fd, user_df_fid);
    if (ret != 0) {
        printf("delete_user_df_file(), error on DF selection\n");
        return ret;
    }

    ret = do_delete_current_fid(fd);
    if (ret != 0) {
        printf("delete_user_df_file(), error on user DF delete operation\n");
        return ret;
    }
    return 0;
}

static int create_user_efarr_file(int fd) {
    uint8_t record_size = 21;
    uint8_t num_records = 4;
    uint16_t file_size = (uint16_t)record_size * (uint16_t)num_records;

    uint8_t   records[4 * 21] = {
        0x80, 0x01, 0b01000110, 0xA4, 0x03, 0x83, 0x01, user_key_id, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x80, 0x01, 0x01, 0x90, 0x00, 0x80, 0x01, 0b01000000, 0XA4, 0x03, 0x83, 0x01, user_key_id, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x80, 0x01, 0x8C, 0x90, 0x00, 0x80, 0x01, 0b01000000, 0XA4, 0x03, 0x83, 0x01, user_key_id, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x80, 0x01, 0b01111011, 0xA4, 0x03, 0x83, 0x01, user_key_id, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    struct EfarrDscr dscr = {
        .file_size = file_size, 
        .fid = user_efarr_fid,
        .sfi = user_efarr_sfi,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x02,
        .record_size = record_size,
        .num_records = num_records,
        .records = &records[0]
    };

    int ret;
    ret = do_create_efarr_file(fd, &dscr);
    if (ret != 0) {
        printf("create_user_efarr_file, Error during EF.arr file creation\n");
        return ret;
    }

    ret = set_file_lcs_to_use(fd, user_efarr_fid);
    if (ret != 0) {
        printf("create_user_efarr_file, Error configure Use state for current file\n");
        return ret;
    }

    return 0;
}

static int create_kf_pin_file(int fd, uint8_t* passwd, size_t passwd_size) {

    struct KfDscr dscr = {
        .fid = user_kf_fid,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x03,

        .key_algo = 0x00,
        .key_purpose = 0x0000,
        .key_id = user_key_id,
        .kf_flags = 0x00,
    
        .passwd = passwd,
        .passwd_size = passwd_size
    };

    do_create_kf_file(fd, &dscr);
    // do_set_lcs_to_use_cur_file(fd);

    set_file_lcs_to_use(fd, user_kf_fid);

    return 0;
}

static sec_storage_status_t create_bf_file_impl(int fd, uint8_t* data, size_t data_size) {
    struct BfDscr dscr = {
        .file_size = data_size,
        .fid = user_bf_fid,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x04,
    
        .data = data,
        .data_size = data_size
    };

    if (do_create_bf_file(fd, &dscr) != 0) {
        printf("create_bf_file_impl(), error during 'do_create_bf_file' function call\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    if (set_file_lcs_to_use(fd, user_bf_fid) != 0) {
        printf("create_bf_file_impl(), error during 'set_file_lcs_to_use' function call\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

static int delete_bf_file_impl(int fd) {

    do_select_fid_no_rsp(fd, user_bf_fid);
    int ret = do_delete_current_fid(fd);
    if (ret != 0) {
        printf("delete_bf_file_impl(), error on user DF delete operation\n");
        return ret;
    }
    return 0;
}

static int open_user_df(int fd, int user_id) {
    return do_select_fid_no_rsp(fd, user_df_fid);
}

static sec_storage_status_t select_user_helper(int fd, int user_id, size_t passwd_size, bool* user_exist_flag) {
    if (do_select_crypto_aid(fd) != 0) {
        wprint("register_new_user(), failed to select Crypto app\n");
        return SEC_STORAGE_AID_SELECT_ERROR;
    }

    if (!is_user_id_ok(user_id)) {
        wprint("register_new_user(), user id is not OK\n");
        return SEC_STORAGE_WRONG_USER_ID;
    }

    if (!is_password_size_ok(passwd_size)) {
        wprint("register_new_user(), password size is not OK\n");
        return SEC_STORAGE_WRONG_PASSWORD_SIZE;
    }

    configure_user_(user_id);

    dprint("run do_check_fid_existence\n");
    if (do_check_fid_existence(fd, user_df_fid, user_exist_flag) != 0) {
        eprint("register_new_user(), Error during 'do_check_fid_existence' function call\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

static sec_storage_status_t select_user_file_helper(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, bool* file_exist_flag) {

    bool user_exist_flag;
    sec_storage_status_t status = select_user_helper(fd, user_id, passwd_size, &user_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("select_user_file_helper(), Error during 'select_user_helper' function call\n");
        return status;
    }

    dprint("select_user_file_helper(), user_exist_flag = %d\n", user_exist_flag);
    if (!user_exist_flag) {
        printf("select_user_file_helper(), current user id is not registered\n");
        return SEC_STORAGE_USER_IS_NOT_REGISTERED;
    }

    if (open_user_df(fd, user_id) != 0) {
        printf("select_user_file_helper(), error during 'open_user_df' function\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    if (!is_file_id_ok(file_id)) {
        wprint("select_user_file_helper(), file id is not OK\n");
        return SEC_STORAGE_WRONG_FILE_ID;
    }

    configure_bf_fid_(file_id);

    dprint("select_user_file_helper, run do_check_fid_existence\n");
    if (do_check_fid_existence(fd, user_bf_fid, file_exist_flag) != 0) {
        eprint("select_user_file_helper(), error during 'do_check_fid_existence' function call\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    dprint("select_user_file_helper, run do_verify_pin\n");
    if (do_verify_pin(fd, user_key_id, passwd, passwd_size) != 0) {
        eprint("select_user_file_helper(), password verification failed\n");
        return SEC_STORAGE_PASSWORD_VERIF_ERROR;
    }

    return SEC_STORAGE_OK;
}

static sec_storage_status_t register_new_user_impl(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    
    bool user_exist_flag;
    sec_storage_status_t status = select_user_helper(fd, user_id, passwd_size, &user_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("register_new_user_impl(), Error during 'select_user_helper' function call\n");
        return status;
    }
    // configure_user_(user_id);

    // bool user_exist_flag;
    // dprint("run do_check_fid_existence\n");
    // if (do_check_fid_existence(fd, user_df_fid, &user_exist_flag) != 0) {
    //     eprint("register_new_user(), Error during 'do_check_fid_existence' function call\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    dprint("register_new_user(), user_exist_flag = %d\n", user_exist_flag);
    if (user_exist_flag) {
        wprint("current user id is already registered\n");
        return SEC_STORAGE_USER_IS_ALREADY_REGISTERED;
    }

    if (create_user_df_file(fd) != 0) {
        eprint("register_new_user(), Error during user DF file creation\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    if (create_user_efarr_file(fd) != 0) {
        eprint("register_new_user(), Error during user EF.arr file creation\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    if (create_kf_pin_file(fd, passwd, passwd_size) != 0) {
        eprint("register_new_user(), Error during user KF file creation\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    if (set_file_lcs_to_use(fd, user_df_fid) != 0) {
        eprint("register_new_user, Error configure Use state for user DF\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }
    
    return SEC_STORAGE_OK;
}

static sec_storage_status_t unregister_user_impl(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    // configure_user_(user_id);

    // check if current user id is already registered
    // bool user_exist_flag;
    // if (do_check_fid_existence(fd, user_df_fid, &user_exist_flag) != 0) {
    //     printf("unregister_user(), Error during 'do_check_fid_existence' function call\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    bool user_exist_flag;
    sec_storage_status_t status = select_user_helper(fd, user_id, passwd_size, &user_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("register_new_user_impl(), Error during 'select_user_helper' function call\n");
        return status;
    }

    dprint("unregister_user(), user_exist_flag = %d\n", user_exist_flag);
    if (!user_exist_flag) {
        printf("unregister_user(), current user id is not registered\n");
        return SEC_STORAGE_USER_IS_NOT_REGISTERED;
    }

    if (do_verify_pin(fd, user_key_id, passwd, passwd_size) != 0) {
        printf("unregister_user(), PIN verification is failed\n");
        return SEC_STORAGE_PASSWORD_VERIF_ERROR;
    }

    if (delete_user_df_file(fd) != 0) {
        printf("unregister_user(), failed to delete usef DF file\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

sec_storage_status_t register_new_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    // if (do_select_crypto_aid(fd) != 0) {
    //     wprint("register_new_user(), failed to select Crypto app\n");
    //     return SEC_STORAGE_AID_SELECT_ERROR;
    // }

    // if (!is_user_id_ok(user_id)) {
    //     wprint("register_new_user(), user id is not OK\n");
    //     return SEC_STORAGE_WRONG_USER_ID;
    // }

    // if (!is_password_size_ok(passwd_size)) {
    //     wprint("register_new_user(), password size is not OK\n");
    //     return SEC_STORAGE_WRONG_PASSWORD_SIZE;
    // }

    return register_new_user_impl(fd, user_id, passwd, passwd_size);
}

sec_storage_status_t unregister_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    // int ret;

    // if (do_select_crypto_aid(fd) != 0) {
    //     printf("unregister_user(), failed to select Crypto app\n");
    //     return SEC_STORAGE_AID_SELECT_ERROR;
    // }

    // if (!is_user_id_ok(user_id)) {
    //     printf("unregister_user(), user id is not OK\n");
    //     return SEC_STORAGE_WRONG_USER_ID;
    // }

    // if (!is_password_size_ok(passwd_size)) {
    //     wprint("unregister_user(), password size is not OK\n");
    //     return SEC_STORAGE_WRONG_PASSWORD_SIZE;
    // }

    return unregister_user_impl(fd, user_id, passwd, passwd_size);
}

sec_storage_status_t is_user_registered(int fd, int user_id, bool* reg_flag) {
    // bool user_exist_flag;
    // sec_storage_status_t status = select_user_helper(fd, user_id, PASSWORD_SIZE_MAX, &user_exist_flag);
    // if (status != SEC_STORAGE_OK) {
    //     eprint("register_new_user_impl(), Error during 'select_user_helper' function call\n");
    //     return status;
    // }


    // if (do_select_crypto_aid(fd) != 0) {
    //     printf("is_user_registered(), failed to select Crypto app\n");
    //     return SEC_STORAGE_AID_SELECT_ERROR;
    // }

    // if (!is_user_id_ok(user_id)) {
    //     wprint("is_user_registered(), user id is not OK\n");
    //     return SEC_STORAGE_WRONG_USER_ID;
    // }

    // configure_user_(user_id);

    // // check if current user id is already registered
    // dprint("run do_check_fid_existence\n");
    // if (do_check_fid_existence(fd, user_df_fid, reg_flag) != 0) {
    //     printf("is_user_registered(), error during 'do_check_fid_existence function\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    return select_user_helper(fd, user_id, PASSWORD_SIZE_MAX, reg_flag);
}

sec_storage_status_t get_list_of_registered_users(int fd, uint8_t* user_ids, size_t* size) {
    if (do_select_crypto_aid(fd) != 0) {
        printf("get_list_of_registered_users(), failed to select Crypto app\n");
        return SEC_STORAGE_AID_SELECT_ERROR;
    }

    struct FcpDscr fcp_dscr;

    bool first_flag = true;
    *size = 0;
    while (do_select_files_cur_dir(fd, first_flag, &fcp_dscr) == 0) {
        first_flag = false;

        // TMP
        // print_fcp_dscr(&fcp_dscr);

        if (!is_df_md_file_type(&fcp_dscr)) {
            continue;
        }
        if ((get_fid(&fcp_dscr) & 0xFF00 )!= 0xDF00) {
            continue;
        }

        if (fcp_dscr.tag_0x83.tag_size > 0) {
            user_ids[*size] = fcp_dscr.tag_0x83.value[1];
            *size += 1;
        }
    }

    free_fcp_dscr(&fcp_dscr);

    return SEC_STORAGE_OK;
}

sec_storage_status_t select_user(int fd, int user_id) {

    bool user_exist_flag;
    sec_storage_status_t status = select_user_helper(fd, user_id, PASSWORD_SIZE_MAX, &user_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("select_user(), Error during 'select_user_helper' function call\n");
        return status;
    }

    // if (do_select_crypto_aid(fd) != 0) {
    //     printf("select_user(), failed to select Crypto app\n");
    //     return SEC_STORAGE_AID_SELECT_ERROR;
    // }

    // if (!is_user_id_ok(user_id)) {
    //     wprint("register_new_user(), user id is not OK\n");
    //     return SEC_STORAGE_WRONG_USER_ID;
    // }

    // configure_user_(user_id);

    // bool user_exist_flag;
    // if (do_check_fid_existence(fd, user_df_fid, &user_exist_flag) != 0) {
    //     printf("select_user(), error during 'do_check_fid_existence' function\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    dprint("select_user(), user_exist_flag = %d\n", user_exist_flag);
    if (!user_exist_flag) {
        printf("select_user(), current user id is not registered\n");
        return SEC_STORAGE_USER_IS_NOT_REGISTERED;
    }

    if (open_user_df(fd, user_id) != 0) {
        printf("select_user(), error during 'open_user_df' function\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

static sec_storage_status_t select_user_file(int fd, int user_id, int file_id) {

    bool user_exist_flag;
    sec_storage_status_t status = select_user_helper(fd, user_id, PASSWORD_SIZE_MAX, &user_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("select_user(), Error during 'select_user_helper' function call\n");
        return status;
    }

    // if (do_select_crypto_aid(fd) != 0) {
    //     printf("select_user(), failed to select Crypto app\n");
    //     return SEC_STORAGE_AID_SELECT_ERROR;
    // }

    // if (!is_user_id_ok(user_id)) {
    //     wprint("register_new_user(), user id is not OK\n");
    //     return SEC_STORAGE_WRONG_USER_ID;
    // }

    // configure_user_(user_id);

    // bool user_exist_flag;
    // if (do_check_fid_existence(fd, user_df_fid, &user_exist_flag) != 0) {
    //     printf("select_user(), error during 'do_check_fid_existence' function\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    dprint("select_user(), user_exist_flag = %d\n", user_exist_flag);
    if (!user_exist_flag) {
        printf("select_user(), current user id is not registered\n");
        return SEC_STORAGE_USER_IS_NOT_REGISTERED;
    }

    if (open_user_df(fd, user_id) != 0) {
        printf("select_user(), error during 'open_user_df' function\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

sec_storage_status_t create_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t data_size) {
    bool file_exist_flag;
    sec_storage_status_t status;
    status = select_user_file_helper(fd, user_id, file_id, passwd, passwd_size, &file_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("create_bf_file(), error on 'select_user_file_helper' function call\n");
        return status;
    }

    // status = select_user(fd, user_id);
    // if (status != SEC_STORAGE_OK) {
    //     eprint("create_bf_file(), user selection failed\n");
    //     return status;
    // }

    // if (!is_file_id_ok(file_id)) {
    //     wprint("create_bf_file(), file id is not OK\n");
    //     return SEC_STORAGE_WRONG_FILE_ID;
    // }

    // ret = configure_user(user_id);
    // if (ret != 0) {
    //     eprint("create_bf_file(), user configuration failed\n");
    //     return ret;
    // }

    // configure_bf_fid_(file_id);

    // ret = open_user_df(fd, user_df_fid);
    // if (ret != 0) {
    //     eprint("create_bf_file(), failed to open user DF\n");
    //     return ret;
    // }

    // dprint("create_bf_file, run do_check_fid_existence\n");
    // bool bf_exist_flag;
    // if (do_check_fid_existence(fd, user_bf_fid, &bf_exist_flag) != 0) {
    //     eprint("create_bf_file(), error during 'do_check_fid_existence' function call\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    // dprint("create_bf_file, run do_verify_pin\n");
    // if (do_verify_pin(fd, user_key_id, passwd, passwd_size) != 0) {
    //     eprint("create_bf_file(), password verification failed\n");
    //     return SEC_STORAGE_PASSWORD_VERIF_ERROR;
    // }

    dprint("create_bf_file(), file_exist_flag = %d\n", file_exist_flag);
    if (file_exist_flag) {
        eprint("create_bf_file(), current file id is already created\n");
        return SEC_STORAGE_FILE_IS_ALREADY_CREATED;
    }

    // ret = 
    // if (ret != 0) {
    //     eprint("create_bf_file(), error during 'create_bf_file_impl' function call\n");
    //     return ret;
    // }

    // if (do_write_data_into_bf_file(fd, user_bf_fid, data, data_size) != 0) {
    //     eprint("create_bf_file(), error during 'do_write_data_into_bf_file' function call\n");
    //     return SEC_STORAGE_UNKNOWN_ERROR;
    // }

    return create_bf_file_impl(fd, data, data_size);
}

sec_storage_status_t read_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t* data_size) {
    // int ret;

    // ret = select_user(fd, user_id);
    // if (ret != 0) {
    //     eprint("read_bf_file(), user selection failed\n");
    //     return ret;
    // }

    bool file_exist_flag;
    sec_storage_status_t status;
    status = select_user_file_helper(fd, user_id, file_id, passwd, passwd_size, &file_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("read_bf_file(), error on 'select_user_file_helper' function call\n");
        return status;
    }

    // ret = configure_user(user_id);
    // if (ret != 0) {
    //     eprint("read_bf_file(), user configuration failed\n");
    //     return ret;
    // }

    // ret = configure_bf_fid(file_id);
    // if (ret != 0) {
    //     eprint("read_bf_file(), user bf configuration failed\n");
    //     return ret;
    // }

    // ret = open_user_df(fd, user_df_fid);
    // if (ret != 0) {
    //     eprint("read_bf_file(), failed to open user DF\n");
    //     return ret;
    // }

    // dprint("read_bf_file, run do_check_fid_existence\n");
    // bool bf_exist_flag;
    // ret = do_check_fid_existence(fd, user_bf_fid, &bf_exist_flag);
    // if (ret != 0) {
    //     eprint("read_bf_file(), error during 'do_check_fid_existence' function call\n");
    //     return ret;
    // }

    dprint("read_bf_file(), file_exist_flag = %d\n", file_exist_flag);
    if (!file_exist_flag) {
        dprint("read_bf_file(), current file id is not created\n");
        return 1;
    }

    // dprint("read_bf_file, run do_verify_pin\n");
    // ret = do_verify_pin(fd, user_key_id, passwd, passwd_size);
    // if (ret != 0) {
    //     eprint("read_bf_file(), password verification failed\n");
    //     return -1;
    // }

    if (do_read_data_from_bf_file(fd, user_bf_fid, data, data_size) != 0) {
        eprint("read_bf_file(), error during 'do_read_data_from_bf_file' function call\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

sec_storage_status_t delete_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size) {
    // int ret;

    // ret = select_user(fd, user_id);
    // if (ret != 0) {
    //     printf("delete_bf_file(), user selection failed\n");
    //     return ret;
    // }

    // // ret = configure_user(user_id);
    // // if (ret != 0) {
    // //     printf("create_bf_file(), user configuration failed\n");
    // //     return ret;
    // // }
    
    // ret = configure_bf_fid(file_id);
    // if (ret != 0) {
    //     printf("delete_bf_file(), user bf configuration failed\n");
    //     return ret;
    // }

    // dprint("delete_bf_file, run do_check_fid_existence\n");
    // bool bf_exist_flag;
    // ret = do_check_fid_existence(fd, user_bf_fid, &bf_exist_flag);
    // if (ret != 0) {
    //     printf("delete_bf_file(), error during 'do_check_fid_existence' function call\n");
    //     return ret;
    // }

    // // ret = open_user_df(fd, user_df_fid);
    // // if (ret != 0) {
    // //     printf("delete_bf_file(), failed to open user DF\n");
    // //     return ret;
    // // }

    bool file_exist_flag;
    sec_storage_status_t status;
    status = select_user_file_helper(fd, user_id, file_id, passwd, passwd_size, &file_exist_flag);
    if (status != SEC_STORAGE_OK) {
        eprint("delete_bf_file(), error on 'select_user_file_helper' function call\n");
        return status;
    }

    dprint("delete_bf_file(), bf_exist_flag = %d\n", bf_exist_flag);
    if (!file_exist_flag) {
        printf("delete_bf_file(), current file id is not created\n");
        return 1;
    }

    // ret = do_verify_pin(fd, user_key_id, passwd, passwd_size);
    // if (ret != 0) {
    //     printf("delete_bf_file(), password verification failed\n");
    //     return ret;
    // }

    if (delete_bf_file_impl(fd) != 0) {
        printf("delete_bf_file(), failure on user BF file deletion\n");
        return SEC_STORAGE_UNKNOWN_ERROR;
    }

    return SEC_STORAGE_OK;
}

sec_storage_status_t get_list_of_user_bf_files(int fd, int user_id, uint8_t* file_ids, uint16_t* file_sizes, size_t* num_files) {
    sec_storage_status_t status;

    status = select_user(fd, user_id);
    if (status != 0) {
        printf("get_list_of_user_bf_files(), user selection failed\n");
        return status;
    }

    struct FcpDscr fcp_dscr;

    bool first_flag = true;
    // *file_id_size = 0;
    *num_files = 0;
    while (do_select_files_cur_dir(fd, first_flag, &fcp_dscr) == 0) {
        first_flag = false;

        if (!is_bf_file_type(&fcp_dscr)) {
            continue;
        }
        if ((get_fid(&fcp_dscr) & 0xFF00 ) != 0xBF00) {
            continue;
        }

        if (fcp_dscr.tag_0x83.tag_size > 0) {
            file_ids[*num_files] = fcp_dscr.tag_0x83.value[1];
            file_sizes[*num_files] = get_file_size(&fcp_dscr);
            *num_files += 1;
        }
    }

    free_fcp_dscr(&fcp_dscr);

    return SEC_STORAGE_OK;
}
