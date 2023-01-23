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

#include <assert.h>
#include <stdint.h>
#include "cmsis_gcc.h"
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
typedef uint32_t StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffff
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

	/* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
	not need to be guarded with a critical section. */
	#define portTICK_TYPE_IS_ATOMIC 1
#endif
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH						( -1 )
#define portTICK_PERIOD_MS						( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT						8
#define portFORCE_INLINE 						inline __attribute__((always_inline))
#define portMEMORY_BARRIER()					__DMB()
/*-----------------------------------------------------------*/

/* Scheduler utilities. */
#define portYIELD() 							vPortYield()
#define portYIELD_FROM_ISR() 					vPortYield()
#define portYIELD_CORE(x)                       vPortYieldCore(x)
/*-----------------------------------------------------------*/

/* Critical section management. */
#define portSET_INTERRUPT_MASK_FROM_ISR()		ulPortDisableInterrupts(); vTaskEnterCritical()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	vTaskExitCritical(); vPortRestoreInterrupts(x)

#define portDISABLE_INTERRUPTS()				ulPortDisableInterrupts()
#define portENABLE_INTERRUPTS()					vPortEnableInterrupts()
#define portRESTORE_INTERRUPTS(x)      	        vPortRestoreInterrupts(x)
#define portENTER_CRITICAL()					vTaskEnterCritical()
#define portEXIT_CRITICAL()						vTaskExitCritical()

#define portGET_ISR_LOCK()      				vPortGetISRLock()
#define portRELEASE_ISR_LOCK()  				vPortReleaseISRLock()
#define portGET_TASK_LOCK()     				vPortGetTaskLock()
#define portRELEASE_TASK_LOCK() 				vPortReleaseTaskLock()

#define portCHECK_IF_IN_ISR()                   (__get_current_exception() > 0)
#define portGET_CORE_ID()                       vPortGetCoreId()
/*-----------------------------------------------------------*/

 /* macros used to allow port/compiler specific language extensions.*/
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )  void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )        void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* assertions enabled */
#define configASSERT( x ) assert( x )

#ifdef __cplusplus
}
#endif
