#include "unity.h"
#include "flight/mixer.h"

void setUp(void)    {}
void tearDown(void) {}

static void test_pure_throttle_equal_outputs(void)
{
    float out[4];
    mixer_update(0.5f, 0.0f, 0.0f, 0.0f, out);
    for (int i = 0; i < 4; i++)
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.5f, out[i]);
}

static void test_roll_differentiates_sides(void)
{
    float out[4];
    mixer_update(0.5f, 0.1f, 0.0f, 0.0f, out);
    /* Right motors (M1 front-right, M3 rear-right) increase. */
    TEST_ASSERT_GREATER_THAN_FLOAT(0.5f, out[1]);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.5f, out[3]);
    /* Left motors (M0 front-left, M2 rear-left) decrease. */
    TEST_ASSERT_LESS_THAN_FLOAT(0.5f, out[0]);
    TEST_ASSERT_LESS_THAN_FLOAT(0.5f, out[2]);
}

static void test_zero_inputs_zero_output(void)
{
    float out[4];
    mixer_update(0.0f, 0.0f, 0.0f, 0.0f, out);
    for (int i = 0; i < 4; i++)
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, out[i]);
}

static void test_output_clamped_at_one(void)
{
    float out[4];
    mixer_update(1.0f, 1.0f, 1.0f, 1.0f, out);
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_LESS_OR_EQUAL_FLOAT(1.0f, out[i]);
        TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, out[i]);
    }
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_pure_throttle_equal_outputs);
    RUN_TEST(test_roll_differentiates_sides);
    RUN_TEST(test_zero_inputs_zero_output);
    RUN_TEST(test_output_clamped_at_one);
    return UNITY_END();
}
