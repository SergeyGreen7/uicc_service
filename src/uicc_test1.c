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
// #include <sys/mman.h>
#include <sys/wait.h>

#include "sec_storage.h" 
#include "printer.h"

// #include <uicc_common.h>

#define BUFFER_SIZE 4096
#define RECONNECT_MAX_TRIAL 32
#define RECONNECT_DELAY_MS 200

// static char buffer_in [BUFFER_SIZE];
// static char buffer_out [BUFFER_SIZE];
static int serial_port = -1;

typedef int (*WriteFuncPtr)(int);
typedef int (*ReadFuncPtr)(int);

int crypto_app_session_id = -1;
bool read_output_flag = true;
bool print_out_flag = false;

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

    int user_id = 1;
    int file_id = 3;
    uint8_t passwd[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    size_t  passwd_size = sizeof(passwd);

    uint8_t* data_write;
    size_t  data_size_write;
    uint8_t data_read[32000];
    size_t  data_size_read;
    bool loop_flag = true;
    
    // tmp
    {
        printf("run open_connection_se\n");
        open_connection_se();
    }
    // tmp
    {
        close_crypto_lchannel_force();
        open_crypto_lchannel();

        printf("run select_crypto_aid\n");
        int ret = select_crypto_aid();

        if (ret != 0) {
            printf("Error during Crypto AID selection\n");
            loop_flag = false;
        }
    }

    while (loop_flag) {
        printf("please enter AT command:\n");
        scanf("%s", cmd_input);
        printf("input command = '%s'\n", cmd_input);

        custom_at_cmd_flag = false;
        if (strcmp(cmd_input, "open_se") == 0) {
            printf("cmd_input == open_se\n");
            printf("run open_connection_se\n");
            open_connection_se();
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "close_se") == 0) {
            printf("cmd_input == close_se\n");
            printf("run close_connection_se\n");
            close_connection_se();
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "exists") == 0) {
            printf("cmd_input == exists\n");
            bool flag = is_se_exists();
            printf("se exist flag = %d\n", flag);
        } else if (strcmp(cmd_input, "open_crypto") == 0) {
            printf("cmd_input == open_crypto\n");
            open_crypto_lchannel();
            // get_random_number(serial_port, &rand_num);
            // printf("rand_num = %d\n", rand_num);
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "close_crypto") == 0) {
            printf("cmd_input == close_crypto\n");
            close_crypto_lchannel();
            // get_random_number(serial_port, &rand_num);
            // printf("rand_num = %d\n", rand_num);
            // tx_func = NULL;
        } else if (strcmp(cmd_input, "select_crypto") == 0) {
            printf("cmd_input == select_crypto\n");
            int ret = select_crypto_aid();
        } else if (strcmp(cmd_input, "get_rand") == 0) {
            printf("cmd_input == get_rand\n");
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
            bool flag;
            int ret = do_check_fid_existence(serial_port, fid, &flag);
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

        } else if (strncmp(cmd_input, "register_user", 13) == 0) {
            printf("cmd_input == register_user\n");

            if (strlen(cmd_input) == 14) {
                user_id = get_digit_from_hex(cmd_input[13]);
                printf("user_id = %d\n", user_id);
            }
            
            printf("run register_new_user\n");
            int ret = register_new_user(serial_port, user_id, passwd, passwd_size);

        } else if (strncmp(cmd_input, "unregister_user", 15) == 0) {
            printf("cmd_input == unregister_user\n");
            
            if (strlen(cmd_input) == 16) {
                user_id = get_digit_from_hex(cmd_input[15]);
                printf("user_id = %d\n", user_id);
            }

            printf("run unregister_user\n");
            int ret = unregister_user(serial_port, user_id, passwd, passwd_size);

        } else if (strncmp(cmd_input, "check_user", 10) == 0) {
            printf("cmd_input == unregister_user\n");
            
            if (strlen(cmd_input) == 11) {
                user_id = get_digit_from_hex(cmd_input[10]);
                printf("user_id = %d\n", user_id);
            }

            printf("run is_user_registered\n");
            bool flag = false;
            int ret = is_user_registered(serial_port, user_id, &flag);
            printf("user %d(0x%02X), register flag = %d \n", user_id, user_id, flag);

        } else if (strncmp(cmd_input, "select_user", 11) == 0) {
            printf("cmd_input == select_user\n");
            
            if (strlen(cmd_input) == 12) {
                user_id = get_digit_from_hex(cmd_input[11]);
                printf("user_id = %d\n", user_id);
            }

            printf("run select_user\n");
            int ret = select_user(serial_port, user_id);

        } else if (strcmp(cmd_input, "get_users") == 0) {
            printf("cmd_input == get_users\n");

            printf("run get_list_of_registered_users\n");
            uint8_t user_ids[128];
            size_t user_id_size = 0;
            int ret = get_list_of_registered_users(serial_port, &user_ids[0], &user_id_size);
            printf("number of registered users = %zu, user IDs:\n", user_id_size);
            for (size_t i = 0; i < user_id_size; i++) {
                printf("  0x%02X\n", user_ids[i]);
            }

        } else if (strncmp(cmd_input, "create_bf", 9) == 0) {
            printf("cmd_input == create_bf\n");

            if (strlen(cmd_input) >= 10) {
                user_id = get_digit_from_hex(cmd_input[9]);
                printf("user_id = %d\n", user_id);
            }
            if (strlen(cmd_input) >= 11) {
                file_id = get_digit_from_hex(cmd_input[10]);
                printf("file_id = %d\n", user_id);
            }

            data_size_write = rand() % 100;
            data_write = malloc(data_size_write);
            for (int i = 0; i < data_size_write; i++) {
                data_write[i] = rand() % 256;
            }
            printf("run unregister_user\n");
            int ret = create_bf_file(serial_port, user_id, file_id, passwd, passwd_size, data_write, data_size_write);
            
            if (ret == 0) {
                printf("data saved into file: data_size_write = %zu\n", data_size_write);
                for (size_t i = 0; i < data_size_write; i++) {
                    printf("data_write[%zu] = %u\n", i, data_write[i]);
                }
            }

            free(data_write);
        } else if (strncmp(cmd_input, "read_bf", 7) == 0) {
            printf("cmd_input == read_bf\n");

            if (strlen(cmd_input) >= 8) {
                user_id = get_digit_from_hex(cmd_input[7]);
                printf("user_id = %d\n", user_id);
            }
            if (strlen(cmd_input) >= 9) {
                file_id = get_digit_from_hex(cmd_input[8]);
                printf("file_id = %d\n", user_id);
            }
            
            printf("run unregister_user\n");
            int ret = read_bf_file(serial_port, user_id, file_id, passwd, passwd_size, data_read, &data_size_read);

            printf("data_size_read = %zu\n", data_size_read);
            for (size_t i = 0; i < data_size_read; i++) {
                printf("data_read[%zu] = %u\n", i, data_read[i]);
            }
        } else if (strncmp(cmd_input, "delete_bf", 9) == 0) {
            printf("cmd_input == delete_bf\n");
            
            if (strlen(cmd_input) >= 9) {
                user_id = get_digit_from_hex(cmd_input[9]);
                printf("user_id = %d\n", user_id);
            }
            if (strlen(cmd_input) >= 10) {
                file_id = get_digit_from_hex(cmd_input[10]);
                printf("file_id = %d\n", user_id);
            }

            printf("run delete_bf_file\n");
            int ret = delete_bf_file(serial_port, user_id, file_id, passwd, passwd_size);
        } else if (strncmp(cmd_input, "get_files", 9) == 0) {
            printf("cmd_input == get_files\n");
            
            if (strlen(cmd_input) == 10) {
                user_id = get_digit_from_hex(cmd_input[9]);
                printf("user_id = %d\n", user_id);
            }

            printf("run get_list_of_user_bf_files\n");
            size_t num_files = 0;
            uint8_t file_ids[128];
            uint16_t file_sizes[128];
            int ret = get_list_of_user_bf_files(serial_port, user_id, file_ids, file_sizes, &num_files);
            printf("user ID: 0x%02X, number of createde files = %zu, file IDs:\n", user_id, num_files);
            for (size_t i = 0; i < num_files; i++) {
                printf("  0x%02X, size = %hu bytes\n", file_ids[i], file_sizes[i]);
            }

        } else if (strcmp(cmd_input, "print_files") == 0) {
            printf("cmd_input == print_files\n");

            int ret = do_print_all_files_cur_dir(serial_port);
        } else if (strcmp(cmd_input, "check_se_exist") == 0) {
            printf("cmd_input == check_se_exist\n");
            check_se_existance(serial_port);
        } else if (strcmp(cmd_input, "exit") == 0) {
            printf("exit command\n");
            close(serial_port);
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
    dprint("connect_to_se - START\n");

    // Basic Setup In C
    char name[] = "/dev/radio/atci1";

    int reconnect_cnrt = 0;
    while (reconnect_cnrt < RECONNECT_MAX_TRIAL) {
        serial_port = open("/dev/radio/atci1", O_RDWR | O_NOCTTY );

        if (serial_port > 0) {
            break;
        }
        dprint("Error %i from open: %s, trying to reconnect...\n", errno, strerror(errno));
        // system("ls -la /dev/radio");
        usleep(RECONNECT_DELAY_MS);
    }

    // Check for errors
    if (serial_port < 0) {
        eprint("Error %i from open: %s\n", errno, strerror(errno));
        // system("ls -la /dev/radio");
        eprint("Couldn't open '%s' serial port\n", name);
        return errno;
    }
    dprint("connect_to_se(), serial_port = %d\n", serial_port);

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
    // cfsetispeed(&tty, B460800);
    // cfsetospeed(&tty, B460800);

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

    if (print_out_flag) {
        printf("connect_to_se - DONE\n");
    }

    return 0;
}

