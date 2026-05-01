

#include "stm32f1xx.h"

#include <math.h>
#include "MPU9250.h"
#include "MahonyAHRS.h"
#include "system.h"
#include "params.h"
#include "MotorControl.h"
#include "main.h"

#include "imu.h"

#include "VL53L0X.h"

//statInfo_t_VL53L0X distanceStr;

#include "log.h"

#define IMU_DEBUG 1

void imu_autoCalibrateByNoize(int stop);
void imu_accCalibrate(void);


struct imuAngle_struct {
	float pitch;
	float roll;
	float yaw;
};
struct axis_struct {
	float x;
	float y;
	float z;
};

typedef union {
	struct axis_struct axis;
	struct imuAngle_struct angle;
	float value[3];
} vector;

vector vector_setConst(vector axes, float Value);
vector vector_muxConst(vector axes, const float Value);
vector vector_divConst(vector axes, const float Value);
vector vector_muxVector(vector axes, const vector axes2);
vector vector_addVector(vector axes, vector axes2);
vector vector_removeVector(vector axes, vector axes2);
vector vector_rearranging(vector axes, int * val);

vector imuAngle = {.angle.pitch=0,.angle.roll=0,.angle.yaw=0};

float imu_getPitch(void){return imuAngle.angle.pitch;}
float imu_getRoll(void){return imuAngle.angle.roll;}
float imu_getYaw(void){return imuAngle.angle.yaw;}
int imu_getAlt(void){
	int alt = 0;//get_altitude();
	int alt_max = (int)params_GetParamValue(PARAM_ALT_MAX);
	if (alt_max > 0) {
		alt = get_altitude();
		alt *= 1000.0f/(float)alt_max;	//convert 
	}
	return alt;
}

vector gyro = {.axis.x=0,.axis.y=0,.axis.z=0};
vector gyro_offs = {.axis.x=0,.axis.y=0,.axis.z=0};

vector acc = {.axis.x=0,.axis.y=0,.axis.z=0};
vector acc_offs = {.axis.x=0,.axis.y=0,.axis.z=0};

static int status =-1;
static int statusCalibrateGyro = -1;
static int statusCalibrateAcc = 0;

int imu_getStatus(void){	// status < 0 - fail imu init; 0 - ok
	return status;
}
int imu_GyroCalibrate_getStatus(void){	// -2 fail, -1 process calib, 0 ok (in the process of recalibration)
	return statusCalibrateGyro;
}
void imu_GyroCalibrate_run(void){
	statusCalibrateGyro = -2;
}
int imu_AccCalibrate_getStatus(void){	// -1 need to calibrate, 0 finish of calibration
	return statusCalibrateAcc;
}
void imu_AccCalibrate_run(void){
	statusCalibrateAcc = -1;
}
void imu_AccOffset_get(float * ox, float * oy, float * oz){
	*ox = acc_offs.axis.x;
	*oy = acc_offs.axis.y;
	*oz = acc_offs.axis.z;
}
void imu_AccOffset_set(float ox, float oy, float oz){
	Printf("acc set %d, %d, %d\n\r", (int)(ox*1000.0f), (int)(oy*1000.0f), (int)(oz*1000.0f));
	acc_offs.axis.x = ox;
	acc_offs.axis.y = oy;
	acc_offs.axis.z = oz;
}
int imu_init(void){
	status = MPU_Init();
	Printf("imu init %d\n\r", status);
	return status;
}

