cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE "/opt/wasi-sdk/share/cmake/wasi-sdk.cmake")
set(WASI_SDK_PREFIX "/opt/wasi-sdk") 
set(CMAKE_SYSROOT "/opt/wasi-sdk/share/wasi-sysroot/") 

project(kms-modules)

set(CMAKE_CXX_STANDARD 20)

set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT DEFINED WASI_SDK_PATH)
    set(WASI_SDK_PATH "/opt/wasi-sdk")
endif()

if(NOT DEFINED WOLFSSL_ROOT)
    set(WOLFSSL_ROOT "/opt/wolfssl")
endif()

if(NOT DEFINED WASM_WOLFSSL_LIB)
    set(WASM_WOLFSSL_LIB "${WOLFSSL_ROOT}/IDE/Wasm")
endif()

if(NOT DEFINED WAMR_PATH)
    set(WAMR_PATH "/wasm-micro-runtime")
endif()

if(NOT DEFINED COMMON_PATH)
    set(COMMON_PATH "/usr/src/app/cscore-common")
endif()

if(NOT DEFINED COMMON_CFLAGS)
    set(COMMON_CFLAGS "${C_WARN} ${WARN} ${INCS} ${DEFS} ${TFLAGS}")
endif()

if(NOT DEFINED CFLAGS)
    set(CFLAGS "${OPTS} ${ASAN} ${COMMON_CFLAGS}")
endif()

if(NOT DEFINED VALGRIND_CFLAGS)
    set(VALGRIND_CFLAGS "${OPTS} ${COMMON_CFLAGS}")
endif()

if(NOT DEFINED VALGRIND_RUN)
    set(VALGRIND_RUN "valgrind --tool=memcheck --gen-suppressions=all --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --error-exitcode=1 --exit-on-first-error=yes")
endif()

set(WOLFSSL "/opt/wolfssl")

add_definitions(-DMG_ENABLE_WOLFSSL=1)
add_definitions(-DMG_ENABLE_WOLFSSL_DEBUG)
add_definitions(-DMG_IO_SIZE=8192*2)
add_definitions(-DMG_MAX_RECV_SIZE=64*1024*1024)
add_definitions(-DMG_MAX_HTTP_HEADERS=7)
add_definitions(-DMG_ENABLE_LINES)
add_definitions(-DMG_ENABLE_SSI=1)
add_definitions(-DMG_ENABLE_PACKED_FS=1)
add_definitions(-DMG_ENABLE_IPV6=0)
add_definitions(-DMG_ARCH=MG_ARCH_WASM)
add_definitions(-DWOLFSSL_WASM=1)
add_definitions(-DSQLITE_OS_OTHER)
add_definitions(-DSQLITE_ENABLE_MEMSYS3)

math(EXPR STACK_SIZE "32 * 1024 * 1024")
math(EXPR INITIAL_MEMORY "128 * 1024 * 1024")

set(CMAKE_CXX_FLAGS_DEBUG "${CFLAGS} ${CMAKE_CXX_FLAGS_DEBUG} \
                                -Wno-unused-command-line-argument \
                                -Wno-deprecated -isysroot \
                                -std=c++2b -O3 -g3 \
                                -Wl,--export=malloc -Wl,--export=free \
                                -Wl,--export=realloc \
                                -Wl,--export=__heap_base \
                                -Wl,--export=__data_end \
                                -Wl,--initial-memory=${INITIAL_MEMORY} \
                                -z stack-size=${STACK_SIZE} \
                                -Wl,--allow-undefined \
                                -fno-omit-frame-pointer \
                                -Wno-vla-extension \
                                --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot/ \
                                --target=wasm32-wasi \
                                -Wl,--strip-all")


set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -DNDEBUG -O3")

set(WAMRC_SIZE_LEVEL 1)
set(WAMRC_OPT_LEVEL 3)
set(WAMRC "${WAMR_PATH}/wamr-compiler/build/wamrc")
set(WAMRC_PARAMS --size-level=${WAMRC_SIZE_LEVEL} --opt-level=${WAMRC_OPT_LEVEL} -sgx)

add_definitions(-DWOLFSSL_WASM=1 -DWOLFSSL_AES_DIRECT -DHAVE_AES_KEYWRAP -DDISABLE_TRACE -DHAVE_WASI_SOCKET -DJSON_HAS_FILESYSTEM=0 -DWAMRC_SIZE_LEVEL=1 -DWAMRC_OPT_LEVEL=3)
link_directories(${WASM_WOLFSSL_LIB})
link_libraries(wolfssl)

include_directories(./)
include_directories(${CMAKE_SOURCE_DIR}/include ${WOLFSSL_ROOT} ${WOLFSSL_ROOT}/wolfssl ${WAMR_PATH}/core/iwasm/libraries/lib-socket/inc/ ${WAMR_PATH}/core/iwasm/libraries/lib-rats)
include_directories(${COMMON_PATH}/Wasm/Modules/ExchangeClient/)
include_directories(${COMMON_PATH}/include)
include_directories(${COMMON_PATH}/Wasm/Modules)
include_directories(${COMMON_PATH}/Wasm/Modules/SignatureGenerator)

set(CORE_SRC testdns.cpp)
set(EXCHANGE_CLIENT 
    ${COMMON_PATH}/Wasm/Modules/ExchangeClient/exchange-client.cpp 
    ${COMMON_PATH}/Wasm/Modules/ExchangeClient/ExchangeHandlers/BaseHandler.cpp)
set(SIGNATURE_GENERATOR ${COMMON_PATH}/Wasm/Modules/SignatureGenerator/signature-generator.cpp ${COMMON_PATH}/Wasm/Modules/SignatureGenerator/Crypto/Crypto.cpp )
set(HTTP_CLIENT ${COMMON_PATH}/Wasm/Modules/HttpClient/http-client.cpp )

add_executable(testdns.wasm 
    testdns.cpp 
    ${CORE_SRC}
    ${EXCHANGE_CLIENT}
    ${HTTP_CLIENT}
    ${SIGNATURE_GENERATOR}
    ${CMAKE_SOURCE_DIR}/mongoose.c
    ${WAMR_PATH}/core/iwasm/libraries/lib-socket/src/wasi/wasi_socket_ext.c
    )


