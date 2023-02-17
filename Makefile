SRCS = mongoose.c cli.cpp 
SSL ?= WOLFSSL

#DEFS ?= -DMG_MAX_HTTP_HEADERS=7 -DMG_ENABLE_LINES -DMG_ENABLE_PACKED_FS=1 -DMG_ENABLE_SSI=1 -DSQLITE_OS_OTHER \

DEFS ?= -DMG_MAX_HTTP_HEADERS=7 -DMG_ENABLE_LINES -DMG_ENABLE_PACKED_FS=1 -DMG_ENABLE_SSI=1 -DSQLITE_OS_OTHER \
    -DSQLITE_ENABLE_MEMSYS3

WARN ?= -pedantic -W -Wall -Wno-error -Wshadow -Wdouble-promotion -fno-common -Wconversion -Wundef
OPTS ?= -O3 -g3
INCS ?= -Isrc -I.
CWD ?= $(realpath $(CURDIR))
ENV ?=  -e Tmp=. -e WINEDEBUG=-all
IPV6 ?= 1
ASAN ?= -fsanitize=address,undefined,alignment -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-common
ASAN_OPTIONS ?= detect_leaks=1
EXAMPLES := $(dir $(wildcard examples/*/Makefile)) $(wildcard examples/stm32/nucleo-*)
PREFIX ?= /usr/local
VERSION ?= $(shell cut -d'"' -f2 src/version.h)
COMMON_CFLAGS ?= $(C_WARN) $(WARN) $(INCS) $(DEFS) -DMG_ENABLE_IPV6=$(IPV6) $(TFLAGS)
CFLAGS ?= $(OPTS) $(ASAN) $(COMMON_CFLAGS)
.PHONY: examples test valgrind mip_test


#ifeq "$(SSL)" "WOLFSSL"
WOLFSSL ?= /opt/wolfssl
CFLAGS  += -DMG_ENABLE_WOLFSSL=1 -I$(WOLFSSL)/include -DMG_ARCH=MG_ARCH_WASM -DMG_IO_SIZE=8192 -DMG_MAX_RECV_SIZE=8*1024*1024
#CFLAGS  += -DMG_ENABLE_WOLFSSL=1 -I$(WOLFSSL)/include 
LDFLAGS ?= -L$(WOLFSSL)/lib -lwolfssl
#ifdef MG_ENABLE_WOLFSSL_DEBUG
CFLAGS += -DMG_ENABLE_WOLFSSL_DEBUG
#endif
#endif

# calculate --initial-memory:


wasm: WASI_SDK_PATH ?= /opt/wasi-sdk
wasm: WAMR_PATH ?= /wasm-micro-runtime
#wasm: ASAN= -fno-omit-frame-pointer -fno-common
wasm: ASAN =
wasm: CFLAGS += -DWOLFSSL_WASM=1 -DWAMR_BUILD_SGX_IPFS=1 
wasm: INCS += -I$(WAMR_PATH)/core/iwasm/libraries/lib-socket/inc
wasm: WARN += -Wno-sign-conversion -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
wasm: IPV6 = 0
wasm: WOLFSSL = /opt/wolfssl
wasm: CFLAGS  += -I$(WOLFSSL) 
wasm: CFLAGS  += -I.
wasm: CFLAGS  += -I$(WAMR_PATH)/core/iwasm/libraries/lib-socket/inc
wasm: LDFLAGS += -L$(WOLFSSL)/IDE/Wasm -lwolfssl 
#-fno-exceptions
#-Wl,--max-memory=536870912
wasm: Makefile mongoose.h $(SRCS)
	if [ ! -d "$(WOLFSSL)" ]; then echo "The WOLFSSL variable does not point on a valid folder: $(WOLFSSL)"; exit 1; fi
	$(WASI_SDK_PATH)/bin/clang++ -std=c++20 -O3 \
		--target=wasm32-wasi \
		-fno-exceptions \
		-Wl,--export=malloc -Wl,--export=free \
 		-Wl,--export=__heap_base -Wl,--export=__data_end \
		-z stack-size=8388608 \
		-Wl,--initial-memory=67108864 \
		--sysroot=$(WASI_SDK_PATH)/share/wasi-sysroot/ \
		-Wl,--allow-undefined \
		-Wl,--allow-undefined-file=$(WASI_SDK_PATH)/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
		-mexec-model=reactor \
		$(CFLAGS) \
		$(LDFLAGS) \
		$(SRCS) $(WAMR_PATH)/core/iwasm/libraries/lib-socket/src/wasi/wasi_socket_ext.c -o cli.wasm \


#wasm: $(shell /opt/wasi-sdk/bin/llvm-ar r libmongoose.a mongoose.o wasi_socket_ext.o )

mongoose.c: Makefile $(wildcard src/*) $(wildcard mip/*.c)
	(cat src/license.h; echo; echo '#include "mongoose.h"' ; (for F in src/*.c mip/*.c ; do echo; echo '#ifdef MG_ENABLE_LINES'; echo "#line 1 \"$$F\""; echo '#endif'; cat $$F | sed -e 's,#include ".*,,'; done))> $@

mongoose.h: $(HDRS) Makefile
	(cat src/license.h; echo; echo '#ifndef MONGOOSE_H'; echo '#define MONGOOSE_H'; echo; cat src/version.h ; echo; echo '#ifdef __cplusplus'; echo 'extern "C" {'; echo '#endif'; cat src/arch.h src/arch_*.h src/config.h src/str.h src/fmt.h src/log.h src/timer.h src/fs.h src/util.h src/url.h src/iobuf.h src/base64.h src/md5.h src/sha1.h src/event.h src/net.h src/http.h src/ssi.h src/tls.h src/tls_mbed.h src/tls_openssl.h src/tls_wolfssl.h src/ws.h src/sntp.h src/mqtt.h src/dns.h src/json.h src/rpc.h mip/mip.h mip/driver_*.h | sed -e '/keep/! s,#include ".*,,' -e 's,^#pragma once,,'; echo; echo '#ifdef __cplusplus'; echo '}'; echo '#endif'; echo '#endif  // MONGOOSE_H')> $@

clean:
	rm -rf $(PROG) *.exe *.o *.dSYM *_test* ut fuzzer *.gcov *.gcno *.gcda *.obj *.exe *.ilk *.pdb slow-unit* _CL_* infer-out data.txt crash-* test/packed_fs.c pack
	@for X in $(EXAMPLES); do $(MAKE) -C $$X clean; done

