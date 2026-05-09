





#include "main.h"
#include "system.h"



#include "radio_config_Si4463.h"

// #include "debug.h"

#include "params.h"
#include "system.h"
#include "mavlink_handler.h"

#include "imu.h"
#include "cb.h"

#include "ModemControl.h"
#include "MotorControl.h"

#include "log.h"

#include "tty.h"
#define MAIN_DEBUG 1

#include <stdlib.h>

#include "network.h"

#define USE_NET
#define BUFFER_LENGTH 2041 // minimum buffer size that can be used with qnx (I don't know why)


char * portname = "/tmp/ttyV0"; //"/dev/ttyUSB0";

// unsigned long GetTime_ms(int reset){
// 	struct timeval tv;
// 	gettimeofday(&tv, NULL);  
// 	uint64_t uS =  ((uint64_t)tv.tv_sec) * 1000000 + tv.tv_usec;
// 	static uint64_t l_uS = 0;
// 	if (reset) l_uS = uS;
// 	if (uS < l_uS) l_uS = uS;	// if overflow
// 	return (uint64_t)(uS - l_uS)/1000;	// return dt ms
// }

int cmdOptionExists(int argc, char *argv[], const char * option)
{
	int f=0;
	while(++f < argc){
		if (strcmp(argv[f], option) == 0) return f;
	}
	return 0;
}

int cmdOptionGetVal(int argc, char *argv[], const char * option, int * val)
{
    int f = cmdOptionExists(argc, argv, option);
	if ((f <= 0) || ((f+1) >= argc)) return 0;
	if (++f <= argc) *val = atoi(argv[f]);
	return f;
}
int cmdOptionGet(int argc, char *argv[], const char * option, char ** val)
{
    int f = cmdOptionExists(argc, argv, option);
	if ((f <= 0) || ((f+1) >= argc)) return 0;
	*val = (char *)argv[f+1];
	// if (++f <= argc) memcpy(*val, argv[f], strlen(argv[f])+1);
	return f;
}


// socat UDP-LISTEN:1444pf=ip4,fork,reuseaddr PTY,link=/tmp/ttyV0,raw,echo=0,wait-slave

int main(int argc, char* argv[]){
    printf("started...\n\r");
//return 0;
	// char * program_name;		// file name for program

	if (cmdOptionExists(argc, argv, "--help") || cmdOptionExists(argc, argv, "-h")) {
		printf("use: %s -D ttydev -P file\n\r", argv[0]);
		printf("-D - tty device: %s default\n\r", portname);
		printf("-P - update program on fly\n\r");
		printf("-q - quite mode (only msg to stdout)\n\r");

		return 0;
	}
	// cmdOptionGetVal(argc, argv, "--delay", &param_delay_us);
	if (cmdOptionGet(argc, argv, "-D", &portname)) printf("tty is: %s\n\r", portname);
	// if (cmdOptionExists(argc, argv, "-q")) quiet = true;
	// if (cmdOptionGet(argc, argv, "-P", &program_name)) {
	// 	int program_file = open (program_name, O_RDONLY);
	// 	if (program_file < 0)
	// 	{
	// 		printf("program name fail\n\r");
	// 	}
	// 	else {
	// 		pthread_t tid;
	// 		int err = pthread_create(&tid, NULL, &thread_program, &program_file);
	// 		if (err != 0) {printf("\ncan't create thread :[%s]", strerror(err)); return -1;}
	// 	}
	// }

	// // init circular buffer for tty
	// if (cb_init(&cb_ttyRead, BUF_SIZE, BUF_PACK_LEN) < 0) {
	// 	printf("cb init cb_ttyRead error\n\r");
	// 	return 0;
	// }
#ifdef USE_TTY
	// tty init
	if (tty_init(portname) < 0){printf("error tty %s\n\r", portname); return -1;}
#endif

#ifdef USE_NET
	// setting up network
	if (network_init() < 0) return -1;
#endif
	// no buffer for stdout
	setvbuf(stdout, NULL, _IONBF, 0);	



    MY_DEBUGF(MAIN_DEBUG, ("main: started...\n\r"));
    
    while (1){
        int thread = Thread_Cycle();
        usleep(1000);

        switch (thread){
            case(THREAD_IMU_LOOP):
              // MY_DEBUGF(MAIN_DEBUG, ("main: loop\n\r"));  
              mavlink_loop(); // if no mc
              imu_loop();
              break;
            case(THREAD_TEST):
            //   MY_DEBUGF(MAIN_DEBUG, ("test\n\r"));  
            //   // static int ii=0;
            //   // Printf("test %d\n\r", ii++);
            // //   HAL_GPIO_TogglePin(PIN_TEST_GPIO_Port, PIN_TEST_Pin);
            // //   HAL_GPIO_TogglePin(PIN_TEST2_GPIO_Port, PIN_TEST2_Pin);
            //   if (MotorControl_isArmed()) system_changeThread(THREAD_TEST, THREAD_T_INTERVAL, 100);
            //   else system_changeThread(THREAD_TEST, THREAD_T_INTERVAL, 500);  //500

            //   if (ModemControl_getStatus() < 0){ // for timeout. if no rx uart data on start and mc fail => reboot;
            //     static int t=0;
            //     if (mavlink_currNRxPack() == 0) t++;
            //     else t=0;
            //     if (t >= 10) system_reboot();
            //   }

            //   // int alt_max = (int)params_GetParamValue(ALT_MAX);
            //   // if ((alt_max > 0) && (!MotorControl_isArmed())){
            //   //   int dist = imu_getAlt();
            //   //   Printf("dist = %d\n\r", dist);
            //   // }
              break;
            case(THREAD_MODEMCONTROL):
              // MY_DEBUGF(MAIN_DEBUG, ("main: modem\n\r"));  
              mavlink_loop();
              if (ModemControl_getStatus()>=0){
                ModemControl_Loop();
              }
                //if (ModemControl_Loop() == 1){  // if rx data pack
#ifdef USE_TTY
                  uint8_t buff_pack[64];
                  int32_t rx_len = 0;//ModemControl_GetPacket(buff_pack); // if exist pack
                  rx_len = tty_read_get(buff_pack, 64);
                  // if (rx_len > 0) MY_DEBUGF(MAIN_DEBUG, ("main: modem data\n\r"));  
                  if (rx_len > 0) mavlink_receive_pack(buff_pack, rx_len);  // send packet to parse
#endif
#ifdef USE_NET
                  uint8_t nbuf[BUFFER_LENGTH];
                  //	receive data from network
                  int n = network_receive(nbuf, BUFFER_LENGTH);
                  if (n > 0) mavlink_receive_pack(nbuf, n);  // send packet to parse
#endif
                //}
              break;
            case(THREAD_MAV_SEND_ATTITUDE):
              // MY_DEBUGF(MAIN_DEBUG, ("main: attitude\n\r"));  
              mavlink_send_attitude();
              break;
            case(THREAD_MAV_SEND_STATUS):
              // MY_DEBUGF(MAIN_DEBUG, ("main: status\n\r"));  
              mavlink_send_heartbeat();
              mavlink_send_status();
              break;
            case(THREAD_ADC):
              // MY_DEBUGF(MAIN_DEBUG, ("main: adc\n\r"));  
              Battery_Read();
              break;
        }
    }

    return 0;
}

