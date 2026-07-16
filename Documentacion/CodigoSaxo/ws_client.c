#include "cliente_ws.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libwebsockets.h>

static struct lws *wsi_global = NULL;
static struct lws_context *context_global = NULL;
static int conectado = 0;

#define QUEUE_SIZE 32
#define MSG_SIZE   128

static char cola[QUEUE_SIZE][MSG_SIZE];
static int q_head = 0; 
static int q_tail = 0; 

static int cola_vacia(void)  { return q_head == q_tail; }
static int cola_llena(void)  { return ((q_tail + 1) % QUEUE_SIZE) == q_head; }

static int callback_ws(struct lws *wsi,
                       enum lws_callback_reasons reason,
                       void *user, void *in, size_t len)
{
    switch (reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("[WS] Conectado al servidor\n");
            conectado = 1;
            wsi_global = wsi;
            ws_send("{\"type\":\"register\",\"role\":\"simulator\"}");    

            if (!cola_vacia())
                lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (conectado && !cola_vacia())
            {
                unsigned char buf[LWS_PRE + MSG_SIZE];
                
                memset(buf, 0, sizeof(buf));

                int n = snprintf((char *)&buf[LWS_PRE], MSG_SIZE, "%s", cola[q_head]);
                
                q_head = (q_head + 1) % QUEUE_SIZE;

                if (n > 0) {
                    lws_write(wsi, &buf[LWS_PRE], n, LWS_WRITE_TEXT);
                    printf("[WS] Enviado con éxito: %s\n", (char *)&buf[LWS_PRE]);
                }

                if (!cola_vacia())
                    lws_callback_on_writable(wsi);
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("[WS] RX: %.*s\n", (int)len, (char *)in);
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
        case LWS_CALLBACK_CLOSED:
            printf("[WS] Desconectado\n");
            conectado = 0;
            wsi_global = NULL;
            break;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {
        .name = "ws",
        .callback = callback_ws,
        .per_session_data_size = 0,
        .rx_buffer_size = 256
    },
    { NULL, NULL, 0, 0 }
};

struct lws_context* ws_init(const char *ip, int port)
{
    lws_set_log_level(LLL_ERR | LLL_WARN, NULL);

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context *context = lws_create_context(&info);
    if (!context) {
        printf("[WS] Error creando contexto LWS\n");
        return NULL;
    }

    context_global = context;

    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));

    ccinfo.context  = context;
    ccinfo.address  = ip;
    ccinfo.port     = port;
    ccinfo.path     = "/";
    ccinfo.host     = ip;
    ccinfo.origin   = ip;
    ccinfo.protocol = protocols[0].name;

    if (!lws_client_connect_via_info(&ccinfo)) {
        printf("[WS] Error conectando WS\n");
        lws_context_destroy(context);
        context_global = NULL;
        return NULL;
    }

    return context;
}

void ws_send(const char *msg)
{
    if (cola_llena()) {
        printf("[WS] AVISO: cola llena, se descarta mensaje: %s\n", msg);
        return;
    }

    memset(cola[q_tail], 0, MSG_SIZE);

    strncpy(cola[q_tail], msg, MSG_SIZE - 1);
    cola[q_tail][MSG_SIZE - 1] = '\0'; 
    
    q_tail = (q_tail + 1) % QUEUE_SIZE;

    if (wsi_global) {
        lws_callback_on_writable(wsi_global);
        lws_cancel_service(context_global);
    }
}

void ws_service(struct lws_context *context)
{
    if (context) {
        lws_service(context, 0); 
    }
}

void ws_cleanup(struct lws_context *context)
{
    if (context)
        lws_context_destroy(context);
}
