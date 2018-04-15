#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"

#include "gpio.h"
#include "i2s.h"

/*
 *  Clock Manager
 */
#define CM_PCMCTL               0x3f101098
#define CM_PCMDIV               0x3f10109c
#define CM_PASSWD               0x5a000000

#define CM_PCMCTL_INT           (0x0 << 9)
#define CM_PCMCTL_1MASH         (0x1 << 9)
#define CM_PCMCTL_2MASH         (0x2 << 9)
#define CM_PCMCTL_3MASH         (0x3 << 9)
#define CM_PCMCTL_FLIP_BIT      (0x1 << 8)
#define CM_PCMCTL_BUSY_BIT      (0x1 << 7)
#define CM_PCMCTL_KILL_BIT      (0x1 << 5)
#define CM_PCMCTL_ENAB_BIT      (0x1 << 4)
#define CM_PCMCTL_SRC_GND       (0 << 0)
#define CM_PCMCTL_SRC_OSC       (1 << 0)
#define CM_PCMCTL_SRC_PLLA      (4 << 0)
#define CM_PCMCTL_SRC_PLLC      (5 << 0)
#define CM_PCMCTL_SRC_PLLD      (6 << 0)
#define CM_PCMDIV_DIVI_SFT      12
#define CM_PCMDIV_DIVI_MSK      (0xfff << 12)
#define CM_PCMDIV_DIVF_SFT      0
#define CM_PCMDIV_DIVF_MSK      (0xfff << 0)

#define CM_RXSEX_BIT            (0x1 << 23)
#define CM_RXF_BIT              (0x1 << 22)
#define CM_TXE_BIT              (0x1 << 21)
#define CM_RXD_BIT              (0x1 << 20)
#define CM_TXD_BIT              (0x1 << 19)
#define CM_RXR_BIT              (0x1 << 18)
#define CM_TXW_BIT              (0x1 << 17)
#define CM_RXERR_BIT            (0x1 << 16)

/*
 *  PCM/I2S
 */
#define PCM_CS_A                0x3f203000
#define PCM_FIFO_A              0x3f203004
#define PCM_MODE_A              0x3f203008
#define PCM_RXC_A               0x3f20300C
#define PCM_TXC_A               0x3f203010
#define PCM_DREQ_A              0x3f203014
#define PCM_INTEN_A             0x3f203018
#define PCM_INTSTC_A            0x3f20301C
#define PCM_GRAY                0x3f203020

#define PCM_CS_SYNC_BIT         (0x1 << 24)
#define PCM_CS_RXSEX_BIT        (0x1 << 23)
#define PCM_CS_RXF_BIT          (0x1 << 22)
#define PCM_CS_TXE_BIT          (0x1 << 21)
#define PCM_CS_RXD_BIT          (0x1 << 20)
#define PCM_CS_TXD_BIT          (0x1 << 19)
#define PCM_CS_RXR_BIT          (0x1 << 18)
#define PCM_CS_TXW_BIT          (0x1 << 17)
#define PCM_CS_RXERR_BIT        (0x1 << 16)
#define PCM_CS_TXERR_BIT        (0x1 << 15)
#define PCM_CS_RXSYNC_BIT       (0x1 << 14)
#define PCM_CS_TXSYNC_BIT       (0x1 << 13)
#define PCM_CS_DMAEN_BIT        (0x1 << 9)
#define PCM_CS_RXTHR_SFT        7
#define PCM_CS_RXTHR_MSK        (0x3 << 7)
#define PCM_CS_RXTHR_SINGLE     (0x0 << 7)
#define PCM_CS_RXTHR_LSTFL      (0x1 << 7)
#define PCM_CS_RXTHR_FULL       (0x3 << 7)
#define PCM_CS_TXTHR_SFT        5
#define PCM_CS_TXTHR_MSK        (0x3 << 5)
#define PCM_CS_TXTHR_EMPTY      (0x0 << 5)
#define PCM_CS_TXTHR_LSSFL      (0x1 << 5)
#define PCM_CS_TXTHR_ONE        (0x3 << 5)
#define PCM_CS_RXCLR_BIT        (0x1 << 4)
#define PCM_CS_TXCLR_BIT        (0x1 << 3)
#define PCM_CS_TXON_BIT         (0x1 << 2)
#define PCM_CS_RXON_BIT         (0x1 << 1)
#define PCM_CS_EN_BIT           (0x1 << 0)

