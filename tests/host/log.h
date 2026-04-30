


#ifdef CMAKE_CROSSCOMPILING
#define LOG_PRINTF 0
#else 
#define LOG_PRINTF 1
#endif



#if LOG_PRINTF
#define MY_PLATFORM_DIAG(x) do {log_print_time(); printf x;} while(0)
#define MY_PLATFORM_DIAG_RAW(x) do {printf x;} while(0)
#else
#define MY_PLATFORM_DIAG(x) do {log_print_time(); printf x;} while(0)
#define MY_PLATFORM_DIAG_RAW(x) do {printf x;} while(0)
#endif

#define MY_DEBUGF(debug, message) do { if (debug) MY_PLATFORM_DIAG(message); } while(0)
#define MY_DEBUGF_RAW(debug, message) do { if (debug) MY_PLATFORM_DIAG_RAW(message); } while(0)

// PRINT_BYTES(("tf type: len: %d, d:", len), ((uint8_t *)data), len);
// #define PRINT_BYTES(s, buf, len) do {printf s; for (int i=0; i < len; i++) printf(" 0x%x,", buf[i]); printf("\n\r");} while(0)


#define MY_DEBUG_ON 1

#if MY_DEBUG_ON
#define FLASH_DEBUG             0
#define ARM_DEBUG             ((1 | LWIP_DBG_ON))
#define MSG_ARM_CLIENT_DEBUG    1   
#define AMMO_SCHEDULER_DEBUG    1   //(1)//((1 | LWIP_DBG_ON))
#define TFTCPS_DEBUG            ((1 | LWIP_DBG_ON))   //((1 | LWIP_DBG_ON))

#define MSG_PB_DEBUG            1

#define FLASH_DEBUG_ERROR 1

#endif

void log_print_time(void);

void log_print(char );
