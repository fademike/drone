

// Используя Unity Test Framework для embedded
#include <stdlib.h>
#include "unity.h"
#include "vector_math.h"
#include "acc_calibration.h"

void setUp(void) {
    // Сброс состояния перед каждым тестом
    acc_calibration_init();
}

void tearDown(void) {
    // Очистка после теста
}


vec3_t injected_bias = {0,0,0};
float noise_level = 0.0f;
float move = 0.1f;

void test_calibration_bias_with_noise(void) {

    injected_bias = (vec3_t){1,0,0};


    for (int i = 0; i < CALIB_DURATION_CNT; i++) {
        vec3_t noise;
        // Добавляем шум
        noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        //noise.y = (float)(rand() % 100) / (100.0f/noise_level);
        //noise.z = (float)(rand() % 100) / (100.0f/noise_level);

        acc_calibration_update(vec3_add(injected_bias, noise), false);
    }
    
    vec3_t acc_bias = acc_calibration_get_bias();

    injected_bias = (vec3_t){0,0,0};

    printf("check: %f, %f\n\r", injected_bias.x, acc_bias.x);

    // погрешность после усреднения
    float avr_lvl=0.1f;

    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, injected_bias.x, acc_bias.x);
    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, injected_bias.y, acc_bias.y);
    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, injected_bias.z, acc_bias.z);

}





void test_calibration(vec3_t set, vec3_t expect, float noise_lvl) {

    for (int i = 0; i < CALIB_DURATION_CNT; i++) {
        vec3_t noise;
        // Добавляем шум
        noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
        noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;

        acc_calibration_update(vec3_add(set, noise), false);
    }
    
    vec3_t acc_bias = acc_calibration_get_bias();

    
    // погрешность после усреднения
    float avr_lvl=0.1f;

    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, expect.x, acc_bias.x);
    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, expect.y, acc_bias.y);
    TEST_ASSERT_FLOAT_WITHIN(noise_level*avr_lvl, expect.z, acc_bias.z);

}

void my_test(void){
  // идеальное тестирование осей
 test_calibration((vec3_t){1,0,0}, (vec3_t){0,0,0}, 0.1f);
 test_calibration((vec3_t){0,1,0}, (vec3_t){0,0,0}, 0.1f);
 test_calibration((vec3_t){0,0,1}, (vec3_t){0,0,0}, 0.1f);
 
 // тестирование со смещением
 test_calibration((vec3_t){1.2,0.2,0.2}, (vec3_t){-0.2,-0.2,-0.2}, 0.1f);
}


// void test_calibration_bias_make_move(void) {

//     float s_noise = move;
//     for (int i = 0; i < CALIB_DURATION_CNT; i++) {
//         vec3_t noise;
//         // Добавляем шум
//         noise.x = (float)(rand() % 100) / (100.0f/noise_level)+s_noise;
//         noise.y = (float)(rand() % 100) / (100.0f/noise_level)+s_noise;
//         noise.z = (float)(rand() % 100) / (100.0f/noise_level)+s_noise;

//         s_noise += move;

//         gyro_calibration_update(vec3_add(injected_bias, noise), false);
//     }
// }


int main(void) {
    UNITY_BEGIN();

    // Тест с большими шумами
    injected_bias = (vec3_t){1,0,0};
    noise_level = 0.1f;

    //RUN_TEST(test_calibration_bias_with_noise);

    RUN_TEST(my_test);

/*
    // Тест с меньшими шумами
    injected_bias = (vec3_t){3,3,3};
    noise_level = 0.1f;

    RUN_TEST(test_calibration_bias_with_noise);


    // Тест в ускорении
    injected_bias = (vec3_t){4,4,4};
    noise_level = 0.2f;

    RUN_TEST(test_calibration_bias_with_acc);

    // Тест идеальный без шумов
    injected_bias = (vec3_t){6,6,6};
    noise_level = 0.01f;
    
    RUN_TEST(test_calibration_bias_with_noise);    // ideal value
    
    // RUN_TEST(test_calibration_fails_on_movement);
    */
    return UNITY_END();
}


