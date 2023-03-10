/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2023 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 08.Jan.2023  |
 |  Based on original M0+rp2040 port from http://www.FreeRTOS.org             |
 |___________________________________________________________________________*/
 
.syntax unified

#if FREERTOS_IN_RAM
#define port_section data
#else
#define port_section text
#endif

/*-----------------------------------------------------------*/
/*                  ulSetInterruptMaskFromISR                */
/*-----------------------------------------------------------*/
.section .port_section.ulSetInterruptMaskFromISR, "ax", %progbits
.global ulSetInterruptMaskFromISR
.type ulSetInterruptMaskFromISR, %function
ulSetInterruptMaskFromISR:
    mrs r0, PRIMASK
    cpsid i
    bx lr

.size ulSetInterruptMaskFromISR, .-ulSetInterruptMaskFromISR

/*-----------------------------------------------------------*/
/*                  vClearInterruptMaskFromISR               */
/*-----------------------------------------------------------*/
.section .port_section.vClearInterruptMaskFromISR, "ax", %progbits
.global vClearInterruptMaskFromISR
.type vClearInterruptMaskFromISR, %function
vClearInterruptMaskFromISR:
    msr PRIMASK, r0
    bx lr

.size vClearInterruptMaskFromISR, .-vClearInterruptMaskFromISR

/*-----------------------------------------------------------*/
/*                   vPortDisableInterrupts                  */
/*-----------------------------------------------------------*/
.section .port_section.vPortDisableInterrupts, "ax", %progbits
.global vPortDisableInterrupts
.type vPortDisableInterrupts, %function
vPortDisableInterrupts:
    cpsid i
    bx lr

.size vPortDisableInterrupts, .-vPortDisableInterrupts

/*-----------------------------------------------------------*/
/*                   vPortEnableInterrupts                   */
/*-----------------------------------------------------------*/
.section .port_section.vPortEnableInterrupts, "ax", %progbits
.global vPortEnableInterrupts
.type vPortEnableInterrupts, %function
vPortEnableInterrupts:
    cpsie i
    bx lr

.size vPortEnableInterrupts, .-vPortEnableInterrupts

/*-----------------------------------------------------------*/
/*                     vPortEnterCritical                    */
/*-----------------------------------------------------------*/
.section .port_section.vPortEnterCritical, "ax", %progbits
.global vPortEnterCritical
.type vPortEnterCritical, %function
vPortEnterCritical:
    cpsid i
    ldr r0, =uxCriticalNesting
    ldr r1, [r0]
    adds r1, r1, #1
    str r1, [r0]
    dsb
    isb
    bx lr

.size vPortEnterCritical, .-vPortEnterCritical

/*-----------------------------------------------------------*/
/*                      vPortExitCritical                    */
/*-----------------------------------------------------------*/
.section .port_section.vPortExitCritical, "ax", %progbits
.global vPortExitCritical
.type vPortExitCritical, %function
vPortExitCritical:
    ldr r0, =uxCriticalNesting
    ldr r1, [r0]
    subs r1, r1, #1
    str r1, [r0]
    cmp r1, #0
    bne 1f
    cpsie i
1:
    bx lr

.size vPortExitCritical, .-vPortExitCritical

.section .data.uxCriticalNesting
.global uxCriticalNesting
.align 4
uxCriticalNesting:
    .word 0xAAAAAAAA

/*-----------------------------------------------------------*/
/*                     vPortStartFirstTask                   */
/*-----------------------------------------------------------*/
.section .port_section.vPortStartFirstTask, "ax", %progbits
.global vPortStartFirstTask
.type vPortStartFirstTask, %function
vPortStartFirstTask:
    /* set the msp back to the start of the stack. */
    ldr r0, =__stack0_top
    msr msp, r0

    /* get the location of the pxCurrentTCB */
    ldr	r2, pxCurrentTCBConst1
    ldr r3, [r2]

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r0, [r3]

    /* discard everything up to r0. */
    adds r0, #32

    /* reset privileges and set the psp as the stack pointer */
    msr psp, r0
    mrs r0, control
    movs r1, #2
    orrs r0, r1
    msr control, r0
    isb

    pop  {r0-r5}                /* Pop the registers that are saved automatically. */
    mov  lr, r5                 /* lr is now in r5. */
    pop  {r3}                   /* Return address is now in r3. */
    pop  {r2}                   /* Pop and discard XPSR. */
    
    /* globally enable interrupts. */
    cpsie i

    /* jump to the first task code */
    bx r3
    
.align 4
pxCurrentTCBConst1: 
    .word pxCurrentTCB
.size vPortStartFirstTask, .-vPortStartFirstTask

/*-----------------------------------------------------------*/
/*                       vPortSVCHandler                     */
/*-----------------------------------------------------------*/
.section .port_section.vPortSVCHandler, "ax", %progbits
.global vPortSVCHandler
.type vPortSVCHandler, %function
vPortSVCHandler:
    /* check what stack pointer to use */
    mov  r1, lr
    movs r2, #4
    tst r1, r2
    bne 2f
1:
    mrs r0, msp
    b 3f
2:
    mrs r0, psp
3:
    bl vPortServiceHandler

.size vPortSVCHandler, .-vPortSVCHandler

/*-----------------------------------------------------------*/
/*                    vPortPendSVHandler                     */
/*-----------------------------------------------------------*/
.section .port_section.vPortPendSVHandler, "ax", %progbits
.global vPortPendSVHandler
.type vPortPendSVHandler, %function
vPortPendSVHandler:
    /* get the process stack pointer */
    mrs r0, psp

    /* get the location of the pxCurrentTCB */
    ldr	r3, pxCurrentTCBConst2
    ldr	r2, [r3]

    /* go down in the stack in order to use the increment after function - M0+ is missing STMDB */
    subs r0, r0, #32 

    /* store in the first 4 bytes of the current context the new stack pointer */
    str r0, [r2]

    /* backup the lower registers to the task stack */
    stmia r0!, {r4-r7}

    /* backup the upper registers to the task stack */
    mov r4, r8
    mov r5, r9
    mov r6, r10
    mov r7, r11
    stmia r0!, {r4-r7}

    /* change the current context (address pointed by pxCurrentTCB) */
    push {r3, r14}
    cpsid i
    bl vTaskSwitchContext
    cpsie i
    pop {r2, r3} /* lr goes in r3. r2 now holds tcb pointer. */

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r1, [r2]                  
    ldr r0, [r1]

    /* restore the upper registers from the task stack */
    adds r0, r0, #16
    ldmia r0!, {r4-r7}
    mov r8, r4
    mov r9, r5
    mov r10, r6
    mov r11, r7

    /* this is now the process stack pointer */
    msr psp, r0

    /* restore the lower registers from the task stack */
    subs r0, r0, #32
    ldmia r0!, {r4-r7}

    /* return from handler mode */
    bx r3

.align 4
pxCurrentTCBConst2: 
    .word pxCurrentTCB

.size vPortPendSVHandler, .-vPortPendSVHandler

.end
