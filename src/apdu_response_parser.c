#include <apdu_response_parser.h>
#include <str_utils.h>

#include <string.h>
#include <utils.h>

static const char cgla_tag[] = "CGLA";
static const char esimexist_tag[] = "ESIMEXIST";

static const char response_status_ok[] = "9000";

void clear_rsp(struct ApduRsp* rsp) {
    rsp->sw1    = 0x00;
    rsp->sw2    = 0x00;
    rsp->size   = 0;
}

bool rsp_status_equals(struct ApduRsp* rsp, uint16_t status) {
    return rsp->sw1 == (status >> 8) && rsp->sw2 == (status & 0xFF);
}

bool is_rsp_status_ok(struct ApduRsp* rsp) {
    return rsp_status_equals(rsp, 0x9000);
}

void print_out_rsp(struct ApduRsp* rsp) {
    char sw[3] = "\0\0\0";
    size_t size = 0;
    append_symbol_asbyte_hex(sw, &size, rsp->sw1);
    printf("sw1 = %s (0x%X)\n", sw, rsp->sw1);

    size = 0;
    append_symbol_asbyte_hex(sw, &size, rsp->sw2);
    printf("sw2 = %s (0x%X)\n", sw, rsp->sw2);

    char* rsp_hex = malloc(2 * rsp->size + 1);
    if (rsp_hex == NULL) {
        return;
    }
    size = 0;
    for (size_t i = 0; i < rsp->size; i++) {
        append_symbol_asbyte_hex(rsp_hex, &size, rsp->rsp[i]);
    }
    printf("rsp_size = %zu, rsp = %s\n", rsp->size, rsp_hex);

    free(rsp_hex);
}

int get_esimexist_response(char* msg, size_t msg_size, int* rsp) {
    char* ptr;
    size_t offset;
    size_t response_size;

    ptr = strstr(msg, esimexist_tag);
    // printf("ptr = '%s'\n", ptr);

    if (ptr == NULL) {
        eprint("wrong response messsage format\n");
        return 1;
    }

    offset = strlen(esimexist_tag);
    if (ptr[offset] != ':') {
        eprint("wrong response messsage format\n");
        return 1;
    }
    offset++;

    if (ptr[offset] != ' ') {
        eprint("wrong response messsage format\n");
        return 1;
    }
    offset++;

    if (ptr[offset] != '1' && ptr[offset] != '0') {
        eprint("wrong response messsage format\n");
        return 1;
    }
    *rsp = ptr[offset] - '0';

    return 0;
}

int get_cgla_response(char* msg, size_t msg_size, struct ApduRsp* apdu_rsp) {
    char* ptr;
    size_t offset;
    size_t response_size;

    ptr = strstr(msg, cgla_tag);
    // printf("ptr = '%s'\n", ptr);

    if (ptr == NULL) {
        eprint("get_cgla_response(), wrong response messsage format\n");
        return 1;
    }

    offset = strlen(cgla_tag);
    if (ptr[offset] != ':') {
        eprint("get_cgla_response(), wrong response messsage format\n");
        return 1;
    }
    offset++;

    if (ptr[offset] != ' ') {
        eprint("get_cgla_response(), wrong response messsage format\n");
        return 1;
    }
    offset++;

    response_size = 0;
    while (ptr[offset] != ',') {
        response_size = 10 * response_size + get_digit(ptr[offset]);
        offset++;
    }
    offset++;

    if (ptr[offset] != '\"') {
        eprint("get_cgla_response(), wrong response messsage format\n");
        return 1;
    }
    offset++;

    clear_rsp(apdu_rsp);
    get_bytes_from_hex(ptr + offset, response_size-4, apdu_rsp->rsp, &apdu_rsp->size);
    apdu_rsp->sw1 = get_byte_from_hex(ptr + offset + response_size-4);
    apdu_rsp->sw2 = get_byte_from_hex(ptr + offset + response_size-2);
    // print_out_rsp(apdu_rsp);

    return 0;
}


int get_file_size_from_fcp(uint8_t* fcp, size_t fcp_size, uint16_t* file_size) {
    // 621782020121830204038A01058B03EF0404800200108801189000
    size_t offset = 0;
    *file_size = 0;
    while (offset < fcp_size) {
        uint8_t tag = fcp[offset];
        offset += 1;
        uint8_t tag_size = fcp[offset];
        offset += 1;

        // printf("tag = 0x%X, tag_size = %hu\n", tag, tag_size);
        
        if (tag == 0x80) {
            // printf("fcp = %p, offset = %zu\n", fcp, offset);
            *file_size = fcp[offset] * 256 + fcp[offset + 1];
            return 0;
        }

        if (tag != 0x62) {
            offset += tag_size;
        }
    }

    return 1;
}