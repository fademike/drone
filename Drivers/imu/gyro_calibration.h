// gyro_calibration.h

#include <stdbool.h>

typedef struct {
    vec3_t bias;
    vec3_t noise_level;
    uint32_t samples_used;
    float temperature_celsius;
} gyro_calibration_result_t;

typedef void (*calibration_callback_t)(const gyro_calibration_result_t* result);

void gyro_calibration_init(calibration_callback_t on_complete);
void gyro_calibration_update(bool force_reset);
bool gyro_calibration_is_completed(void);

vec3_t gyro_calibration_get_bias(void);
int gyro_calibration_get_status(void);

// gyro_calibration.c
static gyro_calibration_result_t result = {0};
static calibration_callback_t callback = NULL;
