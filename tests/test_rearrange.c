



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

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(rearrange);
    
    return UNITY_END();
}

