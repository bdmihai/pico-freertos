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
#include "portmacro.h"
#include "port.h"

/* initial xPSR value on the stack - set the Thumb state to 1 */
#define portINITIAL_XPSR                    0x01000000

/* address where a task should return. used to trap tasks that attempt to
return from their implementing functions (tasks should call vTaskDelete if
they want to exit */
#define portTASK_RETURN_ADDRESS             vPortTaskExitError

/**
 * @brief Simulate the stack frame as it would be created by a context switch interrupt.
 *
 * Setup the stack of a new task so it is ready to be placed under the
 * scheduler control.  The registers have to be placed on the stack in
 * the order that the port expects to find them.
 *
 * @param pxTopOfStack
 * @param pxCode
 * @param pvParameters
 * @return StackType_t*
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t) portINITIAL_XPSR;                         		/* xPSR */

    pxTopOfStack--;
    *pxTopOfStack = (StackType_t) pxCode;                                           /* PC */

    pxTopOfStack--;
    *pxTopOfStack = (StackType_t) portTASK_RETURN_ADDRESS;                			/* LR */

    pxTopOfStack -= 4;                                                      		/* R12, R3, R2 and R1. */

    pxTopOfStack--;
    *pxTopOfStack = (StackType_t) pvParameters;                           			/* R0 */

    pxTopOfStack -= 8;                                                      		/* R11, R10, R9, R8, R7, R6, R5 and R4. */

    return pxTopOfStack;
}
