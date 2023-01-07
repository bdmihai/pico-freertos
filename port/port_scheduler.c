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
#include "port.h"

/**
 * @brief This is the startup of the scheduler. All necessary HW for the RTOS is
 * prepared and configured.
 *
 */
BaseType_t xPortStartScheduler(void)
{
    /* add PendSV_IRQn, SVCall_IRQn and SysTick_IRQn handlers */
    NVIC_SetVector(SVCall_IRQn, (uint32_t)vPortSVCHandler);
    NVIC_SetVector(PendSV_IRQn, (uint32_t)vPortPendSVHandler);
    NVIC_SetVector(SysTick_IRQn, (uint32_t)vPortSysTickHandler);

    /* set PendSV_IRQn, SVCall_IRQn and SysTick_IRQn priority */
    NVIC_SetPriority(SVCall_IRQn, configSVCall_INTERRUPT_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, configPendSV_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SysTick_IRQn, configSysTick_INTERRUPT_PRIORITY);

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    vPortConfigureSysTick();

    /* start the first task */
    vPortStartFirstTask();

    /* Should never get here as the tasks will now be executing!  Call the task
    exit error function to prevent compiler warnings about a static function
    not being called in the case that the application writer overrides this
    functionality. Call vTaskSwitchContext() so link time optimisation does not
    remove the symbol. */
    vTaskSwitchContext();
    vPortTaskExitError();

    /* Should not get here! */
    return 0;
}

/**
 * @brief End scheduler.
 *
 */
void vPortEndScheduler(void)
{
    /* Not implemented in ports where there is nothing to return to.
    Artificially force an assert. */
    configASSERT( uxCriticalNesting == 1000UL );
}

/**
 * @brief Handler for the Supervisor Call.
 * 
 * @param svc_args 
 */
void vPortServiceHandler(uint32_t *svc_args)
{
    uint8_t svc_number = ((char *) svc_args[6])[-2]; //Memory[(Stacked PC)-2]
    // r0 = svc_args[0];
    // r1 = svc_args[1];
    // r2 = svc_args[2];
    // r3 = svc_args[3];

    switch (svc_number)
    {
        case 0:
            vPortYield();
            break;
    }
}

/**
 * @brief Yield the next task that needs to be executed.
 *
 * The PendSV interrupt is activated for changin the context.
 *
 */
void vPortYield(void)
{
    /* context switching is performed in the PendSV interrupt. Pend the PendSV interrupt. */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

    /* Data Synchronization Barrier and Instruction Synchronization Barrier */
    __DSB();
    __ISB();
}