#define PCM_MODE_CLK_DIS_BIT    (0x1 << 28)
#define PCM_MODE_PDMN_BIT       (0x1 << 27)
#define PCM_MODE_PDME_BIT       (0x1 << 26)
#define PCM_MODE_FRXP_BIT       (0x1 << 25)
#define PCM_MODE_FTXP_BIT       (0x1 << 24)
#define PCM_MODE_CLKM_BIT       (0x1 << 23)
#define PCM_MODE_CLKI_BIT       (0x1 << 22)
#define PCM_MODE_FSM_BIT        (0x1 << 21)
#define PCM_MODE_FSI_BIT        (0x1 << 20)
#define PCM_MODE_FLEN_SFT       10
#define PCM_MODE_FLEN_MSK       (0x3ff << 10)
#define PCM_MODE_FSLEN_SFT      0
#define PCM_MODE_FSLEN_MSK      (0x3ff << 0)

#define PCM_RXC_CH1WEX_BIT      (0x1 << 31)
#define PCM_RXC_CH1EN_BIT       (0x1 << 30)
#define PCM_RXC_CH1POS_SFT      20
#define PCM_RXC_CH1POS_MSK      (0x3ff << 20)
#define PCM_RXC_CH1WID_SFT      16
#define PCM_RXC_CH1WID_MSK      (0xf << 16)
#define PCM_RXC_CH2WEX_BIT      (0x1 << 15)
#define PCM_RXC_CH2EN_BIT       (0x1 << 14)
#define PCM_RXC_CH2POS_SFT      4
#define PCM_RXC_CH2POS_MSK      (0x3ff << 4)
#define PCM_RXC_CH2WID_SFT      0
#define PCM_RXC_CH2WID_MSK      (0xf << 0)

#define PCM_TXC_CH1WEX_BIT      (0x1 << 31)
#define PCM_TXC_CH1EN_BIT       (0x1 << 30)
#define PCM_TXC_CH1POS_SFT      20
#define PCM_TXC_CH1POS_MSK      (0x3ff << 20)
#define PCM_TXC_CH1WID_SFT      16
#define PCM_TXC_CH1WID_MSK      (0xf << 16)
#define PCM_TXC_CH2WEX_BIT      (0x1 << 15)
#define PCM_TXC_CH2EN_BIT       (0x1 << 14)
#define PCM_TXC_CH2POS_SFT      4
#define PCM_TXC_CH2POS_MSK      (0x3ff << 4)
#define PCM_TXC_CH2WID_SFT      0
#define PCM_TXC_CH2WID_MSK      (0xf << 0)

#define PCM_INTEN_RXERR_BIT     (0x1 << 3)
#define PCM_INTEN_TXERR_BIT     (0x1 << 2)
#define PCM_INTEN_RXR_BIT       (0x1 << 1)
#define PCM_INTEN_TXW_BIT       (0x1 << 0)

#define PCM_INTSTC_RXERR_BIT    (0x1 << 3)
#define PCM_INTSTC_TXERR_BIT    (0x1 << 2)
#define PCM_INTSTC_RXR_BIT      (0x1 << 1)
#define PCM_INTSTC_TXW_BIT      (0x1 << 0)

void (*pi2s_rcv_func)(void);

