



#include "vector_math.h"



void gen_set_acc(vec3_t a);
void gen_set_gyro(vec3_t g);
void gen_set_noise_lvl(float n);
void change_pos(vec3_t angle, vec3_t speed);
vec3_t gen_acc(float dt);
vec3_t gen_gyro(void);
