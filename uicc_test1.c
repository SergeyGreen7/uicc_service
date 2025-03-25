#include <uicc_test.h>
#include <at_commands.h>
#include <str_utils.h>

// C librarary headers
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/mman.h>
#include <sys/wait.h>

#include "sec_storage.h" 


// #include <uicc_common.h>

#define BUFFER_SIZE 4096
#define RECONNECT_MAX_TRIAL 32
#define RECONNECT_DELAY_MS 200

// static char buffer_in [BUFFER_SIZE];
// static char buffer_out [BUFFER_SIZE];
static int serial_port = -1;

typedef int (*WriteFuncPtr)(int);
typedef int (*ReadFuncPtr)(int);

int *crypto_app_session_id = NULL;
bool read_output_flag = true;
bool print_out_flag = false;

bool is_connection_established() {
    return serial_port > 0;
}

// int send_message(char* msg, size_t size) {

//     pid_t pid;

//     if (BUFFER_SIZE <= size) {
//         perror("Message size is to long\n");
//         exit(1);
//     }
//     memcpy(buffer_in, msg, size);
//     buffer_in[size] = '\r';
//     buffer_in[size+1] = '\0';

//     printf("buffer_in:\n");
//     for (size_t i = 0; i < size+2; i++) {
//         printf("'%d' - '%c'\n", buffer_in[i], buffer_in[i]);
//     }

//     printf("send message: '%s'\n", msg);
//     write(serial_port, buffer_in, size+1);

//     return 0;
// }

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

static void read_message() {
    printf("read_message - START\n");
    // printf("read_output_flag = %d\n", read_output_flag);

    if (!read_output_flag) {
        return;
    }

    char buffer_out [BUFFER_SIZE];
    memset(buffer_out, 0, BUFFER_SIZE);

    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME
    int n = read(serial_port, &buffer_out, sizeof(buffer_out));

    // {
    //     size_t buf_size = strlen(buffer_out);
    //     printf("received message: '%s', size = %zu\n", buffer_out, buf_size);
    //     printf("buffer_out:\n");
    //     for (size_t i = 0; i < buf_size; i++) {
    //         printf("'%d' - '%c'\n", buffer_out[i], buffer_out[i]);
    //     }
    // }

    int num_lines = count_lines(buffer_out, n);

    size_t size = strlen(buffer_out);
    char symbols[] = {'\n', '\r'};
    remove_symbols(buffer_out, &size, symbols, sizeof(symbols));

    size_t buf_size = strlen(buffer_out);
    printf("number of lines = %d\n", num_lines);
    printf("received message origin: '%s'\n", buffer_out);

}

// static void func1() {
    
//     bool skip_at_cmd_flag = false;
//     bool custom_at_cmd_flag = false;
//     char cmd_input[1024];
//     char cmd[1024];
//     int rand_num;
//     size_t cmd_size = 0;
//     WriteFuncPtr tx_func;
//     ReadFuncPtr rx_func;
//     while (1) {
//         printf("please enter AT command:\n");
//         scanf("%s", cmd_input);
//         printf("input command: ='%s'\n", cmd_input);

//         skip_at_cmd_flag = false;
//         custom_at_cmd_flag = false;
//         if (strcmp(cmd_input, "0") == 0) {
//             printf("cmd_input == 0\n");
//             skip_at_cmd_flag = true;
//             printf("crypto_app_session_id = %d\n", *crypto_app_session_id);
//             get_random_number(serial_port, &rand_num);
//         } else if (strcmp(cmd_input, "1") == 0) {
//             printf("cmd_input == 1\n");
//             // get_at_cmd_open_logical_channel_crypto_app(cmd, &cmd_size);
//             tx_func = open_logical_channel_crypto_app;
//             rx_func = receive_responce_from_open_logical_channel_crypto_app;
//         } else if (strcmp(cmd_input, "2") == 0) {
//             printf("cmd_input == 2\n");
//             // get_at_cmd_close_logical_channel_crypto_app(cmd, &cmd_size);
//             tx_func = close_logical_channel_crypto_app;
//             rx_func = receive_responce_from_close_logical_channel_crypto_app;
//         } else if (strcmp(cmd_input, "exit") == 0) {
//             printf("exit command\n");
//             close(serial_port);
//             // kill(pid, SIGKILL);
//             break;
//         } else {
//             custom_at_cmd_flag = true;
//             cmd_size = strlen(cmd_input);
//             strcpy(cmd, cmd_input);
//         }
        
