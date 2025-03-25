#ifndef _APDU_RESPONSE_PARSER_H_
#define _APDU_RESPONSE_PARSER_H_

#include <stdbool.h>
#include <stdint.h>

#define APDU_RSP_BUFFER_SIZE 600

struct ApduRsp {
    uint8_t     sw1;
    uint8_t     sw2;
    uint8_t     rsp[APDU_RSP_BUFFER_SIZE];
    size_t      size;
};

void clear_rsp(struct ApduRsp* rsp);
bool is_rsp_status_ok(struct ApduRsp* rsp);
void print_out_rsp(struct ApduRsp* rsp);

int get_esimexist_response(char* msg, size_t msg_size, int* rsp);
int get_cgla_response(char* msg, size_t msg_size, struct ApduRsp* apdu_rsp);

int get_file_size_from_fcp(uint8_t* fcp, size_t fcp_size, uint16_t* file_size);

#endif // _APDU_RESPONSE_PARSER_H_