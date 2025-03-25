#include <at_commands.h>
#include <str_utils.h>

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // write(), read(), close()
#include <asm/termbits.h>  /* Definition of constants */
#include <sys/ioctl.h>

#include <apdu_commands.h>
#include <apdu_response_parser.h>
#include <fcp_parser.h>
#include <uicc_common.h>

extern int* crypto_app_session_id;
extern bool read_output_flag;
extern bool print_out_flag;

#define BUFFER_SIZE 4096
#define APDU_BUFFER_SIZE 512
#define FCP_BUFFER_SIZE 512
#define BINARY_FILE_CHUNK 2

const int num_response_lines_sim_exist = 4;
const int num_response_lines_open_session_cmd = 4;
const int num_response_lines_close_session_cmd = 2;
const int num_response_lines_select_aid_cmd = 6;
const int num_response_lines_get_challenge_cmd = 6;
const int num_response_lines_select_fid_cmd = 6;
const int num_response_lines_cgla_cmd = 6;

static size_t buffer_in_size;
static char buffer_in [BUFFER_SIZE];
static size_t buffer_out_size;
static char buffer_out [BUFFER_SIZE];

static size_t apdu_size = 0;
static char apdu [APDU_BUFFER_SIZE];

static struct ApduRsp apdu_rsp;
// static size_t apdu_rsp_size = 0;
// static char apdu_rsp [APDU_BUFFER_SIZE];

// tmp
static const char crypto_aid[] = "F04D4552499301";
static const char apdu_select_crypto_aid[] = "00A4040007F04D4552499301";
static const char apdu_get_challange_4_bytes[] = "0084000004";

char ERROR_STR[] = "ERROR";

void clear_buffer_in() {
    buffer_in_size = 0;
    memset(buffer_in, 0, BUFFER_SIZE);
}

void clear_buffer_out() {
    buffer_out_size = 0;
    memset(buffer_out, 0, BUFFER_SIZE);
}

void clear_apdu() {
    apdu_size = 0;
    memset(apdu, 0, APDU_BUFFER_SIZE);
}

// void clear_apdu_rsp() {
//     apdu_rsp_size = 0;
//     memset(apdu_rsp, 0, APDU_BUFFER_SIZE);
// }

static void send_at_copmmand(int fd, char* cmd, size_t size) {
    write(fd, cmd, size);
}

static void get_at_cmd_sim_exist(char* msg, size_t* size) {
    size_t offset = 0;
    char cmd[] = "AT+ESIMEXIST?";
    memcpy(msg + offset, cmd, sizeof(cmd)-1);
    offset += sizeof(cmd)-1;

    msg[offset+1] = '\0';
    *size = offset;
}

static void get_at_cmd_open_logical_channel(char* msg, size_t* size, const char* aid, size_t aid_size) {
    size_t offset = 0;
    char cmd1[] = "AT+CCHO=\"";
    memcpy(msg + offset, cmd1, sizeof(cmd1)-1);
    offset += sizeof(cmd1)-1;
    // printf("msg = '%s', offset = %zu\n", msg, offset);

    memcpy(msg + offset, aid, aid_size);
    offset += aid_size;
    // printf("aid = '%s', aid_size = %zu\n", aid, aid_size);
    // printf("msg = '%s', offset = %zu\n", msg, offset);

    msg[offset++] = '\"';
    // printf("msg = '%s', offset = %zu\n", msg, offset);
    
    msg[offset+1] = '\0';
    *size = offset;
}

static void get_at_cmd_close_logical_channel(char* msg, size_t* size) {
    // if (*session_id != -1) {
    //     printf("Logical channel is already opened");
    //     return *session_id;
    // }

    size_t offset = 0;
    char cmd1[] = "AT+CCHC=1";
    memcpy(msg + offset, cmd1, sizeof(cmd1)-1);
    offset += sizeof(cmd1)-1;
    // printf("msg = '%s', offset = %zu\n", msg, offset);

    msg[offset] = '\0';
    *size = offset;

    // memcpy(buffer_in + offset, dfname, name_size);
    // offset += name_size;
    // printf("buffer_in = '%s', offset = %zu\n", buffer_in, offset);

    // char cmd2[] = "\"";
    // memcpy(buffer_in + offset, cmd2, sizeof(cmd2) - 1);
    // offset += sizeof(cmd2) - 1;
    // printf("buffer_in = '%s', offset = %zu\n", buffer_in, offset);

    // printf("\nbuffer_in = '%s'\n", buffer_in);
    // {
    //     size_t buf_size = strlen(buffer_in);
    //     printf("received message: '%s', size = %zu\n", buffer_in, buf_size);
    //     printf("buffer_in:\n");
    //     for (size_t i = 0; i < buf_size; i++) {
    //         printf("'%d' - '%c'\n", buffer_in[i], buffer_in[i]);
    //     }
    // }

    // send_message(fd, buffer_in, offset);
}

