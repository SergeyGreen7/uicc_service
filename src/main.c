// C librarary headers
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/mman.h>
#include <sys/wait.h>

#include <at_commands.h>
#include <str_utils.h>
#include <uicc_test.h>

enum OperationMode {
    NONE_OP_MODE = 0,
    INTERAC_OP_MODE,
    GET_RAND_NUM_OP_MODE,
};


bool debug_flag = false;
bool read_output = true;
enum OperationMode mode = NONE_OP_MODE;


void parse_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "m:r:d:")) != -1) {
        switch (opt) {
        case 'm': {
            mode = (enum OperationMode)atoi(optarg);
        }
        break;
        case 'r': {
            read_output = (bool)atoi(optarg);
        }
        break;
        case 'd': {
            debug_flag = (bool)atoi(optarg);
        }
        break;
        default:
            fprintf(stderr, "Usage: %s [-m X -r X -d X] \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) {

    printf("UICC test main - START\n");

    parse_args(argc, argv);

    printf("Run UICC test with following poarameters:\n");
    printf(" - mode = %d\n", mode);
    printf(" - debug_flag = %d\n", debug_flag);
    printf(" - read_output = %d\n\n", read_output);

    switch (mode) {
        case INTERAC_OP_MODE: {
            printf("run run_interactive_mode\n");
            run_interactive_mode(read_output, debug_flag);
        }
        break;

        case GET_RAND_NUM_OP_MODE: {
            
            int ret;
            bool flag = is_connection_established();
            printf("connection flag = %d\n", flag);

            printf("run open_connection_se()\n");
            ret = open_connection_se();
            if (ret != 0) {
                printf("Error on SE connection over serial port\n");
            } else {
                printf("SE connection over serial port succedded\n");
            }
            
            ret = open_crypto_lchannel();
            if (ret != 0) {
                printf("Error on Crypto logical channel open\n");
            }

            for (int i = 0; i < 10; i++) {
                int rand_num;
                int ret = get_random_int(&rand_num);
                printf("rand_num = %d\n\n", rand_num);
            }

            ret = close_connection_se();
            if (ret != 0) {
                printf("Error on SE close connection\n");
            }
        }
        break;

        default:
            printf("Unknown operation mode = %d\n", mode);
    }

    printf("UICC test main - FINISH\n");

    exit(0);
}

