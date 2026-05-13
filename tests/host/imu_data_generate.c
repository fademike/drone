



// Используя Unity Test Framework для embedded
#include <stdlib.h>
#include "imu_data_generate.h"

static vec3_t acc = {0,0,1};
static vec3_t angles = {0,0,0};
static vec3_t speed_rate = {0,0,0};
static float angle_change_speed = 0;
static vec3_t gyro = {0,0,0};
static float noise_level = 0.1;

void gen_set_acc(vec3_t a){
    acc = a;
}

void gen_set_gyro(vec3_t g){
    gyro = g;
}

void gen_set_noise_lvl(float n){
    noise_level = n;
}

void change_pos(vec3_t angle, vec3_t speed){
    // angles = angle;
    speed_rate = speed;
}

static vec3_t gen_noise(float noise_level){
  return (vec3_t){(((float)rand() / (float)RAND_MAX) -0.5f )*noise_level,
        (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level,
        (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level};
}

vec3_t gen_acc(float dt){

    angles = vec3_add(angles, vec3_mul_const(speed_rate, dt));

    gyro = speed_rate;

    float roll = angles.x*3.14f/180.0f;
    float pitch = angles.y*3.14f/180.0f;

    acc.x = -sinf(pitch);
    acc.y = sinf(roll)*cosf(pitch);
    acc.z = cosf(roll)*cosf(pitch);

    vec3_t noise = gen_noise(noise_level);
    
    return vec3_add(acc, noise);
}


vec3_t gen_gyro(void){

    vec3_t noise = gen_noise(noise_level);
    return vec3_add(gyro, noise);
}

void gen_set_angle(vec3_t a){
    angles = a;
}

void gen_imu(vec3_t * acc, vec3_t * gyro, vec3_t speed, float dt){
    angles = vec3_add(angles, vec3_mul_const(speed, dt));

    float roll = angles.x*3.14f/180.0f;
    float pitch = angles.y*3.14f/180.0f;

    acc->x = -sinf(pitch);
    acc->y = sinf(roll)*cosf(pitch);
    acc->z = cosf(roll)*cosf(pitch);
    
    *acc = vec3_add(*acc, gen_noise(noise_level));
    *gyro = vec3_add(*gyro, gen_noise(noise_level));
}