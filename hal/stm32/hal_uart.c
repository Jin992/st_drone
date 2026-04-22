#include "hal/hal.h"
#include "stm32f4xx_hal.h"

/* USART2 on PA2 (TX) / PA3 (RX) at 921600 baud — connected to ESP32-C3. */

static UART_HandleTypeDef s_uart;

static void uart_gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {
        .Pin       = GPIO_PIN_2 | GPIO_PIN_3,
        .Mode      = GPIO_MODE_AF_PP,
        .Pull      = GPIO_NOPULL,
        .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF7_USART2,
    };
    HAL_GPIO_Init(GPIOA, &gpio);
}

void hal_uart_bus_init(void)
{
    uart_gpio_init();
    s_uart.Instance        = USART2;
    s_uart.Init.BaudRate   = 921600;
    s_uart.Init.WordLength = UART_WORDLENGTH_8B;
    s_uart.Init.StopBits   = UART_STOPBITS_1;
    s_uart.Init.Parity     = UART_PARITY_NONE;
    s_uart.Init.Mode       = UART_MODE_TX_RX;
    s_uart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    HAL_UART_Init(&s_uart);
}

size_t hal_uart_send(const uint8_t *buf, size_t len)
{
    if (HAL_UART_Transmit(&s_uart, buf, (uint16_t)len, 10) == HAL_OK)
        return len;
    return 0;
}

size_t hal_uart_recv(uint8_t *buf, size_t len)
{
    if (HAL_UART_Receive(&s_uart, buf, (uint16_t)len, 1) == HAL_OK)
        return len;
    return 0;
}
