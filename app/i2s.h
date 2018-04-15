#ifndef I2S_H
#define I2S_H

#define INHNO_I2S       IRQNO_PCM
#define INTNO_I2S       IRQNO_PCM
#define INTPRI_I2S      -2
#define INTATR_I2S      0U

#define I2S_MSTR        0
#define I2S_SLAV        1

extern void i2s_init(int master_mode, int sr, int bit);
extern void i2s_read(uint32_t* val_l, uint32_t* val_r);
extern void i2s_write(uint32_t* val_l, uint32_t* val_r);

#endif /* I2S_H */
