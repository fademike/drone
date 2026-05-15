



// Используя Unity Test Framework для embedded
#include <stdlib.h>
#include "unity.h"
#include "vector_math.h"
#include "imu_rearrange.h"
#include "imu_data_generate.h"

void setUp(void) {
    // Сброс состояния перед каждым тестом
}

static vec3_t gen_noise(float noise_level){
  return (vec3_t){(((float)rand() / (float)RAND_MAX) -0.5f )*noise_level,
        (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level,
        (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level};
}

void tearDown(void) {
    // Очистка после теста
}

void rearrange(void){
  load_rearange(0x01);
    int * rr = rr_get_rr();
    vec3_t acc_sig = rr_acc_sig();
    vec3_t gyro_sig = rr_gyro_sig();
    
    TEST_ASSERT_INT_WITHIN(0, 1, rr[0]);
    TEST_ASSERT_INT_WITHIN(0, 0, rr[1]);
    TEST_ASSERT_INT_WITHIN(0, 0, rr[2]);
    
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.x);
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.x);
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.x);
    
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, acc_sig.x);
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, acc_sig.x);
    TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, acc_sig.x);

    

}

static int rr[3]={0,0,0};
static vec3_t a_sig={0,0,0};
static vec3_t g_sig={0,0,0};
vec3_t local_rr(vec3_t in){
  float data[3] = {in.x, in.y, in.z};
  return (vec3_t){data[rr[0]], data[rr[1]], data[rr[3]]};
}

void calc_orientation(vec3_u acc, int * expect, vec3_u e_sig, int a1, int a2){
  int state=0;

    vec3_t gyro = (vec3_t){0,0,0};
    float noise_level = 0.1f;
    gen_set_noise_lvl( 0.1);
    vec3_t speed = {0,0,0};

    load_rearange(0x0);// reset
    gen_set_angle((vec3_t){0,0,0});
    imu_calc_rearrange((vec3_t){0,0,0},(vec3_t){0,0,0}, 1);

vec3_t aa={0,0,1};
vec3_t gg ={0,0,0};
    // static position
    for (int i = 0; i < 1000; i++) {
      gen_imu(&aa, &gg, speed, 0.01);
      imu_calc_rearrange(acc.axis, gyro, 0);
        //vec3_t noise = gen_noise(noise_level);
        //imu_calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }
    // static position
    for (int i = 0; i < 3000; i++) {
        vec3_t noise = gen_noise(noise_level);
        acc.data[a1]+=0.01f;
        imu_calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }
    // static position
    for (int i = 0; i < 3000; i++) {
        vec3_t noise = gen_noise(noise_level);;

        acc.data[a2]+=0.01f;
        //acc.axis.x+=0.01f;
        state = imu_calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }

    int * rr = rr_get_rr();

    vec3_t acc_sig = rr_acc_sig();
    vec3_t gyro_sig = rr_gyro_sig();
    
    TEST_ASSERT_INT_WITHIN(0, 3, state);
    
    TEST_ASSERT_INT_WITHIN(0, expect[0], rr[0]);
    TEST_ASSERT_INT_WITHIN(0, expect[1], rr[1]);
    TEST_ASSERT_INT_WITHIN(0, expect[2], rr[2]);
    
    // TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.x);
    // TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.y);
    // TEST_ASSERT_FLOAT_WITHIN(0, 1.0f, gyro_sig.z);
    
    TEST_ASSERT_FLOAT_WITHIN(0, e_sig.axis.x, acc_sig.x);
    TEST_ASSERT_FLOAT_WITHIN(0, e_sig.axis.y, acc_sig.y);
    TEST_ASSERT_FLOAT_WITHIN(0, e_sig.axis.y, acc_sig.z);

}


void orientation(void){
    calc_orientation((vec3_u){0,0,1}, (int[AXES_ALL]){AXIS_X, AXIS_Y, AXIS_Z}, (vec3_u){1,1,1}, AXIS_Y, AXIS_X);
    calc_orientation((vec3_u){1,0,0}, (int[AXES_ALL]){AXIS_Y, AXIS_Z, AXIS_X}, (vec3_u){1,1,-1}, AXIS_Z, AXIS_Y);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(rearrange);
    // RUN_TEST(calc_orientation);
    RUN_TEST(orientation);
    
    return UNITY_END();
}

