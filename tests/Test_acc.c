void test_acc_calibration(void) {
    // Мокаем акселерометр, показывающий 1g по оси Z
    acc = (vec3_t){0, 0, 9.81f};
    
    acc_calibration_start();
    
    // Симулируем проход времени
    for (int i = 0; i < 150; i++) {
        delay_ms(10);
        acc_calibration_update();
    }
    
    const acc_calibration_result_t* result = acc_calibration_get_result();
    
    // Ожидаем смещение ~0 по X,Y и ~0 по Z (после вычитания 1g)
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, result->offset.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, result->offset.y);
    TEST_ASSERT_FLOAT_WITHIN(0.2f, -9.81f, result->offset.z);
}