void run_interactive_mode(bool read_output, bool print_flag) {
    printf("run_interactive_mode - START\n");

    read_output_flag = read_output;
    print_out_flag = print_flag;

    func2();

    printf("run_interactive_mode - FINISH\n");
}

bool is_connection_established() {
    return serial_port > 0;
}

int open_connection_se() {
    if (is_connection_established()) {
        return 0;
    }
    return connect_to_se();
}

int close_connection_se() {
    if (is_connection_established()) {
        close(serial_port);
        serial_port = -1;
        dprint("serail_port = %d\n", serial_port);
    }
    return 0;
}

int get_serial_port_fd() {
    return serial_port;
}

bool is_se_exists() {
    int flag = check_se_existance(serial_port);
    return flag;
}

bool logical_channel_opened() {
    return (crypto_app_session_id != -1);
}

bool is_crypto_lchannel_opened() {
    return (crypto_app_session_id != -1);
}

int open_crypto_lchannel() {
    if (logical_channel_opened()) {
        return 0;
    }

    if (!is_connection_established()) {
        eprint("Connection with SE over serial port is not established\n");
        return -1;
    }

    dprint("run open_logical_channel_crypto_app()\n");
    return open_logical_channel_crypto_app(serial_port);
}

int close_crypto_lchannel() {
    if (!is_connection_established()) {
        eprint("Connection with SE over serial port is not established\n");
        return -1;
    }

    dprint("run close_logical_channel_crypto_app()\n");
    close_logical_channel_crypto_app(serial_port, false);

    crypto_app_session_id = -1;
    return 0;
}

int close_crypto_lchannel_force() {
    if (!is_connection_established()) {
        eprint("Connection with SE over serial port is not established\n");
        return -1;
    }

    dprint("run close_logical_channel_crypto_app()\n");
    close_logical_channel_crypto_app(serial_port, true);

    crypto_app_session_id = -1;
    return 0;
}

int select_crypto_aid() {
    if (!is_connection_established()) {
        printf("Connection with SE over serial port is not established\n");
        return -1;
    }

    if (crypto_app_session_id == -1) {
        printf("Logical channel with Crypto is not opened\n");
        return -1;
    }

    int ret = do_select_crypto_aid(serial_port);
    return 0;
}

int get_random_int(int* rand_num) {
    if (!is_connection_established()) {
        eprint("Connection with SE over serial port is not established\n");
        return 1;
    }

    if (crypto_app_session_id == -1) {
        eprint("\nLogical channel with Crypto app is not opened\n");
        return 1;
    }

    dprint("\nrun do_get_random_number\n");
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