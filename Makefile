
BINS = C:\Users\User\AppData\Local\Android\Sdk\ndk\28.0.12674087\toolchains\llvm\prebuilt\windows-x86_64\bin
PREBUILTS_DIR = ${ROOT}/prebuilts

CLANG = ${BINS}/aarch64-linux-android31-clang
# CLANGXX = /data/ssosnin/repos/aq_aosp11/prebuilts/clang/md32rv/linux-x86/bin/clang++

# RUST_BINS = /data/ssosnin/repos/aq_aosp11/prebuilts/rust/linux-x86/1.40.0/bin/

INCDIRS = -I./include
# INCDIRS  = -I$(ROOT)/system/core/include
# INCDIRS += -I$(ROOT)/hardware/libhardware/include
# INCDIRS += -I$(ROOT)/hardware/ril/include
# INCDIRS += -I$(ROOT)/dalvik/libnativehelper/include
# INCDIRS += -I$(ROOT)/frameworks/base/include   # ICS
# INCDIRS += -I$(ROOT)/frameworks/native/include # Jellybean
# INCDIRS += -I$(ROOT)/external/skia/include
# INCDIRS += -I$(ROOT)/out/target/product/generic/obj/include
# INCDIRS += -I$(ROOT)/bionic/libc/arch-arm/include
# INCDIRS += -I$(ROOT)/bionic/libc/include
# INCDIRS += -I$(ROOT)/bionic/libc/kernel/uapi
# INCDIRS += -I$(ROOT)/bionic/libc/kernel/uapi/asm-arm
# INCDIRS += -I$(ROOT)/bionic/libstdc++/include
# INCDIRS += -I$(ROOT)/bionic/libc/kernel/common
# INCDIRS += -I$(ROOT)/bionic/libc/kernel/arch-arm
# INCDIRS += -I$(ROOT)/bionic/libm/include
# INCDIRS += -I$(ROOT)/bionic/libm/include/arch/arm
# INCDIRS += -I$(ROOT)/bionic/libthread_db/include
# INCDIRS += -I$(ROOT)/bionic/libm/arm
# INCDIRS += -I$(ROOT)/bionic/libm
# # INCDIRS += -I$(ROOT)/out/target/product/generic/obj/SHARED_LIBRARIES/libm_intermediates
# INCDIRS += -I$(ROOT)/kernel-4.19/include
# INCDIRS += -I$(ROOT)/kernel-4.19/include/uapi
# INCDIRS += -I$(ROOT)/kernel-4.19/arch/arm/include/
# INCDIRS += -I$(ROOT)/kernel-4.19/arch/arm/include/uapi
# INCDIRS += -I$(ROOT)/kernel-4.19/arch/arm64/include
# INCDIRS += -I$(ROOT)/kernel-4.19/arch/arm64/include/uapi

# LIBDIRS = -L/data/ssosnin/firmware/aqaosp-11-ns220re-out-userdebug/target/product/tb8768p1_64_bsp/obj/STATIC_LIBRARIES/libc_intermediates/
# LIBDIRS += -L/data/ssosnin/firmware/aqaosp-11-ns220re-out-userdebug/target/product/tb8768p1_64_bsp/obj/STATIC_LIBRARIES/libm_intermediates/
# LIBDIRS += -L/data/ssosnin/firmware/aqaosp-11-ns220re-out-userdebug/target/product/tb8768p1_64_bsp/obj/STATIC_LIBRARIES/libdl_intermediates/

# INCDIRS = -I/data/ssosnin/repos/aq_aosp11/bionic/libc/include \
# 		  -I/data/ssosnin/repos/aq_aosp11/bionic/libc/kernel/uapi/asm-arm64 \
# 		  -I/data/ssosnin/repos/aq_aosp11/bionic/libc/kernel/uapi \
# 		  -I/data/ssosnin/repos/aq_aosp11/kernel-4.19/include
# 		  # -I/data/ssosnin/repos/aq_aosp11/kernel-4.19/arch/arm64/include

# INCDIRS = -I/data/ssosnin/repos/aq_aosp11/optee/optee_os/lib/libutils/isoc/include \
		  -I/data/ssosnin/repos/aq_aosp11/optee/optee_os/lib/libutils/ext/include \

FLAGS = -D__STDC_LIMIT_MACROS 
FLAGS += -lc -lm -ldl
# FLAGS += -D__ASSEMBLY__
# FLAGS += -D__ANDROID_API__=29
# FLAGS += -D__LP64__

SRCS = ./apdu_commands.c
SRCS += ./apdu_response_parser.c
SRCS += ./at_commands.c
SRCS += ./fcp_parser.c
SRCS += ./main.c
SRCS += ./uicc_test1.c
SRCS += ./uicc_common.c
SRCS += ./sec_storage.c
SRCS += ./str_utils.c

# <toolchain>/arm-linux-androideabi-g++ ~/test.c -o ~/test --sysroot=/home/user/android-ndk/platforms/android-9/arch-arm/

# --sysroot=/data/ssosnin/repos/aq_aosp11/prebuilts/ndk/r21/platforms/android-29/arch-arm64/usr/lib

build:
	${CLANG} ${SRCS} ${INCDIRS} -o out/uicc_test --target=aarch64-linux-android21 -std=c11