void i2s_init(int master_mode, int sr, int bit)
{
    /*
     *  GPIOをI2Sに切り替え
     */
    gpio_fsel(18, GPIO_FSEL_0);
    gpio_fsel(19, GPIO_FSEL_0);
    gpio_fsel(20, GPIO_FSEL_0);
    gpio_fsel(21, GPIO_FSEL_0);

    if (master_mode == I2S_MSTR) {
        /*
        *  Clock Manager Audio Clocks Control
        */
        // oscillator 19.2Mhz
        // PLLC 1000Mhz
        // PLLD 500Mhz
        switch (sr) {
        case 48000:
            // DIVI=6, DIVF=1024
            sil_wrw_mem((uint32_t *)CM_PCMCTL, CM_PASSWD |
                    CM_PCMCTL_KILL_BIT | CM_PCMCTL_SRC_OSC);
            sil_wrw_mem((uint32_t *)CM_PCMDIV, CM_PASSWD |
                    (6 << CM_PCMDIV_DIVI_SFT) | (1024 << CM_PCMDIV_DIVF_SFT));
            sil_wrw_mem((uint32_t *)CM_PCMCTL, CM_PASSWD |
                    CM_PCMCTL_1MASH | CM_PCMCTL_ENAB_BIT | CM_PCMCTL_SRC_OSC);
            break;
        case 96000:
            // DIVI=3, DIVF=512
            sil_wrw_mem((uint32_t *)CM_PCMCTL, CM_PASSWD |
                    CM_PCMCTL_KILL_BIT | CM_PCMCTL_SRC_OSC);
            sil_wrw_mem((uint32_t *)CM_PCMDIV, CM_PASSWD |
                    (3 << CM_PCMDIV_DIVI_SFT) | (512 << CM_PCMDIV_DIVF_SFT));
            sil_wrw_mem((uint32_t *)CM_PCMCTL, CM_PASSWD |
                    CM_PCMCTL_1MASH | CM_PCMCTL_ENAB_BIT | CM_PCMCTL_SRC_OSC);
            break;
        default:
            break;
        }

        /*
        *  I2Sを初期化
        */
        switch (bit) {
        case 16:
            break;
        case 24:
        case 32:
            sil_wrw_mem((uint32_t *)PCM_MODE_A,
                    PCM_MODE_CLKI_BIT |
                    PCM_MODE_FSI_BIT |
                    ((64 - 1) << PCM_MODE_FLEN_SFT) |
                    (32 << PCM_MODE_FSLEN_SFT));
            sil_wrw_mem((uint32_t *)PCM_RXC_A,
                    PCM_RXC_CH1WEX_BIT | PCM_RXC_CH1EN_BIT |
                    (1 << PCM_RXC_CH1POS_SFT) | (8 << PCM_RXC_CH1WID_SFT) |
                    PCM_RXC_CH2WEX_BIT | PCM_RXC_CH2EN_BIT |
                    (33 << PCM_RXC_CH2POS_SFT) | (8 << PCM_RXC_CH2WID_SFT));
            sil_wrw_mem((uint32_t *)PCM_TXC_A,
                    PCM_TXC_CH1WEX_BIT | PCM_TXC_CH1EN_BIT |
                    (1 << PCM_TXC_CH1POS_SFT) | (8 << PCM_TXC_CH1WID_SFT) |
                    PCM_TXC_CH2WEX_BIT | PCM_TXC_CH2EN_BIT |
                    (33 << PCM_TXC_CH2POS_SFT) | (8 << PCM_TXC_CH2WID_SFT));
            sil_wrw_mem((uint32_t *)PCM_CS_A,
                    PCM_CS_SYNC_BIT | PCM_CS_RXERR_BIT | PCM_CS_TXERR_BIT |
                    PCM_CS_RXTHR_SINGLE | PCM_CS_TXTHR_ONE |
                    PCM_CS_RXCLR_BIT | PCM_CS_TXCLR_BIT |
                    PCM_CS_TXON_BIT | PCM_CS_RXON_BIT | PCM_CS_EN_BIT);
            break;
        default:
            break;
        }
    } else {
        /*
        *  I2Sを初期化
        */
        switch (bit) {
        case 16:
            break;
        case 24:
        case 32:
            sil_wrw_mem((uint32_t *)PCM_MODE_A,
                    PCM_MODE_CLKM_BIT | PCM_MODE_CLKI_BIT |
                    PCM_MODE_FSM_BIT | PCM_MODE_FSI_BIT |
                    ((256 - 1) << PCM_MODE_FLEN_SFT) |
                    (128 << PCM_MODE_FSLEN_SFT));
            sil_wrw_mem((uint32_t *)PCM_RXC_A,
                    PCM_RXC_CH1WEX_BIT | PCM_RXC_CH1EN_BIT |
                    (1 << PCM_RXC_CH1POS_SFT) | (8 << PCM_RXC_CH1WID_SFT) |
                    PCM_RXC_CH2WEX_BIT | PCM_RXC_CH2EN_BIT |
                    (129 << PCM_RXC_CH2POS_SFT) | (8 << PCM_RXC_CH2WID_SFT));
            sil_wrw_mem((uint32_t *)PCM_TXC_A,
                    PCM_TXC_CH1WEX_BIT | PCM_TXC_CH1EN_BIT |
                    (1 << PCM_TXC_CH1POS_SFT) | (8 << PCM_TXC_CH1WID_SFT) |
                    PCM_TXC_CH2WEX_BIT | PCM_TXC_CH2EN_BIT |
                    (129 << PCM_TXC_CH2POS_SFT) | (8 << PCM_TXC_CH2WID_SFT));
            sil_wrw_mem((uint32_t *)PCM_CS_A,
                    PCM_CS_SYNC_BIT | PCM_CS_RXERR_BIT | PCM_CS_TXERR_BIT |
                    PCM_CS_RXTHR_SINGLE | PCM_CS_TXTHR_ONE |
                    PCM_CS_RXCLR_BIT | PCM_CS_TXCLR_BIT |
                    PCM_CS_TXON_BIT | PCM_CS_RXON_BIT | PCM_CS_EN_BIT);
            break;
        default:
            break;
        }
    }
}

