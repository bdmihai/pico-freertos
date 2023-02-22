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

#include "picortos.h"
#include "cmsis_rp2040.h"
#include "port.h"
#include "pico/time.h"
#include "pico/multicore.h"

/**
 * FIFO handler for triggering a yeald from the other core.
*/
void vPortFIFOHandler(void)
{
    /* remove the contents (which we don't care about)
     * to clear the IRQ */
    multicore_fifo_drain();

    /* and explicitly clear any other IRQ flags */
    multicore_fifo_clear_irq();

    /* yeald on the current core */
    vPortYield();
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
            vPortSetFirstTaskContext(portGET_CORE_ID());
            break;
        case 1:
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

void vPortYieldCore(int32_t core)
{
    (void) core;
    sio_hw->fifo_wr = 0;
}

uint32_t vPortGetStatsTimerValue(void)
{
    return time_us_64();
}
