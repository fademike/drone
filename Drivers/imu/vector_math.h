


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
vec3_t vec3_create(float x, float y, float z);

vec3_t vec3_zero(void);

// === Базовые операции (замена ваших функций) ===

// vec3 = vec3 + scalar
vec3_t vec3_add_const(vec3_t v, float scalar);

// vec3 = vec3 * scalar (ваш vector_muxConst)
vec3_t vec3_mul_const(vec3_t v, float scalar);

// vec3 = vec3 / scalar (ваш vector_divConst) 
vec3_t vec3_div_const(vec3_t v, float scalar);

// Поэлементное сложение векторов
vec3_t vec3_add(vec3_t a, vec3_t b);

// Поэлементное вычитание (ваш vector_removeVector)
vec3_t vec3_sub(vec3_t a, vec3_t b);

// Поэлементное умножение (ваш vector_muxVector, Hadamard product)
vec3_t vec3_mul(vec3_t a, vec3_t b);

// === Полезные операции ===

// Скалярное произведение
float vec3_dot(vec3_t a, vec3_t b);

// // Длина вектора
// float vec3_length(vec3_t v);

// // Нормализация
// vec3_t vec3_normalize(vec3_t v);

// === Перестановка осей (аналог вашего vector_rearranging) ===
// permutation[0..2] содержит новые индексы для x,y,z соответственно
vec3_t vec3_rearrange(vec3_t v, const int permutation[3]);

// Установка всех компонент в одно значение
vec3_t vec3_fill(float value);

vec3_t vec3_min(vec3_t a, vec3_t b);
vec3_t vec3_max(vec3_t a, vec3_t b);

#endif // VECTOR_MATH_H