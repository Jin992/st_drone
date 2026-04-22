#pragma once

typedef struct {
    float pressure_pa;  /* Pascal    */
    float temp_c;       /* degrees C */
    float altitude_m;   /* metres above sea level (derived) */
} baro_data_t;