static void get_at_cmd_send_apdu(char* msg, size_t* size, int session_id, const char* apdu, size_t apdu_size) {
    size_t offset = 0;
    char cmd1[] = "AT+CGLA=";
    memcpy(msg + offset, cmd1, sizeof(cmd1)-1);
    offset += sizeof(cmd1)-1;
    // printf("add '%s'\n", cmd1);
    // printf("msg = '%s', offset = %zu\n", msg, offset);

    append_number(msg, &offset, session_id);
    // printf("msg = '%s', offset = %zu\n", msg, offset);
    msg[offset] = ',';
    offset++;

    append_number(msg, &offset, apdu_size);
    // printf("msg = '%s', offset = %zu\n", msg, offset);
    msg[offset] = ',';
    offset++;

    msg[offset] = '\"';
    offset++;
    memcpy(msg + offset, apdu, apdu_size);
    // printf("msg = '%s', offset = %zu\n", msg, offset);
    offset += apdu_size;

    msg[offset++] = '\"';
    msg[offset] = '\0';
    *size = offset;
}

static void receive_responcefrom_open_logical_channel_cmd(int fd, char* msg, size_t* size) {
    // if (*session_id != -1) {
    //     printf("Logical channel is already opened");
    //     return *session_id;
    // }

    size_t offset = 0;
    char cmd1[] = "AT+CCHC=1";
    memcpy(msg + offset, cmd1, sizeof(cmd1)-1);
    offset += sizeof(cmd1)-1;
    // printf("msg = '%s', offset = %zu\n", msg, offset);

    msg[offset] = '\0';
    *size = offset;

    // memcpy(buffer_in + offset, dfname, name_size);
    // offset += name_size;
    // printf("buffer_in = '%s', offset = %zu\n", buffer_in, offset);

    // char cmd2[] = "\"";
    // memcpy(buffer_in + offset, cmd2, sizeof(cmd2) - 1);
    // offset += sizeof(cmd2) - 1;
    // printf("buffer_in = '%s', offset = %zu\n", buffer_in, offset);

    // printf("\nbuffer_in = '%s'\n", buffer_in);
    // {
    //     size_t buf_size = strlen(buffer_in);
    //     printf("received message: '%s', size = %zu\n", buffer_in, buf_size);
    //     printf("buffer_in:\n");
    //     for (size_t i = 0; i < buf_size; i++) {
    //         printf("'%d' - '%c'\n", buffer_in[i], buffer_in[i]);
    //     }
    // }

    // send_message(fd, buffer_in, offset);
}

static void add_fcp_tag(uint8_t* fcp, size_t* fcp_size, uint8_t tag, size_t num_bytes, uint32_t value) {
    fcp[(*fcp_size)++] = tag;
    fcp[(*fcp_size)++] = num_bytes;
    if (num_bytes > 3) {
        fcp[(*fcp_size)++] = value / 256 / 256 / 256;
    }
    if (num_bytes > 2) {
        fcp[(*fcp_size)++] = value / 256 / 256;
    }
    if (num_bytes > 1) {
        fcp[(*fcp_size)++] = value / 256;
    }
    fcp[(*fcp_size)++] = value % 256;
}

at_cmd_status_t send_message(int fd, char* msg, size_t size) {
    if (BUFFER_SIZE <= size) {
        printf("Message size is to long\n");
        return AT_CMD_MES_TOO_LONG;
    }

    if (buffer_in != msg) {
        clear_buffer_in();
        memcpy(buffer_in, msg, size);
    } else {
        memset(buffer_in + size, 0, BUFFER_SIZE - size);
    }

    buffer_in[size] = '\r';
    buffer_in[size+1] = '\0';

    if (print_out_flag) {
        printf("buffer_in:\n");
        for (size_t i = 0; i < size+2; i++) {
            printf("'%d' - '%c'\n", buffer_in[i], buffer_in[i]);
        }
    }

    printf("send message: '%s'\n", msg);
    write(fd, buffer_in, size+1);

    return AT_CMD_OK;
}

static int count_lines(char* msg, size_t size) {
    // printf("count_lines - START\n");
    // printf("size = %zu, msg = '%s'\n", size, msg);
    int cntr = 0;
    for (size_t i = 0; i < size; i++) {
        if (i > 0 && msg[i-1] == '\r' && msg[i] == '\n') {
            cntr++;
        }
    }
    return cntr;
}