/*
void UpdateOrientation(int * dat_r, float * dat_a_sig, float * dat_g_sig){
	
	static float orientation = 0;
	float param_orientation = params_GetParamValue(PARAM_ORIENTATION);

	// 0, 24 - norm
	// 60 - up
	// dataset: 6 bit: sig_third,sig_second,sig_first,second_axis,[first_axis:2]
	// first_axis <= x=0, y=1, z=2;
	// second_axis = the remaining axes: if first_axit = x(0) => y=0,z=1; if first_axit = z(2) => x=0,y=1...
	// sig_first, sig_second, sig_third = sig of choised axis; 0- normal; 1 - sig minus
	if (orientation != param_orientation){
		orientation = param_orientation;
		if ((0 <= orientation) && (orientation <= 0xFFFF)){
			int i_o = (int)param_orientation;
			int v1 = i_o&0x3;
			if (v1>=AXIS_Z) v1 = AXIS_Z;
			dat_r[0] = v1;	// set first axis
			int v2 = (i_o>>2)&0x1;
			if ((v1 == AXIS_X) && (v2 == AXIS_Y)) v2++;
			if (v1 == v2) v2++;
			int v3 = 0;
			if ((v1 == v3) || (v2 == v3)) v3++;
			if ((v1 == v3) || (v2 == v3)) v3++;

			dat_r[0] = v1;
			dat_r[1] = v2;
			dat_r[2] = v3;

			int sig = (i_o>>4)&0x7;
			int s=0;
			for (s=0;s<3;s++) dat_a_sig[s] = 1.0;
			for (s=0;s<3;s++) if (sig&(0x1<<s)) dat_a_sig[s] = -1.0;
			sig = (i_o>>8)&0x7;
			for (s=0;s<3;s++) dat_g_sig[s] = 1.0;
			for (s=0;s<3;s++) if (sig&(0x1<<s)) dat_g_sig[s] = -1.0;

			Printf("set new (%d). %d, %d, %d sig a %d %d, %d sig g %d %d, %d\n\r", i_o, v1,v2,v3,
					(int)dat_a_sig[0],(int)dat_a_sig[1],(int)dat_a_sig[2], (int)dat_g_sig[0],(int)dat_g_sig[1],(int)dat_g_sig[2]);
		}
	}
}
*/

// Предопределенные комбинации осей
static const int axis_permutations[6][3] = {
    {AXIS_X, AXIS_Y, AXIS_Z},  // 0: XYZ
    {AXIS_X, AXIS_Z, AXIS_Y},  // 1: XZY
    {AXIS_Y, AXIS_X, AXIS_Z},  // 2: YXZ
    {AXIS_Y, AXIS_Z, AXIS_X},  // 3: YZX
    {AXIS_Z, AXIS_X, AXIS_Y},  // 4: ZXY
    {AXIS_Z, AXIS_Y, AXIS_X}   // 5: ZYX
};

void UpdateOrientation(int *dat_r, float *dat_a_sig, float *dat_g_sig) {
    static float orientation = 0;
    float param_orientation = params_GetParamValue(PARAM_ORIENTATION);
    
    if (orientation == param_orientation) return;
    orientation = param_orientation;
    
    if (orientation < 0 || orientation > 0xFFFF) return;
    
    int config = (int)orientation;
    
    // 2 бита - первая ось, 1 бит - порядок оставшихся
    int first_axis = config & 0x3;
    int swap_remaining = (config >> 2) & 0x1;
    
    // Выбираем перестановку
    int perm_index = first_axis * 2 + swap_remaining;
    dat_r[0] = axis_permutations[perm_index][0];
    dat_r[1] = axis_permutations[perm_index][1];
    dat_r[2] = axis_permutations[perm_index][2];
    
    // Знаки (прозрачно)
    for (int i = 0; i < 3; i++) {
        dat_a_sig[i] = ((config >> (4 + i)) & 1) ? -1.0f : 1.0f;
        dat_g_sig[i] = ((config >> (8 + i)) & 1) ? -1.0f : 1.0f;
    }
}


