#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
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

#ifndef WOLFSSL_SHA512
#define WOLFSSL_SHA512
#endif 

const std::size_t N_RETRY = 10;
const std::size_t SLEEP_TIME = 500;

int
main(int argc, char* argv[])
{

    printf("[WASM-CORE] Started Core module\n");

    create_cert_files("","");

    std::string json_reqstr = "{\"apikey\":\"u81IzdOu686FWSc2xwCIl5myzDWaDspKQOvjdnMGIYcPmLfIZi8QzMW23RbTAgsb\",\"exchange\":\"binance\", \"key_id\":\"5e6af16d-146b-4af1-b675-ef5d449669b3\",\"secret\":\"Fgs615IKjLJjAsfdibBxEf0wZzlWk1eIJ8lAKjfzNSAVc93xIKwoegY9Gsc46jwN\",\"uuid\":\"7a0cf704-8d0b-4b4a-b5fa-6ef04ea418d1\"}";
    std::string result;
    //auto resfunc =
    auto resp = exchange_client::request(json_reqstr);

    if (!resp)
    {
       std::cout << "ERROR::exchange-client::make_request() failed\n" << std::endl;
       return EXIT_FAILURE;
    }

    std::cout << *resp << '\n';

    for (std::size_t n =0; n < N_RETRY; ++n)
    {
       if (resp->result && resp->code == 200)
       {
          std::cout << "Success: \n" << *resp << '\n'; 
          break;
       }
       std::cout << "Retry: " << n << std::endl;
       
       //sleep for 1 second
       usleep(SLEEP_TIME * 1000);

       resp = exchange_client::request(json_reqstr);
    }

    return 0;
}
