#ifndef _FCP_PARSER_H_
#define _FCP_PARSER_H_

#include <stdbool.h>
#include <stdint.h>

// #define APDU_RSP_BUFFER_SIZE 600

struct TagDscr {
    uint8_t     tag;
    uint8_t     tag_size;
    uint8_t*    value;
    uint8_t     value_size;
};

struct TlvDscr {
    struct TagDscr* tags;
    uint8_t         size;
};

struct TagDscr_0xA5_KF {
    struct TagDscr tag_0x85;
    struct TagDscr tag_0x86;
    struct TagDscr tag_0x87;
    struct TagDscr tag_0x89;
    struct TagDscr tag_0x8B;
    struct TagDscr tag_0x8E;
};

struct TagDscr_0xA5_BF_FRF_CRF {
    struct TagDscr tag_0x81;
};

struct FcpDscr {
    struct TagDscr tag_0x80;
    struct TagDscr tag_0x82;
    struct TagDscr tag_0x83;
    struct TagDscr tag_0x84;
    struct TagDscr tag_0x88;
    struct TagDscr tag_0x8A;
    struct TagDscr tag_0x8B;
    struct TagDscr_0xA5_KF tag_kf_0xA5;
    struct TagDscr_0xA5_BF_FRF_CRF tag_bf_frf_crf_0xA5;
};

void init_fcp_dscr(struct FcpDscr* dscr);
void free_fcp_dscr(struct FcpDscr* dscr);
void print_fcp_dscr(struct FcpDscr* dscr);
void config_fcp_dscr(struct FcpDscr* dscr, uint8_t* fcp, size_t fcp_size);

#endif // _FCP_PARSER_H_