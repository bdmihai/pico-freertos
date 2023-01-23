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

#include "picortos.h"
#include "cmsis_rp2040.h"
#include "task.h"
#include "port.h"
#include "portmacro.h"

/**
 * @brief Setup the systick timer to generate the tick interrupts at the required frequency.
 *
 */
void vPortConfigureSysTick(void)
{
    NVIC_SetVector(SysTick_IRQn, (uint32_t)vPortSysTickHandler);
    NVIC_SetPriority(SysTick_IRQn, configSysTick_INTERRUPT_PRIORITY);
    
    /* stop and clear the SysTick. */
    SysTick->CTRL = 0UL;
    SysTick->VAL  = 0UL;

    /* configure SysTick to interrupt at the requested rate. */
    SysTick->LOAD = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    SysTick->CTRL = ( SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );
}

/**
 * @brief Handler for the SysTick interupt.
 *
 * The SysTick runs at the lowest interrupt priority, so when this interrupt
 * executes all interrupts must be unmasked.  There is therefore no need to
 * save and then restore the interrupt mask value as its value is already
 * known.
 */
void vPortSysTickHandler(void)
{
    uint32_t ulPreviousMask;
    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        /* increment the RTOS tick. If necessary trigger a context switch using
        the PendSV interrupt */
        if( xTaskIncrementTick() != pdFALSE ) {
            /* a context switch is required to allow the next task to run */
            vPortYield();
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
}