bool_t i2s_rcv_rdy(void) {
    bool_t ready;

    if ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_RXD_BIT)
            == PCM_CS_RXD_BIT) {
        ready = true;
    } else {
        ready = false;
    }
    return ready;
}

bool_t i2s_snd_rdy(void) {
    bool_t ready;

    if ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_TXD_BIT)
            == PCM_CS_TXD_BIT) {
        ready = true;
    } else {
        ready = false;
    }
    return ready;
}

void i2s_rcv_data(uint32_t *val_l, uint32_t *val_r)
{
    while ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_RXD_BIT) == 0x0);
    *val_l = sil_rew_mem((uint32_t *)PCM_FIFO_A);
    while ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_RXD_BIT) == 0x0);
    *val_r = sil_rew_mem((uint32_t *)PCM_FIFO_A);
}

void i2s_snd_data(uint32_t *val_l, uint32_t *val_r)
{
    while ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_TXD_BIT) == 0x0);
    sil_wrw_mem((uint32_t *)PCM_FIFO_A, *val_l);
    while ((sil_rew_mem((uint32_t *)PCM_CS_A) & PCM_CS_TXD_BIT) == 0x0);
    sil_wrw_mem((uint32_t *)PCM_FIFO_A, *val_r);
}

void i2s_rcv_int_ena(void) {
    uint32_t tmp;

    tmp = sil_rew_mem((uint32_t *)PCM_CS_A);
    tmp |= PCM_CS_SYNC_BIT | PCM_CS_RXERR_BIT;
    tmp |= PCM_CS_RXTHR_SINGLE;
    tmp |= PCM_CS_RXCLR_BIT;
    sil_wrw_mem((uint32_t *)PCM_CS_A, tmp);

    tmp = sil_rew_mem((uint32_t *)PCM_INTEN_A);
    tmp |= PCM_INTEN_RXR_BIT;
    sil_wrw_mem((uint32_t *)PCM_INTEN_A, tmp);
}

void i2s_rcv_int_dis(void) {
    uint32_t tmp;

    tmp = sil_rew_mem((uint32_t *)PCM_INTEN_A);
    tmp &= ~PCM_INTEN_RXR_BIT;
    sil_wrw_mem((uint32_t *)PCM_INTEN_A, tmp);

    tmp = sil_rew_mem((uint32_t *)PCM_INTSTC_A);
    tmp |= PCM_INTSTC_RXR_BIT;
    sil_wrw_mem((uint32_t *)PCM_INTSTC_A, tmp);
}

void i2s_snd_int_ena(void) {
}

void i2s_snd_int_dis(void) {
}

void sio_isr(intptr_t exinf)
{
    if ((sil_rew_mem((uint32_t *)PCM_INTSTC_A) & PCM_INTSTC_RXR_BIT)
            == PCM_INTSTC_RXR_BIT) {
        // call function
        sil_wrw_mem((uint32_t *)PCM_INTSTC_A,
                sil_rew_mem((uint32_t *)PCM_INTSTC_A) | PCM_INTSTC_RXR_BIT);
    }
}
