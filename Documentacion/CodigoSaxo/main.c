#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/input.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <lo/lo.h>

#include "teclado.h"
#include "cliente_ws.h"

#define SERVER_IP   "10.52.223.50"
#define SERVER_PORT 3000

#define OSC_PORT      "5513"
#define I2C_DEV       "/dev/i2c-3"
#define I2C_ADDR      0x4d
#define UMBRAL_RUIDO  220
#define DELTA_MAX     850.0f

#define Z_BIT (1U << 0)
#define W_BIT (1U << 1)
#define R_BIT (1U << 2)
#define T_BIT (1U << 3)
#define U_BIT (1U << 4)
#define I_BIT (1U << 5)
#define O_BIT (1U << 6)
#define M_BIT (1U << 7)
#define C_BIT (1U << 8)
#define E_BIT (1U << 9)
#define D_BIT (1U << 10)
#define J_BIT (1U << 11)
#define Y_BIT (1U << 12)
#define N_BIT (1U << 13)
#define K_BIT (1U << 14)
#define H_BIT (1U << 15)
#define G_BIT (1U << 16)
#define A_BIT (1U << 17)
#define S_BIT (1U << 18)
#define X_BIT (1U << 19)
#define V_BIT (1U << 20)

static int mapear_estado_a_nota(uint32_t s)
{
    uint32_t base = s & ~Z_BIT;
    int oct = (s & Z_BIT) ? 1 : 0;

    switch (base)
    {
        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT|M_BIT): return oct ? 12 : 0;
        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT):       return oct ? 14 : 2;
        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT|N_BIT): return oct ? 15 : 3;
        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT):             return oct ? 16 : 4;
        case (W_BIT|R_BIT|T_BIT|U_BIT):                   return oct ? 17 : 5;
        case (W_BIT|R_BIT|T_BIT|I_BIT):                   return oct ? 18 : 6;
        case (W_BIT|R_BIT|T_BIT):                         return oct ? 19 : 7;
        case (W_BIT|R_BIT|T_BIT|G_BIT):                   return oct ? 20 : 8;
        case (W_BIT|R_BIT):                               return oct ? 21 : 9;
        case (W_BIT|R_BIT|V_BIT):                         return oct ? 22 : 10;
        case (W_BIT):                                     return oct ? 23 : 11;
        case (R_BIT):                                     return oct ? 24 : 12;
        case (0):                                         return oct ? 25 : 13;
        default:                                          return -1;
    }
}

static void actualizar_bits_teclado(Teclado *t, struct input_event *ie)
{
    int on = ie->value; 
    if (ie->value == 2) return; 

    switch (ie->code)
    {
        case KEY_Z: t->state = on ? (t->state | Z_BIT) : (t->state & ~Z_BIT); break;
        case KEY_W: t->state = on ? (t->state | W_BIT) : (t->state & ~W_BIT); break;
        case KEY_R: t->state = on ? (t->state | R_BIT) : (t->state & ~R_BIT); break;
        case KEY_T: t->state = on ? (t->state | T_BIT) : (t->state & ~T_BIT); break;
        case KEY_U: t->state = on ? (t->state | U_BIT) : (t->state & ~U_BIT); break;
        case KEY_I: t->state = on ? (t->state | I_BIT) : (t->state & ~I_BIT); break;
        case KEY_O: t->state = on ? (t->state | O_BIT) : (t->state & ~O_BIT); break;
        case KEY_M: t->state = on ? (t->state | M_BIT) : (t->state & ~M_BIT); break;
        case KEY_N: t->state = on ? (t->state | N_BIT) : (t->state & ~N_BIT); break;
        case KEY_K: t->state = on ? (t->state | K_BIT) : (t->state & ~K_BIT); break;
        case KEY_H: t->state = on ? (t->state | H_BIT) : (t->state & ~H_BIT); break;
        case KEY_J: t->state = on ? (t->state | J_BIT) : (t->state & ~J_BIT); break;
        case KEY_G: t->state = on ? (t->state | G_BIT) : (t->state & ~G_BIT); break;
        case KEY_A: t->state = on ? (t->state | A_BIT) : (t->state & ~A_BIT); break;
        case KEY_S: t->state = on ? (t->state | S_BIT) : (t->state & ~S_BIT); break;
        case KEY_D: t->state = on ? (t->state | D_BIT) : (t->state & ~D_BIT); break;
        case KEY_X: t->state = on ? (t->state | X_BIT) : (t->state & ~X_BIT); break;
        case KEY_C: t->state = on ? (t->state | C_BIT) : (t->state & ~C_BIT); break;
        case KEY_V: t->state = on ? (t->state | V_BIT) : (t->state & ~V_BIT); break;
        default: break;
    }
}

static volatile int running = 1;
static volatile float presion_global = 0.0f;
static struct lws_context *ctx_global = NULL;

static const char *nota_to_string(int nota)
{
    switch (nota)
    {
        case 0:  return "Do4";
        case 2:  return "Re4";
        case 3:  return "Mib4";
        case 4:  return "Mi4";
        case 5:  return "Fa4";
        case 6:  return "Fa#4";
        case 7:  return "Sol4";
        case 8:  return "Sol#4";
        case 9:  return "La4";
        case 10: return "Sib4";
        case 11: return "Si4";
        case 12: return "Do5";
        case 14: return "Re5";
        case 15: return "Mib5";
        case 16: return "Mi5";
        case 17: return "Fa5";
        case 18: return "Fa#5";
        case 19: return "Sol5";
        case 20: return "Sol#5";
        case 21: return "La5";
        case 22: return "Sib5";
        case 23: return "Si5";
        case 24: return "Do6";
        default: return NULL;
    }
}

