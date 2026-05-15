

#include "imu_rearrange.h"
/*
typedef struct {
  uint8_t x :2;
  uint8_t y :2;
 uint8_t z :2;
 uint8_t gyro_sig :3;
 uint8_t acc_sig :3;
};*/


  vec3_u gyro_sig, acc_sig;
  
//vec3_t acc_sig ={0,0,0};
//vec3_t gyro_sig = {0,0,0};

int rr_axis[AXES_ALL] = {AXIS_X,AXIS_Y,AXIS_Z};

#define MASK_AXIS 0x3

#include "stdio.h"
#define CALIB_LOG(d, str) do{ printf str; } while(0)

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

int get_abs_big_ptr(float * d, int cnt){
  if (cnt<=0) return 0;
  int index=0;
  float max= fabsf(d[0]);
  for (int i=1;i<cnt;i++) {
    float new =fabsf(d[i]);
    if(max<new) {max=new; index=i;}
  }
  
  //int axis = ((fabsf(avg_a.axis.x) > fabsf(avg_a.axis.y)) && (fabsf(avg_a.axis.x) > fabsf(avg_a.axis.z))) ? AXIS_X : \
                   (fabsf(avg_a.axis.y) > fabsf(avg_a.axis.z)) ? AXIS_Y : AXIS_Z;
  
  return index;
}

int crr = 0;
int crr_state =0;
int crr_timeout=0;

int imu_calc_rr_get_state(void){
  return crr_state;
}

int imu_calc_rearrange(vec3_t acc, vec3_t gyro, int stop){
  static vec3_u sum_a={0,0,0};
  static vec3_u sum_g={0,0,0};
  static vec3_u a_bias={0,0,0};
  static vec3_u g_bias={0,0,0};

  gyro = vec3_sub(gyro, g_bias.axis);
  acc = vec3_sub(acc, a_bias.axis);

  static int timeout = 0; //ms
  sum_g.axis = vec3_add(sum_g.axis, gyro);
  
  if (stop){
    crr_state = 0;
    crr=0;
    return crr_state;
  }
  if (crr_state == 0){  // static posision. set acc axis_z
    if (crr == 0) {
      sum_a.axis = (vec3_t){0,0,0};
      sum_g.axis = gyro;//(vec3_t){0,0,0};
      a_bias.axis = (vec3_t){0,0,0};
      g_bias.axis = (vec3_t){0,0,0};
    }
    if(crr++<100){
      sum_a.axis = vec3_add(sum_a.axis, acc);
    }
    else{
      a_bias.axis = vec3_div_const(sum_a.axis, 100);
      vec3_u avg_a = a_bias;
      g_bias.axis = vec3_div_const(sum_g.axis, 100);
      int axis = get_abs_big_ptr(avg_a.data,3);
      rr_axis[AXIS_Z] =axis;
      acc_sig.data[AXIS_Z] = (avg_a.data[axis] < 0) ? -1.0f : 1.0f;
      gyro_sig.data[AXIS_Y] = (avg_a.data[axis] < 0) ? 1.0f : -1.0f;

      //if ()
      //int a_big = ((absf(avr.x)>absf(avr.y)) && (absf(avr.x)>absf(avr.z))) ? AXIS_X : ()
      crr_state ++;
      timeout = 0;
      printf("rr: static completed. z is %d, sig: %d\n\r", rr_axis[AXIS_Z], (int)acc_sig.data[AXIS_Z]);
      printf("rr: static completed. bias acc is %f, %f, %f\n\r", a_bias.axis.x, a_bias.axis.y, a_bias.axis.z);
      printf("rr: static completed. bias gyro is %f, %f, %f\n\r", g_bias.axis.x, g_bias.axis.y, g_bias.axis.z);
    }
  }
  if (crr_state == 1){ // move to pitch. set acc y, gyro x
    crr++;
    vec3_u a = {.axis = acc};
    // int completed = 0;
    for (int i=0; i< AXES_ALL; i++){
      if (i == rr_axis[AXIS_Z]) continue; // it is Z axis
      if (fabsf(a.data[i]) > 0.2f) { // 0.2 threshold gravity
        rr_axis[AXIS_Y] = i; 
        acc_sig.data[AXIS_Y] = (a.data[i] < 0) ? -1.0f : 1.0f;
        // int last_axis = AXES_ALL - (i + rr_axis[AXIS_Z]);
        // rr_axis[AXIS_X] = last_axis; 
        gyro_sig.data[AXIS_X] = acc_sig.data[AXIS_Y];
        //rr_axis[AXIS_Y] = i; 
        // gyro_sig.data[AXIS_Y] = (a.data[i] < 0) ? -1.0f : 1.0f;
        // completed++;
        printf("rr: cnt: %d\n\r", crr);
        printf("rr: pitch acc founded. is %d, sig: %d\n\r", rr_axis[AXIS_Y], (int)acc_sig.data[AXIS_Y]);
        // printf("rr: rol acc founded. is %d, sig: %d\n\r", rr_axis[AXIS_X], (int)acc_sig.data[AXIS_X]);
        crr_state++;
      }
      // if (sum_g.data[i] > 20.0f) {  //20.0 threshold angel
      //   rr_axis[AXIS_Y] = i; 
      //   acc_sig.data[AXIS_Y] = (a.data[i] < 0) ? -1.0f : 1.0f;
      //   completed++;}
    
    }
    
  }
  if (crr_state == 2){ // move to pitch. set acc y, gyro x
    crr++;
    vec3_u a = {.axis = acc};
    int last_axis = AXES_ALL - (rr_axis[AXIS_Y] + rr_axis[AXIS_Z]);
    if (fabsf(a.data[last_axis]) > 0.2f) {
      rr_axis[AXIS_X] = last_axis; 
      acc_sig.data[AXIS_X] = (a.data[last_axis] < 0) ? -1.0f : 1.0f;
      gyro_sig.data[AXIS_Y] = acc_sig.data[AXIS_X];

      printf("rr: cnt: %d\n\r", crr);
      // printf("rr: pitch acc founded. is %d, sig: %d\n\r", rr_axis[AXIS_Y], (int)acc_sig.data[AXIS_Y]);
      printf("rr: roll acc founded. is %d, sig: %d\n\r", rr_axis[AXIS_X], (int)acc_sig.data[AXIS_X]);
      crr_state++;
    }
    
  }
  return crr_state;
}



// 00p  000
// 0-0  -00
// +00  0-0
// 000  00-

// p00 000
// 0-0 00+
// 00- 0-0
// 000 -00

// m00 000
// 0+0 00+
// 00- 0+0
// 000 +00

// 0p0 000
// +00 00+
// 00- +00
// 000 0-0

// 0m0 000
// -00 00+
// 00- -00
// 000 0+0