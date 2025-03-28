#include <uicc_test1_test.h>

#include <uicc_test.h>

extern int crypto_app_session_id;

void serial_port_open_test() {
    TEST_ASSERT_EQUAL(is_connection_established(), false);
    TEST_ASSERT_EQUAL(open_connection_se(), 0);
    TEST_ASSERT_EQUAL(is_connection_established(), true);
}

void serial_port_close_test() {
    TEST_ASSERT_EQUAL(is_connection_established(), true);
    TEST_ASSERT_EQUAL(close_connection_se(), 0);
    TEST_ASSERT_EQUAL(is_connection_established(), false);
}

void serial_port_open_close_test() {
    serial_port_open_test();
    serial_port_close_test();
}

void crypto_lchannel_open_test() {
    TEST_ASSERT_EQUAL(is_crypto_lchannel_opened(), false);
    TEST_ASSERT_EQUAL(open_crypto_lchannel(), 0);
    TEST_ASSERT_EQUAL(is_crypto_lchannel_opened(), true);
}

void crypto_lchannel_close_test() {
    TEST_ASSERT_EQUAL(is_crypto_lchannel_opened(), true);
    TEST_ASSERT_EQUAL(close_crypto_lchannel(), 0);
    TEST_ASSERT_EQUAL(is_crypto_lchannel_opened(), false);
}

void crypto_lchannel_open_close_test() {
    serial_port_open_test();

    crypto_lchannel_open_test();
    crypto_lchannel_close_test();

    serial_port_close_test();
}

void get_random_int_test() {
    int num1 = 0, num2 = 0;

    TEST_ASSERT_NOT_EQUAL(get_random_int(&num1), 0);
    TEST_ASSERT_EQUAL(num1, 0);

    crypto_lchannel_open_test();

    TEST_ASSERT_EQUAL(get_random_int(&num1), 0);
    TEST_ASSERT_NOT_EQUAL(num1, 0);
    TEST_ASSERT_EQUAL(get_random_int(&num2), 0);
    TEST_ASSERT_NOT_EQUAL(num2, 0);
    TEST_ASSERT_NOT_EQUAL(num1, num2);

    crypto_lchannel_close_test();
    num1 = 0;
    TEST_ASSERT_NOT_EQUAL(get_random_int(&num1), 0);
    TEST_ASSERT_EQUAL(num1, 0);
}