void imu_loop(void){
	static uint64_t l_time_us = 0;
	static int sensor_exist = 0;
	// MY_DEBUGF(IMU_DEBUG, ("imu: loop\n\r"));
#ifdef CMAKE_CROSSCOMPILING
	if ((status != 0 ) && ((sensor_exist == 1) || (IMU_MUST_BE))){imu_init(); while(1){}; return ;} //wait wdt if no sensor
#endif
	if (status == 0) sensor_exist = 1;
	
	vector EstA = {0,0,0}, EstG = {0,0,0};
	short t;
#ifdef CMAKE_CROSSCOMPILING
	if (status == 0) if (MPU_GetDataFloat(EstA.value, EstG.value, &t) != HAL_OK ) {imu_init(); return;}
#else
	EstA.axis.z = 1.0f;
	EstG.angle.yaw = 0.2f;
#endif

	uint64_t c_time_us = system_getTime_us();
	if (l_time_us  == 0 ) {l_time_us = c_time_us; return;} //first cycle. when haven't l_time.

	// Preparing gyro data..
#ifdef CMAKE_CROSSCOMPILING
	gyro = vector_addVector(vector_muxConst(EstG, lsb2dps_gyro), gyro_offs);
	imu_autoCalibrateByNoize(MotorControl_getState() == MOTOR_STATUS_LAUNCHED);	// calibrate gyro, if motor not launched, if necessary

	// Preparing acc data..	 convert "raw" data to "g" data and add offset
	acc = vector_addVector(vector_muxConst(EstA, lsb2g_acc), acc_offs);
#else
	gyro = EstG;
	acc = EstA;
#endif
	if (MotorControl_getState() != MOTOR_STATUS_LAUNCHED) imu_accCalibrate(); 	// calibrate acc, if motor not launched, if necessary

	// TODO heavy while launched motors
	// #define sq(x) ((x)*(x))
	// 
	// if (fabs(sqrt(sq(acc.axis.x)+sq(acc.axis.y)+sq(acc.axis.z))-1.0f)>0.1){	 //if heavy (more than 1g +-0.1g)
	// 	HAL_GPIO_WritePin(PIN_TEST_GPIO_Port, PIN_TEST_Pin, GPIO_PIN_SET);
	// 	params_GetParamValue(PARAM_FL_KP_ARM);
	// }
	// else {
	// 	HAL_GPIO_WritePin(PIN_TEST_GPIO_Port, PIN_TEST_Pin, GPIO_PIN_RESET);
	// 	params_GetParamValue(PARAM_FL_KP);
	// }

	float dt = (c_time_us - l_time_us);
	float ki = params_GetParamValue(PARAM_P_KI);
	float kp;

	l_time_us = c_time_us;
	if (MotorControl_getState() != MOTOR_STATUS_LAUNCHED) kp = params_GetParamValue(PARAM_P_KP);
	else kp = params_GetParamValue(PARAM_P_KP_ARM);

	MahonyUpdateVariables(dt*1e-6f, kp, ki);

	// updating params for imu orientation
	static int dat_r[3] = {AXIS_X,AXIS_Y,AXIS_Z};	// orientation imu for rearranging
	static vector dat_a_sig = {.axis.x=1.0f,.axis.y=1.0f,.axis.z=1.0f};
	static vector dat_g_sig = {.axis.x=1.0f,.axis.y=1.0f,.axis.z=1.0f};
	UpdateOrientation(dat_r, dat_a_sig.value, dat_g_sig.value);

	// convert deg to rad, change sig and rearranging values from orientation
	vector g = vector_rearranging(vector_muxVector(vector_muxConst(gyro, M_PI/180.0f), dat_g_sig), dat_r);
	// change sig and rearranging values from orientation
	vector a = vector_rearranging(vector_muxVector(acc, dat_a_sig), dat_r);

	vector AngleTmp;

	MahonyAHRSupdateIMU(g.axis.x, g.axis.y, g.axis.z, a.axis.x, a.axis.y, a.axis.z);
	MahonyGetAngles(&AngleTmp.angle.pitch, &AngleTmp.angle.roll, &AngleTmp.angle.yaw);

	imuAngle = vector_muxConst(AngleTmp, 180.0/M_PI); // convert rad to deg

// static int d=0;
// if (d++ >=0){
// 	d=0;
// 	Printf("x %d, y %d, z %d, t %d\n\r", (int)imuAngle.angle.pitch, (int)imuAngle.angle.roll, (int)imuAngle.angle.yaw, (int)(c_time_us));
// }

	MotorControl_loop();
}
/*
#define LIMIT 3000
#define DEF_UPDATE_MIN(a, b) {if (a.x>b.x) a.x = b.x; if (a.y>b.y) a.y = b.y; if (a.z>b.z) a.z = b.z;}
#define DEF_UPDATE_MAX(a, b) {if (a.x<b.x) a.x = b.x; if (a.y<b.y) a.y = b.y; if (a.z<b.z) a.z = b.z;}

void imu_autoCalibrateByNoize(int stop){
	static uint32_t l_time = 0;
	uint32_t time = system_getTime_ms();
	static vector average; 		//for get average data
	static vector min,max;
	static vector diff_last = {.axis.x=2*LIMIT,.axis.y=2*LIMIT,.axis.z=2*LIMIT};	// save the best result is in terms of noise
	//
	static int nStop = 1; 	// not to count the first time and set default values
	static uint32_t cnt = 0;	// count new data

	if (statusCalibrateGyro < -1) {	// reset calibrate
		statusCalibrateGyro=-1;
		diff_last = vector_setConst(diff_last, 2*LIMIT);
		l_time = time;
	}

	uint32_t timeWait = time-l_time;
	if (timeWait < 1000)//(GyroClbCnt < (1*gyro_freq))//1000)
	{
		//for (i=0;i<3;i++)GyroClbAcc_axis[i] += sVel_axis[i];
		DEF_UPDATE_MIN(min.axis, gyro.axis);
		DEF_UPDATE_MAX(max.axis, gyro.axis);

		average = vector_addVector(average, gyro);
		cnt++;
		nStop += stop;	// if calibration has been stopped
	}
	else
	{
		vector diff_res = {max.axis.x-min.axis.x, max.axis.y-min.axis.y, max.axis.z-min.axis.z};
		if ((cnt >=10) && 	// minimum count
				(nStop == 0) &&		// if there was no calibration stop
				((diff_res.axis.x<diff_last.axis.x) && (diff_res.axis.y<diff_last.axis.y) && (diff_res.axis.z<diff_last.axis.z))	// if the best result is in terms of noise
				){
			
			// gyro_offs -= average gyro data
			gyro_offs = vector_removeVector(gyro_offs, vector_divConst(average, (float)cnt));
			diff_last = diff_res;
			statusCalibrateGyro = 0;
		}
		// reset values:
		nStop = 0;
		cnt = 0;
		average = vector_setConst(average, 0);
		min = vector_setConst(min, LIMIT);
		max = vector_setConst(max, -LIMIT);
		l_time = time;
	}
}
*/
// gyro_calibration.h
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

