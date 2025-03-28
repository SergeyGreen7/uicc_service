#include <sec_storage_test.h>
#include <sec_storage.h>
#include <uicc_test.h>
#include <at_commands.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

static int get_number_of_registered_users() {
    uint8_t user_ids[128];
    size_t size = 0;

    get_list_of_registered_users(get_serial_port_fd(), user_ids, &size);
    return (int)size;
}

static void gen_rand_bytes(uint8_t* data, size_t num_bytes) {
    time_t epoch;
    time(&epoch);
    srand(epoch);

    for (size_t i = 0; i < num_bytes; i++) {
        data[i] = rand() % 256;
    }
}

void get_user_list_test() {
    int fd = get_serial_port_fd();
    uint8_t user_ids[128];
    size_t size = 0;

    TEST_ASSERT_EQUAL(open_crypto_lchannel(), 0);

    TEST_ASSERT_EQUAL(get_list_of_registered_users(fd, user_ids, &size), 0);
    printf("size = %zu, users:\n", size);
    for (size_t i = 0; i < size; i++) {
        printf("%hu\n", user_ids[i]);
    }

    bool reg_flag;
    for (size_t i = 0; i < size; i++) {
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_ids[i], &reg_flag), 0);
        TEST_ASSERT_EQUAL(reg_flag, true);
    }

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void check_user_id_test() {
    int fd = get_serial_port_fd();

    TEST_ASSERT_EQUAL(open_crypto_lchannel(), 0);

    bool reg_flag;
    TEST_ASSERT_NOT_EQUAL(is_user_registered(fd, 0, &reg_flag), 0);
    TEST_ASSERT_EQUAL(is_user_registered(fd, 1, &reg_flag), 0);
    TEST_ASSERT_EQUAL(is_user_registered(fd, 15, &reg_flag), 0);
    TEST_ASSERT_NOT_EQUAL(is_user_registered(fd, 16, &reg_flag), 0);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void check_password_size_test() {
    int fd = get_serial_port_fd();

    uint8_t passwd[64] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    };
    size_t passwd_sizes[10] = {8, 10, 17, 21, 37, 49, 51, 56, 60, 64};

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int new_user_id;
    for (int user_id = 0x01; user_id < 16; user_id++) {
        printf("user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            new_user_id = user_id;
            break;
        }
    }
    
    TEST_ASSERT_NOT_EQUAL(register_new_user(fd, new_user_id, passwd, 7), 0);
    TEST_ASSERT_NOT_EQUAL(register_new_user(fd, new_user_id, passwd, 257), 0);

    size_t num_size = sizeof(passwd_sizes) / sizeof(passwd_sizes[0]);
    for (size_t i = 0; i < num_size; i++) {
        size_t passwd_size = passwd_sizes[i];
        printf("passwd_size = %zu\n", passwd_size);
        TEST_ASSERT_EQUAL(register_new_user(fd, new_user_id, passwd, passwd_size), 0);
        TEST_ASSERT_EQUAL(is_user_registered(fd, new_user_id, &reg_flag), 0);
        TEST_ASSERT_EQUAL(reg_flag, true);

        TEST_ASSERT_EQUAL(unregister_user(fd, new_user_id, passwd, passwd_size), 0);
        TEST_ASSERT_EQUAL(is_user_registered(fd, new_user_id, &reg_flag), 0);
        TEST_ASSERT_EQUAL(reg_flag, false);
    }

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void add_delete_user_test_1() {
    int fd = get_serial_port_fd();
    uint8_t user_ids[128];
    size_t size = 0;

    uint8_t passwd[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    };

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int num_users = get_number_of_registered_users();
    for (int user_id = 0x01; user_id < 16; user_id++) {
        printf("user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            TEST_ASSERT_EQUAL(register_new_user(fd, user_id, passwd, sizeof(passwd)), 0);
            TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);
            TEST_ASSERT_EQUAL(reg_flag, true);

            TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users + 1);

            TEST_ASSERT_EQUAL(unregister_user(fd, user_id, passwd, sizeof(passwd)), 0);
            TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);
            TEST_ASSERT_EQUAL(reg_flag, false);

            // break;
        }
    }
    TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void add_delete_user_test_2() {
    int fd = get_serial_port_fd();
    uint8_t user_ids[128];
    size_t size = 0;
    uint8_t passwd1[8] = {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F};

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int num_users = get_number_of_registered_users();
    
    for (int user_id = 0x01; user_id < 16; user_id++) {
        printf("register user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            TEST_ASSERT_EQUAL(register_new_user(fd, user_id, passwd1, sizeof(passwd1)), 0);
            // break;
            user_ids[size] = user_id;
            size += 1;
        }
    }

    TEST_ASSERT_EQUAL(get_number_of_registered_users(), size);

    for (int i = 0; i < size; i++) {
        int user_id = user_ids[i];
        printf("unregister user_id = %d\n", user_id);
        TEST_ASSERT_EQUAL(unregister_user(fd, user_id, passwd1, sizeof(passwd1)), 0);
    }

    TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void create_delete_file_test_1() {
    int fd = get_serial_port_fd();
    uint8_t files_ids[128];
    size_t size = 0;
    uint8_t passwd[8] = {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F};

    uint8_t data[8] = {0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB};
    size_t data_size = sizeof(data);
    size_t data_read_size;
    uint8_t data_read[32768];
    size_t num_files_read;
    uint8_t file_ids_read[128];
    uint16_t file_sizes_read[128];
    // const int num_files = sizeof(data_sizes) / sizeof(data_sizes[0]);
    const int num_files = 15;

    gen_rand_bytes(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int num_users = get_number_of_registered_users();
    for (int user_id = 0x01; user_id < 16; user_id++) {
        // printf("user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            TEST_ASSERT_EQUAL(register_new_user(fd, user_id, passwd, sizeof(passwd)), 0);
            
            printf("user_id = %d\n", user_id);

            TEST_ASSERT_NOT_EQUAL(user_id, -1);

            for (int id = 0; id < num_files; id++) {
                int file_id = 0x01 + id;
                TEST_ASSERT_EQUAL(create_bf_file(fd, user_id, file_id, passwd, sizeof(passwd), data, sizeof(data)), 0);
            }

            TEST_ASSERT_EQUAL(get_list_of_user_bf_files(fd, user_id, file_ids_read, file_sizes_read, &num_files_read), 0);
            TEST_ASSERT_EQUAL(num_files, num_files_read);

            for (int id = 0; id < num_files_read; id++) {
                printf("id = %d, file_id = %hu, file_size = %hu\n", id, file_ids_read[id], file_sizes_read[id]);
            }

            for (int id = 0; id < num_files; id++) {
                int file_id = 0x01 + id;

                TEST_ASSERT_EQUAL(read_bf_file(fd, user_id, file_id, passwd, sizeof(passwd), data_read, &data_read_size), 0);
                TEST_ASSERT_EQUAL(data_read_size, data_size);
                int res = strncmp((char*)data, (char*)data_read, data_size);
                if (res != 0) {
                    printf("id = %d, data_size = %zu\n", id, data_size);
                    for (int i = 0; i < data_size; i++) {
                        printf("  data_read[%d] = 0x%02x, data[%d] = 0x%02x\n", i, data_read[i], i, data[i]);
                    }
                }
                TEST_ASSERT_EQUAL(strncmp((char*)data, (char*)data_read, data_size), 0);
            }

            for (int id = 0; id < num_files; id++) {
                int file_id = 0x01 + id;
                TEST_ASSERT_EQUAL(delete_bf_file(fd, user_id, file_id, passwd, sizeof(passwd)), 0);
            }

            TEST_ASSERT_EQUAL(get_list_of_user_bf_files(fd, user_id, file_ids_read, file_sizes_read, &num_files_read), 0);
            TEST_ASSERT_EQUAL(0, num_files_read);

            TEST_ASSERT_EQUAL(unregister_user(fd, user_id, passwd, sizeof(passwd)), 0);
        }
    }

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void create_delete_file_test_2() {
    int fd = get_serial_port_fd();
    uint8_t files_ids[128];
    size_t size = 0;
    uint8_t passwd[8] = {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F};

    uint8_t data[32768];
    size_t data_sizes[15] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    size_t data_read_size;
    uint8_t data_read[32768];

    gen_rand_bytes(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int new_user_id = -1;
    int num_users = get_number_of_registered_users();
    for (int user_id = 0x01; user_id < 16; user_id++) {
        // printf("user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            new_user_id = user_id;
            TEST_ASSERT_EQUAL(register_new_user(fd, new_user_id, passwd, sizeof(passwd)), 0);
            break;
        }
    }

    printf("user_id = %d\n", new_user_id);

    TEST_ASSERT_NOT_EQUAL(new_user_id, -1);

    const int num_files = sizeof(data_sizes) / sizeof(data_sizes[0]);
    for (int id = 0; id < num_files; id++) {
        int file_id = 0x01 + id;
        TEST_ASSERT_EQUAL(create_bf_file(fd, new_user_id, file_id, passwd, sizeof(passwd), data, data_sizes[id]), 0);
    }

    size_t num_files_read;
    uint8_t file_ids_read[128];
    uint16_t file_sizes_read[128]; 

    TEST_ASSERT_EQUAL(get_list_of_user_bf_files(fd, new_user_id, file_ids_read, file_sizes_read, &num_files_read), 0);
    TEST_ASSERT_EQUAL(num_files, num_files_read);

    for (int id = 0; id < num_files_read; id++) {
        printf("id = %d, file_id = %hu, file_size = %hu\n", id, file_ids_read[id], file_sizes_read[id]);
    }

    for (int id = 0; id < num_files; id++) {
        int file_id = 0x01 + id;

        TEST_ASSERT_EQUAL(read_bf_file(fd, new_user_id, file_id, passwd, sizeof(passwd), data_read, &data_read_size), 0);
        TEST_ASSERT_EQUAL(data_read_size, data_sizes[id]);
        int res = strncmp((char*)data, (char*)data_read, data_sizes[id]);
        if (res != 0) {
            printf("id = %d, data_size = %zu\n", id, data_sizes[id]);
            for (int i = 0; i < data_sizes[id]; i++) {
                printf("  data_read[%d] = 0x%02x, data[%d] = 0x%02x\n", i, data_read[i], i, data[i]);
            }
        }
        TEST_ASSERT_EQUAL(strncmp((char*)data, (char*)data_read, data_sizes[id]), 0);
    }

    for (int id = 0; id < num_files; id++) {
        int file_id = 0x01 + id;
        TEST_ASSERT_EQUAL(delete_bf_file(fd, new_user_id, file_id, passwd, sizeof(passwd)), 0);
    }

    TEST_ASSERT_EQUAL(get_list_of_user_bf_files(fd, new_user_id, file_ids_read, file_sizes_read, &num_files_read), 0);
    TEST_ASSERT_EQUAL(0, num_files_read);

    // bool reg_flag;
    // int num_users = get_number_of_registered_users();
    
    // for (int user_id = 0x01; user_id < 16; user_id++) {
    //     printf("register user_id = %d\n", user_id);
    //     reg_flag = false;
    //     TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

    //     if (!reg_flag) {
    //         TEST_ASSERT_EQUAL(register_new_user(fd, user_id, passwd1, sizeof(passwd1)), 0);
    //         // break;
    //         user_ids[size] = user_id;
    //         size += 1;
    //     }
    // }

    // TEST_ASSERT_EQUAL(get_number_of_registered_users(), size);

    // for (int i = 0; i < size; i++) {
    //     int user_id = user_ids[i];
    //     printf("unregister user_id = %d\n", user_id);
    //     TEST_ASSERT_EQUAL(unregister_user(fd, user_id, passwd1, sizeof(passwd1)), 0);
    // }

    // TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users);

    TEST_ASSERT_EQUAL(unregister_user(fd, new_user_id, passwd, sizeof(passwd)), 0);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void create_delete_file_test_3() {
    int fd = get_serial_port_fd();
    uint8_t user_ids[128];
    size_t size = 0;
    uint8_t passwd[8] = {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F};

    uint8_t data[8] = {0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB};
    size_t data_size = sizeof(data);
    size_t data_read_size;
    uint8_t data_read[32768];
    size_t num_files_read;
    uint8_t file_ids_read[128];
    uint16_t file_sizes_read[128];
    const int num_files = 15;

    gen_rand_bytes(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int num_users = get_number_of_registered_users();
    printf("num_users = %d\n", num_users);
    for (int user_id = 0x01; user_id < 16; user_id++) {
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            user_ids[size] = user_id;
            size += 1;
            
            TEST_ASSERT_EQUAL(register_new_user(fd, user_id, passwd, sizeof(passwd)), 0);
            
            printf("user_id = %d\n", user_id);

            TEST_ASSERT_NOT_EQUAL(user_id, -1);

            for (int id = 0; id < num_files; id++) {
                int file_id = 0x01 + id;
                TEST_ASSERT_EQUAL(create_bf_file(fd, user_id, file_id, passwd, sizeof(passwd), data, sizeof(data)), 0);
            }

            TEST_ASSERT_EQUAL(get_list_of_user_bf_files(fd, user_id, file_ids_read, file_sizes_read, &num_files_read), 0);
            TEST_ASSERT_EQUAL(num_files, num_files_read);

            for (int id = 0; id < num_files_read; id++) {
                printf("id = %d, file_id = %hu, file_size = %hu\n", id, file_ids_read[id], file_sizes_read[id]);
            }

            for (int id = 0; id < num_files; id++) {
                int file_id = 0x01 + id;

                TEST_ASSERT_EQUAL(read_bf_file(fd, user_id, file_id, passwd, sizeof(passwd), data_read, &data_read_size), 0);
                TEST_ASSERT_EQUAL(data_read_size, data_size);
                int res = strncmp((char*)data, (char*)data_read, data_size);
                if (res != 0) {
                    printf("id = %d, data_size = %zu\n", id, data_size);
                    for (int i = 0; i < data_size; i++) {
                        printf("  data_read[%d] = 0x%02x, data[%d] = 0x%02x\n", i, data_read[i], i, data[i]);
                    }
                }
                TEST_ASSERT_EQUAL(strncmp((char*)data, (char*)data_read, data_size), 0);
            }
        }
    }

    TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users + size);

    for (int i = 0; i < size; i++) {
        int user_id = user_ids[i];
        TEST_ASSERT_EQUAL(unregister_user(fd, user_id, passwd, sizeof(passwd)), 0);
    }

    printf("size = %zu, num_users = %d\n", size, num_users);
    TEST_ASSERT_EQUAL(get_number_of_registered_users(), num_users);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}

void check_file_id_test() {
    int fd = get_serial_port_fd();

    uint8_t passwd[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    };
    uint8_t data[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    };

    TEST_ASSERT_EQUAL(0, open_crypto_lchannel());

    bool reg_flag;
    int new_user_id;
    for (int user_id = 0x01; user_id < 16; user_id++) {
        printf("user_id = %d\n", user_id);
        reg_flag = false;
        TEST_ASSERT_EQUAL(is_user_registered(fd, user_id, &reg_flag), 0);

        if (!reg_flag) {
            new_user_id = user_id;
            break;
        }
    }

    TEST_ASSERT_EQUAL(register_new_user(fd, new_user_id, passwd, sizeof(passwd)), 0);

    TEST_ASSERT_NOT_EQUAL(create_bf_file(fd, new_user_id, 0, passwd, sizeof(passwd), data, sizeof(data)), 0);
    TEST_ASSERT_NOT_EQUAL(create_bf_file(fd, new_user_id, 16, passwd, sizeof(passwd), data, sizeof(data)), 0);

    for (size_t id = 0; id < 16; id++) {
        int file_id = 0x01 + id;
        TEST_ASSERT_EQUAL(create_bf_file(fd, new_user_id, file_id, passwd, sizeof(passwd), data, sizeof(data)), 0);
        TEST_ASSERT_EQUAL(delete_bf_file(fd, new_user_id, file_id, passwd, sizeof(passwd)), 0);
    }

    TEST_ASSERT_EQUAL(unregister_user(fd, new_user_id, passwd, sizeof(passwd)), 0);

    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
}