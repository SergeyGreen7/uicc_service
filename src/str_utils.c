#include <str_utils.h>

#include <string.h>


void remove_symbols(char* str, size_t* size, char s[], size_t slen) {
    for (size_t i = 0; i < slen; i++) {
        remove_symb(str, size, s[i]);
        // printf("str = '%s', size = %zu, removed symbol = '%c'\n", str, *size, s[i]);
    }
}

void remove_symb(char* str, size_t* size, char s) {
    size_t i1 = 0;
    size_t i2 = 0;
    while (i1 < *size) {
        // printf("i1 = %zu, str[i1] = '%d', i2 = %zu\n", i1, str[i1], i2);
        if (str[i1] != s) {
            if (i1 > i2) {
                str[i2] = str[i1];
            }
            i2++;
        }
        i1++;
    }
    *size = i2;
    str[*size] = '\0';
}

void remove_symbols_from_start(char* str, size_t* size, char s[], size_t slen) {
    for (size_t i = 0; i < slen; i++) {
        remove_symb_from_start(str, size, s[i]);
    }
}

void remove_symb_from_start(char* str, size_t* size, char s) {
    size_t i1 = 0;
    while (str[i1] == s) {
        i1++;
    }
    
    size_t i2 = 0;
    while (i1 < *size) {
        str[i2++] = str[i1++];
    }

    *size = i2;
    str[*size] = '\0';
}

void remove_symbols_from_end(char* str, size_t* size, char s[], size_t slen) {
    for (size_t i = 0; i < slen; i++) {
        remove_symb_from_end(str, size, s[i]);
    }
}

void remove_symb_from_end(char* str, size_t* size, char s) {
    size_t i1 = *size-1;
    while (str[i1] == s) {
        *size = i1;
        i1--;
    }
    str[*size] = '\0';
}

bool is_digit(char s) {
    return s >= '0' && s <= '9';
}

int get_digit(char s) {
    return s - '0';
}

int get_digit_from_hex(char s) {
    if (is_digit(s)) {
        return get_digit(s);
    } else if (s >= 'A' && s <= 'F') {
        return 10 + s - 'A';
    } else if (s >= 'a' && s <= 'f') {
        return 10 + s - 'a';
    }
    return 0;
}

uint8_t get_byte_from_hex(char* str) {
    return get_digit_from_hex(*str) * 16 + get_digit_from_hex(*(str+1));
}

void get_bytes_from_hex(char* str, size_t str_size, uint8_t* data, size_t* data_size) {
    *data_size = str_size / 2;
    for (size_t i = 0; i < *data_size; i++) {
        data[i] = get_byte_from_hex(str + 2*i);
    }
}

int get_int_from_hex(char* start, char* end) {
    int res = 0;
    for (char* ptr = start; ptr < end; ptr++) {
        res = 16 * res + get_digit_from_hex(*ptr);
    }
    return res;
}

void append_symbol(char* str, size_t* size, char s) {
    str[*size] = s;
    (*size)++;
}

void append_number(char* str, size_t* size, int n) {
    char n_str[16];
    sprintf(n_str, "%d", n);

    size_t n_str_size = strlen(n_str);
    // printf("n = %d, n_str = '%s', n_str_size = %zu\n", n, n_str, n_str_size);
    for (int i = 0; i < n_str_size; i++) {
        append_symbol(str, size, n_str[i]);
    }
}

void append_symbol_asbyte_hex(char* str, size_t* size, int n) {
    char n_str[3];
    sprintf(n_str, "%02X", n);

    size_t n_str_size = strlen(n_str);
    for (int i = 0; i < n_str_size; i++) {
        append_symbol(str, size, n_str[i]);
    }
}

void append_symbol_hex(char* str, size_t* size, int num_bytes, int n) {
    if (num_bytes < 1 || num_bytes > 4) {
        printf("Wrong number of bytes\n");
        return;
    }

    char n_str[9];
    char format[5];
    format[0] = '%';
    format[1] = '0';
    format[2] = '0' + 2 * num_bytes;
    format[3] = 'X';
    format[4] = '\0';
    sprintf(n_str, format, n);

    size_t n_str_size = strlen(n_str);
    if (2 * (size_t)num_bytes < n_str_size) {
        printf("number of bytes is less than the number hex representation.\n");
        return;
    }

    for (int i = 0; i < n_str_size; i++) {
        append_symbol(str, size, n_str[i]);
    }
}

void append_fid(char* str, size_t* size, uint16_t fid) {

}

bool check_if_contains(char* str1, size_t size1, char* str2, size_t size2) {
    if (size1 < size2) {
        return false;
    }
    size_t i1 = 0;
    size_t i2 = 0;
    size_t size_diff = size1 - size2 + 1;
    while (i1 < size_diff) {
        while (str1[i1] == str2[i2]) {
            i1++;
            i2++;
            if (i2 == size2) {
                return true;
            }
        }
        i1++;
    }
    return false;
}