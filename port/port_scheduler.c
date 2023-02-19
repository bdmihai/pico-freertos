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
#include "hardware/regs/sio.h"
#include "hardware/structs/sio.h"
#include "pico/multicore.h"

#define ISR_LOCK_BIT   0x01U
#define TASK_LOCK_BIT  0x02U

static uint8_t ucOwnedByCore[configNUM_CORES] = { 0, 0 };
static uint8_t ucISRLockCount = 0;
static uint8_t ucTaskLockCount = 0;

static void prvFIFOInterruptHandler(void)
{
    /* We must remove the contents (which we don't care about)
     * to clear the IRQ */
    multicore_fifo_drain();
    /* And explicitly clear any other IRQ flags */
    multicore_fifo_clear_irq();

    /* yeald on the current core */
    vPortYield();
}

void vPortStartSchedulerOnCore(void)
{
    /* make sure all interupts are disabled */
    portDISABLE_INTERRUPTS();

    /* add PendSV_IRQn, SVCall_IRQn and SysTick_IRQn handlers */
    NVIC_SetVector(SVCall_IRQn, (uint32_t)vPortSVCHandler);
    NVIC_SetVector(PendSV_IRQn, (uint32_t)vPortPendSVHandler);
    NVIC_SetVector(SIO_IRQ_PROC0_IRQn + portGET_CORE_ID() , (uint32_t)prvFIFOInterruptHandler);

    /* set PendSV_IRQn, SVCall_IRQn and SysTick_IRQn priority */
    NVIC_SetPriority(SVCall_IRQn, configSVCall_INTERRUPT_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, configPendSV_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SIO_IRQ_PROC0_IRQn + portGET_CORE_ID() , configSioProc_INTERRUPT_PRIORITY);

    /* enable the SIO_IRQ_PROC0_IRQn or the SIO_IRQ_PROC1_IRQn */
    NVIC_EnableIRQ(SIO_IRQ_PROC0_IRQn + portGET_CORE_ID() );

    /* start the first task */
    vPortStartFirstTask(portGET_CORE_ID());

    /* Should never get here as the tasks will now be executing!  Call the task
    exit error function to prevent compiler warnings about a static function
    not being called in the case that the application writer overrides this
    functionality. Call vTaskSwitchContext() so link time optimisation does not
    remove the symbol. */
    vTaskSwitchContext(portGET_CORE_ID());
    vPortTaskExitError();
}

/**
 * @brief This is the startup of the scheduler. All necessary HW for the RTOS is
 * prepared and configured.
 *
 */
BaseType_t xPortStartScheduler(void)
{
    /* claim the 2 spinlocks designed for the rtos scheduler */
    spin_lock_claim(PICO_SPINLOCK_ID_OS1);
    spin_lock_claim(PICO_SPINLOCK_ID_OS2);

    /* trigger the second core */
    multicore_launch_core1(vPortStartSchedulerOnCore);

    /* start the timer that generates the tick ISR. Interrupts are disabled
    here already. */
    vPortConfigureSysTick();

    /* start the scheduler on the main core */
    vPortStartSchedulerOnCore();
    return 0;
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

void vPortYieldCore(int32_t core)
{
    (void) core;
    sio_hw->fifo_wr = 0;
}

void vPortGetISRLock()
{
    volatile uint32_t *pxSpinLock = spin_lock_instance(PICO_SPINLOCK_ID_OS1);
    uint32_t ulCoreNum = get_core_num();

    if(__builtin_expect(!*pxSpinLock, 0))
    {
        if(ucOwnedByCore[ulCoreNum] & ISR_LOCK_BIT)
        {
            configASSERT(ucISRLockCount != 255u);
            ucISRLockCount++;
            return;
        }
        while (__builtin_expect(!*pxSpinLock, 0));
    }
    __DMB();
    configASSERT(ucISRLockCount == 0);
    ucISRLockCount = 1;
    ucOwnedByCore[ulCoreNum] |= ISR_LOCK_BIT;
}

void vPortReleaseISRLock()
{
    volatile uint32_t *pxSpinLock = spin_lock_instance(PICO_SPINLOCK_ID_OS1);
    uint32_t ulCoreNum = get_core_num();

    configASSERT((ucOwnedByCore[ulCoreNum] & ISR_LOCK_BIT) != 0 );
    configASSERT(ucISRLockCount != 0);
    if(!--ucISRLockCount)
    {
        ucOwnedByCore[ulCoreNum] &= ~ISR_LOCK_BIT;
        __DMB();
        *pxSpinLock = 1;
    }
}

void vPortGetTaskLock()
{
    volatile uint32_t *pxSpinLock = spin_lock_instance(PICO_SPINLOCK_ID_OS2);
    uint32_t ulCoreNum = get_core_num();

    if( __builtin_expect(!*pxSpinLock, 0))
    {
        if(ucOwnedByCore[ulCoreNum] & TASK_LOCK_BIT)
        {
            configASSERT(ucTaskLockCount != 255u);
            ucTaskLockCount++;
            return;
        }
        while (__builtin_expect(!*pxSpinLock, 0));
    }
    __DMB();
    configASSERT(ucTaskLockCount== 0);
    ucTaskLockCount = 1;
    ucOwnedByCore[ulCoreNum] |= TASK_LOCK_BIT;
}

void vPortReleaseTaskLock()
{
    volatile uint32_t *pxSpinLock = spin_lock_instance(PICO_SPINLOCK_ID_OS2);
    uint32_t ulCoreNum = get_core_num();

    configASSERT((ucOwnedByCore[ulCoreNum] & TASK_LOCK_BIT) != 0);
    configASSERT(ucTaskLockCount != 0);
    if(!--ucTaskLockCount)
    {
        ucOwnedByCore[ulCoreNum] &= ~TASK_LOCK_BIT;
        __DMB();
        *pxSpinLock = 1;
    }
}

uint32_t vPortGetStatsTimerValue(void)
{
    return time_us_64();
}
