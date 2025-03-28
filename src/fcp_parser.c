#include "fcp_parser.h"

#include <stdlib.h>
#include <string.h>

// Table 4
static char* get_file_type(uint8_t file_type) {
    if (file_type == 0x38 || file_type == 0x78) {
        return "DF / MF";
    } else if (file_type == 0x01 || file_type == 0x41) {
        return "BF";
    } else if (file_type == 0x02 || file_type == 0x42) {
        return "FRF";
    } else if (file_type == 0x06 || file_type == 0x46) {
        return "CRF";
    } else if (file_type == 0x11 || file_type == 0x51) {
        return "KF";
    } else if (file_type == 0x39 || file_type == 0x79) {
        return "TF";
    }
    return "unknown file type";
}

// Table 6
static char* get_lyfe_cycle_state(uint8_t state_id) {
    if (state_id == 0x03) {
        return "Init";
    } else if (state_id == 0x05) {
        return "Use";
    } else if (state_id == 0x04) {
        return "Block";
    } else if (state_id == 0x0C) {
        return "Terminate";
    }
    return "unknown LCS";
}

void init_fcp_dscr(struct FcpDscr* dscr) {
    struct FcpDscr tmp = {
        .tag_0x80 = { .tag = 0x80, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x82 = { .tag = 0x82, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x83 = { .tag = 0x83, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x84 = { .tag = 0x84, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x88 = { .tag = 0x88, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x8A = { .tag = 0x8A, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_0x8B = { .tag = 0x8B, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        .tag_kf_0xA5 = {
            .tag_0x85 = { .tag = 0x85, .tag_size = 0x00, .value = NULL, .value_size = 0 },
            .tag_0x86 = { .tag = 0x86, .tag_size = 0x00, .value = NULL, .value_size = 0 },
            .tag_0x87 = { .tag = 0x87, .tag_size = 0x00, .value = NULL, .value_size = 0 },
            .tag_0x89 = { .tag = 0x89, .tag_size = 0x00, .value = NULL, .value_size = 0 },
            .tag_0x8B = { .tag = 0x8B, .tag_size = 0x00, .value = NULL, .value_size = 0 },
            .tag_0x8E = { .tag = 0x8E, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        },
        .tag_bf_frf_crf_0xA5 = {
            .tag_0x81 = { .tag = 0x81, .tag_size = 0x00, .value = NULL, .value_size = 0 },
        },
    };
    *dscr = tmp;
}

void free_fcp_dscr(struct FcpDscr* dscr) {
    free(dscr->tag_0x80.value);
    free(dscr->tag_0x82.value);
    free(dscr->tag_0x83.value);
    free(dscr->tag_0x84.value);
    free(dscr->tag_0x88.value);
    free(dscr->tag_0x8A.value);
    free(dscr->tag_0x8B.value);
    free(dscr->tag_kf_0xA5.tag_0x85.value);
    free(dscr->tag_kf_0xA5.tag_0x86.value);
    free(dscr->tag_kf_0xA5.tag_0x87.value);
    free(dscr->tag_kf_0xA5.tag_0x89.value);
    free(dscr->tag_kf_0xA5.tag_0x8B.value);
    free(dscr->tag_kf_0xA5.tag_0x8E.value);
    free(dscr->tag_bf_frf_crf_0xA5.tag_0x81.value);
}

void print_fcp_dscr(struct FcpDscr* dscr) {
    if (dscr->tag_0x80.tag_size > 0) {
        uint16_t file_size = dscr->tag_0x80.value[0] * 16 + dscr->tag_0x80.value[1];
        printf("tag 0x80 (file size):\n - %hu (0x%x) bytes\n", file_size, file_size);
    }

    bool is_kf_flag = false;
    if (dscr->tag_0x82.tag_size > 0) {
        is_kf_flag = dscr->tag_0x82.value[0] == 0x11 || dscr->tag_0x82.value[0] == 0x51;
        printf("tag 0x82 (file type):\n - file type 0x%X (%s)\n", dscr->tag_0x82.value[0], get_file_type(dscr->tag_0x82.value[0]));
        if (dscr->tag_0x82.tag_size > 3) {
            printf(" - record size %hu\n", dscr->tag_0x82.value[3]);
        }
        if (dscr->tag_0x82.tag_size > 4) {
            printf(" - record number %hu\n", dscr->tag_0x82.value[4]);
        }
    }

    if (dscr->tag_0x83.tag_size > 0) {
        printf("tag_0x83 (FID):\n");
        printf(" - 0x%02X%02X\n",  dscr->tag_0x83.value[0], dscr->tag_0x83.value[1]);
    }

    if (dscr->tag_0x84.tag_size > 0) {
        printf("tag_0x84 (AID), tag_size = %hu\n", dscr->tag_0x84.tag_size);
        printf(" - 0x");
        for (size_t i = 0; i < dscr->tag_0x84.tag_size; i++) {
            printf("%02X", dscr->tag_0x84.value[i]);
        }
        printf("\n");
    }

    if (dscr->tag_0x88.tag_size > 0) {
        printf("tag_0x88 (SFI):\n");
        printf(" - 0x%02X\n", dscr->tag_0x88.value[0]);
    }

    if (dscr->tag_0x8A.tag_size > 0) {
        printf("tag_0x8A (LCS): \n");
        printf(" - 0x%02X (%s)\n", dscr->tag_0x8A.value[0], get_lyfe_cycle_state(dscr->tag_0x8A.value[0]));
    }

    if (dscr->tag_0x8B.tag_size > 0) {
        printf("tag_0x8B(EF_arr reference): \n");
        printf(" - 0x%02X%02X, record number %hu (0x%02X)\n", 
            dscr->tag_0x8B.value[0], dscr->tag_0x8B.value[1], dscr->tag_0x8B.value[2], dscr->tag_0x8B.value[2]);
    }

    if (is_kf_flag) {
        struct TagDscr_0xA5_KF* dscr_0xA5 = &dscr->tag_kf_0xA5;
        if (dscr_0xA5->tag_0x85.tag_size > 0) {
            printf("tag 0xA5 - 0x85: key algorithm - %hu\n", dscr_0xA5->tag_0x85.value[0]);
        }

        if (dscr_0xA5->tag_0x86.tag_size > 0) {
            printf("tag 0xA5 - 0x86: key purpose - 0x%X%X\n", 
                dscr_0xA5->tag_0x86.value[0], dscr_0xA5->tag_0x86.value[1]);
        }

        if (dscr_0xA5->tag_0x87.tag_size > 0) {
            printf("tag 0xA5 - 0x87: key ID - 0x%X\n", dscr_0xA5->tag_0x87.value[0]);
        }

        if (dscr_0xA5->tag_0x89.tag_size > 0) {
            printf("tag 0xA5 - 0x89: max try number - %hu\n", dscr_0xA5->tag_0x89.value[0]);
        }

        if (dscr_0xA5->tag_0x8B.tag_size > 0) {
            printf("tag 0xA5 - 0x8B: KF flags - %hu\n", dscr_0xA5->tag_0x8B.value[2]);
        }

        if (dscr_0xA5->tag_0x8E.tag_size > 0) {
            printf("tag 0xA5 - 0x8E: algorithm parameters for GOST P34.10-12 - %hu\n", dscr_0xA5->tag_0x8E.value[0]);
        }
    } else {
        struct TagDscr_0xA5_BF_FRF_CRF* dscr_0xA5 = &dscr->tag_bf_frf_crf_0xA5;
        if (dscr_0xA5->tag_0x81.tag_size > 0) {
            printf("tag 0xA5 - 0x81: TBD\n");
        }
    }
}

static void int_tag_dscr(struct TagDscr* dscr, uint8_t tag_size, uint8_t* value) {
    dscr->tag_size = tag_size;
    dscr->value = malloc(tag_size);
    memcpy(dscr->value, value, tag_size);
}

static void config_0xA5_KF_dscr(struct TagDscr_0xA5_KF* dscr, uint8_t* value, size_t value_size) {
    size_t offset = 0;
    while (offset < value_size) {
        uint8_t tag = value[offset];
        offset += 1;
        uint8_t tag_size = value[offset];
        offset += 1;

        if (tag == 0x85) {
            int_tag_dscr(&dscr->tag_0x85, tag_size, value + offset);
        } else if (tag == 0x86) {
            int_tag_dscr(&dscr->tag_0x86, tag_size, value + offset);
        } else if (tag == 0x87) {
            int_tag_dscr(&dscr->tag_0x87, tag_size, value + offset);
        } else if (tag == 0x89) {
            int_tag_dscr(&dscr->tag_0x89, tag_size, value + offset);
        } else if (tag == 0x8B) {
            int_tag_dscr(&dscr->tag_0x8B, tag_size, value + offset);
        } else if (tag == 0x8E) {
            int_tag_dscr(&dscr->tag_0x8E, tag_size, value + offset);
        }
        offset += tag_size;
    }
}

static void config_0xA5_BF_FRF_CRF(struct TagDscr_0xA5_BF_FRF_CRF* dscr, uint8_t* value, size_t value_size) {
    size_t offset = 0;
    while (offset < value_size) {
        uint8_t tag = value[offset];
        offset += 1;
        uint8_t tag_size = value[offset];
        offset += 1;

        if (tag == 0x81) {
            int_tag_dscr(&dscr->tag_0x81, tag_size, value + offset);
        }
        offset += tag_size;
    }
}

void config_fcp_dscr(struct FcpDscr* dscr, uint8_t* fcp, size_t fcp_size) {
    // 6217
    // 82 - 02 01 21 
    // 83 - 02 04 03
    // 8A - 01 05
    // 8B - 03 EF 04 04 
    // 80 - 02 00 53
    // 88 01 18 
    // 9000

    init_fcp_dscr(dscr);

    bool is_kf_flag = false;
    size_t offset = 0;
    while (offset < fcp_size) {
        uint8_t tag = fcp[offset];
        offset += 1;
        uint8_t tag_size = fcp[offset];
        offset += 1;

        if (tag == 0x80) {
            int_tag_dscr(&dscr->tag_0x80, tag_size, fcp + offset);
        } else if (tag == 0x82) {
            int_tag_dscr(&dscr->tag_0x82, tag_size, fcp + offset);
            is_kf_flag = fcp[offset] == 0x11 || fcp[offset] == 0x51;
        } else if (tag == 0x83) {
            int_tag_dscr(&dscr->tag_0x83, tag_size, fcp + offset);
        } else if (tag == 0x84) {
            int_tag_dscr(&dscr->tag_0x84, tag_size, fcp + offset);
        } else if (tag == 0x88) {
            int_tag_dscr(&dscr->tag_0x88, tag_size, fcp + offset);
        } else if (tag == 0x8A) {
            int_tag_dscr(&dscr->tag_0x8A, tag_size, fcp + offset);
        } else if (tag == 0x8B) {
            int_tag_dscr(&dscr->tag_0x8B, tag_size, fcp + offset);
        } else if (tag == 0xA5) {
            if (is_kf_flag) {
                config_0xA5_KF_dscr(&dscr->tag_kf_0xA5, fcp + offset, tag_size);
            } else {
                config_0xA5_BF_FRF_CRF(&dscr->tag_bf_frf_crf_0xA5, fcp + offset, tag_size);
            }
        }

        if (tag != 0x62) {
            offset += tag_size;
        }
    }

}

bool is_df_md_file_type(struct FcpDscr* dscr) {
    return dscr->tag_0x82.tag_size > 0 && 
        (dscr->tag_0x82.value[0] == 0x38 || dscr->tag_0x82.value[0] == 0x78);
}

bool is_bf_file_type(struct FcpDscr* dscr) {
    return dscr->tag_0x82.tag_size > 0 && 
        (dscr->tag_0x82.value[0] == 0x01 || dscr->tag_0x82.value[0] == 0x41);
}

uint16_t get_fid(struct FcpDscr* dscr) {
    uint16_t fid = 0;
    if (dscr->tag_0x83.tag_size > 0) {
        fid = dscr->tag_0x83.value[0] * 256 + dscr->tag_0x83.value[1];
    }
    return fid;
}

uint16_t get_file_size(struct FcpDscr* dscr) {
    uint16_t size = 0;
    if (dscr->tag_0x80.tag_size > 0) {
        size = dscr->tag_0x80.value[0] * 256 + dscr->tag_0x80.value[1];
    }
    return size;
}