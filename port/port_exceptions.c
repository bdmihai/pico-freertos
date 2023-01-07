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

#include "picortos.h"
#include "cmsis_rp2040.h"
#include "task.h"
#include "portmacro.h"
#include "port.h"

/**
 * @brief panic.
 *
 * This is a dummy function as the real panic should be implemented by application.
 *
 */
__attribute__((weak)) void panic(const char *fmt, ...)
{
    (void)(fmt);
}

/**
 * @brief Used to catch tasks that attempt to return from their implementing function.
 *
 * A function that implements a task must not exit or attempt to return to its caller
 * as there is nothing to return to. If a task wants to exit it should instead call
 * vTaskDelete.
 */
void vPortTaskExitError( void )
{
    volatile uint32_t ulDummy = 0UL;

    /* Artificially force an assert() to be triggered if configASSERT() is defined,
    then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();
    while( ulDummy == 0 ) {
        /* ulDummy is used purely to quieten other warnings
        about code appearing after this function is called - making ulDummy
        volatile makes the compiler think the function could return and
        therefore not output an 'unreachable code' warning for code that appears
        after it. */
    }
}

/**
 * @brief Blink two short pulses if malloc fails.
 *
 * This function will only be called if configUSE_MALLOC_FAILED_HOOK is set to
 * 1 in FreeRTOSConfig.h.  It is a hook function that will get called if a call
 * to pvPortMalloc() fails. pvPortMalloc() is called internally by the kernel
 * whenever a task, queue, timer or semaphore is created.  It is also called by
 * various parts of the demo application.  If heap_1.c or heap_2.c are used,
 * then the size of the heap available to pvPortMalloc() is defined by
 * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API
 * function can be used to query the size of free heap space that remains
 * (although it does not provide information on how the remaining heap might be
 * fragmented).
 */
void vApplicationMallocFailedHook()
{
    panic("vApplicationMallocFailedHook: malloc failed");
}

/**
 * @brief Run time stack overflow checking hook function.
 *
 * Run time stack overflow checking is performed if configCHECK_FOR_STACK_OVERFLOW
 * is defined to 1 or 2.  This hook function is called if a stack overflow is detected.
 * \param[in] pxTask Task handle
 * \param[in] pcTaskName Task name
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;
    panic("vApplicationStackOverflowHook: stack overflow is detected");
}
