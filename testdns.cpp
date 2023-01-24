#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <json.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#define IS_SGX
#include "mongoose.h"
#include "exchange-client.h"
#include "side-core-utils.hpp"


std::string privKey;
std::string certificate;
static const char* s_https_addr = "https://0.0.0.0:8866";
static const char* s_root_dir = ".";
static uint64_t computation_salt;



int
main(int argc, char* argv[])
{

    printf("[WASM-CORE] Started Core module\n");
    struct mg_mgr mgr; // Event manager
    mg_log_set(MG_LL_DEBUG); // Set log level

    create_cert_files("",
                  "");

    std::string json_reqstr = "{\"apikey\":\"u81IzdOu686FWSc2xwCIl5myzDWaDspKQOvjdnMGIYcPmLfIZi8QzMW23RbTAgsb\",\"exchange\":\"binance\", \"key_id\":\"5e6af16d-146b-4af1-b675-ef5d449669b3\",\"secret\":\"Fgs615IKjLJjAsfdibBxEf0wZzlWk1eIJ8lAKjfzNSAVc93xIKwoegY9Gsc46jwN\",\"uuid\":\"7a0cf704-8d0b-4b4a-b5fa-6ef04ea418d1\"}";
    std::string result;
    auto resfunc =
        make_request((uint32_t)(uintptr_t)json_reqstr.c_str(),
                      json_reqstr.length(),
                      result);

    return 0;
}