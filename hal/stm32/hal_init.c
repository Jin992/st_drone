#include "stm32f4xx_hal.h"

/* Configure SYSCLK to 168 MHz using HSI + PLL.
   HSI 16 MHz → PLL → 168 MHz (VCO 336 MHz, /2).
   APB1 = 42 MHz, APB2 = 84 MHz.
   TIM6 clock = 84 MHz (APB1 × 2 because APB1 prescaler ≠ 1). */
static void clock_config(void)
{
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState       = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM       = 8;
    osc.PLL.PLLN       = 168;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 7;
    HAL_RCC_OscConfig(&osc);

    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                         RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5);
}

static void tim6_init(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* TIM6 clock = 84 MHz. Prescaler 83 → 1 MHz tick. Period 999 → 1 kHz. */
    TIM6->PSC = 83;
    TIM6->ARR = 999;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1  |= TIM_CR1_CEN;

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

extern void hal_uart_bus_init(void);

void hal_init(void)
{
    HAL_Init();
    clock_config();
    hal_uart_bus_init();
    tim6_init();
}