static at_cmd_status_t read_message(int fd, char* msg, size_t* size, int num_lines_ref) {
    printf("read_message - START\n");
    // printf("read_output_flag = %d\n", read_output_flag);

    if (!read_output_flag) {
        return AT_CMD_OK;
    }

    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME
    // int n = read(fd, &buffer_out, sizeof(buffer_out));
    int read_bytes = 0;
    clear_buffer_out();

    // int nread1;
    // ioctl(fd, FIONREAD, &nread1);
    // printf("before 'read', nread1 = %d\n", nread1);

    bool read_flag = true;
    while (read_flag) {
        // printf("&buffer_out[0] = %p\n", &buffer_out);
        // printf("&buffer_out[0] + read_bytes = %p\n", &buffer_out[0] + read_bytes);
        // read_bytes += read(fd, &buffer_out + read_bytes, sizeof(buffer_out));
        // printf("run read()\n");
        ssize_t n = read(fd, &buffer_out[0] + read_bytes, sizeof(buffer_out));
        if (n == -1) {
            printf("Error during read procedure: error = %i (%s)\n", errno, strerror(errno));
            return AT_CMD_CONNECTION_LOST;
        } else if (n == 0) {
            printf("0 bytes read, it seems that connection is lost\n");
            return AT_CMD_CONNECTION_LOST;
        }
        // printf("n = %zd\n", n);
        read_bytes += n;
        // printf("run count_lines()\n");
        int line_num = count_lines(buffer_out, read_bytes);
        if (print_out_flag) {
            printf("line_num = %d, num_lines_ref = %d\n", line_num, num_lines_ref);
        }

        read_flag = !(num_lines_ref == line_num);
        // printf("check_if_contains()\n");
        if (check_if_contains(buffer_out, read_bytes, ERROR_STR, 5)) {
            read_flag = false;
        }
        // printf("while loop again\n");

        if (print_out_flag) {
            printf("buffer_out = '%s', read_bytes = %d\n", buffer_out, read_bytes);
            for (size_t i = 0; i < strlen(buffer_out); i++) {
                printf("'%d' - '%c'\n", buffer_out[i], buffer_out[i]);
            }
        }
    }

    int nread;
    ioctl(fd, FIONREAD, &nread);
    if (nread > 0) {
        ssize_t n = read(fd, &buffer_out[0], sizeof(buffer_out));
        printf("n = %zd\n", n);
        printf("buffer_out = '%s'\n", buffer_out);
    }

    size_t buf_size = strlen(buffer_out);
    char symbols[] = {};
    remove_symb(buffer_out, &buf_size, '\r');
    remove_symb_from_start(buffer_out, &buf_size, '\n');
    remove_symb_from_end(buffer_out, &buf_size, '\n');
    // printf("buffer_out = '%s'\n", buffer_out);
    // printf("buffer_out:\n");
    // for (size_t i = 0; i < buf_size; i++) {
    //     printf("'%d' - '%c'\n", buffer_out[i], buffer_out[i]);
    // }

    *size = buf_size;
    strncpy(msg, buffer_out, *size);

    return AT_CMD_OK;
}

int get_at_cmd_open_logical_channel_crypto_app(char* msg, size_t* size) {
    if (*crypto_app_session_id == -1) {
        get_at_cmd_open_logical_channel(msg, size, crypto_aid, strlen(crypto_aid));
    } else {
        printf("logical session with crypto app is already opened\n");
    }

    return 0;
}

int get_at_cmd_close_logical_channel_crypto_app(char* msg, size_t* size) {
    get_at_cmd_close_logical_channel(msg, size);
    return 0;
}

// int open_logical_channel_crypto_app(int fd) {

//     printf("open_logical_channel_crypto_app - START\n");
//     printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

//     size_t size = 0;
//     if (*crypto_app_session_id == -1) {
//         clear_buffer_in();
//         get_at_cmd_open_logical_channel(buffer_in, &size, crypto_aid, sizeof(crypto_aid)-1);
//     } else {
//         printf("logical channel with crypto app is already opened\n");
//         return 1;
//     }

//     printf("open logical channel, command = '%s', size = %zu\n", buffer_in, size);
//     send_message(fd, buffer_in, size);

//     return 0;
// }

