#include "mongoose.h"

static const char *s_url = "http://info.cern.ch/";
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 5500;  // Connect timeout in milliseconds

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
      //struct mg_tls_opts opts = {.ca = "ca.pem", .srvname = host};
      mg_tls_init(c, &opts);
    }

    // Send request
    int content_length = s_post_data ? strlen(s_post_data) : 0;
    mg_printf(c,
          "GET %s HTTP/1.2\r\n"
          "Host: %.*s\r\n"
          "\r\n",
          mg_url_uri(s_url), (int) host.len, host.ptr);

    printf(
          "GET %s HTTP/1.2\r\n"
          "Host: %.*s\r\n"
          "\r\n",
          mg_url_uri(s_url), (int) host.len, host.ptr);

    printf("s_url: %s\n", mg_url_uri(s_url));

    mg_send(c, s_post_data, content_length);
  } else if (ev == MG_EV_HTTP_MSG) {
    printf("RESP!\n");
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("RESPONSE: %.*s", (int) hm->message.len, hm->message.ptr);
    c->is_closing = 1;         // Tell mongoose to close this connection
    *(bool *) fn_data = true;  // Tell event loop to stop
  } else if (ev == MG_EV_ERROR) {
    *(bool *) fn_data = true;  // Error, tell event loop to stop
  }
}

int main(int argc, char *argv[]) {
  const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log level
  if (log_level == NULL) log_level = "0";       // Default is verbose

  printf("ENTER\n");
  struct mg_mgr mgr;              // Event manager
  bool done = false;              // Event handler flips it to true
  if (argc > 1) s_url = argv[1];  // Use URL provided in the command line
  //mg_log_set(atoi(log_level));    // Set to 0 to disable debug
  mg_mgr_init(&mgr);              // Initialise event manager
  mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
  while (!done) mg_mgr_poll(&mgr, 50);      // Event manager loops until 'done'
  mg_mgr_free(&mgr);                        // Free resources
  return 0;
}