//         if (skip_at_cmd_flag) {
//             continue;
//         }

//         pid_t pid = fork();
//         if (pid < 0) {
//             perror("failed to fork()\n");
//             exit(1);
//         }

//         if (pid == 0) {
//             printf("UICC test - read thread - START\n");
//             // read_message();
//             if (custom_at_cmd_flag) {
//                 read_message();
//             } else {
//                 rx_func(serial_port);
//             }
//             printf("UICC test - read thread - FINISH\n");
//             exit(0);
//         } else {
//             printf("UICC test - write thread - START\n");
//             if (custom_at_cmd_flag) {
//                 printf("command = '%s', size = %zu\n", cmd, cmd_size);
//                 send_message(serial_port, cmd, cmd_size);
//             } else {
//                 tx_func(serial_port);
//             }
//             // open_logical_channel_crypto_app(serial_port);
//             // waitpid(pid, NULL, 0);
//             printf("UICC test - write thread - FINISH\n");
//         }
//         wait(NULL);
//     }
// }

static void func2() {
    time_t epoch = 0;
    time(&epoch);
    srand(epoch);

    bool custom_at_cmd_flag = false;
    char cmd_input[1024];
    char cmd[1024];
    int rand_num;
    size_t cmd_size = 0;
    // WriteFuncPtr tx_func;
    // ReadFuncPtr rx_func;

    int user_id = 4;
    int file_id = 3;
    uint8_t passwd[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    size_t  passwd_size = sizeof(passwd);

    uint8_t passwd_user[5][8] = {
        {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
        {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
        {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        {0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05},
    };
    size_t  passwd_size_user[5] = {8, 8, 8, 8, 8};

    uint8_t* data_write;
    size_t  data_size_write;
    uint8_t data_read[32000];
    size_t  data_size_read;

    while (1) {
        printf("please enter AT command:\n");
        scanf("%s", cmd_input);
        printf("input command: ='%s'\n", cmd_input);

        custom_at_cmd_flag = false;
        if (strcmp(cmd_input, "open_se") == 0) {
            printf("cmd_input == open_se\n");
            // printf("crypto_app_session_id = %d\n", *crypto_app_session_id);
            printf("run create_connection_se\n");
            create_connection_se();
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "close_se") == 0) {
            printf("cmd_input == close_se\n");
            // printf("crypto_app_session_id = %d\n", *crypto_app_session_id);
            printf("run close_connection_se\n");
            close_connection_se();
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "exists") == 0) {
            printf("cmd_input == exists\n");
            bool flag = is_se_exists();
            printf("se exist flag = %d\n", flag);
        } else if (strcmp(cmd_input, "open_crypto") == 0) {
            printf("cmd_input == open_crypto\n");
            open_log_channel_crypto();
            // printf("crypto_app_session_id = %d\n", *crypto_app_session_id);
            // get_random_number(serial_port, &rand_num);
            // printf("rand_num = %d\n", rand_num);
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "close_crypto") == 0) {
            printf("cmd_input == close_crypto\n");
            close_log_channel_crypto();
            // printf("crypto_app_session_id = %d\n", *crypto_app_session_id);
            // get_random_number(serial_port, &rand_num);
            // printf("rand_num = %d\n", rand_num);
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "select_crypto") == 0) {
            if (crypto_app_session_id != NULL && *crypto_app_session_id != 1) {
                *crypto_app_session_id = 1;
            }
            printf("cmd_input == select_crypto\n");
            int ret = select_crypto_aid();
        } else if (strcmp(cmd_input, "get_rand") == 0) {
            printf("cmd_input == get_rand\n");
            if (crypto_app_session_id != NULL && *crypto_app_session_id != 1) {
                *crypto_app_session_id = 1;
            }
            int ret = get_random_int(&rand_num);
            printf("rand_num = %d\n", rand_num);
        } else if (strcmp(cmd_input, "select_file") == 0) {
            printf("cmd_input == select_file\n");

            uint16_t fid = 0xEFAB;
            do_select_fid_no_rsp(serial_port, fid);
            do_select_fid_fcp(serial_port, fid);
        } else if (strcmp(cmd_input, "exist_file") == 0) {
            printf("cmd_input == exist_file\n");

            uint16_t fid = 0xEFAB;
            bool flag = do_check_fid_existence(serial_port, fid);
            printf("file exists = %d\n", flag);
        } else if (strcmp(cmd_input, "create_df") == 0) {
            printf("cmd_input == create_df\n");

            struct DirFcp df_fcp = {
                .fid = 0xAABB,
                .life_cycle_state = 0x03,
                .efarr_fid = 0xEFAB,
                .efarr_line_number = 0x01,
            };

            do_create_df(serial_port, &df_fcp);

        }  else if (strcmp(cmd_input, "delete_df") == 0) {
            printf("cmd_input == delete_df\n");

            uint16_t fid = 0xAABB;
            do_select_fid_no_rsp(serial_port, fid);
            do_delete_current_fid(serial_port);

        } else if (strcmp(cmd_input, "register_user") == 0) {
            printf("cmd_input == register_user\n");
            
            printf("run register_new_user\n");
            int ret = register_new_user(serial_port, user_id, passwd, passwd_size);

        } else if (strcmp(cmd_input, "unregister_user") == 0) {
            printf("cmd_input == unregister_user\n");
            
            printf("run unregister_user\n");
            int ret = unregister_user(serial_port, user_id, passwd, passwd_size);

        } else if (strcmp(cmd_input, "create_bf") == 0) {
            printf("cmd_input == create_bf\n");

            data_size_write = rand() % 100;
            data_write = malloc(data_size_write);
            for (int i = 0; i < data_size_write; i++) {
                data_write[i] = rand() % 256;
            }
            printf("run unregister_user\n");
            int ret = create_bf_file(serial_port, user_id, file_id, passwd, passwd_size, data_write, data_size_write);
            
            printf("data saved into file: data_size_write = %zu\n", data_size_write);
            for (size_t i = 0; i < data_size_write; i++) {
                printf("data_write[%zu] = %u\n", i, data_write[i]);
            }

            free(data_write);
        } else if (strcmp(cmd_input, "read_bf") == 0) {
            printf("cmd_input == read_bf\n");
            
            printf("run unregister_user\n");
            int ret = read_bf_file(serial_port, user_id, file_id, passwd, passwd_size, data_read, &data_size_read);

            printf("data_size_read = %zu\n", data_size_read);
            for (size_t i = 0; i < data_size_read; i++) {
                printf("data_read[%zu] = %u\n", i, data_read[i]);
            }
        } else if (strcmp(cmd_input, "delete_bf") == 0) {
            printf("cmd_input == delete_bf\n");
            
            printf("run delete_bf_file\n");
            int ret = delete_bf_file(serial_port, user_id, file_id, passwd, passwd_size);
        }
        else if (strcmp(cmd_input, "2") == 0) {
            printf("cmd_input == 2\n");
            // get_at_cmd_close_logical_channel_crypto_app(cmd, &cmd_size);
            // tx_func = close_logical_channel_crypto_app_new;
            // rx_func = receive_responce_from_close_logical_channel_crypto_app;
        } else if (strcmp(cmd_input, "3") == 0) {
            printf("cmd_input == 3\n");
            check_se_existance(serial_port);
        } else if (strcmp(cmd_input, "exit") == 0) {
            printf("exit command\n");
            close(serial_port);
            // kill(pid, SIGKILL);
            break;
        } else {
            custom_at_cmd_flag = true;
            cmd_size = strlen(cmd_input);
            strcpy(cmd, cmd_input);
        }

        // printf("UICC test - write thread - START\n");
        if (custom_at_cmd_flag) {
            if (serial_port == -1) {
                printf("Connection with SE is not established, please run 'open' command\n");
                continue;
            }
            printf("command = '%s', size = %zu\n", cmd, cmd_size);
            send_message(serial_port, cmd, cmd_size);
            read_message();
        }
        // open_logical_channel_crypto_app(serial_port);
        // waitpid(pid, NULL, 0);
        // printf("UICC test - write thread - FINISH\n");

        // // if (pid == 0) {
        // printf("UICC test - read thread - START\n");
        // // read_message();
        // if (custom_at_cmd_flag) {
        //     read_message();
        // } else {
        //     rx_func(serial_port);
        // }
        // printf("UICC test - read thread - FINISH\n");
    }

    if (serial_port != -1) {
        printf("Connection with SE is still established, run 'close_connection_se' command\n");
        close_connection_se();
    }
}

