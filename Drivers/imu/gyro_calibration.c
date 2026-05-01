

#include <stdint.h>
#include "vector_math.h"
#include "system.h"
#include "gyro_calibration.h"

#define FLT_MAX 1000
#define LOG_ERROR 1
#define LOG_INFO 1
#define LOG_WARNING 1
#define CALIB_DURATION_MS 1000
#define MIN_SAMPLES 100

vec3_t gyro = {0,0,0};

#include "stdio.h"
#define CALIB_LOG(d, str) do{ printf str; } while(0)

void gyro_calibration_init(calibration_callback_t on_complete){}

int imu_is_static(void){return 1;}
void gyro_calibration_update(bool force_reset) {
    static struct {
        vec3_t sum;
        vec3_t min;
        vec3_t max;
        uint32_t count;
        uint32_t start_time;
        bool has_movement;
    } session = {0};
    
    if (force_reset) {
        memset(&session, 0, sizeof(session));
        result.samples_used = 0;
        return;
    }
    
    if (session.start_time == 0) {
        session.start_time = system_getTime_ms();
        session.min = vec3_fill(FLT_MAX);
        session.max = vec3_fill(-FLT_MAX);
        CALIB_LOG(LOG_INFO, ("Calibration session started\n\r"));
    }
    
    uint32_t elapsed = system_getTime_ms() - session.start_time;
    
    if (elapsed < CALIB_DURATION_MS) {
        if (!imu_is_static()) {
            session.has_movement = true;
            CALIB_LOG(LOG_WARNING, ("Movement detected, resetting session\n\r"));
            memset(&session, 0, sizeof(session));
            return;
        }
        
        session.sum = vec3_add(session.sum, gyro);
        session.min = vec3_min(session.min, gyro);
        session.max = vec3_max(session.max, gyro);
        session.count++;
        
        // Прогресс-бар
        if (elapsed % 500 == 0) {
            CALIB_LOG(LOG_INFO, ("Collecting: %u%% complete\n\r", elapsed * 100 / CALIB_DURATION_MS));
        }
    } 
    else if (result.samples_used == 0) { // Finalize once
        result.samples_used = session.count;
        result.bias = vec3_div_const(session.sum, session.count);
        result.noise_level = vec3_sub(session.max, session.min);
        
        bool success = (session.count >= MIN_SAMPLES && !session.has_movement);
        
        if (success && callback) {
            CALIB_LOG(LOG_INFO, ("Calibration successful: bias=(%.2f,%.2f,%.2f)\n\r", 
                      result.bias.x, result.bias.y, result.bias.z));
            callback(&result);
        } else {
            CALIB_LOG(LOG_ERROR, ("Calibration failed: samples=%u, movement=%d\n\r", 
                      session.count, session.has_movement));
        }
    }
}



vec3_t gyro_calibration_get_bias(void){
    return result.bias;
}
int gyro_calibration_get_status(void){
    return (result.samples_used) ? 1 : 0;
}

