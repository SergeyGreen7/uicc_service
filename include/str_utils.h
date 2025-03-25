#ifndef _STR_UTILS_H_
#define _STR_UTILS_H_ 

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

void remove_symbols(char* str, size_t* size, char s[], size_t slen);
void remove_symb(char* str, size_t* size, char s);

void remove_symbols_from_start(char* str, size_t* size, char s[], size_t slen);
void remove_symb_from_start(char* str, size_t* size, char s);

void remove_symbols_from_end(char* str, size_t* size, char s[], size_t slen);
void remove_symb_from_end(char* str, size_t* size, char s);

bool is_digit(char s);
int get_digit(char s);
int get_digit_from_hex(char s);

uint8_t get_byte_from_hex(char* str);
void get_bytes_from_hex(char* str, size_t str_size, uint8_t* data, size_t* data_size);
int get_int_from_hex(char* start, char* end);

void append_symbol(char* str, size_t* size, char s);
void append_symbol_asbyte_hex(char* str, size_t* size, int n);
void append_symbol_hex(char* str, size_t* size, int num_bytes, int symbol);
void append_fid(char* str, size_t* size, uint16_t fid);
void append_number(char* str, size_t* size, int n);

bool check_if_contains(char* str1, size_t size1, char* str2, size_t size2);


#endif // _STR_UTILS_H_