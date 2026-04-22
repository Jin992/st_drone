#pragma once

/* Quad-X motor layout (top view):
 *
 *   M0(CW)  ←front→  M1(CCW)
 *     \                /
 *      \              /
 *   M2(CCW)        M3(CW)
 *
 * M0 front-left, M1 front-right, M2 rear-left, M3 rear-right.
 * Positive roll → right side up (M1, M3 increase; M0, M2 decrease).
 * Positive pitch → nose up (M0, M1 increase; M2, M3 decrease).
 * Positive yaw → nose right (CCW motors M1, M2 increase).
 *
 * All inputs and outputs are in [0.0, 1.0].
 * Outputs are clamped to [0.0, 1.0]. */
void mixer_update(float throttle, float roll, float pitch, float yaw,
                  float out[4]);
