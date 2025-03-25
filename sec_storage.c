#include "sec_storage.h"

#include "at_commands.h"
#include "str_utils.h"

#include <stdint.h>

static uint8_t user_key_id = 0x00;
static uint16_t user_kf_fid      = 0xCC00;
static uint16_t user_df_fid      = 0xDF00;
static uint16_t user_bf_fid      = 0x0000;
static uint16_t user_efarr_fid   = 0xEF00;
static uint16_t user_efarr_sfi   = 0xF000;

static bool is_user_id_ok(int user_id) {
    return (user_id >= 1 && user_id <= 15);
}

static bool is_file_id_ok(int file_id) {
    return (file_id >= 1 && file_id <= 15);
}

static int configure_user(int user_id) {
    if (!is_user_id_ok(user_id)) {
        printf("user id is not OK\n");
        return 1;
    }

    user_df_fid = 0xDF00 + user_id;
    user_kf_fid = 0xCC00 + user_id;
    user_efarr_fid = 0xEF00 + user_id;
    user_efarr_sfi = 0xF000 + user_id;
    user_key_id = user_id;

    return 0;
}

static int configure_bf_fid(int user_id, int file_id) {
    if (!is_user_id_ok(user_id)) {
        printf("user id is not OK\n");
        return 1;
    }

    if (!is_file_id_ok(file_id)) {
        printf("file id is not OK\n");
        return 1;
    }
    user_bf_fid = user_id * 256 + file_id;
    return 0;
}

static int set_file_lcs_to_use(int fd, uint16_t fid) {
    do_select_fid_no_rsp(fd, fid);
    do_set_lcs_to_use_cur_file(fd);
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

    do_create_efarr_file(fd, &dscr);
    set_file_lcs_to_use(fd, user_efarr_fid);

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

static int create_bf_file_impl(int fd, uint8_t* data, size_t data_size) {

    struct BfDscr dscr = {
        .file_size = data_size,
        .fid = user_bf_fid,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x04,
    
        .data = data,
        .data_size = data_size
    };

    do_create_bf_file(fd, &dscr);
    set_file_lcs_to_use(fd, user_bf_fid);

    return 0;
}

int register_new_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    if (!is_user_id_ok(user_id)) {
        printf("user id is not OK\n");
        return 1;
    }

    // user_df_fid = [0xDF, user_id]
    configure_user(user_id);

    // check if current user id is already registered
    printf("run do_check_fid_existence\n");
    bool user_exist_flag = do_check_fid_existence(fd, user_df_fid);
    printf("register_new_user(), user_exist_flag = %d\n", user_exist_flag);

    if (user_exist_flag) {
        printf("current user id is already registered\n");
        return 1;
    }

    // print("try to select user DF")
    // cmd = apdu.adpu_add_data([0x00, 0xA4, 0x00, 0x0C], user_df_fid)
    // response, sw1, sw2 = service.transmit(cmd)

    // if [sw1, sw2] != [0x6A, 0x82]:
    //     print("current user id = " + utils.int_2_hexstr(user_id, add0x=True) + " is already registered")
    //     return

    // create_df_directory_user(service, user_id)
    struct DirFcp df_fcp = {
        .fid = user_df_fid,
        .life_cycle_state = 0x03,
        .efarr_fid = user_efarr_fid,
        .efarr_line_number = 0x01,
    };
    do_create_df(fd, &df_fcp);

    // create_efarr_file_user(service, user_id)
    create_user_efarr_file(fd);

    // create_kf_pin_file_user(service, user_id, user_password_test[user_id])
    create_kf_pin_file(fd, passwd, passwd_size);

    // activate_df_directory_user(service, user_id)
    set_file_lcs_to_use(fd, user_df_fid);

    return 0;
}

int unregister_user(int fd, int user_id, uint8_t* passwd, size_t passwd_size) {
    if (!is_user_id_ok(user_id)) {
        printf("user id is not OK\n");
        return 1;
    }

    // user_df_fid = [0xDF, user_id]
    configure_user(user_id);

    // check if current user id is already registered
    bool user_exist_flag = do_check_fid_existence(fd, user_df_fid);
    printf("unregister_user(), user_exist_flag = %d\n", user_exist_flag);

    if (!user_exist_flag) {
        printf("current user id is not registered\n");
        return 1;
    }

    do_verify_pin(fd, user_key_id, passwd, passwd_size);

    do_select_fid_no_rsp(fd, user_df_fid);
    do_delete_current_fid(fd);

    return 0;
}

static int open_user_df(int fd, int user_id) {
    configure_user(user_id);

    do_select_fid_no_rsp(fd, user_df_fid);

    return 0;
}

int create_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t data_size) {

    configure_user(user_id);
    configure_bf_fid(user_id, file_id);

    printf("create_bf_file, run open_user_df\n");
    open_user_df(fd, user_df_fid);

    printf("create_bf_file, run do_verify_pin\n");
    int ret = do_verify_pin(fd, user_key_id, passwd, passwd_size);
    if (ret != 0) {
        printf("password verification failed\n");
        return -1;
    }

    printf("create_bf_file, run do_check_fid_existence\n");
    bool bf_exist_flag = do_check_fid_existence(fd, user_bf_fid);
    printf("create_bf_file(), bf_exist_flag = %d\n", bf_exist_flag);

    if (bf_exist_flag) {
        printf("current file id is already created\n");
        return 1;
    }

    create_bf_file_impl(fd, data, data_size);

    return do_write_data_into_bf_file(fd, user_bf_fid, data, data_size);;
}

int read_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size, uint8_t* data, size_t* data_size) {
    configure_user(user_id);
    configure_bf_fid(user_id, file_id);
    
    printf("read_bf_file, run open_user_df\n");
    open_user_df(fd, user_df_fid);

    printf("read_bf_file, run do_check_fid_existence\n");
    bool bf_exist_flag = do_check_fid_existence(fd, user_bf_fid);
    printf("create_bf_file(), bf_exist_flag = %d\n", bf_exist_flag);

    if (!bf_exist_flag) {
        printf("current file id is not created\n");
        return 1;
    }

    printf("read_bf_file, run do_verify_pin\n");
    int ret = do_verify_pin(fd, user_key_id, passwd, passwd_size);
    if (ret != 0) {
        printf("password verification failed\n");
        return -1;
    }

    do_read_data_from_bf_file(fd, user_bf_fid, data, data_size);

    return 0;
}

int delete_bf_file(int fd, int user_id, int file_id, uint8_t* passwd, size_t passwd_size) {
    configure_user(user_id);
    configure_bf_fid(user_id, file_id);

    open_user_df(fd, user_df_fid);

    int ret = do_verify_pin(fd, user_key_id, passwd, passwd_size);
    if (ret != 0) {
        printf("password verification failed\n");
        return -1;
    }

    printf("read_bf_file, run do_check_fid_existence\n");
    bool bf_exist_flag = do_check_fid_existence(fd, user_bf_fid);
    printf("delete_bf_file(), bf_exist_flag = %d\n", bf_exist_flag);

    if (!bf_exist_flag) {
        printf("current file id is not created\n");
        return 1;
    }

    do_select_fid_no_rsp(fd, user_bf_fid);
    do_delete_current_fid(fd);

    return 0;
}

