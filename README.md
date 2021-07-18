# Arduino real time operating system
## General description
This software demonstrate the concept of a real time operating system, running
on an atemga32 / arduino board.
This is a multitask os, using a timer.

Considere two task and a timer.
The first task is running initialy. When timer interruption occurs:
- We save the stack in process space memory and the return address
- We load the stack of second process
- We override the return address, in order to continue second process

Be carreful, atmega32 has limited cpu and ram resources. You can run only a few
(10) simple task. This is a microcontroler, not a gamming computer !

## What is a process
A process is a C function. It as a reserved stack space in memory, and is
allowed to run in a time quantum. When time quauntum expire, the changing
process runtime stop the process and lanch an other.
A process with a low priority have more time quantum. 0 is quasi realtime

## How to use
os.h and os.c provided utility function

    #define PROCESS_NUMBER (2+1)
This will set the number of process you want. Remember, you have to add a +1
for the operating system

    #define TICK_SYS 100 /* us */
This is the duration of a tick system. setting a low value is not recommended as
time interruption will always occurs and os will always switch context. Setting
a higher value will give the impression of not running in paralle. Duration is
µs

    #define Sx xx
These constants adjust the stack size needed for processus n. Adjusting then as
your need, in order to exploit all available memory of the atmega32

    void waitTick(uint16_t value);
This is a sleep equivalent. When calling, the process will be suspend for
value of tick system. The switch context runtine is called. During this time,
operating system will concentrate on other process. If all process are waiting,
the black hole (no op loop, if launched) will be running.

    void changePriority(uint8_t process, uint8_t value);
Change priority of processid process, setting new priority to value

    void mutexIni(uint8_t *mutex);
Create a mutex

    void mutexSet(uint8_t process, uint8_t *mutex);
Set a mutex for process process

    void mutexRelease(uint8_t process, uint8_t *mutex);
Relase a mutex for process process
