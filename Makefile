HOME_MODULES = /usr/src/app/kms/Enclave/Wasm/Modules
HOME_CSCORE_COMMON = /usr/src/app/cscore-common
MODULE_NAME = Core
MAIN_FILE = kms-core

SRCS = mongoose.c ${HOME_MODULES}/${MODULE_NAME}/${MAIN_FILE}.cpp \
	${HOME_MODULES}/Common/StorageHandlers/BaseStorageHandler.cpp \
	${HOME_MODULES}/Common/Privaton/privaton.cpp \
	${HOME_CSCORE_COMMON}/Wasm/Modules/HttpClient/http-client.cpp \
	${HOME_CSCORE_COMMON}/Wasm/Modules/ExchangeClient/exchange-client.cpp  \
	${HOME_CSCORE_COMMON}/Wasm/Modules/ExchangeClient/ExchangeHandlers/BaseHandler.cpp \
	${HOME_CSCORE_COMMON}/Wasm/Modules/SignatureGenerator/signature-generator.cpp \
	${HOME_CSCORE_COMMON}/Wasm/Modules/SignatureGenerator/Crypto/Crypto.cpp  


DEFS ?= -DMG_MAX_HTTP_HEADERS=7 -DMG_ENABLE_LINES -DMG_ENABLE_PACKED_FS=1 -DMG_ENABLE_SSI=1 -DSQLITE_OS_OTHER \
    -DSQLITE_ENABLE_MEMSYS3
