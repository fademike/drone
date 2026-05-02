
#include <math.h>
#include "vector_math.h"



// Инициализация и константы
vec3_t vec3_create(float x, float y, float z) {
    vec3_t v = {x, y, z};
    return v;
}

vec3_t vec3_zero(void) {
    return (vec3_t){0.0f, 0.0f, 0.0f};
}

// === Базовые операции (замена ваших функций) ===

// vec3 = vec3 + scalar
vec3_t vec3_add_const(vec3_t v, float scalar) {
    return (vec3_t){v.x + scalar, v.y + scalar, v.z + scalar};
}

// vec3 = vec3 * scalar (ваш vector_muxConst)
vec3_t vec3_mul_const(vec3_t v, float scalar) {
    return (vec3_t){v.x * scalar, v.y * scalar, v.z * scalar};
}

// vec3 = vec3 / scalar (ваш vector_divConst) 
vec3_t vec3_div_const(vec3_t v, float scalar) {
    // Защита от деления на ноль
    if (scalar == 0.0f) return v;
    float inv = 1.0f / scalar; // одно деление вместо трех
    return (vec3_t){v.x * inv, v.y * inv, v.z * inv};
}

// Поэлементное сложение векторов
vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

// Поэлементное вычитание (ваш vector_removeVector)
vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

// Поэлементное умножение (ваш vector_muxVector, Hadamard product)
vec3_t vec3_mul(vec3_t a, vec3_t b) {
    return (vec3_t){a.x * b.x, a.y * b.y, a.z * b.z};
}

// === Полезные операции ===

// Скалярное произведение
float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// // Длина вектора
// float vec3_length(vec3_t v) {
//     return sqrtf(vec3_dot(v, v));
// }

// // Нормализация
// vec3_t vec3_normalize(vec3_t v) {
//     float len = vec3_length(v);
//     if (len == 0.0f) return vec3_zero();
//     return vec3_div_const(v, len);
// }

// === Перестановка осей (аналог вашего vector_rearranging) ===
// permutation[0..2] содержит новые индексы для x,y,z соответственно
vec3_t vec3_rearrange(vec3_t v, const int permutation[3]) {
    float data[3] = {v.x, v.y, v.z};
    return (vec3_t){
        data[permutation[0]],
        data[permutation[1]], 
        data[permutation[2]]
    };
}

// Установка всех компонент в одно значение
vec3_t vec3_fill(float value) {
    return (vec3_t){value, value, value};
}

vec3_t vec3_min(vec3_t a, vec3_t b) {
    vec3_t res;
    res.x = (a.x<b.x)? a.x : b.x;
    res.y = (a.y<b.y)? a.y : b.y;
    res.z = (a.z<b.z)? a.z : b.z;
    return res;
}


vec3_t vec3_max(vec3_t a, vec3_t b) {
    vec3_t res;
    res.x = (a.x>b.x)? a.x : b.x;
    res.y = (a.y>b.y)? a.y : b.y;
    res.z = (a.z>b.z)? a.z : b.z;
    return res;
}

