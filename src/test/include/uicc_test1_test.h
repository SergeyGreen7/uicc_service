#ifndef _UICC_TEST1_TEST_H_
#define _UICC_TEST1_TEST_H_

#include "unity.h"
#include <stdbool.h>
#include <stdint.h>

void serial_port_open_test();
void serial_port_close_test();
void crypto_lchannel_open_test();
void crypto_lchannel_close_test();

void serial_port_open_close_test();

void crypto_lchannel_open_close_test();

void get_random_int_test();

#endif // _UICC_TEST1_TEST_H_