static int connect_to_se() {
    printf("connect_to_se - START\n");

    // Basic Setup In C
    char name[] = "/dev/radio/atci1";

    int reconnect_cnrt = 0;
    while (reconnect_cnrt < RECONNECT_MAX_TRIAL) {
        serial_port = open("/dev/radio/atci1", O_RDWR | O_NOCTTY );

        if (serial_port > 0) {
            break;
        }
        printf("Error %i from open: %s, trying to reconnect...\n", errno, strerror(errno));
        system("ls -la /dev/radio");
        usleep(RECONNECT_DELAY_MS);
    }

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        system("ls -la /dev/radio");
        printf("Couldn't open '%s' serial port\n", name);
        return errno;
    }

    // Configuration Setup
    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return errno;
    }

    // Check if the terminal is in cannonical mode
    if (tty.c_lflag & ICANON) {
        printf("Terminal is in canonical mode\n");
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    // tty.c_cflag &= ~PARENB;   // No parity
    // tty.c_cflag &= ~CSTOPB;   // 1 stop bit
    // tty.c_cflag &= ~CSIZE;
    // tty.c_cflag |= CS8;       // 8 data bits
    // tty.c_cflag |= CREAD | CLOCAL; // Enable reading

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return errno;
    }

    printf("connect_to_se - DONE\n");

    return 0;
}

