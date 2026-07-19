#include "ti_msp_dl_config.h"
#include "encoder_user.h"
#include "lengke_gyro.h"
#include "maix_cam.h"

#define UART_CLEAR_MASK                                                        \
    (DL_UART_INTERRUPT_RX | DL_UART_INTERRUPT_TX |                            \
        DL_UART_INTERRUPT_RX_TIMEOUT_ERROR | DL_UART_INTERRUPT_OVERRUN_ERROR | \
        DL_UART_INTERRUPT_BREAK_ERROR | DL_UART_INTERRUPT_PARITY_ERROR |       \
        DL_UART_INTERRUPT_FRAMING_ERROR | DL_UART_INTERRUPT_NOISE_ERROR)

void CAM_UART_INST_IRQHandler(void)
{
    DL_UART_IIDX interrupt_index = DL_UART_getPendingInterrupt(CAM_UART_INST);

    if (interrupt_index == DL_UART_IIDX_RX) {
        camera_irq_handler(interrupt_index);
    } else {
        DL_UART_clearInterruptStatus(CAM_UART_INST, UART_CLEAR_MASK);
    }
}

void GYRO_UART_INST_IRQHandler(void)
{
    lengke_gyro_uart_irq_handler();
}

void GROUP1_IRQHandler(void)
{
    if (DL_Interrupt_getStatusGroup(DL_INTERRUPT_GROUP_1, PORTA_INT_IIDX) ||
        DL_Interrupt_getStatusGroup(DL_INTERRUPT_GROUP_1, PORTB_INT_IIDX)) {
        encoder_group1_irq_handler();
    }
}
