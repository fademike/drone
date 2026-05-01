

// Используя Unity Test Framework для embedded
#include "unity.h"
#include "vector_math.h"
#include "gyro_calibration.h"

int mock_time = 0;
vec3_t mock_gyro = {0,0,0};
int injected_bias=0;

void setUp(void) {
    // Сброс состояния перед каждым тестом
    gyro_calibration_init(NULL);
    mock_time = 0;
}
#define GYRO_CALIB_FAILED 0

void tearDown(void) {
    // Очистка после теста
}

void test_calibration_bias_calculation(void) {
    // Имитируем чистое смещение
    vec3_t injected_bias = {1.0f, -0.5f, 0.25f};
    
    for (int i = 0; i < 200; i++) {
        mock_gyro = injected_bias;
        mock_time = i * 10;
        gyro_calibration_update(false);
    }
    
    mock_time = 3000;
    gyro_calibration_update(false);
    
    vec3_t result = gyro_calibration_get_bias();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, injected_bias.x, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, injected_bias.y, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, injected_bias.z, result.z);
}

void test_calibration_fails_on_movement(void) {
    // Первые 100 сэмплов статические
    for (int i = 0; i < 100; i++) {
        mock_gyro = (vec3_t){0.1f, 0.1f, 0.1f};
        mock_time = i * 10;
        gyro_calibration_update(false);
    }
    
    // Затем движение
    mock_gyro = (vec3_t){10.0f, 10.0f, 10.0f};
    mock_time = 1000;
    gyro_calibration_update(false); // Должен сброситься
    
    mock_time = 3000;
    gyro_calibration_update(false);
    
    TEST_ASSERT_EQUAL(GYRO_CALIB_FAILED, gyro_calibration_get_status());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_calibration_bias_calculation);
    RUN_TEST(test_calibration_fails_on_movement);
    return UNITY_END();
}


// /////

//  generate data output
 
//  // test_data_generator.c
// void generate_test_case(const char* name, vec3_t bias, float noise_level, 
//                         int duration_ms, bool add_movement) {
//     FILE* f = fopen(name, "w");
//     fprintf(f, "# Test case: %s\n", name);
//     fprintf(f, "# Expected bias: %.3f, %.3f, %.3f\n", bias.x, bias.y, bias.z);
//     fprintf(f, "timestamp_ms,gyro_x,gyro_y,gyro_z,is_static\n");
    
//     for (int ms = 0; ms < duration_ms; ms += 10) {
//         vec3_t sample = bias;
        
//         // Добавляем шум
//         sample.x += ((float)rand() / RAND_MAX - 0.5f) * noise_level;
//         sample.y += ((float)rand() / RAND_MAX - 0.5f) * noise_level;
//         sample.z += ((float)rand() / RAND_MAX - 0.5f) * noise_level;
        
//         bool is_static = true;
        
//         // Добавляем движение в середине
//         if (add_movement && ms > duration_ms/3 && ms < 2*duration_ms/3) {
//             sample.x += 5.0f;
//             is_static = false;
//         }
        
//         fprintf(f, "%d,%.3f,%.3f,%.3f,%d\n", 
//                 ms, sample.x, sample.y, sample.z, is_static);
//     }
//     fclose(f);
// }

// //// make

// # Пример Makefile для тестов
// test: test_gyro_calibration
// 	./test_gyro_calibration --verbose

// test_gyro_calibration: test_gyro_calibration.c gyro_calibration.c
// 	gcc -DUNIT_TEST -DTEST -Wall -o $@ $^ -lm
// 	./$@
	

