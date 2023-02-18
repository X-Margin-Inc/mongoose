// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, make SSL=OPENSSL or make SSL=MBEDTLS

#define HAVE_SNI

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>

#include "mongoose.h"
#include <map>
#include <string>
#include <string_view>
#include <array>



//constexpr const char *s_url = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=eth";
//static const char *s_url = "https://google.com";
//static const char* s_url = "https://api.coingecko.com";
const char * s_url = "https://api.coingecko.com";

const std::array<const char*, 10> requests
{
    "/api/v3/coins/markets?vs_currency=eth",
    "/api/v3/coins/markets?vs_currency=link",
    "/api/v3/coins/markets?vs_currency=btc",
    "/api/v3/coins/markets?vs_currency=ada",
    "/api/v3/coins/markets?vs_currency=bnb",
    "/api/v3/coins/markets?vs_currency=xrp",
    "/api/v3/coins/markets?vs_currency=ltc",
    "/api/v3/coins/markets?vs_currency=doge",
    "/api/v3/asset_platforms",
    "/api/v3/search/trending"
};

const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds
                                            
std::size_t IDX = 0;
std::size_t SUCCESS = 0;

// Print HTTP response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->label
    *(uint64_t *) c->label = mg_millis() + s_timeout_ms;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *) c->label &&
        (c->is_connecting || c->is_resolving)) {
      mg_error(c, "Connect timeout");
    }
  } else if (ev == MG_EV_CONNECT) {
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url);

    // If s_url is https://, tell client connection to use TLS
    if (mg_url_is_ssl(s_url)) {
      struct mg_tls_opts opts = {.srvname = host};
      mg_tls_init(c, &opts);
    }

    // Send request
    mg_printf(c,"GET %s HTTP/1.1\r\n"
                "Host: api.coingecko.com\r\n\r\n", requests[IDX]);


    //int content_length = s_post_data ? strlen(s_post_data) : 0;
    //mg_printf(c,"GET /api/v3/coins/markets?vs_currency=eth HTTP/1.1\r\n"
    //            "Host: api.coingecko.com\r\n\r\n");
    //mg_printf(c,"GET /api/v3/coins/markets?vs_currency=eth HTTP/1.1\r\n"
                //"Host: %s\r\n\r\n");

    
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    //printf("%.*s", (int) hm->body.len, hm->body.ptr);
    std::string response(hm->body.ptr, hm->body.len);
    //printf("##RESULT: %s\n", response.c_str());
    ++SUCCESS;
    printf("##SUCCESS: %d\n", SUCCESS);
    //process_output(response.c_str());
    c->is_closing = 1;         // Tell mongoose to close this connection
    *(bool *) fn_data = true;  // Tell event loop to stop
  } else if (ev == MG_EV_ERROR) {
      printf("!!!ERROR\n");
      exit(EXIT_FAILURE);
    *(bool *) fn_data = true;  // Error, tell event loop to stop
  }
}

[[clang::export_name("entry")]]
int entry() {
  //const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log level
  //if (log_level == NULL) log_level = "4";       // Default is verbose


  //mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection


  printf("Entry!\n");
  printf("SIZE: %d\n", requests.size());
  for (std::size_t idx = 0; idx < requests.size(); ++idx)
  {
      printf("\n===REQUEST %lu : %s\n", idx, requests[idx]);

      struct mg_mgr mgr;              // Event manager
      bool done = false;              // Event handler flips it to true
      //mg_log_set(MG_LL_DEBUG);    // Set to 0 to disable debug
      mg_mgr_init(&mgr);              // Initialise event manager  
      IDX = idx;
      mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
      printf("poll!\n");
      while (!done)
        mg_mgr_poll(&mgr, 500); 

      done = false;
      mg_mgr_free(&mgr);                        // Free resources
      sleep(2);
  }

  printf("Successful queries: %lu out of %lu \n", SUCCESS, requests.size());

  //printf("prepare polling\n");
  //while (!done) 
  //{
  //    printf("poll!\n");
  //    mg_mgr_poll(&mgr, 500); 
  //    //usleep(1000*500);
  //}
  


  return 0;
}