// gyro_calibration.c
static gyro_calibration_result_t result = {0};
static calibration_callback_t callback = NULL;

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
        CALIB_LOG(LOG_INFO, "Calibration session started");
    }
    
    uint32_t elapsed = system_getTime_ms() - session.start_time;
    
    if (elapsed < CALIB_DURATION_MS) {
        if (!imu_is_static()) {
            session.has_movement = true;
            CALIB_LOG(LOG_WARNING, "Movement detected, resetting session");
            memset(&session, 0, sizeof(session));
            return;
        }
        
        session.sum = vec3_add(session.sum, gyro);
        session.min = vec3_min(session.min, gyro);
        session.max = vec3_max(session.max, gyro);
        session.count++;
        
        // Прогресс-бар
        if (elapsed % 500 == 0) {
            CALIB_LOG(LOG_INFO, "Collecting: %u%% complete", elapsed * 100 / CALIB_DURATION_MS);
        }
    } 
    else if (result.samples_used == 0) { // Finalize once
        result.samples_used = session.count;
        result.bias = vec3_div_const(session.sum, session.count);
        result.noise_level = vec3_sub(session.max, session.min);
        
        bool success = (session.count >= MIN_SAMPLES && !session.has_movement);
        
        if (success && callback) {
            CALIB_LOG(LOG_INFO, "Calibration successful: bias=(%.2f,%.2f,%.2f)", 
                      result.bias.x, result.bias.y, result.bias.z);
            callback(&result);
        } else {
            CALIB_LOG(LOG_ERROR, "Calibration failed: samples=%u, movement=%d", 
                      session.count, session.has_movement);
        }
    }
}
/*
void imu_accCalibrate(void){
	static uint32_t l_time = 0;
	static int start = 1;
	static vector average = {.axis.x=0,.axis.y=0,.axis.z=0};
	static uint32_t cnt = 0;

	// if no needs calibrate
	if (statusCalibrateAcc >= 0) {return;}

	uint32_t time = system_getTime_ms();
	if (start != 0){
		start=0;
		l_time = time;
		cnt = 0;
		average = vector_setConst(average, 0);
		acc_offs = vector_setConst(acc_offs, 0);	// reseting offset to get data without offset
		return;
	}

	uint32_t dt = time-l_time;
	if (time < l_time){dt = 0; l_time = time;}	//when the counter overflows
	if (dt < 1000)
	{
		average = vector_addVector(average, acc);
		cnt++;
	}
	else
	{
		if (cnt == 0) {statusCalibrateAcc = -1; start = 1; return;}	// for avoid div zerro error

		average = vector_divConst(average, (float)(cnt*1.0f));	// get the average value 
		vector offset_tmp = average;	// get the average value 

		float A_abs[3];
		for (int i=0;i<AXES_ALL;i++) A_abs[i] = fabs(offset_tmp.value[i]);

		int max_axis = AXIS_X;	// seeking main axis: (and remove 1g from the main axis)
		if (A_abs[AXIS_Y] > A_abs[max_axis]) max_axis = AXIS_Y;
		if (A_abs[AXIS_Z] > A_abs[max_axis]) max_axis = AXIS_Z;
		if (offset_tmp.value[max_axis]<0) offset_tmp.value[max_axis] += 1.0f;	// remove 1g
		else offset_tmp.value[max_axis] -= 1.0f;	// remove 1g

		acc_offs = vector_removeVector(acc_offs, offset_tmp);	// change result offset, by new data

		Printf("average c: %d, %d, %d\n\r", (int)(average.value[0]*1000), (int)(average.value[1]*1000), (int)(average.value[2]*1000));
		Printf("acc c: %d, %d, %d\n\r", (int)(acc_offs.axis.x*1000), (int)(acc_offs.axis.y*1000), (int)(acc_offs.axis.z*1000));
		statusCalibrateAcc = 0;
		start = 1;
		cnt=0;
	}
}
*/


