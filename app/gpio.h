#ifndef GPIO_H
#define GPIO_H

#define GPIO_FSEL_IN    0
#define GPIO_FSEL_OUT   1
#define GPIO_FSEL_0     2
#define GPIO_FSEL_1     3
#define GPIO_FSEL_2     4
#define GPIO_FSEL_3     5
#define GPIO_FSEL_4     6
#define GPIO_FSEL_5     7

#define GPIO_LEVEL_HIGH   0
#define GPIO_LEVEL_LOW    1

extern void gpio_init(void);
extern void gpio_fsel(int id, int fsel);
extern void gpio_set(int id, int level);
extern int gpio_get(int id);

#endif /* GPIO_H */
