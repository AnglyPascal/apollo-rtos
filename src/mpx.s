@@@ Copyright (c) 2018 J. M. Spivey        
@@@ Inspired by Mike's work

@@@ Hardware multiplexing for the ARM Cortex-M0

    .syntax unified
    .text

@@@ __run -- enter process mode
    .global __run
    .thumb_func
__run:
    mrs r2, msp 
    str r2, [r1]
    @@ msr msp, r1              @ Set up the stack
    @@ movs r2, #2              @ Use psp for stack pointer
    @@ msr control, r2
    isb                         @ Drain the pipeline
    bx r0                       @ Call the body
        
@@@ Stack layout for interrupt frames (17 words, 68 bytes)
@@@ --------------------------------------
@@@ 16  PSR  Status register
@@@ 15  PC   Program counter
@@@ 14  LR   Link register
@@@ 13  R12
@@@ 12  R3
@@@ 11  R2           (Saved by hardware)
@@@ 10  R1
@@@  9  R0
@@@ --------------------------------------
@@@  0  LR'  Magic value <-- Stack pointer
@@@  4  R7
@@@  3  R6
@@@  2  R5
@@@  1  R4

@@@  8  R11   
@@@  7  R10
@@@  6  R9
@@@  5  R8           (Saved manually)
@@@ --------------------------------------

@@@ The magic value for exception return is carefully preserved for each
@@@ process.  On Cortex-M0, it will always be 0xfffffff9, but on other
@@@ chips it encodes info about the hardware-saved frame layout.

@@@ save context for system call
    .macro isave
    push {r4-r7, lr}
    mov r4, r8                  @ Copy from high to low
    mov r5, r9
    mov r6, r10
    mov r7, r11
    push {r4-r7}
    mrs r0, msp
    .endm                       @ Return new thread sp

@@@ restore context after system call
    .macro irestore             @ Expect process sp in r0

    msr msp, r0
    pop {r4-r7}
    mov r11, r7 
    mov r10, r6 
    mov r9, r5 
    mov r8, r4
    pop {r4-r7}
    pop {r4}

    bx r4
    .endm

@@@ handler for PendSV interupt (context switch)
    .global pendsv_handler
    .thumb_func
pendsv_handler:
    isave                       @ Complete saving of process state
    bl cxt_switch               @ Choose a new process
    irestore                    @ Restore state for that process

@@@ trigger a soft reset by setting the SYSRESETREQ bit of the AIRCR register
    .global trigger_reset
    .thumb_func
trigger_reset:
    ldr r0, =0xE000ED0C       @ Load the address of AIRCR into r0
    ldr r1, =0x05FA0004       @ Load the VECTKEY (0x5FA << 16) | SYSRESETREQ bit
    str r1, [r0]              @ Write the value directly to AIRCR

