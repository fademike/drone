// gyro_calibration.h

#include <stdbool.h>

#define CALIB_DURATION_CNT 1000

typedef enum {
    GYRO_CALIBRATE_NONE = 0,
    GYRO_CALIBRATE_PROCESS = 0,
    GYRO_CALIBRATE_FINISH = 0

} GYRO_CALIBRATE_STATE;

// typedef struct {
//     vec3_t bias;
//     vec3_t noise_level;
//     uint32_t samples_used;
//     float temperature_celsius;
// } gyro_calibration_result_t;

// typedef void (*calibration_callback_t)(const gyro_calibration_result_t* result);

void gyro_calibration_init(void);
void gyro_calibration_update(vec3_t gyro, bool force_reset);
bool gyro_calibration_is_completed(void);

vec3_t gyro_calibration_get_bias(void);
int gyro_calibration_get_status(void);

