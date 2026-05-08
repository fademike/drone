// acc_calibration.h

#include <stdbool.h>

#include "vector_math.h"

#define CALIB_DURATION_CNT 1000

typedef enum {
    ACC_CALIBRATE_NONE = 0,
    ACC_CALIBRATE_PROCESS = 1,
    ACC_CALIBRATE_FINISH = 2

} ACC_CALIBRATE_STATE;

// typedef struct {
//     vec3_t bias;
//     vec3_t noise_level;
//     uint32_t samples_used;
//     float temperature_celsius;
// } acc_calibration_result_t;

// typedef void (*calibration_callback_t)(const acc_calibration_result_t* result);

void acc_calibration_init(void);
void acc_calibration_update(vec3_t gyro, bool force_reset);
bool acc_calibration_is_completed(void);

vec3_t acc_calibration_get_bias(void);
int acc_calibration_get_status(void);

