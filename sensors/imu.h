#pragma once

typedef struct {
    float ax, ay, az;   /* accelerometer m/s^2 */
    float gx, gy, gz;   /* gyroscope rad/s    */
    float mx, my, mz;   /* magnetometer uT    */
    float temperature;  /* degrees C          */
} imu_data_t;