int close_logical_channel_crypto_app(int fd) {

    printf("close_logical_channel_crypto_app - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    size_t size = 0;
    if (*crypto_app_session_id != -1) {
        clear_buffer_in();
        get_at_cmd_close_logical_channel(buffer_in, &size);
    } else {
        printf("logical channel with crypto app is not opened\n");
        return 1;
    }

    printf("close logical channel, command = '%s', size = %zu\n", buffer_in, size);
    send_message(fd, buffer_in, size);

    return 0;
}

int open_logical_channel_crypto_app_new(int fd) {

    printf("open_logical_channel_crypto_app_new - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    size_t size = 0;
    if (*crypto_app_session_id == -1) {
        clear_buffer_in();
        get_at_cmd_open_logical_channel(buffer_in, &size, crypto_aid, strlen(crypto_aid));
    } else {
        printf("logical channel with crypto app is already opened\n");
        return 1;
    }

    // send data
    printf("open logical channel, command = '%s', size = %zu\n", buffer_in, size);
    send_message(fd, buffer_in, size);

    // receive response
    receive_responce_from_open_logical_channel_crypto_app(fd);

    return 0;
}

int close_logical_channel_crypto_app_new(int fd) {
    printf("close_logical_channel_crypto_app - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    size_t size = 0;
    if (*crypto_app_session_id != -1) {
        clear_buffer_in();
        get_at_cmd_close_logical_channel(buffer_in, &size);
    } else {
        printf("logical channel with crypto app is not opened\n");
        return 1;
    }

    // send data
    printf("close logical channel, command = '%s', size = %zu\n", buffer_in, size);
    send_message(fd, buffer_in, size);

    // receive response
    receive_responce_from_close_logical_channel_crypto_app(fd);

    return 0;
}

int receive_responce_from_check_sim_exist(int fd) {

    printf("receive_responce_from_check_sim_exist - START\n");

    size_t size = 0;
    read_message(fd, buffer_out, &size, num_response_lines_sim_exist);
    printf("received response on sim exist cmd = '%s'\n", buffer_out);

    int sim_exist_rsp = 0;
    get_esimexist_response(buffer_out, size, &sim_exist_rsp);
    printf("sim_exist_rsp = %d\n", sim_exist_rsp);

    return sim_exist_rsp;
}

int receive_responce_from_open_logical_channel_crypto_app(int fd) {

    printf("receive_responce_from_open_logical_channel_crypto_app - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    if (*crypto_app_session_id != -1) {
        return 0;
    }

    size_t size = 0;
    read_message(fd, buffer_out, &size, num_response_lines_open_session_cmd);

    printf("received response on open session cmd = '%s'\n", buffer_out);
    if (strncmp(buffer_out, ERROR_STR, 5) != 0) {
        size_t offset = 0;
        int number = 0;
        for (size_t i = 0; i < size; i++) {
            if (is_digit(buffer_out[i])) {
                number = 10 * number + get_digit(buffer_out[i]);
            } else {
                break;
            }
        }
        *crypto_app_session_id = number;
        printf("number = %d, crypto_app_session_id = %d\n", number, *crypto_app_session_id);
    }

    return 0;
}

int receive_responce_from_close_logical_channel_crypto_app(int fd) {
    
    printf("receive_responce_from_close_logical_channel_crypto_app - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    if (*crypto_app_session_id == -1) {
        return 0;
    }

    size_t size = 0;
    read_message(fd, buffer_out, &size, num_response_lines_close_session_cmd);

    printf("received response on close session cmd = '%s'\n", buffer_out);
    if (strncmp(buffer_out, "OK", 2) == 0) {
        *crypto_app_session_id = -1;
    } else {
        printf("Unknown response = '%s'\n", buffer_out);
    }

    return 0;
}

int receive_responce_from_select_aid(int fd) {
    
    printf("receive_responce_from_select_aid - START\n");
    printf("crypto_app_session_id = %d\n", *crypto_app_session_id);

    if (*crypto_app_session_id == -1) {
        return 0;
    }

    size_t size = 0;
    read_message(fd, buffer_out, &size, num_response_lines_select_aid_cmd);

    printf("received response on select aid cmd = '%s', size = %zu\n", buffer_out, size);

    return 0;
}

int receive_response_from_cgla(int fd, int num_lines) {

    size_t size = 0;
    read_message(fd, buffer_out, &size, num_lines);

    printf("received response on cgla cmd = '%s', size = %zu\n", buffer_out, size);

    // clear_apdu_rsp();
    get_cgla_response(buffer_out, size, &apdu_rsp);
    // printf("apdu_rsp = '%s', apdu_rsp_size = %zu\n", apdu_rsp, apdu_rsp_size);

    // int sw1 = get_int_from_hex(&apdu_rsp[0] + apdu_rsp_size-4, &apdu_rsp[0] + apdu_rsp_size-2);
    // int sw2 = get_int_from_hex(&apdu_rsp[0] + apdu_rsp_size-2, &apdu_rsp[0] + apdu_rsp_size);
    // *rand_num = get_int_from_hex(&apdu_rsp[0], &apdu_rsp[0] + apdu_rsp_size-4);
    // printf("sw1 = %d, sw2 = %d, rand_num = %d\n", sw1, sw2, *rand_num);

    return 0;
}

int receive_responce_from_get_challenge(int fd, int* rand_num) {
    printf("receive_responce_from_get_challenge - START\n");

    receive_response_from_cgla(fd, num_response_lines_get_challenge_cmd);
    // size_t size = 0;
    // read_message(fd, buffer_out, &size, num_response_lines_get_challenge_cmd);

    // printf("received response on get challenge cmd = '%s', size = %zu\n", buffer_out, size);

    // size_t apdu_rsp_size = 0;
    // clear_apdu_rsp();
    // get_cgla_response(buffer_out, size, apdu_rsp, &apdu_rsp_size);
    // printf("apdu_rsp = '%s', apdu_rsp_size = %zu\n", apdu_rsp, apdu_rsp_size);
    
    // !is_rsp_status_ok()

    int sw1 = apdu_rsp.sw1;
    int sw2 = apdu_rsp.sw2;
    // *rand_num = *((int*)apdu_rsp.rsp);
    *rand_num = 16777216 * apdu_rsp.rsp[0] + 65536 * apdu_rsp.rsp[1] + 256 * apdu_rsp.rsp[2] + apdu_rsp.rsp[3];
    printf("sw1 = %d, sw2 = %d, rand_num = %d\n", sw1, sw2, *rand_num);

    return 0;
}

int check_se_existance(int fd) {
    printf("check_se_existance - START\n");
    
    size_t size = 0;
    clear_buffer_in();
    get_at_cmd_sim_exist(buffer_in, &size);
    
    // send data
    printf("check if sim exists, command = '%s', size = %zu\n", buffer_in, size);
    send_message(fd, buffer_in, size);
    
    // receive response
    int sim_exist_flag = receive_responce_from_check_sim_exist(fd);
    
    return sim_exist_flag;
}

bool get_se_exist_flag(int fd) {
    printf("get_se_exist_flag - START\n");
    
    size_t size = 0;
    clear_buffer_in();
    get_at_cmd_sim_exist(buffer_in, &size);
    
    // send data
    printf("check if sim exists, command = '%s', size = %zu\n", buffer_in, size);
    send_message(fd, buffer_in, size);
    
    // receive response
    return (bool)receive_responce_from_check_sim_exist(fd);;
}

at_cmd_status_t do_select_crypto_aid(int fd) {
    if (*crypto_app_session_id == -1) {
        printf("Logical channel with crypto app is not opened\n");
        return AT_CMD_NO_LOG_CHANNEL;
    }
    at_cmd_status_t status;

    clear_apdu();
    get_apdu_select_crypto_app(apdu, &apdu_size);
    printf("do_select_crypto_aid, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    status = do_send_apdu(fd, apdu, apdu_size);

    if (status != AT_CMD_OK) {
        return status;
    }
    // size_t size = 0;
    // printf("run get_at_cmd_send_apdu()\n");
    // clear_buffer_in();
    // get_at_cmd_send_apdu(buffer_in, &size, *crypto_app_session_id, apdu, apdu_size);

    // printf("select crypto aid, command = '%s', size = %zu\n", buffer_in, size);
    // send_message(fd, buffer_in, size);

    // receive_responce_from_select_aid(fd);

    size_t size = 0;
    clear_buffer_out();
    status = read_message(fd, buffer_out, &size, num_response_lines_select_aid_cmd);
    printf("received response on select aid cmd = '%s', size = %zu\n", buffer_out, size);

    return status;
}

int do_set_lcs_to_use_cur_file(int fd) {
    if (*crypto_app_session_id == -1) {
        printf("Logical channel with crypto app is not opened\n");
        return 1;
    }

    clear_apdu();
    get_apdu_set_lcs_use(apdu, &apdu_size);
    printf("do_set_lcs_to_use_cur_file, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    return 0;
}

int do_get_random_number(int fd, int* rand_num) {
    if (*crypto_app_session_id == -1) {
        printf("Logical channel with crypto app is not opened\n");
        return 1;
    }

    // // select Crypto by AID
    // {
    //     size_t apdu_size = 0;
    //     get_apdu_select_crypto_app(apdu, &apdu_size);
    //     printf("get_random_number, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    //     size_t size = 0;
    //     printf("run get_at_cmd_send_apdu()\n");
    //     clear_buffer_in();
    //     get_at_cmd_send_apdu(buffer_in, &size, *crypto_app_session_id, apdu, apdu_size);

    //     printf("select crypto aid, command = '%s', size = %zu\n", buffer_in, size);
    //     send_message(fd, buffer_in, size);

    //     receive_responce_from_select_aid(fd);
    // }

    // Get challenge with 4 random bytes
    {
        clear_apdu();
        get_apdu_get_challenge_4_bytes(apdu, &apdu_size);
        printf("do_get_random_number, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);
        
        do_send_apdu(fd, apdu, apdu_size);
        // size_t size = 0;
        // printf("run get_at_cmd_send_apdu()\n");
        // clear_buffer_in();
        // get_at_cmd_send_apdu(buffer_in, &size, *crypto_app_session_id, apdu, apdu_size);

        // printf("get challenge with 4 random bytes, command = '%s', size = %zu\n", buffer_in, size);
        // send_message(fd, buffer_in, size);

        receive_responce_from_get_challenge(fd, rand_num);
    }

    return 0;
}

// int do_send_apdu(int fd, char* apdu, size_t apdu_size, char* rsp, size_t* size) {
at_cmd_status_t do_send_apdu(int fd, char* apdu, size_t apdu_size) {
    size_t size = 0;
    clear_buffer_in();
    get_at_cmd_send_apdu(buffer_in, &size, *crypto_app_session_id, apdu, apdu_size);

    printf("do_send_apdu, command = '%s', size = %zu\n", buffer_in, size);
    return send_message(fd, buffer_in, size);
}

int do_select_fid_no_rsp(int fd, uint16_t fid) {
    clear_apdu();
    get_apdu_select_fid_no_rsp(apdu, &apdu_size, fid);
    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    return 0;
}

int do_select_fid_fcp(int fd, uint16_t fid) {
    clear_apdu();
    get_apdu_select_fid_fcp(apdu, &apdu_size, fid);
    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    return 0;
}

bool do_check_fid_existence(int fd, uint16_t fid) {
    do_select_fid_no_rsp(fd, fid);
    print_out_rsp(&apdu_rsp);
    // printf("do_check_fid_existence(), apdu_rsp = '%s', apdu_rsp_size = %zu\n", apdu_rsp, apdu_rsp_size);

    return is_rsp_status_ok(&apdu_rsp);
}

int do_create_df(int fd, const struct DirFcp* fcp_dscr) {

    size_t fcp_size = 0;
    uint8_t fcp[FCP_BUFFER_SIZE];

    fcp[fcp_size++] = 0x62;
    fcp[fcp_size++] = 0x10;

    // add file type
    fcp[fcp_size++] = 0x82;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = 0x38;
    fcp[fcp_size++] = 0x21;

    // add FID
    uint16_t fid = fcp_dscr->fid;
    fcp[fcp_size++] = 0x83;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = fid / 256;
    fcp[fcp_size++] = fid % 256;

    // add life cycle state as 'Init'
    uint8_t life_cycle_state = fcp_dscr->life_cycle_state;
    fcp[fcp_size++] = 0x8A;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = life_cycle_state;

    // add access attribute file reference
    uint16_t efarr_fid = fcp_dscr->efarr_fid;
    uint8_t efarr_line_number = fcp_dscr->efarr_line_number;
    fcp[fcp_size++] = 0x8B;
    fcp[fcp_size++] = 0x03;
    fcp[fcp_size++] = efarr_fid / 256;
    fcp[fcp_size++] = efarr_fid % 256;
    fcp[fcp_size++] = efarr_line_number;

    // 00E00000126210820238218302DF018A01038B03EF0101
    // 00840000126210820238218302AABB8A01038B03EFAB01

    clear_apdu();
    get_apdu_create_file(apdu, &apdu_size, fcp, fcp_size);
    printf("do_create_df, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    return 0;
}

int do_delete_current_fid(int fd) {

    clear_apdu();
    get_apdu_delete_current_file(apdu, &apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    return 0;
}

int do_create_efarr_file(int fd, const struct EfarrDscr* dscr) {
    size_t fcp_size = 0;
    uint8_t fcp[FCP_BUFFER_SIZE];

    fcp[fcp_size++] = 0x62;
    fcp[fcp_size++] = 0x19;

    // add file size
    uint16_t file_size = dscr->file_size;
    fcp[fcp_size++] = 0x80;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = file_size / 256;
    fcp[fcp_size++] = file_size % 2256;

    // add file type
    uint8_t record_size = dscr->record_size;
    fcp[fcp_size++] = 0x82;
    fcp[fcp_size++] = 0x04;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = 0x21;
    fcp[fcp_size++] = 0x00;
    fcp[fcp_size++] = record_size;

    // add FID
    uint16_t fid = dscr->fid;
    fcp[fcp_size++] = 0x83;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = fid / 256;
    fcp[fcp_size++] = fid % 256;

    // add SFI
    uint8_t sfi = dscr->sfi;
    fcp[fcp_size++] = 0x88;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = sfi;

    // add life cycle state as 'Init'
    uint8_t life_cycle_state = dscr->life_cycle_state;
    fcp[fcp_size++] = 0x8A;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = life_cycle_state;

    // add access attribute file reference
    uint16_t efarr_fid = dscr->efarr_fid;
    uint8_t efarr_line_number = dscr->efarr_line_number;
    fcp[fcp_size++] = 0x8B;
    fcp[fcp_size++] = 0x03;
    fcp[fcp_size++] = efarr_fid / 256;
    fcp[fcp_size++] = efarr_fid % 256;
    fcp[fcp_size++] = efarr_line_number;

    clear_apdu();
    get_apdu_create_file(apdu, &apdu_size, fcp, fcp_size);
    printf("do_create_efarr_file, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during EF.ARR file creation\n");
        return -1;
    }

    do_add_record_efarr_file(fd, fid, dscr->records, dscr->num_records, dscr->record_size);

    return 0;
}

int do_add_record_efarr_file(int fd, uint16_t fid, uint8_t* records, uint8_t num_records, uint8_t record_size) {
    clear_apdu();
    get_apdu_select_fid_no_rsp(apdu, &apdu_size, fid);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during file selection\n");
        return -1;
    }

    size_t offset = 0;
    for (size_t iRecord = 0; iRecord < num_records; iRecord++) {
        offset = iRecord * record_size;

        clear_apdu();
        get_apdu_append_record(apdu, &apdu_size, records + offset, record_size);

        do_send_apdu(fd, apdu, apdu_size);
        receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

        if (!is_rsp_status_ok(&apdu_rsp)) {
            printf("Error occured during APPEND RECORD command\n");
            return -1;
        }
    }

    return 0;
}

int do_create_kf_file(int fd, const struct KfDscr* dscr) {
    size_t fcp_size = 0;
    uint8_t fcp[FCP_BUFFER_SIZE];

    fcp[fcp_size++] = 0x62;
    fcp[fcp_size++] = 0x21;

    // add file type
    fcp[fcp_size++] = 0x82;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = 0x51;
    fcp[fcp_size++] = 0x21;

    // add FID
    uint16_t fid = dscr->fid;
    fcp[fcp_size++] = 0x83;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = fid / 256;
    fcp[fcp_size++] = fid % 256;

    // add life cycle state as 'Init'
    uint8_t life_cycle_state = dscr->life_cycle_state;
    fcp[fcp_size++] = 0x8A;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = life_cycle_state;

    // add access attribute file reference
    uint16_t efarr_fid = dscr->efarr_fid;
    uint8_t efarr_line_number = dscr->efarr_line_number;
    fcp[fcp_size++] = 0x8B;
    fcp[fcp_size++] = 0x03;
    fcp[fcp_size++] = efarr_fid / 256;
    fcp[fcp_size++] = efarr_fid % 256;
    fcp[fcp_size++] = efarr_line_number;

    // add A5 tag specific for KF
    fcp[fcp_size++] = 0xA5;
    fcp[fcp_size++] = 0x0F;

    {
        // add key algorithm
        uint8_t key_algo = dscr->key_algo;
        fcp[fcp_size++] = 0x85;
        fcp[fcp_size++] = 0x01;
        fcp[fcp_size++] = key_algo;

        // add key algorithm
        uint8_t key_purpose = dscr->key_purpose;
        fcp[fcp_size++] = 0x86 ;
        fcp[fcp_size++] = 0x02;
        fcp[fcp_size++] = key_purpose / 256;
        fcp[fcp_size++] = key_purpose % 256;

        // add key ID
        uint8_t key_id = dscr->key_id;
        fcp[fcp_size++] = 0x87;
        fcp[fcp_size++] = 0x01;
        fcp[fcp_size++] = key_id;

        // add KF flags
        uint8_t kf_flags = dscr->kf_flags;
        fcp[fcp_size++] = 0x8B;
        fcp[fcp_size++] = 0x03;
        fcp[fcp_size++] = 0x81;
        fcp[fcp_size++] = 0x01;
        fcp[fcp_size++] = kf_flags;
    }

    clear_apdu();
    get_apdu_create_file(apdu, &apdu_size, fcp, fcp_size);
    printf("do_create_kf_file, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during KF file creation\n");
        return -1;
    }

    do_change_ref_data_kf_file(fd, fid, dscr->passwd, dscr->passwd_size);

    return 0;
}

int do_change_ref_data_kf_file(int fd, uint16_t fid, uint8_t* passwd, size_t passwd_size) {
    clear_apdu();
    get_apdu_select_fid_no_rsp(apdu, &apdu_size, fid);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during file selection\n");
        return -1;
    }

    clear_apdu();
    get_apdu_change_refrence_data_pin(apdu, &apdu_size, passwd, passwd_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during change reference data in KF\n");
        return -1;
    }

    return 0;
}

int do_create_bf_file(int fd, const struct BfDscr* dscr) {
    size_t fcp_size = 0;
    uint8_t fcp[FCP_BUFFER_SIZE];

    fcp[fcp_size++] = 0x62;
    fcp[fcp_size++] = 0x14;

    // add file size
    uint16_t file_size = dscr->file_size;
    fcp[fcp_size++] = 0x80;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = file_size / 256;
    fcp[fcp_size++] = file_size % 256;

    // add file type
    fcp[fcp_size++] = 0x82;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = 0x21;

    // add FID
    uint16_t fid = dscr->fid;
    fcp[fcp_size++] = 0x83;
    fcp[fcp_size++] = 0x02;
    fcp[fcp_size++] = fid / 256;
    fcp[fcp_size++] = fid % 256;

    // add life cycle state as 'Init'
    uint8_t life_cycle_state = dscr->life_cycle_state;
    fcp[fcp_size++] = 0x8A;
    fcp[fcp_size++] = 0x01;
    fcp[fcp_size++] = life_cycle_state;

    // add access attribute file reference
    uint16_t efarr_fid = dscr->efarr_fid;
    uint8_t efarr_line_number = dscr->efarr_line_number;
    fcp[fcp_size++] = 0x8B;
    fcp[fcp_size++] = 0x03;
    fcp[fcp_size++] = efarr_fid / 256;
    fcp[fcp_size++] = efarr_fid % 256;
    fcp[fcp_size++] = efarr_line_number;

    clear_apdu();
    get_apdu_create_file(apdu, &apdu_size, fcp, fcp_size);
    printf("do_create_bf_file, apdu = '%s', apdu_size = %zu\n", apdu, apdu_size);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during BF file creation\n");
        return -1;
    }

    do_write_data_into_bf_file(fd, fid, dscr->data, dscr->data_size);

    return 0;
}

int do_write_data_into_bf_file(int fd, uint16_t fid, uint8_t* data, size_t data_size) {
    clear_apdu();
    get_apdu_select_fid_no_rsp(apdu, &apdu_size, fid);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during file selection\n");
        return -1;
    }

    size_t offset = 0;
    while (offset < data_size) {
        size_t data_chunk_size = BINARY_FILE_CHUNK;
        if (data_chunk_size < data_size-offset) {
            data_chunk_size = data_size-offset;
        }

        clear_apdu();
        get_apdu_update_binary(apdu, &apdu_size, offset, data + offset, data_chunk_size);
        offset += data_chunk_size;

        do_send_apdu(fd, apdu, apdu_size);
        receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

        if (!is_rsp_status_ok(&apdu_rsp)) {
            printf("Error occured during binary update in BF\n");
            return -1;
        }
    }

    return 0;
}

int do_read_data_from_bf_file(int fd, uint16_t fid, uint8_t* data, size_t* data_size) {
    clear_apdu();
    get_apdu_select_fid_fcp(apdu, &apdu_size, fid);

    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during file selection\n");
        return -1;
    }
    
    struct FcpDscr fcp_dscr;
    config_fcp_dscr(&fcp_dscr, apdu_rsp.rsp, apdu_rsp.size);
    print_fcp_dscr(&fcp_dscr);
    *data_size = fcp_dscr.tag_0x80.value[0] * 256 + fcp_dscr.tag_0x80.value[1];
    free_fcp_dscr(&fcp_dscr);

    // uint16_t file_size;
    // int ret = get_file_size_from_fcp(apdu_rsp.rsp, apdu_rsp.size, &file_size);
    // printf("ret = %d, file_size = %u\n", ret, file_size);

    size_t offset = 0;
    // *data_size = file_size;
    while (offset < *data_size) {
        size_t data_chunk_size = BINARY_FILE_CHUNK;
        if (data_chunk_size < *data_size-offset) {
            data_chunk_size = *data_size-offset;
        }

        clear_apdu();
        get_apdu_read_binary(apdu, &apdu_size, offset, data_chunk_size);
        
        do_send_apdu(fd, apdu, apdu_size);
        receive_response_from_cgla(fd, num_response_lines_cgla_cmd);
        
        if (!is_rsp_status_ok(&apdu_rsp)) {
            printf("Error occured during binary update in BF\n");
            return -1;
        }
        
        memcpy(data, apdu_rsp.rsp, apdu_rsp.size);
        offset += data_chunk_size;
    }

    return 0;
}

int do_verify_pin(int fd, uint8_t key_id, uint8_t* passwd, size_t passwd_size) {
    clear_apdu();
    printf("run get_apdu_verify_pin\n");
    get_apdu_verify_pin(apdu, &apdu_size, key_id, passwd, passwd_size);

    printf("run do_send_apdu\n");
    do_send_apdu(fd, apdu, apdu_size);
    receive_response_from_cgla(fd, num_response_lines_cgla_cmd);

    if (!is_rsp_status_ok(&apdu_rsp)) {
        printf("Error occured during PIN verification\n");
        return -1;
    }
    
    return 0;
}

int do_print_all_files(int fd) {
    struct FcpDscr fcp_dscr;


    
    config_fcp_dscr(&fcp_dscr, apdu_rsp.rsp, apdu_rsp.size);
    print_fcp_dscr(&fcp_dscr);
    
    free_fcp_dscr(&fcp_dscr);

    return 0;
}