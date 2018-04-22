#ifndef I2S_H
#define I2S_H

#define INHNO_I2S       IRQNO_PCM
#define INTNO_I2S       IRQNO_PCM
#define INTPRI_I2S      -3
#define INTATR_I2S      0U

#define INHNO_I2SPCM    IRQNO_I2SPCM
#define INTNO_I2SPCM    IRQNO_I2SPCM
#define INTPRI_I2SPCM   -3
#define INTATR_I2SPCM   0U

#define I2S_MSTR        0
#define I2S_SLAV        1

#ifndef TOPPERS_MACRO_ONLY

extern void i2s_init(int master_mode, int sr, int bit);
extern bool_t i2s_rcv_isrdy(void);
extern bool_t i2s_snd_isrdy(void);
extern void i2s_rcv_data(uint32_t *val_l, uint32_t *val_r);
extern void i2s_snd_data(uint32_t *val_l, uint32_t *val_r);
extern void i2s_rcv_int_ena(void (*callback)(void));
extern void i2s_rcv_int_dis(void);
extern void i2s_snd_int_ena(void (*callback)(void));
extern void i2s_snd_int_dis(void);
extern void i2s_isr(intptr_t exinf);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* I2S_H */
