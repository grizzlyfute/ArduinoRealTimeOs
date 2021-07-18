// .include" avr/io.h"
#include <avr/io.h>
.global TIMER0_COMPA_vect
.global launchProcess
.extern pendingProcess
.extern currentProcess
.extern tableProcess
.extern blackHole
// PROCESS_SIZE: .db 5
.equ PROCESS_SIZE, 5
.data
.global interruptCounter
interruptCounter: .byte 0
//.section .data
//.byte someconstant = 1
//.org 0x0
.section .text

// call from time interrput
// Note: do not call any subruntim because no place in stack for this...
TIMER0_COMPA_vect:
	// start ISR
	push r31
	push r30
	push r25
	push r24
	// save status register
	in r25, _SFR_IO_ADDR(SREG)
	push r25
	// debuger
	ldi r24,(1 << 4) //$$$$$
	in r25,_SFR_IO_ADDR(PORTB) //$$$$$
	eor r25,r24 //$$$$$
	out _SFR_IO_ADDR(PORTB),r25 //$$$$$
	// signaling tick system occurs
	lds r25, interruptCounter
	inc r25
	sts interruptCounter,r25
	// if pending process different from zero, switch to it
	lds r24, currentProcess
	lds r25, pendingProcess
	cpse r25,r24
	rjmp beginroutime
	rjmp endofroutime
beginroutime:
	// save other registers
	push r0
	push r1
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r18
	push r19
	push r20
	push r21
	push r22
	push r23
	push r26
	push r27
	push r28
	push r29
	// save our stack pointer [For More 64 kbytes, use RAMPD]
	ldi r18, PROCESS_SIZE
	// begin _gotSPForProcess
	mul r24,r18
	clr r1
	ldi ZH, hi8(tableProcess)
	ldi ZL, lo8(tableProcess)
	add ZL, r0
	adc ZH, r1
	// end _gotSPForProcess
	in r23, _SFR_IO_ADDR(SPH)
	in r22, _SFR_IO_ADDR(SPL)
	std Z+1, r23
	st Z, r22
	// define current process as pending process and pendingProcess as 0
	sts currentProcess, r25
	sts pendingProcess, r1
	// now got pending process [For More 64 kbytes, use RAMPD]
	// begin _gotSPForProcess
	mul r25,r18
	clr r1
	ldi ZH, hi8(tableProcess)
	ldi ZL, lo8(tableProcess)
	add ZL, r0
	adc ZH, r1
	// end _gotSPForProcess
	ldd r23, Z+1
	ld r22, Z
	out _SFR_IO_ADDR(SPH), r23
	out _SFR_IO_ADDR(SPL), r22
	// restore other registers from new stack
	pop r29
	pop r28
	pop r27
	pop r26
	pop r23
	pop r22
	pop r21
	pop r20
	pop r19
	pop r18
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	pop r1
	pop r0
	// bye
endofroutime:
	// restore sreg
	pop r25
	out _SFR_IO_ADDR(SREG), r25
	// end ISR
	pop r24
	pop r25
	pop r30
	pop r31
	reti

// doing in assembly because we hack SP address
// stack protector should be disabled
// r25:r24 == process id (r25 = 0, r24 = pid (uint8_t))
// r23:r22 == process fn
// r20:r21 == arg ptr
// r18:r19 == stack offset
launchProcess:
	// process interrupt will use value we defined. Ensure we never interrupt by it
	// TODO: only disable sys time interrupt
	cli
	// prepare our stack pointer to be unfold by the interrupt
	push r31
	push r30
	push r25
	push r24
	in r25, _SFR_IO_ADDR(SREG)
	push r25
	push r0
	push r1
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r18
	push r19
	push r20
	push r21
	push r22
	push r23
	push r26
	push r27
	push r28
	push r29
	// save fonction arg
	movw r2,r20
	// save our stack pointer (to pid 0)
	ldi ZH, hi8(tableProcess)
	ldi ZL, lo8(tableProcess)
	in r21, _SFR_IO_ADDR(SPH)
	in r20, _SFR_IO_ADDR(SPL)
	std Z+1, r21
	st Z, r20
	// prepare process stack pointer (SP -= r20:r21)
	sub r20, r18
	sbc r21, r19
	// change currentProcess to pendingProcess
	sts currentProcess, r24
	sts pendingProcess, r1
	// Change stack pointer to the fonction. Will be saved on inetrrupt
	//(do not use call, because we had had just hacked the return address)
	out _SFR_IO_ADDR(SPH), r21
	out _SFR_IO_ADDR(SPL), r20
	// set return address to black hole
	ldi r25, hi8(blackHole)
	push r25
	ldi r25, lo8(blackHole)
	push r25
	// function arg
	movw r24, r2
	// function start address
	movw Z, r22
	// re-enable interrupt
	sei
	// now run our fonction. Use jmp is safer than rjmp
	// [use EIJMP for more than 64 kbytes]
	ijmp
	// no ret. We will return by interrupt (reti)
.end
