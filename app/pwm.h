#ifndef PWM_H
#define PWM_H

#define INHNO_PWM       IRQNO_PWA0
#define INTNO_PWM       IRQNO_PWA0
#define INTPRI_PWM      -3
#define INTATR_PWM      0U

#ifndef TOPPERS_MACRO_ONLY

extern void pwm_init(int sr, int bit);
extern void pwm_write(uint32_t* val_l, uint32_t* val_r);
extern void pwm_write_16(int16_t* val_l, int16_t* val_r);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* PWM_H */
