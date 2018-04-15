#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"

#include "gpio.h"

void gpio_init()
{
    sil_wrw_mem((uint32_t *)GPIO_GPPUD, 0x0);
}

void gpio_fsel(int id, int fsel)
{
    uint32_t tmp;
    uint32_t sel;
    int i;

    switch (fsel) {
    case GPIO_FSEL_IN:
        sel = 0x0;
        break;
    case GPIO_FSEL_OUT:
        sel = 0x1;
        break;
    case GPIO_FSEL_0:
        sel = 0x4;
        break;
    case GPIO_FSEL_1:
        sel = 0x5;
        break;
    case GPIO_FSEL_2:
        sel = 0x6;
        break;
    case GPIO_FSEL_3:
        sel = 0x7;
        break;
    case GPIO_FSEL_4:
        sel = 0x3;
        break;
    case GPIO_FSEL_5:
        sel = 0x2;
        break;
    default:
        sel = 0x0;
        break;
    }

    if ((id >= 0) && (id < 10)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL0);
        tmp &= ~(0x7 << (3 * (id - 0)));
        tmp |= sel << (3 * (id - 0));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL0, tmp);
    } else if ((id >= 10) && (id < 20)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL1);
        tmp &= ~(0x7 << (3 * (id - 10)));
        tmp |= sel << (3 * (id - 10));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL1, tmp);
    } else if ((id >= 20) && (id < 30)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL2);
        tmp &= ~(0x7 << (3 * (id - 20)));
        tmp |= sel << (3 * (id - 20));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL2, tmp);
    } else if ((id >= 30) && (id < 40)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL3);
        tmp &= ~(0x7 << (3 * (id - 30)));
        tmp |= sel << (3 * (id - 30));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL3, tmp);
    } else if ((id >= 40) && (id < 50)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL4);
        tmp &= ~(0x7 << (3 * (id - 40)));
        tmp |= sel << (3 * (id - 40));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL4, tmp);
    } else if ((id >= 50) && (id <= 53)) {
        tmp = sil_rew_mem((uint32_t *)GPIO_GPFSEL5);
        tmp &= ~(0x7 << (3 * (id - 50)));
        tmp |= sel << (3 * (id - 50));
        sil_wrw_mem((uint32_t *)GPIO_GPFSEL5, tmp);
    }

    for (i = 0; i < 150; i++) {
        asm volatile("nop");
    }

    if ((id >= 0) && (id < 32)) {
        sil_wrw_mem((uint32_t *)GPIO_GPPUDCLK0, (0x1 << (id - 0)));
    } else if ((id >= 32) && (id <= 53)) {
        sil_wrw_mem((uint32_t *)GPIO_GPPUDCLK1, (0x1 << (id - 32)));
    }

    for (i = 0; i < 150; i++) {
        asm volatile("nop");
    }

    if ((id >= 0) && (id < 32)) {
        sil_wrw_mem((uint32_t *)GPIO_GPPUDCLK0, 0);
    } else if ((id >= 32) && (id <= 53)) {
        sil_wrw_mem((uint32_t *)GPIO_GPPUDCLK1, 0);
    }
}

void gpio_set(int id, int set)
{
    if ((id >= 0) && (id < 32)) {
        if (set == GPIO_SET_HIGH) {
            sil_wrw_mem((uint32_t *)GPIO_GPSET0, (0x1 << (id - 0)));
        } else {
            sil_wrw_mem((uint32_t *)GPIO_GPCLR0, (0x1 << (id - 0)));
        }
    } else if ((id >= 32) && (id <= 53)) {
        if (set == GPIO_SET_HIGH) {
            sil_wrw_mem((uint32_t *)GPIO_GPSET1, (0x1 << (id - 32)));
        } else {
            sil_wrw_mem((uint32_t *)GPIO_GPCLR1, (0x1 << (id - 32)));
        }
    } 
}
