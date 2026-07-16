#include "teclado.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <linux/input.h>

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

int teclado_init(Teclado *t, const char *device)
{
    t->fd = open(device, O_RDONLY | O_NONBLOCK);
    if (t->fd < 0) {
        perror("[TECLADO] Error open()");
        return -1;
    }

    t->state = 0;
    printf("[TECLADO] Conectado a %s (fd=%d)\n", device, t->fd);
    return 0;
}

static void update(Teclado *t, struct input_event *ie)
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

static int map_note(uint32_t s)
{
    uint32_t base = s & ~Z_BIT;
    int oct = (s & Z_BIT) ? 1 : 0;

    switch (base)
    {
        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT|M_BIT):
            return oct ? 12 : 0;

        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT):
            return oct ? 14 : 2;

        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT|N_BIT):
            return oct ? 15 : 3;

        case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT):
            return oct ? 16 : 4;

        case (W_BIT|R_BIT|T_BIT|U_BIT):
            return oct ? 17 : 5;

        case (W_BIT|R_BIT|T_BIT|I_BIT):
            return oct ? 18 : 6;

        case (W_BIT|R_BIT|T_BIT):
            return oct ? 19 : 7;

        case (W_BIT|R_BIT|T_BIT|G_BIT):
            return oct ? 20 : 8;

        case (W_BIT|R_BIT):
            return oct ? 21 : 9;

        case (W_BIT|R_BIT|V_BIT):
            return oct ? 22 : 10;

        case (W_BIT):
            return oct ? 23 : 11;

        case (R_BIT):
            return oct ? 24 : 12;

        case (0):
            return oct ? 25 : 13;

        default:
            return -1;
    }
}

int teclado_leer(Teclado *t, int *nota)
{
    struct input_event ie;
    int got_note = 0;

    ssize_t n = read(t->fd, &ie, sizeof(ie));

    if (n < 0) {
        return 0;
    }

    if (n == sizeof(ie)) {
        printf("[TECLADO] Tipo: %d | Código: %d | Valor: %d\n", ie.type, ie.code, ie.value);

        if (ie.type == EV_KEY) {
            update(t, &ie);

            if (ie.value == 1) {
                int nmap = map_note(t->state);
                if (nmap >= 0) {
                    *nota = nmap;
                    got_note = 1;
                }
            }
        }
    }

    return got_note;
}

void teclado_close(Teclado *t)
{
    if (t->fd >= 0) {
        close(t->fd);
    }
}
