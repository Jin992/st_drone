#include "flight/mixer.h"

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

void mixer_update(float throttle, float roll, float pitch, float yaw,
                  float out[4])
{
    out[0] = throttle - roll + pitch + yaw;  /* front-left  CW  */
    out[1] = throttle + roll + pitch - yaw;  /* front-right CCW */
    out[2] = throttle - roll - pitch - yaw;  /* rear-left   CCW */
    out[3] = throttle + roll - pitch + yaw;  /* rear-right  CW  */

    for (int i = 0; i < 4; i++)
        out[i] = clampf(out[i], 0.0f, 1.0f);
}
