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
 |  Author: Mihai Baneu                           Last modified: 23.Jan.2023  |
 |  Based on original M0+rp2040 port from http://www.FreeRTOS.org             |
 |___________________________________________________________________________*/

#pragma once

/* get the core id and interupt number*/
extern int32_t vPortGetCoreId();
extern unsigned int __get_current_exception();

extern uint32_t ulPortDisableInterrupts();
extern void vPortEnableInterrupts();
extern void vPortRestoreInterrupts(uint32_t mask);

/* critical section handling */
extern void vTaskEnterCritical();
extern void vTaskExitCritical();

/* locks handling */
extern void vPortGetISRLock();
extern void vPortReleaseISRLock();
extern void vPortGetTaskLock();
extern void vPortReleaseTaskLock();

/* used to catch tasks that attempt to return from their implementing function. */
extern void vPortTaskExitError(void);

/* setup the systick timer */
extern void vPortConfigureSysTick(void);

/* yield the next highest prio task */
extern void vPortYield(void);
extern void vPortYieldCore(int32_t core);

/* isr handlers */
extern void vPortSVCHandler();
extern void vPortPendSVHandler();
extern void vPortSysTickHandler();

/* start the first task */
extern void vPortStartFirstTask(int32_t core);

/* svc C handler */
extern void vPortServiceHandler(uint32_t *svc_args);

/* stats gathering function */
extern uint32_t vPortGetStatsTimerValue(void);

