#ifndef _AT_COMMANDS_H_
#define _AT_COMMANDS_H_

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#include <fcp_parser.h>

struct DirFcp {
    uint16_t    fid;
    uint8_t     life_cycle_state;
    uint16_t    efarr_fid;
    uint8_t     efarr_line_number;
};

struct EfarrDscr {
    uint16_t    file_size;
    uint16_t    fid;
    uint8_t     sfi;
    uint8_t     life_cycle_state;
    uint16_t    efarr_fid;
    uint8_t     efarr_line_number;

    uint8_t     record_size;
    uint8_t     num_records;
    uint8_t*    records;
};

struct KfDscr {
    uint16_t    fid;
    uint8_t     life_cycle_state;
    uint16_t    efarr_fid;
    uint8_t     efarr_line_number;

    uint8_t     key_algo;
    uint16_t    key_purpose;
    uint8_t     key_id;
    uint8_t     kf_flags;

    uint8_t*    passwd;
    size_t      passwd_size;
};

struct BfDscr {
    uint16_t    file_size;
    uint16_t    fid;
    uint8_t     life_cycle_state;
    uint16_t    efarr_fid;
    uint8_t     efarr_line_number;

    uint8_t*    data;
    size_t      data_size;
};

typedef int at_cmd_status_t;

enum AT_CMD_STATUS {
    AT_CMD_OK = 0,
    AT_CMD_MES_TOO_LONG,
    AT_CMD_CONNECTION_ISSUE,
    AT_CMD_NO_LOG_CHANNEL,

    AT_CMD_ERROR,
};




// open logical channel accorfin to the section 8.45 from TS 27.007 v 15.2.0
int send_message(int fd, char* msg, size_t size);
// int read_mesage(int fd);

int get_at_cmd_open_logical_channel_crypto_app(char* msg, size_t* size);
int get_at_cmd_close_logical_channel_crypto_app(char* msg, size_t* size);

int open_logical_channel_crypto_app(int fd);
int close_logical_channel_crypto_app(int fd, bool force_flag);

int check_se_existance(int fd);
bool get_se_exist_flag(int fd);

at_cmd_status_t do_select_crypto_aid(int fd);
int do_set_lcs_to_use_cur_file(int fd);

int do_get_random_number(int fd, int* rand_num);

// int do_create_efarr_file(int fd, );

int receive_responce_from_open_logical_channel_crypto_app(int fd);
int receive_responce_from_close_logical_channel_crypto_app(int fd);

at_cmd_status_t do_send_apdu(int fd, char* apdu, size_t apdu_size);
int do_select_fid_no_rsp(int fd, uint16_t fid);
int do_select_fid_fcp(int fd, uint16_t fid);
int do_check_fid_existence(int fd, uint16_t fid, bool* flag);

int do_create_df(int fd, const struct DirFcp* fcp_dscr);
int do_delete_current_fid(int fd);

int do_create_efarr_file(int fd, const struct EfarrDscr* dscr);
int do_add_record_efarr_file(int fd, uint16_t fid, uint8_t* records, uint8_t num_records, uint8_t record_size);

int do_create_kf_file(int fd, const struct KfDscr* dscr);
int do_change_ref_data_kf_file(int fd, uint16_t fid, uint8_t* passwd, size_t passwd_size);

int do_create_bf_file(int fd, const struct BfDscr* dscr);
int do_write_data_into_bf_file(int fd, uint16_t fid, uint8_t* data, size_t data_size);
int do_read_data_from_bf_file(int fd, uint16_t fid, uint8_t* data, size_t* data_size);

int do_verify_pin(int fd, uint8_t key_id, uint8_t* passwd, size_t passwd_size);

int do_select_files_cur_dir(int fd, bool first_flag, struct FcpDscr* fcp_dscr);
int do_print_all_files_cur_dir(int fd);
// int do_print_files_cur_dir(int fd);

#endif // _AT_COMMANDS_H_