// acc_calib.h - Плотный интерфейс
typedef struct { float x,y,z; } v3;
typedef enum { ACC_IDLE, ACC_RUN, ACC_DONE, ACC_FAIL } acc_stat;

void acc_calib_start(void);
void acc_calib_update(void);
acc_stat acc_calib_status(void);
v3 acc_calib_offset(void);

// acc_calib.c
static struct {
    struct { uint32_t t; uint16_t cnt; v3 sum; } s;
    acc_stat status;
    v3 off;
} acc = {.status = ACC_IDLE};

static inline v3 v3_add(v3 a, v3 b) { return (v3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline v3 v3_div(v3 v, float d) { return (v3){v.x/d, v.y/d, v.z/d}; }
static inline v3 v3_sub(v3 a, v3 b) { return (v3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline float v3_abs_max_axis(v3 v) {
    float ax = fabsf(v.x), ay = fabsf(v.y), az = fabsf(v.z);
    return (ax > ay && ax > az) ? 0 : (ay > az) ? 1 : 2;
}

void acc_calib_start(void) {
    acc.s.t = system_getTime_ms();
    acc.s.cnt = 0;
    acc.s.sum = (v3){0};
    acc.off = (v3){0};
    acc.status = ACC_RUN;
}

void acc_calib_update(void) {
    if (acc.status != ACC_RUN) return;
    
    uint32_t dt = system_getTime_ms() - acc.s.t;
    
    if (dt < 1000) {
        acc.s.sum = v3_add(acc.s.sum, acc_raw);
        acc.s.cnt++;
        return;
    }
    
    if (!acc.s.cnt) { acc.status = ACC_FAIL; return; }
    
    v3 avg = v3_div(acc.s.sum, acc.s.cnt);
    int axis = v3_abs_max_axis(avg);
    avg.value[axis] -= (avg.value[axis] < 0) ? -1.0f : 1.0f;
    
    acc.off = v3_sub(acc.off, avg);
    acc.status = ACC_DONE;
}

acc_stat acc_calib_status(void) { return acc.status; }
v3 acc_calib_offset(void) { return acc.off; }

/// 2in once

// imu_calib.h - Всё в одном
#define CALIB_DURATION_MS 2000
#define STATIC_THRESH 0.2f

typedef struct { v3 sum, min, max; uint32_t cnt, start; bool moving; } calib_ctx;

static inline void calib_reset(calib_ctx *c) {
    *c = (calib_ctx){.min = {LIMIT,LIMIT,LIMIT}, .max = {-LIMIT,-LIMIT,-LIMIT}};
    c->start = system_getTime_ms();
}

static inline void calib_update_minmax(calib_ctx *c, v3 val) {
    if (val.x < c->min.x) c->min.x = val.x; if (val.x > c->max.x) c->max.x = val.x;
    if (val.y < c->min.y) c->min.y = val.y; if (val.y > c->max.y) c->max.y = val.y;
    if (val.z < c->min.z) c->min.z = val.z; if (val.z > c->max.z) c->max.z = val.z;
}

static inline bool is_static(v3 g) {
    return fabsf(g.x) < STATIC_THRESH && fabsf(g.y) < STATIC_THRESH && fabsf(g.z) < STATIC_THRESH;
}

// Гироскоп
static calib_ctx gyro_ctx;
static v3 gyro_bias;

void gyro_calib_update(bool reset) {
    if (reset) { calib_reset(&gyro_ctx); return; }
    
    if (!gyro_ctx.start) { calib_reset(&gyro_ctx); return; }
    
    uint32_t dt = system_getTime_ms() - gyro_ctx.start;
    
    if (dt < CALIB_DURATION_MS) {
        if (!is_static(gyro_raw)) { calib_reset(&gyro_ctx); return; }
        gyro_ctx.sum = v3_add(gyro_ctx.sum, gyro_raw);
        calib_update_minmax(&gyro_ctx, gyro_raw);
        gyro_ctx.cnt++;
    } 
    else if (gyro_ctx.cnt >= 10 && !gyro_ctx.moving) {
        gyro_bias = v3_div(gyro_ctx.sum, gyro_ctx.cnt);
    }
}

// Акселерометр
static calib_ctx acc_ctx;
static v3 acc_bias;

void acc_calib_update(void) {
    if (!acc_ctx.start) { calib_reset(&acc_ctx); return; }
    
    uint32_t dt = system_getTime_ms() - acc_ctx.start;
    
    if (dt < 1000) {
        acc_ctx.sum = v3_add(acc_ctx.sum, acc_raw);
        acc_ctx.cnt++;
    } 
    else if (acc_ctx.cnt) {
        v3 avg = v3_div(acc_ctx.sum, acc_ctx.cnt);
        int axis = (fabsf(avg.x) > fabsf(avg.y) && fabsf(avg.x) > fabsf(avg.z)) ? 0 :
                   (fabsf(avg.y) > fabsf(avg.z)) ? 1 : 2;
        avg.value[axis] -= (avg.value[axis] < 0) ? -1.0f : 1.0f;
        acc_bias = v3_sub(acc_bias, avg);
        acc_ctx.start = 0; // завершено
    }
}


/// 2in 1 v2

// imu_calib_compact.h - Плотно, но читаемо
typedef struct { float x,y,z; } vec;

static inline vec vec_add(vec a, vec b) { return (vec){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline vec vec_sub(vec a, vec b) { return (vec){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline vec vec_div(vec v, float d) { return (vec){v.x/d, v.y/d, v.z/d}; }
static inline vec vec_fill(float v) { return (vec){v,v,v}; }

typedef struct {
    vec sum, min, max;
    uint32_t cnt, start;
    bool moving;
} calib_t;

static calib_t gyro = {.min = vec_fill(3000), .max = vec_fill(-3000)};
static calib_t acc = {0};
static vec gyro_bias = {0}, acc_bias = {0};

void calib_gyro_update(bool force) {
    if (force) { gyro = (calib_t){.min=vec_fill(3000), .max=vec_fill(-3000), .start=system_getTime_ms()}; return; }
    if (!gyro.start) return;
    
    uint32_t dt = system_getTime_ms() - gyro.start;
    bool static_ok = fabsf(gyro_raw.x) < 0.2f && fabsf(gyro_raw.y) < 0.2f && fabsf(gyro_raw.z) < 0.2f;
    
    if (dt < 2000 && static_ok) {
        gyro.sum = vec_add(gyro.sum, gyro_raw);
        gyro.cnt++;
    } else if (gyro.cnt > 10 && !gyro.moving) {
        gyro_bias = vec_div(gyro.sum, gyro.cnt);
        gyro.start = 0;
    }
}

void calib_acc_update(void) {
    if (!acc.start) { acc = (calib_t){.start=system_getTime_ms()}; return; }
    
    if (system_getTime_ms() - acc.start < 1000) {
        acc.sum = vec_add(acc.sum, acc_raw);
        acc.cnt++;
    } else if (acc.cnt) {
        vec avg = vec_div(acc.sum, acc.cnt);
        int axis = (fabsf(avg.x) > fabsf(avg.y) && fabsf(avg.x) > fabsf(avg.z)) ? 0 :
                   (fabsf(avg.y) > fabsf(avg.z)) ? 1 : 2;
        avg.value[axis] -= (avg.value[axis] < 0) ? -1.0f : 1.0f;
        acc_bias = vec_sub(acc_bias, avg);
        acc.start = 0;
    }
}


/*
vector vector_setConst(vector axes, float Value){
	for (int i=0;i<AXES_ALL;i++) axes.value[i]=Value;
	return axes;
}
// vector vector_addConst(vector axes, float Value){
// 	for (int i=0;i<AXES_ALL;i++) axes.value[i]+=Value;
// 	return axes;
// }
vector vector_muxConst(vector axes, const float Value){
	for (int i=0;i<AXES_ALL;i++) axes.value[i]*=Value;
	return axes;
}
vector vector_divConst(vector axes, const float Value){
	for (int i=0;i<AXES_ALL;i++) axes.value[i]/=Value;
	return axes;
}
// vector vector_removeConst(vector axes, const float Value){
// 	for (int i=0;i<AXES_ALL;i++) axes.value[i]-=Value;
// 	return axes;
// }
vector vector_muxVector(vector axes, const vector axes2){
	for (int i=0;i<AXES_ALL;i++) axes.value[i] *= axes2.value[i];
	return axes;
}
vector vector_addVector(vector axes, vector axes2){
	for (int i=0;i<AXES_ALL;i++) axes.value[i]+=axes2.value[i];
	return axes;
}
vector vector_removeVector(vector axes, vector axes2){
	for (int i=0;i<AXES_ALL;i++) axes.value[i]-=axes2.value[i];
	return axes;
}
vector vector_rearranging(vector axes, int * val){
	vector res;
	for (int i=0;i<AXES_ALL;i++) if (val[i]<AXES_ALL)res.value[val[i]] = axes.value[i];
	return res;
}
*/

#include "vector_math.h"

#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <math.h>   // только если нужны sqrtf, sinf, cosf
#include <string.h> // для memcpy при необходимости

// Константы осей
#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXES_ALL 3

// Тип данных: 3-мерный вектор (12 байт, выровнен)
// Аналог вашей структуры 'vector', но с явными полями [citation:9]
typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

// Инициализация и константы
static inline vec3_t vec3_create(float x, float y, float z) {
    vec3_t v = {x, y, z};
    return v;
}

static inline vec3_t vec3_zero(void) {
    return (vec3_t){0.0f, 0.0f, 0.0f};
}

// === Базовые операции (замена ваших функций) ===

// vec3 = vec3 + scalar
static inline vec3_t vec3_add_const(vec3_t v, float scalar) {
    return (vec3_t){v.x + scalar, v.y + scalar, v.z + scalar};
}

// vec3 = vec3 * scalar (ваш vector_muxConst)
static inline vec3_t vec3_mul_const(vec3_t v, float scalar) {
    return (vec3_t){v.x * scalar, v.y * scalar, v.z * scalar};
}

// vec3 = vec3 / scalar (ваш vector_divConst) 
static inline vec3_t vec3_div_const(vec3_t v, float scalar) {
    // Защита от деления на ноль
    if (scalar == 0.0f) return v;
    float inv = 1.0f / scalar; // одно деление вместо трех
    return (vec3_t){v.x * inv, v.y * inv, v.z * inv};
}

// Поэлементное сложение векторов
static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

// Поэлементное вычитание (ваш vector_removeVector)
static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

// Поэлементное умножение (ваш vector_muxVector, Hadamard product)
static inline vec3_t vec3_mul(vec3_t a, vec3_t b) {
    return (vec3_t){a.x * b.x, a.y * b.y, a.z * b.z};
}

// === Полезные операции ===

// Скалярное произведение
static inline float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Длина вектора
static inline float vec3_length(vec3_t v) {
    return sqrtf(vec3_dot(v, v));
}

// Нормализация
static inline vec3_t vec3_normalize(vec3_t v) {
    float len = vec3_length(v);
    if (len == 0.0f) return vec3_zero();
    return vec3_div_const(v, len);
}

// === Перестановка осей (аналог вашего vector_rearranging) ===
// permutation[0..2] содержит новые индексы для x,y,z соответственно
static inline vec3_t vec3_rearrange(vec3_t v, const int permutation[3]) {
    float data[3] = {v.x, v.y, v.z};
    return (vec3_t){
        data[permutation[0]],
        data[permutation[1]], 
        data[permutation[2]]
    };
}

// Установка всех компонент в одно значение
static inline vec3_t vec3_fill(float value) {
    return (vec3_t){value, value, value};
}

#endif // VECTOR_MATH_H