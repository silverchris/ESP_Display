#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include <freertos/ringbuf.h>

#include "gui.h"


#include <esp_http_server.h>

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "httpd";

#define MAXBUFLEN 10000

static const int BMP_HEADER_LEN = 54;

typedef struct {
    uint32_t filesize;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
    uint32_t dibheadersize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t ypixelpermeter;
    uint32_t xpixelpermeter;
    uint32_t numcolorspallette;
    uint32_t mostimpcolor;
} bmp_header_t;

bool bmpheader(char *buf, uint16_t width, uint16_t height) {
    int pix_count = width * height;
    size_t out_size = (pix_count * 3) + BMP_HEADER_LEN;

    buf[0] = 'B';
    buf[1] = 'M';
    bmp_header_t *bitmap = (bmp_header_t *) &buf[2];
    bitmap->reserved = 0;
    bitmap->filesize = out_size;
    bitmap->fileoffset_to_pixelarray = BMP_HEADER_LEN;
    bitmap->dibheadersize = 40;
    bitmap->width = width;
    bitmap->height = -height;//set negative for top to bottom
    bitmap->planes = 1;
    bitmap->bitsperpixel = 32;
    bitmap->compression = 0;
    bitmap->imagesize = pix_count * 3;
    bitmap->ypixelpermeter = 0x0B13; //2835 , 72 DPI
    bitmap->xpixelpermeter = 0x0B13; //2835 , 72 DPI
    bitmap->numcolorspallette = 0;
    bitmap->mostimpcolor = 0;

    return true;
}

static esp_err_t screenshot_get_handler(httpd_req_t *req) {

    httpd_resp_set_type(req, "image/bmp");
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    RingbufHandle_t buf = screenshot_init();

    char header[54];

    bmpheader(header, 320, 480);
    httpd_resp_send_chunk(req, header, 54);


    size_t size;
    const char *data;

    while (1) {
        data = xRingbufferReceiveUpTo(buf, &size, 250, 512);
        if (data != NULL) {
            httpd_resp_send_chunk(req, data, size);
            vRingbufferReturnItem(buf, (void *) data);
        } else {
            httpd_resp_send_chunk(req, NULL, 0);
            break;
        }
        taskYIELD();
    }
    vRingbufferDelete(buf);

    return ESP_OK;
}

static const httpd_uri_t screenshot_page = {
        .uri       = "/screenshot.bmp",
        .method    = HTTP_GET,
        .handler   = screenshot_get_handler,
};

/* An HTTP GET handler */
static esp_err_t file_get_handler(httpd_req_t *req) {

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char *resp_str = (const char *) req->user_ctx;
    char *buf = calloc(sizeof(char), MAXBUFLEN);
    char file[50] = "/spiffs/";
    strncat(file, resp_str, sizeof(file) - strlen(file) - 1);
    ESP_LOGI(TAG, "Serving %s", file);
    FILE *fp = fopen(file, "r");
    if (fp != NULL) {
        size_t newLen = fread(buf, sizeof(char), MAXBUFLEN, fp);
        if (ferror(fp) != 0) {
            fputs("Error reading file", stderr);
        } else {
            buf[newLen + 1] = '\0'; /* Just to be safe. */
        }
        fclose(fp);
    }


    httpd_resp_send(req, buf, strlen(buf));
    free(buf);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t index_page = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = file_get_handler,
        /* Let's pass response string in user
         * context to demonstrate it's usage */
        .user_ctx  = "index.html"
};

static const httpd_uri_t config_page = {
        .uri       = "/config",
        .method    = HTTP_GET,
        .handler   = file_get_handler,
        /* Let's pass response string in user
         * context to demonstrate it's usage */
        .user_ctx  = "config.json"
};

///* An HTTP POST handler */
//static esp_err_t echo_post_handler(httpd_req_t *req) {
//    char buf[100];
//    int ret, remaining = req->content_len;
//
//    while (remaining > 0) {
//        /* Read the data for the request */
//        if ((ret = httpd_req_recv(req, buf,
//                                  MIN(remaining, sizeof(buf)))) <= 0) {
//            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//                /* Retry receiving if timeout occurred */
//                continue;
//            }
//            return ESP_FAIL;
//        }
//
//        /* Send back the same data */
//        httpd_resp_send_chunk(req, buf, ret);
//        remaining -= ret;
//
//        /* Log data received */
//        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
//        ESP_LOGI(TAG, "%.*s", ret, buf);
//        ESP_LOGI(TAG, "====================================");
//    }
//
//    // End response
//    httpd_resp_send_chunk(req, NULL, 0);
//    return ESP_OK;
//}
//
//static const httpd_uri_t echo = {
//        .uri       = "/echo",
//        .method    = HTTP_POST,
//        .handler   = echo_post_handler,
//        .user_ctx  = NULL
//};

///* This handler allows the custom error handling functionality to be
// * tested from client side. For that, when a PUT request 0 is sent to
// * URI /ctrl, the /hello and /echo URIs are unregistered and following
// * custom error handler http_404_error_handler() is registered.
// * Afterwards, when /hello or /echo is requested, this custom error
// * handler is invoked which, after sending an error message to client,
// * either closes the underlying socket (when requested URI is /echo)
// * or keeps it open (when requested URI is /hello). This allows the
// * client to infer if the custom error handler is functioning as expected
// * by observing the socket state.
// */
//esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
//    if (strcmp("/hello", req->uri) == 0) {
//        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
//        /* Return ESP_OK to keep underlying socket open */
//        return ESP_OK;
//    } else if (strcmp("/echo", req->uri) == 0) {
//        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
//        /* Return ESP_FAIL to close underlying socket */
//        return ESP_FAIL;
//    }
//    /* For any other URI send 404 and close socket */
//    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
//    return ESP_FAIL;
//}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &index_page);
        httpd_register_uri_handler(server, &config_page);
        httpd_register_uri_handler(server, &screenshot_page);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server) {
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void *arg, __unused esp_event_base_t event_base, __unused int32_t event_id,
                               __unused void *event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void *arg, __unused esp_event_base_t event_base, __unused int32_t event_id,
                            __unused void *event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}


void httpd_init(void) {
    static httpd_handle_t server = NULL;


    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));


    /* Start the server for the first time */
    server = start_webserver();

    ESP_LOGI(TAG, "HTTPD_INIT FINISHED");
}