void run_interactive_mode(bool read_output, bool print_flag) {
    printf("run_interactive_mode - START\n");

    read_output_flag = read_output;
    print_out_flag = print_flag;

    // char str[] = "   absdasdasd    ";
    // size_t size1 = strlen(str);
    // printf("str = '%s', size1 = %zu\n", str, size1);
    // remove_symb_from_start(str, &size1, ' ');
    // printf("str = '%s', size1 = %zu\n", str, size1);
    // remove_symb_from_end(str, &size1, ' ');
    // printf("str = '%s', size1 = %zu\n", str, size1);

    crypto_app_session_id = (int*)mmap(
        NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *crypto_app_session_id = -1;

    // // Basic Setup In C
    // serial_port = open("/dev/radio/atci1", O_RDWR);

    // // Check for errors
    // if (serial_port < 0) {
    //     printf("Error %i from open: %s\n", errno, strerror(errno));
    // }

    // // Configuration Setup
    // // Create new termios struct, we call it 'tty' for convention
    // // No need for "= {0}" at the end as we'll immediately write the existing
    // // config to this struct
    // struct termios tty;

    // // Read in existing settings, and handle any error
    // // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // // must have been initialized with a call to tcgetattr() overwise behaviour
    // // is undefined
    // if(tcgetattr(serial_port, &tty) != 0) {
    //     printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    // }

    // cfsetispeed(&tty, B115200);
    // cfsetospeed(&tty, B115200);

    // // Save tty settings, also checking for error
    // if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    //     printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    // }

    func2();

    munmap(crypto_app_session_id, sizeof(int));

    printf("run_interactive_mode - FINISH\n");
}

int create_connection_se() {
    if (serial_port != -1) {
        return 0;
    }
    return connect_to_se();
}

int close_connection_se() {
    if (serial_port == -1) {
        return 0;
    }
    close(serial_port);
    serial_port = -1;
    printf("serail_port = %d\n", serial_port);
    return 0;
}

bool is_se_exists() {
    int flag = check_se_existance(serial_port);
    return flag;
}

int open_log_channel_crypto() {
    crypto_app_session_id = (int*)mmap(
        NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *crypto_app_session_id = -1;

    if (!is_connection_established()) {
        printf("Connection with SE over serial port is not established\n");
        return -1;
    }

    printf("run open_logical_channel_crypto_app_new()\n");
    open_logical_channel_crypto_app_new(serial_port);

    return 0;
}

int close_log_channel_crypto() {
    if (!is_connection_established()) {
        printf("Connection with SE over serial port is not established\n");
        return -1;
    }

    printf("run close_logical_channel_crypto_app_new()\n");
    close_logical_channel_crypto_app_new(serial_port);

    munmap(crypto_app_session_id, sizeof(int));
    crypto_app_session_id = NULL;
    return 0;
}

int select_crypto_aid() {
    if (!is_connection_established()) {
        printf("Connection with SE over serial port is not established\n");
        return -1;
    }

    if (crypto_app_session_id == NULL) {
        printf("Logical channel with Crypto is not opened\n");
        return -1;
    }

    int ret = do_select_crypto_aid(serial_port);
    return 0;
}

int get_random_int(int* rand_num) {
    if (!is_connection_established()) {
        printf("Connection with SE over serial port is not established\n");
        return -1;
    }

    if (crypto_app_session_id == NULL) {
        printf("Logical channel with Crypto is not opened\n");
        return -1;
    }

    return do_get_random_number(serial_port, rand_num);
}

// int register_new_user(int user_id, char* passwd, size_t size) {

//     if (user_id <= 0 || user_id > 15) {
//         printf("Wrong usewr ID value = %d\n", user_id);
//     }

//     if (user_id <= 0 || user_id > 15) {
//         printf("Wrong usewr ID value = %d\n", user_id);
//     }

//     return 0;
// }

// int unregister_user(int user_id, char* passwd, size_t size) {
    
//     return 0;
// }

void configure_modem_se() {
    // Disable Specific Reporting Commands

    // AT+CMER - configuration of Mobile termination event reporting +CMER
    // AT+EOPS - 
    // AT+COPS - operator selection
    // AT+CREG - network registration status
    // AT+CEREG - Check General Registration Reporting

    // Disable network registration reports:
    // AT+CREG=0

    // Turn Off All Unsolicited Event Reporting
    // AT+CMER=0,0,0,0,0,0

    // If the above doesn’t work, reset the modem to factory defaults to clear any lingering configurations:
    // AT&F

    // Disable Verbose Responses (if applicable)
    // ATV0
    // This switches to numeric responses (e.g., 0 instead of OK), but it might also suppress some unsolicited messages. Follow with:
    // ATQ1
    // to disable result codes entirely (though this might mute all responses, so use cautiously).

    // Some MediaTek modems use custom commands like +EGREG for enhanced debugging or LTE status. Try disabling tracing or logging:
    // AT+TRACE=0 - NOT SUPPORTED

    ///////////////////////////////////////////////////////////////////

    // reload the SIM card in modem slot
    // AT+CFUN=0
    // after 'OK'
    // AT+CFUN=1

    // Controls SMS-related URCs, but it might influence other notifications too.
    // AT+CNMI:


    // AT+CREG, AT+CGREG, or AT+CEREG: Controls network registration URCs
    // AT+CREG=0 - circuit-switched network
    // AT+CGREG=0 - GPRS network
    // AT+CEREG=0 - EPS/LTE network
    // AT+EGREG=0 - 
    // AT+COPS=0  - Manages operator selection, which might be tied to +EOPS
    // or 
    // AT+COPS=2 (deregister from network)

    // https://m2msupport.net/m2msupport/atcreg-network-registration/
    // at+creg=3 - registration denied
	// at+ecreg=0
    
    // https://onomondo.com/blog/at-command-cgreg/
	// at+cgreg=0
    // at+ecgreg=0

    // https://onomondo.com/blog/at-command-cereg/#at-cereg-1
	// at+cereg=0
	// at+ecereg=0

	// at+CIREG=0
	// at+EIREG=0
	// at+eregchk=0
	// at+ereginfo=0


    // AT+CMER=0,0,0,0,0,0 - disable unsolicited response signaling
    // AT+EOPS=0
    // AT+EIND=0 - Disables extended indicator URCs (e.g., network state). Check AT+EIND=?.

    // not clear what are following commands:



    // at+cpin=\"<PIN_VALUE>\" - enter PIN

    // Check if SIM is inserted into SIM slot:
    // at+esimexist? - 1/0 depending on SIM presence

    ///////////////////////////////////////////////////////////////////

    // ATQ1 - enable quiet mode
    // ATQ0 - disable quiet mode
    // 
    // 

    // The AT+COPS command, which we discussed earlier, can be used to deregister the modem from the network, effectively stopping the SIM from communicating with the cellular network without physically powering it off:

    // Command: AT+COPS=2
    // Effect: This deregisters the modem from the current network. The SIM remains powered and readable (e.g., for commands like AT+CCID to get the SIM’s ICCID), but it won’t be used for network connectivity until re-registered (e.g., with AT+COPS=0 for automatic mode).
    // Response: Typically OK if successful.
    // This doesn’t "switch off" the SIM’s power but halts its network-related functions, which might suffice depending on your goal.

    // Powering Down the Modem
    // Some modems allow you to power down the entire radio module (which includes the SIM interface) using manufacturer-specific commands. These vary by modem, but here are examples:

    // Generic Command: AT+CFUN
    // Syntax: AT+CFUN=<fun>
    // Options:
    // AT+CFUN=0: Minimum functionality (disables both transmit and receive RF circuits, often including SIM access).
    // AT+CFUN=1: Full functionality (default mode).
    // AT+CFUN=4: Flight mode (disables RF but keeps SIM accessible for some operations).
    // Effect: Setting AT+CFUN=0 typically shuts down the modem’s radio stack, and in many cases, the SIM becomes inactive or inaccessible until the modem is brought back to full functionality.
    // Response: OK if supported.
    // Manufacturer-Specific Examples:
    // Quectel Modems: AT+QPOWD powers off the module entirely, including the SIM interface.
    // AT+QPOWD=1 (immediate power-off).
    // Simcom (e.g., SIM800/SIM900): AT+CPOWD=1 powers off the module, effectively disabling the SIM.
    // u-blox: AT+CPWROFF or AT+CFUN=15 may power down the module, depending on the model.
    // These commands often turn off the modem itself, not just the SIM, so the device might need a hardware reset or power cycle to restart.

    // Disabling SIM Functionality
    // Some modems support commands to disable specific SIM-related features:

    // AT+CSIM or AT+CRSM: These low-level commands interact with the SIM card directly. While they don’t "switch off" the SIM, they can lock it or restrict access (e.g., requiring a PIN or entering an invalid state), but this is advanced and risks locking the SIM unintentionally.
    // AT+CLCK: Locks or unlocks SIM facilities (e.g., requiring a PIN).
    // Example: AT+CLCK="SC",1,"1234" enables PIN lock with code "1234". Without the PIN, the SIM won’t function on the next power-up.
    // Practical Considerations
    // Physical Power Control: SIM cards are typically powered by the modem’s SIM slot, and there’s no standard AT command to cut power to the SIM independently. Some modems have proprietary GPIO controls or firmware options to toggle SIM power, but this isn’t universal.
    // Intent: If your goal is to stop network activity, AT+COPS=2 or AT+CFUN=4 (flight mode) is usually enough. If you need the SIM completely inactive, powering off the modem (AT+CFUN=0 or a power-off command) is the next step.
    // Reversibility: Ensure you can re-enable the modem or SIM afterward. For example, after AT+COPS=2, use AT+COPS=0 to reconnect. After a power-off command, you might need a physical reset.
}