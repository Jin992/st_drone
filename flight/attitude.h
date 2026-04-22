#pragma once

#include "sensors/imu.h"

/* Mahony complementary filter state. */
typedef struct {
    float q[4];          /* quaternion [w, x, y, z] */
    float integralFBx;   /* integral feedback x */
    float integralFBy;   /* integral feedback y */
    float integralFBz;   /* integral feedback z */
    float Kp;            /* proportional gain */
    float Ki;            /* integral gain     */
} attitude_t;

/* Initialise filter with default gains (Kp=2.0, Ki=0.005). */
void attitude_init(attitude_t *att);

/* Run one filter step.
   imu — latest IMU sample (accel + gyro in SI units)
   dt  — time step in seconds */
void attitude_update(attitude_t *att, const imu_data_t *imu, float dt);

/* Extract Euler angles in radians from current quaternion state. */
void attitude_get_euler(const attitude_t *att,
                        float *roll, float *pitch, float *yaw);
