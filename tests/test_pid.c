#include "unity.h"
#include "flight/pid.h"
#include <string.h>

void setUp(void)    {}
void tearDown(void) {}

static void test_proportional_step(void)
{
    pid_t pid = { .kp = 1.0f, .ki = 0.0f, .kd = 0.0f, .integral_limit = 1.0f };
    float out = pid_update(&pid, 0.001f, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, out);
}

static void test_pure_integral(void)
{
    pid_t pid = { .kp = 0.0f, .ki = 1.0f, .kd = 0.0f, .integral_limit = 10.0f };
    float out = pid_update(&pid, 1.0f, 2.0f);  /* integral = 2.0 × 1.0 = 2.0 */
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 2.0f, out);
}

static void test_integrator_clamp(void)
{
    pid_t pid = { .kp = 0.0f, .ki = 1.0f, .kd = 0.0f, .integral_limit = 5.0f };
    /* Drive integral far past limit. */
    for (int i = 0; i < 100; i++)
        pid_update(&pid, 0.1f, 10.0f);
    /* Output must be clamped at ki × limit = 1.0 × 5.0 = 5.0 */
    float out = pid_update(&pid, 0.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 5.0f, out);
}

static void test_derivative(void)
{
    pid_t pid = { .kp = 0.0f, .ki = 0.0f, .kd = 1.0f, .integral_limit = 1.0f };
    pid_update(&pid, 0.001f, 0.0f);           /* first call: prev_error = 0 */
    float out = pid_update(&pid, 0.001f, 1.0f); /* d = (1-0)/0.001 = 1000 */
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 1000.0f, out);
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_proportional_step);
    RUN_TEST(test_pure_integral);
    RUN_TEST(test_integrator_clamp);
    RUN_TEST(test_derivative);
    return UNITY_END();
}
