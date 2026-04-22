#include "flight/attitude.h"
#include <math.h>
#include <string.h>

void attitude_init(attitude_t *att)
{
    memset(att, 0, sizeof(*att));
    att->q[0] = 1.0f;   /* identity quaternion */
    att->Kp   = 2.0f;
    att->Ki   = 0.005f;
}

/* Fast inverse square root. */
static float inv_sqrt(float x)
{
    return 1.0f / sqrtf(x);
}

void attitude_update(attitude_t *att, const imu_data_t *imu, float dt)
{
    float q0 = att->q[0], q1 = att->q[1], q2 = att->q[2], q3 = att->q[3];
    float gx = imu->gx, gy = imu->gy, gz = imu->gz;
    float ax = imu->ax, ay = imu->ay, az = imu->az;

    /* Normalise accelerometer. */
    float norm = inv_sqrt(ax*ax + ay*ay + az*az);
    if (!isfinite(norm)) return;
    ax *= norm; ay *= norm; az *= norm;

    /* Estimated direction of gravity from quaternion. */
    float vx = 2.0f*(q1*q3 - q0*q2);
    float vy = 2.0f*(q0*q1 + q2*q3);
    float vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

    /* Error: cross product of estimated and measured gravity. */
    float ex = ay*vz - az*vy;
    float ey = az*vx - ax*vz;
    float ez = ax*vy - ay*vx;

    /* Integral feedback. */
    att->integralFBx += att->Ki * ex * dt;
    att->integralFBy += att->Ki * ey * dt;
    att->integralFBz += att->Ki * ez * dt;

    /* Apply feedback to gyro. */
    gx += att->Kp * ex + att->integralFBx;
    gy += att->Kp * ey + att->integralFBy;
    gz += att->Kp * ez + att->integralFBz;

    /* Integrate quaternion rate. */
    float half_dt = 0.5f * dt;
    float dq0 = (-q1*gx - q2*gy - q3*gz) * half_dt;
    float dq1 = ( q0*gx + q2*gz - q3*gy) * half_dt;
    float dq2 = ( q0*gy - q1*gz + q3*gx) * half_dt;
    float dq3 = ( q0*gz + q1*gy - q2*gx) * half_dt;

    q0 += dq0; q1 += dq1; q2 += dq2; q3 += dq3;

    /* Normalise quaternion. */
    norm = inv_sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    att->q[0] = q0 * norm;
    att->q[1] = q1 * norm;
    att->q[2] = q2 * norm;
    att->q[3] = q3 * norm;
}

void attitude_get_euler(const attitude_t *att,
                        float *roll, float *pitch, float *yaw)
{
    float q0 = att->q[0], q1 = att->q[1], q2 = att->q[2], q3 = att->q[3];
    *roll  = atan2f(2.0f*(q0*q1 + q2*q3), 1.0f - 2.0f*(q1*q1 + q2*q2));
    *pitch = asinf( 2.0f*(q0*q2 - q3*q1));
    *yaw   = atan2f(2.0f*(q0*q3 + q1*q2), 1.0f - 2.0f*(q2*q2 + q3*q3));
}
