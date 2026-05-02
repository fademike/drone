

#include <stdint.h>
#include "vector_math.h"
#include "system.h"
#include "gyro_calibration.h"

#define FLT_MAX 1000
#define LOG_ERROR 1
#define LOG_INFO 1
#define LOG_WARNING 1
#define MIN_SAMPLES 100


// typedef struct {
static vec3_t gyro_bias;
//     vec3_t noise_level;
//     uint32_t samples_used;
//     float temperature_celsius;
// } gyro_calibration_result_t;

// gyro_calibration.c
static GYRO_CALIBRATE_STATE state = GYRO_CALIBRATE_NONE;
// static calibration_callback_t callback = NULL;

#include "stdio.h"
#define CALIB_LOG(d, str) do{ printf str; } while(0)

void gyro_calibration_init(void){
    gyro_bias = vec3_fill(0);
    gyro_calibration_update((vec3_t){0,0,0}, true);
}

// int imu_is_static(void){return 1;}
void gyro_calibration_update(vec3_t gyro, bool force_reset) {
    static struct {
        vec3_t sum;
        vec3_t min;
        vec3_t max;
        // float noise_level;
        vec3_t noise;
        uint32_t count;
        // uint32_t start_time;
        // bool has_movement;
    } session = {.count = 0, .noise = {FLT_MAX,FLT_MAX,FLT_MAX}};
    
    if (force_reset) {
        session.count = 0;
        // memset(&session, 0, sizeof(session));
        // result.samples_used = 0;
        // session.count = 0;
        return;
    }
    
    if (session.count == 0){//(session.start_time == 0) {
        // session.start_time = system_getTime_ms();
        session.min = vec3_fill(FLT_MAX);
        session.max = vec3_fill(-FLT_MAX);
        session.sum = vec3_fill(0);
        CALIB_LOG(LOG_INFO, ("Calibration session started\n\r"));
    }
    
    // uint32_t elapsed = system_getTime_ms() - session.start_time;
    
    if (session.count++ < CALIB_DURATION_CNT) {
        // if (!imu_is_static()) {
            // session.has_movement = true;
            // CALIB_LOG(LOG_WARNING, ("Movement detected, resetting session\n\r"));
            // memset(&session, 0, sizeof(session));
            // return;
        // }
        
        session.sum = vec3_add(session.sum, gyro);
        session.min = vec3_min(session.min, gyro);
        session.max = vec3_max(session.max, gyro);
        
        // Прогресс-бар
        if (session.count % (CALIB_DURATION_CNT/10) == 0) {
            // CALIB_LOG(LOG_INFO, ("Collecting: %u%% complete\n\r", session.count  * 100 / CALIB_DURATION_CNT));
        }
    } 
    //else { // Finalize once
    if (session.count == CALIB_DURATION_CNT) {
        // result.samples_used = session.count;
        vec3_t bias = vec3_div_const(session.sum, session.count);
        vec3_t noise = vec3_sub(session.max, session.min);

        // float level = (noise.x + noise.y + noise.z);
        // session.noise_level = vec3_sub(session.max, session.min);
        
        // bool success = (session.count >= MIN_SAMPLES && !session.has_movement);
        
        // if (session.noise_level > level){
        if ((noise.x <= session.noise.x) && (noise.y <= session.noise.y) && (noise.z <= session.noise.z)){
            session.noise = noise;
            gyro_bias = bias;
            CALIB_LOG(LOG_INFO, ("Calibration successful: bias=(%.2f,%.2f,%.2f)\n\r", 
                      bias.x, bias.y, bias.z));
            // callback(&result);
        } else {
            CALIB_LOG(LOG_INFO, ("Calibration pass: bias=(%.2f,%.2f,%.2f)\n\r", 
                      bias.x, bias.y, bias.z));
        }

        session.count = 0;
    }
}



vec3_t gyro_calibration_get_bias(void){
    return gyro_bias;
}
int gyro_calibration_get_status(void){
    return (int)state;
}