WARN ?= -pedantic -W -Wall -Wno-error -Wshadow -Wdouble-promotion -fno-common -Wconversion -Wundef
OPTS ?= -O3 -g3
INCS ?= -Isrc -I.
CWD ?= $(realpath $(CURDIR))
ENV ?=  -e Tmp=. -e WINEDEBUG=-all
DOCKER ?= docker run --platform linux/amd64 --rm $(ENV) -v $(CWD):$(CWD) -w $(CWD)
VCFLAGS = /nologo /W3 /O2 /MD /I. $(DEFS) $(TFLAGS)
IPV6 ?= 1
ASAN ?= -fsanitize=address,undefined,alignment -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-common
ASAN_OPTIONS ?= detect_leaks=1
EXAMPLES := $(dir $(wildcard examples/*/Makefile)) $(wildcard examples/stm32/nucleo-*)
PREFIX ?= /usr/local
VERSION ?= $(shell cut -d'"' -f2 src/version.h)
COMMON_CFLAGS ?= $(C_WARN) $(WARN) $(INCS) $(DEFS) -DMG_ENABLE_IPV6=$(IPV6) $(TFLAGS)
CFLAGS ?= $(OPTS) $(ASAN) $(COMMON_CFLAGS)
VALGRIND_CFLAGS ?= $(OPTS) $(COMMON_CFLAGS)
VALGRIND_RUN ?= valgrind --tool=memcheck --gen-suppressions=all --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --error-exitcode=1 --exit-on-first-error=yes
.PHONY: examples test valgrind mip_test


ifeq "$(SSL)" "WOLFSSL"
WOLFSSL ?= /usr/local
CFLAGS  += -DMG_ENABLE_WOLFSSL=1 -I$(WOLFSSL)/include
LDFLAGS ?= -L$(WOLFSSL)/lib -lwolfssl
ifdef MG_ENABLE_WOLFSSL_DEBUG
CFLAGS += -DMG_ENABLE_WOLFSSL_DEBUG
endif
endif

wasm: WASI_SDK_PATH ?= /opt/wasi-sdk
wasm: WAMR_PATH ?= /opt/wamr
wasm: ASAN=
wasm: CFLAGS += -DWOLFSSL_WASM=1 -DWAMR_BUILD_SGX_IPFS=1
wasm: INCS += -I$(WAMR_PATH)/core/iwasm/libraries/lib-socket/inc
wasm: WARN += -Wno-sign-conversion -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
wasm: IPV6 = 0
wasm: WOLFSSL = ../wolfssl
wasm: CFLAGS  += -I$(WOLFSSL) -I${HOME_MODULES}/Common -I${HOME_MODULES}/${MODULE_NAME} \
	 -I${HOME_CSCORE_COMMON}/Wasm/Modules -I${HOME_CSCORE_COMMON}/Wasm/Modules/ExchangeClient \
	 -I${HOME_CSCORE_COMMON}/include -I$(WAMR_PATH)/core/iwasm/libraries/lib-rats \
	 -I${HOME_CSCORE_COMMON}/Wasm/Modules/SignatureGenerator
wasm: LDFLAGS += -L$(WOLFSSL)/IDE/Wasm -lwolfssl -L${HOME_MODULES}/Common/Sqlite -lsqlite
#-fno-exceptions
#-Wl,--max-memory=536870912
wasm: Makefile $(SRCS)
	if [ ! -d "$(WOLFSSL)" ]; then echo "The WOLFSSL variable does not point on a valid folder: $(WOLFSSL)"; exit 1; fi
	$(WASI_SDK_PATH)/bin/clang++ \
		-DJSON_HAS_FILESYSTEM=0 \
		-fcxx-exceptions -isysroot/ \
		-std=c++2b -O3 --target=wasm32-wasi \
		 -Wl,--export=malloc -Wl,--export=free \
		-Wl,--export=realloc \
		-Wl,--export=__heap_base \
		-Wl,--export=__data_end \
		-Wl,--initial-memory=10485760 \
		-z stack-size=8388608 \
		--sysroot=$(WASI_SDK_PATH)/share/wasi-sysroot/ \
		-Wl,--allow-undefined \
		-Wl,--allow-undefined-file=$(WASI_SDK_PATH)/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
		-Wl,--strip-all \
		-Wno-vla-extension \
		$(CFLAGS) \
		$(LDFLAGS) \
		-o ${HOME_MODULES}/${MODULE_NAME}/${MAIN_FILE}.wasm \
		$(SRCS) $(WAMR_PATH)/core/iwasm/libraries/lib-socket/src/wasi/wasi_socket_ext.c

# mongoose.c: Makefile $(wildcard src/*) $(wildcard mip/*.c)
# 	(cat src/license.h; echo; echo '#include "mongoose.h"' ; (for F in src/*.c mip/*.c ; do echo; echo '#ifdef MG_ENABLE_LINES'; echo "#line 1 \"$$F\""; echo '#endif'; cat $$F | sed -e 's,#include ".*,,'; done))> $@

# mongoose.h: $(HDRS) Makefile
# 	(cat src/license.h; echo; echo '#ifndef MONGOOSE_H'; echo '#define MONGOOSE_H'; echo; cat src/version.h ; echo; echo '#ifdef __cplusplus'; echo 'extern "C" {'; echo '#endif'; cat src/arch.h src/arch_*.h src/config.h src/str.h src/fmt.h src/log.h src/timer.h src/fs.h src/util.h src/url.h src/iobuf.h src/base64.h src/md5.h src/sha1.h src/event.h src/net.h src/http.h src/ssi.h src/tls.h src/tls_mbed.h src/tls_openssl.h src/tls_wolfssl.h src/ws.h src/sntp.h src/mqtt.h src/dns.h src/json.h src/rpc.h mip/mip.h mip/driver_*.h | sed -e '/keep/! s,#include ".*,,' -e 's,^#pragma once,,'; echo; echo '#ifdef __cplusplus'; echo '}'; echo '#endif'; echo '#endif  // MONGOOSE_H')> $@


clean:
	rm -rf $(PROG) *.exe *.o *.dSYM *_test* ut fuzzer *.gcov *.gcno *.gcda *.obj *.exe *.ilk *.pdb slow-unit* _CL_* infer-out data.txt crash-* test/packed_fs.c pack
	@for X in $(EXAMPLES); do $(MAKE) -C $$X clean; done