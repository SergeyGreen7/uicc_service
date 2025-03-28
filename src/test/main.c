#include "unity.h"
#include <uicc_test1_test.h>

#include <uicc_test.h>
#include <sec_storage_test.h>

int func_add(int a, int b) {
    return a + b;
}

void setUp(void) {
    // printf("\nsetUp - START\n");
    // set stuff up here
}

void tearDown(void) {
    // printf("\ntearDown - START\n");
    // if (is_crypto_lchannel_opened()) {
    //     close_crypto_lchannel();
    // }

    // if (is_connection_established()) {
    //     close_connection_se();
    // }
}

void test_function_should_1(void) {
    //test stuff
    TEST_ASSERT_EQUAL(1, 1);
}

void test_function_should_2(void) {
    //more test stuff
    TEST_ASSERT_EQUAL(2, 2);
}

// not needed when using generate_test_runner.rb
int main(void) {

    serial_port_open_test();
    close_crypto_lchannel_force();

    UNITY_BEGIN();
    // RUN_TEST(serial_port_open_close_test);
    // RUN_TEST(crypto_lchannel_open_close_test);

    // RUN_TEST(get_random_int_test);
    // RUN_TEST(check_user_id_test);
    // RUN_TEST(check_password_size_test);
    // RUN_TEST(get_user_list_test);
    // RUN_TEST(add_delete_user_test_1);
    // RUN_TEST(add_delete_user_test_2); 
    RUN_TEST(create_delete_file_test_1);
    RUN_TEST(create_delete_file_test_2);
    RUN_TEST(create_delete_file_test_3);
    // RUN_TEST(check_file_id_test);

    // RUN_TEST(test_function_should_1);
    // RUN_TEST(test_function_should_2);

    close_crypto_lchannel_force();
    close_connection_se();

    return UNITY_END();
}