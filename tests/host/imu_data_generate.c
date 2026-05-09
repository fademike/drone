



// Используя Unity Test Framework для embedded
#include <stdlib.h>
#include "imu_data_generate.h"

static vec3_t acc = {0,0,1};
static vec3_t acc_change = {0,0,1};
static float acc_change_speed = 0;
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

void change_pos(vec3_t a, float speed){
    acc_change = a;
    acc_change_speed = speed;
}

vec3_t gen_acc(void){

    if (acc_change.x > acc.x) {acc.x += (acc_change.x)*acc_change_speed; gyro.y += (acc_change.x)*acc_change_speed;}
    else if (acc_change.x < acc.x) {acc.x -= (acc_change.x)*acc_change_speed; gyro.y -= (acc_change.x)*acc_change_speed;}
    else gyro.y = 0;

    vec3_t noise;
    noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    
    return vec3_add(acc, noise);
}


vec3_t gen_gyro(void){

    vec3_t noise;
    noise.x = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    noise.y = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    noise.z = (((float)rand() / (float)RAND_MAX) -0.5f )*noise_level;
    
    return vec3_add(gyro, noise);
}

