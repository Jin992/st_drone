#include "stm32f4xx_hal.h"
#include "hal/hal.h"
#include "flight/pid.h"
#include "flight/mixer.h"
#include "flight/attitude.h"
#include "comms/mavlink_tx.h"

/* ── Control loop state ─────────────────────────────────────────────────── */

static attitude_t g_att;
static pid_t      g_pid_roll;
static pid_t      g_pid_pitch;
static pid_t      g_pid_yaw;

static volatile int  g_loop_running;
static volatile uint32_t g_overruns;

#define DT_S  0.001f  /* 1 kHz → 1 ms */

/* ── 1 kHz control loop (TIM6 ISR) ─────────────────────────────────────── */

void TIM6_DAC_IRQHandler(void)
{
    if (TIM6->SR & TIM_SR_UIF) {
        TIM6->SR &= ~TIM_SR_UIF;

        if (g_loop_running) {
            g_overruns++;
            return;
        }
        g_loop_running = 1;

        imu_data_t  imu;
        float       motor[4];

        hal_imu_read(&imu);
        attitude_update(&g_att, &imu, DT_S);

        float roll, pitch, yaw;
        attitude_get_euler(&g_att, &roll, &pitch, &yaw);

        float cmd_roll  = pid_update(&g_pid_roll,  DT_S, 0.0f - roll);
        float cmd_pitch = pid_update(&g_pid_pitch, DT_S, 0.0f - pitch);
        float cmd_yaw   = pid_update(&g_pid_yaw,   DT_S, 0.0f - yaw);

        mixer_update(0.0f, cmd_roll, cmd_pitch, cmd_yaw, motor);
        hal_pwm_set(motor);

        g_loop_running = 0;
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ── Application entry ──────────────────────────────────────────────────── */

int main(void)
{
    hal_init();

    attitude_init(&g_att);

    g_pid_roll  = (pid_t){ .kp = 0.5f, .ki = 0.01f, .kd = 0.1f, .integral_limit = 1.0f };
    g_pid_pitch = (pid_t){ .kp = 0.5f, .ki = 0.01f, .kd = 0.1f, .integral_limit = 1.0f };
    g_pid_yaw   = (pid_t){ .kp = 1.0f, .ki = 0.0f,  .kd = 0.0f, .integral_limit = 1.0f };

    uint32_t last_hb = 0;

    for (;;) {
        /* Telemetry heartbeat at 1 Hz. */
        uint32_t now = HAL_GetTick();
        if (now - last_hb >= 1000u) {
            mavlink_send_heartbeat();
            mavlink_send_attitude(&g_att, now);
            last_hb = now;
        }
    }
}
