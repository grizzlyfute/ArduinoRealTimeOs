#include "os.h"
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

Process tableProcess[PROCESS_NUMBER];
volatile uint8_t currentProcess;
volatile uint8_t pendingProcess;
extern volatile uint8_t interruptCounter;
extern void launchProcess(uint8_t process, void (*fn)(void*), void * arg, uint16_t stackOffset);

#define stopTimerInterrupt() do { TIMSK0 &= ~(1 << OCIE0A); } while(0)
#define startTimerInterrupt() do { TIMSK0 |= (1 << OCIE0A); } while(0)

void blackHole(void)
{
	// register r24, r25 contains the return value if needed
	tableProcess[currentProcess].flags_prio |= WAIT_MASK;
	while (1) _NOP();
}

// Inline is important here (do not use stack)
inline void waitInterrupt(void)
{
	register uint8_t a;
	// As we use uint8_t as counter, getting interruptCounter will be perform in only one cycle. Disabling interrupt is unnecessary
	a = interruptCounter;
	while (a == interruptCounter);
}

void waitTick(uint16_t value)
{
	stopTimerInterrupt();
	tableProcess[currentProcess].quantum = value;
	startTimerInterrupt();
	waitInterrupt();
}

inline void changePriority(uint8_t process, uint8_t value)
{
	tableProcess[process].flags_prio = value & PRIO_MASK;
	// continue program, priority will be effect at next system tick
}

inline void mutexIni(uint8_t *mutex)
{
	*mutex = 1;
}
void mutexSet(uint8_t process, uint8_t *mutex)
{
	while (*mutex <= 0)
	{
		stopTimerInterrupt();
		tableProcess[process].flags_prio |= WAIT_MASK;
		startTimerInterrupt();
		waitInterrupt();
	}
	(*mutex)--;
}
inline void mutexRelease(uint8_t process, uint8_t *mutex)
{
	tableProcess[process].flags_prio &= ~WAIT_MASK;
	(*mutex)++;
}

void writeScope(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void demo(void* arg __attribute__((unused)))
{
	//writeScope(tableProcess[0].sp >> 8, tableProcess[0].sp & 0xFF, tableProcess[1].sp >> 8, tableProcess[1].sp & 0xFF);
	DDRB |= (1 << 5);
	do
	{
		PORTB ^= (1 << 5);
		// _delays_ms will return at NUMBER_PROCESS*time
		waitTick(5000);
	}
	while (1);
}

void demo1(void* arg __attribute__((unused)))
{
	//writeScope(tableProcess[0].sp >> 8, tableProcess[0].sp & 0xFF, tableProcess[1].sp >> 8, tableProcess[1].sp & 0xFF);
	DDRB |= (1 << 2);
	do
	{
		PORTB ^= (1 << 2);
		// _delays_ms will return at NUMBER_PROCESS*time
		waitTick(5000);
	}
	while (1);
}

#define launchPrx(ind, fn, arg) launchProcess(ind, fn, arg, PROCESS_STACK_OFFSET(ind))

int main(void)
{
	uint8_t i = 0, j = 0, oldProcess = 0, oldCounter = 0;
	memset(tableProcess, 0, sizeof(tableProcess));
	currentProcess = 0;
	pendingProcess = 0;

	tableProcess[0].sp = SP;
	tableProcess[0].flags_prio = WAIT_MASK | PRIO_MASK;

	//System tick. every 100 us
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS00) | (1<< CS01);
	OCR0A = 25;
	TIMSK0 = 1 << OCIE0A;
	startTimerInterrupt();

	// Launch the process when first system tick occurs, we will reswap to this function and launching the next
	launchPrx(1, demo, NULL);
	launchPrx(2, demo1, NULL);
	sei();
	//writeScope(tableProcess[0].sp & 0xFF, tableProcess[1].sp & 0xFF, currentProcess, pendingProcess);

	// At this point, we should'nt call any not inlined function, declare any variable or launch any new process

	do
	{
		PORTB ^= (1 << 3);
		// Got an interruption : it's time to change our process
		//no need to check for overflow (modular arithmetic) a + 0xFF -x [0xFF] :=: a -x [0xFF]
		j = interruptCounter - oldCounter;
		oldCounter = interruptCounter;
		j *= (PRIO_MASK + 1) - (tableProcess[oldProcess].flags_prio & PRIO_MASK);

		// Update quatum
		for (i = 0; i < PROCESS_NUMBER; i++)
		{
			// protect against interrupt, but do not miss them
			stopTimerInterrupt();
			if (tableProcess[i].quantum > j)
				tableProcess[i].quantum -= j;
			else
				tableProcess[i].quantum = 0;
			startTimerInterrupt();
		}

		// select new process
		for (j = 0; j < PROCESS_NUMBER; j++)
		{
			i = j + oldProcess;
			if (i >= PROCESS_NUMBER) i = 0;

			stopTimerInterrupt();

			if (!(tableProcess[i].flags_prio & WAIT_MASK) && tableProcess[i].quantum == 0)
			{
				oldProcess = i;
				pendingProcess = i;
				startTimerInterrupt();
				break;
			}
			startTimerInterrupt();
		}
		if (j >= PROCESS_NUMBER) oldProcess = 0;
		OCR0A = 25*((PRIO_MASK + 1) - (tableProcess[oldProcess].flags_prio & PRIO_MASK));
		waitInterrupt();
	}
	while (1);

	return 1;
}
