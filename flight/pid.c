#include "flight/pid.h"

void pid_reset(pid_t *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float pid_update(pid_t *pid, float dt, float error)
{
    float p = pid->kp * error;

    pid->integral += error * dt;
    if (pid->integral >  pid->integral_limit) pid->integral =  pid->integral_limit;
    if (pid->integral < -pid->integral_limit) pid->integral = -pid->integral_limit;
    float i = pid->ki * pid->integral;

    float d = (dt > 0.0f) ? pid->kd * (error - pid->prev_error) / dt : 0.0f;
    pid->prev_error = error;

    return p + i + d;
}
