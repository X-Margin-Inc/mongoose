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


static void
fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
    nlohmann::json jresp;
    jresp["result"] = "error";

    if(ev == MG_EV_ACCEPT && fn_data != NULL) {
        struct mg_tls_opts opts = {
            //.ca = "ca.pem",         // Uncomment to enable two-way SSL
            .cert = certificate
                        .c_str(), // "/mongoose_certs/cert.pem"
            .certkey = privKey.c_str() // "/mongoose_certs/privkey.pem"
        };
        mg_tls_init(c, &opts);
    }
    else if(ev == MG_EV_HTTP_MSG) {

        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        struct mg_str url_params[3];
        if(mg_match(hm->uri, mg_str("/apikeys/*/*"), url_params)) {
          std::string exchange_str =
              std::string(url_params[0].ptr, url_params[0].len);
          std::string operation =
              std::string(url_params[1].ptr, url_params[1].len);
          struct mg_str request = hm->body;

          std::string json_msg = std::string(request.ptr, request.len);

          if(!nlohmann::json::accept(json_msg)) {
              jresp["result"] = "error";
              jresp["reason"] = "invalid-json";
              mg_http_reply(c, 400, "", jresp.dump().c_str(), hm->uri.ptr);
              return;
          }
          auto jdoc = nlohmann::json::parse(json_msg);
          if(!jdoc.contains("uuid") || !jdoc.contains("key_id") ||
              !jdoc.contains("apikey")) {
              jresp["result"] = "error";
              jresp["reason"] = "missing-json-field";
              mg_http_reply(c, 400, "", jresp.dump().c_str(), hm->uri.ptr);
              return;
          }

          std::string uuid = jdoc["uuid"].get<std::string>();
          std::string key_id = jdoc["key_id"].get<std::string>();
          if(operation == "insert") {  
              printf("[WASM-CORE] INSERT request received - Exchange: %s - "
                      "User: %s\n\n",
                      exchange_str.c_str(),
                      uuid.c_str());

              jdoc["exchange"] = exchange_str;

              // Skipping the key verification with DeFi venues. Just pushing
              // in the storage

              std::string json_reqstr = jdoc.dump();
              std::string result;
              auto resfunc =
                  make_request((uint32_t)(uintptr_t)json_reqstr.c_str(),
                                json_reqstr.length(),
                                result);

              if(resfunc == -1) {
                  jresp["reason"] = "invalid-venue";
                  mg_http_reply(c,
                                401,
                                "",
                                jresp.dump().c_str(),
                                (int)hm->uri.len,
                                hm->uri.ptr);
                  return;
              }

              nlohmann::json j_result_req = nlohmann::json::parse(result);
              if(j_result_req["code"].get<int>() > 200) {
                  jresp["reason"] = j_result_req["result"];
                  mg_http_reply(c,
                                401,
                                "",
                                jresp.dump().c_str(),
                                (int)hm->uri.len,
                                hm->uri.ptr);
                  return;
              }
          }
        }
        else {
            jresp["result"] = "error";
            jresp["reason"] = "URI-not-found";
            mg_http_reply(c,
                          404,
                          "",
                          jresp.dump().c_str(),
                          (int)hm->uri.len,
                          hm->uri.ptr);
        }
    }
    (void)fn_data;
}


int
main(int argc, char* argv[])
{

    printf("[WASM-CORE] Started Core module\n");
    struct mg_mgr mgr; // Event manager
    mg_log_set(MG_LL_DEBUG); // Set log level

    create_cert_files("",
                  "");


    // mg_mgr_init(&mgr); // Initialise event manager
    // mg_http_listen(&mgr, s_https_addr, fn, (void*)1); // HTTPS listener
    // for(;;) {
    //     mg_mgr_poll(&mgr, 300);
    // }
    // mg_mgr_free(&mgr);

    return 0;
}