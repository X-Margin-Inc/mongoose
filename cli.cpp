// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, make SSL=OPENSSL or make SSL=MBEDTLS


#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>

#include "mongoose.h"
#include <map>
#include <string>



constexpr const char *s_url = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=eth";
//static const char *s_url = "https://google.com";

static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds
                                            

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
    int content_length = s_post_data ? strlen(s_post_data) : 0;
    mg_printf(c,
              "%s %s HTTP/1.0\r\n"
              "Host: %.*s\r\n"
              "Content-Type: octet-stream\r\n"
              "Content-Length: %d\r\n"
              "\r\n",
              s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int) host.len,
              host.ptr, content_length);
    mg_send(c, s_post_data, content_length);
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("%.*s", (int) hm->body.len, hm->body.ptr);
    std::string response(hm->body.ptr, hm->body.len);
    //process_output(response.c_str());
    c->is_closing = 1;         // Tell mongoose to close this connection
    *(bool *) fn_data = true;  // Tell event loop to stop
  } else if (ev == MG_EV_ERROR) {
    *(bool *) fn_data = true;  // Error, tell event loop to stop
  }
}

[[clang::export_name("entry")]]
int entry() {
  //const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log level
  //if (log_level == NULL) log_level = "4";       // Default is verbose

  struct mg_mgr mgr;              // Event manager
  bool done = false;              // Event handler flips it to true

  mg_log_set(MG_LL_DEBUG);    // Set to 0 to disable debug
  mg_mgr_init(&mgr);              // Initialise event manager  

  mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
  printf("prepare polling\n");
  while (!done) 
  {
      printf("poll!\n");
      mg_mgr_poll(&mgr, 500); 
      //usleep(1000*500);
  }
  mg_mgr_free(&mgr);                        // Free resources
  return 0;
}
