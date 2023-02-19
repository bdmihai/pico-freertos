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
 |  Author: Mihai Baneu                           Last modified: 19.Feb.2023  |
 |  Based on original M0+rp2040 port from http://www.FreeRTOS.org             |
 |___________________________________________________________________________*/
 
#include "hardware/platform_defs.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"

.syntax unified

#define INITIAL_EXC_RETURN  0XFFFFFFFD

#if FREERTOS_IN_RAM
#define port_section data
#else
#define port_section text
#endif

/*-----------------------------------------------------------*/
/*                        vPortGetCoreId                     */
/*-----------------------------------------------------------*/
.section .port_section.vPortGetCoreId, "ax", %progbits
.global vPortGetCoreId
.type vPortGetCoreId, %function
vPortGetCoreId:
    ldr r0, =(SIO_BASE + SIO_CPUID_OFFSET)
    ldr r0, [r0]
    bx lr

.size vPortGetCoreId, .-vPortGetCoreId

/*-----------------------------------------------------------*/
/*                   vPortDisableInterrupts                  */
/*-----------------------------------------------------------*/
.section .port_section.ulPortDisableInterrupts, "ax", %progbits
.global ulPortDisableInterrupts
.type ulPortDisableInterrupts, %function
ulPortDisableInterrupts:
    mrs r0, PRIMASK
    cpsid i
    bx lr

.size ulPortDisableInterrupts, .-ulPortDisableInterrupts

/*-----------------------------------------------------------*/
/*                   vPortRestoreInterrupts                  */
/*-----------------------------------------------------------*/
.section .port_section.vPortEnableInterrupts, "ax", %progbits
.global vPortEnableInterrupts
.type vPortEnableInterrupts, %function
vPortEnableInterrupts:
    cpsie i
    bx lr

.size vPortEnableInterrupts, .-vPortEnableInterrupts

/*-----------------------------------------------------------*/
/*                    vPortRestoreInterrupts                 */
/*-----------------------------------------------------------*/
.section .port_section.vPortRestoreInterrupts, "ax", %progbits
.global vPortRestoreInterrupts
.type vPortRestoreInterrupts, %function
vPortRestoreInterrupts:
    msr PRIMASK, r0
    bx lr

.size vPortRestoreInterrupts, .-vPortRestoreInterrupts

/*-----------------------------------------------------------*/
/*                     vPortStartFirstTask                   */
/*-----------------------------------------------------------*/
.section .text.vPortStartFirstTask, "ax", %progbits
.global vPortStartFirstTask
.type vPortStartFirstTask, %function

/* This function starts the first task by executing the Supervisor Call command. In 
this way the context of the first task is executed. */
vPortStartFirstTask:
   /* set the msp back to the start of the stack - r0 has the core index */
    adr r1, StackPointers
    lsls r0, #2
    ldr r1, [r1, r0]
    msr msp, r1

    /* globally enable interrupts. */
    cpsie i
    
    /* system call to start first task. */
    svc 0

.align 4
StackPointers:
.word __stack0_top
.word __stack1_top

.size vPortStartFirstTask, .-vPortStartFirstTask

/*-----------------------------------------------------------*/
/*                  vPortSetFirstTaskContext                 */
/*-----------------------------------------------------------*/
.section .text.vPortSetFirstTaskContext, "ax", %progbits
.global vPortSetFirstTaskContext
.type vPortSetFirstTaskContext, %function
vPortSetFirstTaskContext:
    /* get the location of the pxCurrentTCB */
    ldr	r2, =pxCurrentTCBs
    lsls r0, #2
    adds r2, r0
    ldr r3, [r2]

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r0, [r3]

    /* discard everything up to r0 */
    adds r0, #32

    /* set the new process stack pointer */
    msr psp, r0
    isb

    /* return from handler mode to thread mode: basically pops r0-r4, r12, r14(lr), r15(pc), psr */
    ldr r0, =INITIAL_EXC_RETURN
    bx r0

.align 4
.word pxCurrentTCBs

.size vPortSetFirstTaskContext, .-vPortSetFirstTaskContext

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
    b vPortServiceHandler

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
    ldr r1, =(SIO_BASE + SIO_CPUID_OFFSET)
    ldr r2, [r1]
    lsls r2, #2
    ldr	r3, =pxCurrentTCBs
    adds r3, r2
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
    ldr r0, [r1]  /* r0 = core index */
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
.word pxCurrentTCBs

.size vPortPendSVHandler, .-vPortPendSVHandler

.end
