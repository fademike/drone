



// Используя Unity Test Framework для embedded
#include <stdlib.h>
#include "unity.h"
#include "vector_math.h"
#include "imu_rearrange.h"

void setUp(void) {
    // Сброс состояния перед каждым тестом
   
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


void calc_orientation(vec3_u acc, int * expect, vec3_u e_sig, int a1, int a2){

    // vec3_t acc = (vec3_t){0,0,1};
    vec3_t gyro = (vec3_t){0,0,0};
    float noise_level = 0.1f;

    load_rearange(0x0);// reset
    calc_rearrange((vec3_t){0,0,0},(vec3_t){0,0,0}, 1);

    // static position
    for (int i = 0; i < 1000; i++) {
        vec3_t noise;
        // Добавляем шум
        noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;

        calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }
    // static position
    for (int i = 0; i < 3000; i++) {
        vec3_t noise;
        // Добавляем шум
        noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;

        acc.data[a1]+=0.01f;
        //acc.axis.z-=0.01f;
        //vec3_u * au = (vec3_u *)&acc;
        //au->data[a1]+=0,01f;

        calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }
    // static position
    for (int i = 0; i < 3000; i++) {
        vec3_t noise;
        // Добавляем шум
        noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;

        acc.data[a2]+=0.01f;
        //acc.axis.x+=0.01f;

        calc_rearrange(vec3_add(acc.axis, noise), vec3_add(gyro, noise), 0);
    }

    int * rr = rr_get_rr();

    vec3_t acc_sig = rr_acc_sig();
    vec3_t gyro_sig = rr_gyro_sig();
    
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