void *hilo_presion(void *arg)
{
    int fd = open(I2C_DEV, O_RDWR);
    if (fd < 0) { perror("[I2C] Error al abrir bus"); return NULL; }
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) { perror("[I2C] Error ioctl"); return NULL; }

    long suma = 0;
    uint8_t buf[2];
   
    printf("[I2C] Calibrando sensor... no soples\n");
    for (int i = 0; i < 50; i++) {
        if (read(fd, buf, 2) == 2)
            suma += ((buf[0] & 0x0F) << 8) | buf[1];
        usleep(10000);
    }
    int baseline = (int)(suma / 50);
    printf("[I2C] Baseline calibrado: %d\n", baseline);

    while (running)
    {
        if (read(fd, buf, 2) == 2)
        {
            int raw   = ((buf[0] & 0x0F) << 8) | buf[1];
            int delta = raw - baseline;
            if (delta < 0) delta = 0;
           
            float p = (float)delta / DELTA_MAX;
            if (p > 1.0f) p = 1.0f;
            if (delta < UMBRAL_RUIDO) p = 0.0f;
           
            presion_global = p;
        }
        usleep(10000);
    }

    close(fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);

    const char *device = "/dev/input/event0";
    if (argc > 1) device = argv[1];

    Teclado t_keys;
    int note_prev = -1;
    float presion_prev = -1.0f;

    if (teclado_init(&t_keys, device) < 0) {
        printf("[ERROR] No se pudo abrir el teclado\n");
        return EXIT_FAILURE;
    }

    int flags = fcntl(t_keys.fd, F_GETFL, 0);
    fcntl(t_keys.fd, F_SETFL, flags | O_NONBLOCK);

    ctx_global = ws_init(SERVER_IP, SERVER_PORT);
    if (!ctx_global) {
        printf("[ERROR] No se pudo conectar al servidor WebSocket\n");
        teclado_close(&t_keys);
        return EXIT_FAILURE;
    }

    lo_address t_osc = lo_address_new("localhost", OSC_PORT);
    if (!t_osc) {
        fprintf(stderr, "[ERROR] Error creando direccion OSC\n");
        return EXIT_FAILURE;
    }

    pthread_t pres_thread;
    pthread_create(&pres_thread, NULL, hilo_presion, NULL);

    sleep(1);
    printf("\n>>> TRANSMISIÓN INTEGRADA ACTIVA CONTINUA (Sin colisión de hilos) <<<\n\n");

    while (1)
    {
        float p = presion_global;
        int hay_soplo = (p > 0.0f);

        if (p - presion_prev > 0.005f || presion_prev - p > 0.005f)
        {
            presion_prev = p;
            lo_send(t_osc, "/sax/presion", "f", p);

            if (hay_soplo && note_prev == -1) {
                lo_send(t_osc, "/sax/error",  "f", 0.0f);
                lo_send(t_osc, "/sax/note",   "f", 12.0f);
                lo_send(t_osc, "/sax/activo", "f", 1.0f);
                note_prev = 12;
            }
            else if (!hay_soplo && note_prev >= 0) {
                lo_send(t_osc, "/sax/error",  "f", 0.0f);
                lo_send(t_osc, "/sax/activo", "f", 0.0f);
                note_prev = -1;
            }
        }

        struct input_event ie;
        while (read(t_keys.fd, &ie, sizeof(ie)) == sizeof(ie))
        {
            if (ie.type == EV_KEY)
            {
                actualizar_bits_teclado(&t_keys, &ie);
            }
        }

        char msg[128];

        if (t_keys.state == 0)
        {
            snprintf(msg, sizeof(msg),
                     "{\"type\":\"note\",\"value\":\"none\",\"pressure\":%.3f}",
                     p);
            ws_send(msg);
        }
        else
        {
            int nota_actual = mapear_estado_a_nota(t_keys.state);
            const char *nombre = nota_to_string(nota_actual);

            if (nombre != NULL)
            {
                snprintf(msg, sizeof(msg),
                         "{\"type\":\"note\",\"value\":\"%s\",\"pressure\":%.3f}",
                         nombre, p);
                ws_send(msg);

                if (hay_soplo || p == 0.0f) {
                    lo_send(t_osc, "/sax/error",  "f", 0.0f);
                    lo_send(t_osc, "/sax/note",   "f", (float)nota_actual);
                    lo_send(t_osc, "/sax/activo", "f", 1.0f);
                    note_prev = nota_actual;
                }
            }
            else
            {
                snprintf(msg, sizeof(msg),
                         "{\"type\":\"note\",\"value\":\"none\",\"pressure\":%.3f}",
                         p);
                ws_send(msg);
            }
        }

        ws_service(ctx_global);

        usleep(15000);
    }

    running = 0;
    pthread_join(pres_thread, NULL);

    lo_address_free(t_osc);
    teclado_close(&t_keys);
    ws_cleanup(ctx_global);

    return 0;
}
