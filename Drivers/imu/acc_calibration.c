

#include <stdint.h>
#include "system.h"
#include "acc_calibration.h"

#define FLT_MAX 1000
#define LOG_ERROR 1
#define LOG_INFO 1
#define LOG_WARNING 1
#define MIN_SAMPLES 100


// typedef struct {
static vec3_t acc_bias;
//     vec3_t noise_level;
//     uint32_t samples_used;
//     float temperature_celsius;
// } acc_calibration_result_t;

// acc_calibration.c
static ACC_CALIBRATE_STATE state = ACC_CALIBRATE_NONE;
// static calibration_callback_t callback = NULL;

#include "stdio.h"
#define CALIB_LOG(d, str) do{ printf str; } while(0)

void acc_calibration_init(void){
    acc_bias = vec3_fill(0);
    state = ACC_CALIBRATE_NONE;
    acc_calibration_update((vec3_t){0,0,0}, true);
}

// int imu_is_static(void){return 1;}
void acc_calibration_update(vec3_t acc, bool force_reset) {
    static struct {
        vec3_t sum;
        // vec3_t min;
        // vec3_t max;
        // float noise_level;
        // vec3_t noise;
        uint32_t count;
        // uint32_t start_time;
        // bool has_movement;
    } session = {.count = 0};//, .noise = {FLT_MAX,FLT_MAX,FLT_MAX}};
    
    if (force_reset) {
        session.count = 0;
        state = ACC_CALIBRATE_NONE; 
        // memset(&session, 0, sizeof(session));
        // result.samples_used = 0;
        // session.count = 0;
        return;
    }
    
    if (session.count == 0){//(session.start_time == 0) {
        // session.start_time = system_getTime_ms();
        // session.min = vec3_fill(FLT_MAX);
        // session.max = vec3_fill(-FLT_MAX);
        session.sum = vec3_fill(0);
        CALIB_LOG(LOG_INFO, ("Calibration session started\n\r"));
    }
    
    // uint32_t elapsed = system_getTime_ms() - session.start_time;
    
    if (session.count++ < CALIB_DURATION_CNT) {

        
        session.sum = vec3_add(session.sum, acc);

        // Прогресс-бар
        if (session.count % (CALIB_DURATION_CNT/10) == 0) {
            // CALIB_LOG(LOG_INFO, ("Collecting: %u%% complete\n\r", session.count  * 100 / CALIB_DURATION_CNT));
        }
    } 
    //else { // Finalize once
    if (session.count >= CALIB_DURATION_CNT) {
        // result.samples_used = session.count;
        vec3_t avg = vec3_div_const(session.sum, session.count);
        // vec3_t noise = vec3_sub(session.max, session.min);

        // float level = (noise.x + noise.y + noise.z);
        // session.noise_level = vec3_sub(session.max, session.min);
        
        // bool success = (session.count >= MIN_SAMPLES && !session.has_movement);
        
        //float * f_ptr = &avg.x;
        // v3 avg = vec3_div_const(acc_ctx.sum, session.count);
        int axis = (fabsf(avg.x) > fabsf(avg.y) && fabsf(avg.x) > fabsf(avg.z)) ? 0 :
                   (fabsf(avg.y) > fabsf(avg.z)) ? 1 : 2;
       if((fabsf(avg.x) > fabsf(avg.y) && fabsf(avg.x) > fabsf(avg.z))){
         avg.x -= (avg.x < 0) ? -1.0f : 1.0f;
       }
        else if(fabsf(avg.y) > fabsf(avg.z)){
          avg.y -= (avg.y < 0) ? -1.0f : 1.0f;
        }
        else {
          avg.z -= (avg.z < 0) ? -1.0f : 1.0f;
        }
        
        //f_ptr[axis] -= (f_ptr[axis] < 0) ? -1.0f : 1.0f;
        acc_bias = vec3_sub(acc_bias, avg);
        CALIB_LOG(LOG_INFO, ("Calibration pass: bias=(%.2f,%.2f,%.2f)\n\r", 
                      acc_bias.x, acc_bias.y, acc_bias.z));
        session.count = 0; // завершено
        state = ACC_CALIBRATE_FINISH;
    }

}



vec3_t acc_calibration_get_bias(void){
    return acc_bias;
}
int acc_calibration_get_status(void){
    return (int)state;
}


