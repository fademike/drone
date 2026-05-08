

#include "imu_rearrange.h"
/*
typedef struct {
  uint8_t x :2;
  uint8_t y :2;
 uint8_t z :2;
 uint8_t gyro_sig :3;
 uint8_t acc_sig :3;
};*/

typedef union vec3_u {
    vec3_t axis;
    float data[AXES_ALL];
  } gyro_sig, acc_sig;
  
//vec3_t acc_sig ={0,0,0};
//vec3_t gyro_sig = {0,0,0};

int rr_axis[AXES_ALL] = {AXIS_X,AXIS_Y,AXIS_Z};

#define MASK_AXIS 0x3

int load_rearange(uint16_t value){
  
  for (int i=0; i<AXES_ALL; i++){
    rr_axis[i]=(value>>(2*i))&MASK_AXIS;
  }
  for (int i=0; i<AXES_ALL; i++){
    gyro_sig.data[i] = (value>>(3+i)&0x1)? -1.0f : 1.0f;
    //rr_axis[i]=(value>>(2*i))&MASK_AXIS;
  }
  for (int i=0; i<AXES_ALL; i++){
    acc_sig.data[i] = (value>>(6+i)&0x1)? -1.0f : 1.0f;
    //rr_axis[i]=(value>>(2*i))&MASK_AXIS;
  }
  return 0;
}

int * rr_get_rr(void){
  return rr_axis;
}

vec3_t rr_acc_sig(void){
  return acc_sig.axis;
}
vec3_t rr_gyro_sig(void){
  return gyro_sig.axis;
}

int get_abs_big_ptr(float *);

int crr_state =0;
int crr_timeout=0;
int calc_rearrange(vec3_t acc, vec3_t gyro){
  static vec3_u sum_a={0,0,0};
  static vec3_u sum_g={0,0,0};
  static vec3_u a_bias={0,0,0};
  static vec3_u g_bias={0,0,0};
  if (crr_state == 0){
    if (crr == 0) {
      sum_a.axis = (vec3_t){0,0,0};
      sum_g.axis = (vec3_t){0,0,0};
    }
    if(crr++<100){
      sum_a.axis = vec3_sum(sum_a.axis, acc);
      sum_g.axis = vec3_sum(sum_g.axis, gyro);
    }
    else{
      vec3_t avg_a = vec3_div_const(sum_a.axis, const);
      a_bias.axis = avg_a;
      g_bias.axis = vec3_div_const(sum_g.axis, const);
      int axis = (fabsf(avg_a.x) > fabsf(avg_a.y) && fabsf(avg_a.x) > fabsf(avg_a.z)) ? AXIS_X :
                   (fabsf(avg_a.y) > fabsf(avg_a.z)) ? AXIS_Y : AXIS_Z;
      rr_axis[AXIS_Z] =axis;
      acc_sig.data[AXIS_Z] = (avg_a.data[AXIS]<0) ? -1.0f : 1.0f;
      //if ()
      //int a_big = ((absf(avr.x)>absf(avr.y)) && (absf(avr.x)>absf(avr.z))) ? AXIS_X : ()
      crr_state ++;
    }
  }
  if (crr_state ==1){
    
  }
  return 0;
}
