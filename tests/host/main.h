

#include <stdio.h>

#define HAL_OK 0
#define HAL_ERROR 1
#define HAL_BUSY 2
#define HAL_TIMEOUT 3
// enum
// {
//   HAL_OK       = 0x00U,
//   HAL_ERROR    = 0x01U,
//   HAL_BUSY     = 0x02U,
//   HAL_TIMEOUT  = 0x03U
// };

#define Printf printf

#define SI4463
#define GetTime_ms(x) 0
#include <unistd.h> 

#define msleep(x) usleep(x*1000)

// #define HAL_GetTick(x) 0

#define IMU_MUST_BE 0