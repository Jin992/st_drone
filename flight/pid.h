#pragma once

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float integral_limit;  /* clamp |integral| to this value */
    float prev_error;
} pid_t;

/* Reset integrator and derivative state. */
void  pid_reset(pid_t *pid);

/* Run one PID step.
   dt    — time delta in seconds since last call
   error — setpoint minus measurement
   Returns the control output. */
float pid_update(pid_t *pid, float dt, float error);
