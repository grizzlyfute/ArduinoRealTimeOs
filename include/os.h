#ifndef OS_H
#define OS_H
#include <stdint.h>

//32 registers + sreg +  'blackhole' + reti
#define MINSTACK_SIZE (33*sizeof(uint8_t)+2*sizeof(uint16_t))

#define PROCESS_NUMBER (2+1)

#define TICK_SYS 100 /* us */

//stack size for processus without interrupt size
#define S0 0
#define S1 64
#define S2 0
#define S3 0
#define S4 0
#define S5 0
#define S6 0
#define S7 0
#define S8 0
#define S9 0

// offset from launching process without minstack size
#define PROCESS_STACK_OFFSET(pid) \
(\
	(S1 + MINSTACK_SIZE) * (pid > 1) + \
	(S2 + MINSTACK_SIZE) * (pid > 2) + \
	(S3 + MINSTACK_SIZE) * (pid > 3) + \
	(S4 + MINSTACK_SIZE) * (pid > 4) + \
	(S5 + MINSTACK_SIZE) * (pid > 5) + \
	(S6 + MINSTACK_SIZE) * (pid > 6) + \
	(S7 + MINSTACK_SIZE) * (pid > 7) + \
	(S8 + MINSTACK_SIZE) * (pid > 8) + \
	(S9 + MINSTACK_SIZE) * (pid > 9) + \
	0 \
)

#define WAIT_MASK 0x80
#define PRIO_MASK 0x07

// update os_asm.S if modified
typedef struct Process Process;
struct Process
{
	uint16_t sp; // allway first (optimizing assembly)
	uint16_t quantum; // is tick_sys/2
	uint8_t flags_prio; // flags or priority (7 = slow, 0 = high)
};

extern volatile uint8_t currentProcess;

extern Process tableProcess[PROCESS_NUMBER];

void waitTick(uint16_t value);
// Wait event: for waiting : changePriority(process, 0xFF);
// signaling event changePriority(process, 0x10)
void changePriority(uint8_t process, uint8_t value);
void mutexIni(uint8_t *mutex);
void mutexSet(uint8_t process, uint8_t *mutex);
void mutexRelease(uint8_t process, uint8_t *mutex);

#endif /* OS_H*/
