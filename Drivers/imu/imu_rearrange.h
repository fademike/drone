

#include <stdint.h>
#include "vector_math.h"

typedef union {
    vec3_t axis;
    float data[AXES_ALL];
  } vec3_u;

int load_rearange(uint16_t value);
int * rr_get_rr(void);
vec3_t rr_acc_sig(void);
vec3_t rr_gyro_sig(void);
int calc_rearrange(vec3_t acc, vec3_t gyro, int stop);


