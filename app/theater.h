#include "target_test.h"

#define TASK1_PRIORITY	9
#define MAIN_PRIORITY	5

#ifndef TASK_PORTID_G_SYSLOG
#define	TASK_PORTID_G_SYSLOG	1
#endif /* TASK_PORTID_G_SYSLOG */

#ifndef STACK_SIZE
#define	STACK_SIZE		4096
#endif /* STACK_SIZE */

#ifndef TOPPERS_MACRO_ONLY

extern void	cyclic_handler(intptr_t exinf);
extern void	task1(intptr_t exinf);
extern void	main_task(intptr_t exinf);

#endif /* TOPPERS_MACRO_ONLY */
